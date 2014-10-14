#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/util.h"
#include "queue/queue.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define NUM_THREADS 4

pthread_mutex_t queue_lock;
pthread_mutex_t dns_lock;
pthread_mutex_t file_lock;
pthread_cond_t queue_not_full;
pthread_cond_t queue_not_empty;

int NUM_FILES;
int FILES_FINISHED_PROCESSING;

/* Make our queue available to all threads */
queue address_queue;
int queue_size;

/* Each thread reads it own txt file to avoid race conditions */
void* read_file(void* filename)
{
  char hostname[SBUFSIZE];

  /* Read File and Process*/
  FILE* file = fopen((char*) filename, "r");

  /* Scan through files, push hostname to queue while locked */
  while (fscanf(file, INPUTFS, hostname) > 0) {
    pthread_mutex_lock(&queue_lock);

    /* Check to see that there is space in the queue */
    while (queue_is_full(&address_queue)){
      pthread_cond_wait(&queue_not_full, &queue_lock);
    }

    queue_push(&address_queue, hostname);
    pthread_cond_signal(&queue_not_empty);

    pthread_mutex_unlock(&queue_lock);

    /* fscan is not atomic, so we sleep #headaches */
    usleep(100);
  }

  FILES_FINISHED_PROCESSING++;

  fclose(file);

  return NULL;
}

void* req_pool(void* files)
{
  char** filenames = (char**) files;
  pthread_t req_threads[NUM_FILES];

  /* Thread pool for reading files */
  for (int i = 0; i < NUM_FILES; i++) {
    char* filename = filenames[i];
    int thread_id = pthread_create(&(req_threads[i]), NULL, read_file, (void*) filename);
    pthread_join(req_threads[i], NULL);
  }

  pthread_cond_signal(&queue_not_empty);

  return NULL;
}

void* res_pool()
{
  FILE* outputfp = fopen("hardcoded_output.txt", "w");
  char firstipstr[INET6_ADDRSTRLEN];

  while(1){
    pthread_mutex_lock(&queue_lock);

    while(queue_is_empty(&address_queue)){

      /* Queue is empty and no more files left, we're done */
      if (FILES_FINISHED_PROCESSING == NUM_FILES){
        return NULL;
      }

      pthread_cond_wait(&queue_not_empty, &queue_lock);
    }

    char* hostname = (char*) queue_pop(&address_queue);
    char* hostname_copy;

    hostname_copy = malloc(sizeof(char) * strlen(hostname));
    pthread_cond_signal(&queue_not_full);
    strcpy(hostname_copy, hostname);

    pthread_mutex_lock(&file_lock);


    pthread_mutex_lock(&dns_lock);
    if(dnslookup(hostname_copy, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE) {
      fprintf(stderr, "dnslookup error: %s\n", hostname);
      strncpy(firstipstr, "", sizeof(firstipstr));
    }
    pthread_mutex_unlock(&dns_lock);

    /* fprintf(outputfp, "%s, %s\n", hostname, firstipstr); */
    /* printf("%s, %s\n", hostname, firstipstr); */
    printf("%s %s %d\n", hostname_copy, firstipstr, sizeof(firstipstr));
    pthread_mutex_unlock(&file_lock);
    pthread_mutex_unlock(&queue_lock);

  }

  fclose(outputfp);

  return NULL;
}

void init_variables(int argc)
{
  FILES_FINISHED_PROCESSING = 0;
  NUM_FILES = argc - 2;
  queue_init(&address_queue, 16);
  pthread_cond_init(&queue_not_full, NULL);
  pthread_cond_init(&queue_not_empty, NULL);
  pthread_mutex_init(&queue_lock, NULL);
  pthread_mutex_init(&file_lock, NULL);
  pthread_mutex_init(&dns_lock, NULL);
}

int main(int argc, char* argv[])
{
  /* Check arguments */
  if(argc < MINARGS) {
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);

    return EXIT_FAILURE;
  }

  /* Initialize vars */
  init_variables(argc);

  /* Extract the filenames from argv */
  char* filenames[NUM_FILES];
  for (int i = 1; i < (argc - 1); i++)
    filenames[i - 1] = argv[i];

  /* IDs for producer and consumer threads */
  pthread_t producer, consumer;

  /* Start producer thread pool */
  int producer_thread;
  producer_thread = pthread_create(&(producer), NULL, req_pool, (void*)filenames);

  /* Start consumer thread pool */
  int consumer_thread;
  consumer_thread = pthread_create(&(consumer), NULL, res_pool, NULL);

  pthread_join(consumer, NULL);

  /* Destroy the locks */
  pthread_mutex_destroy(&queue_lock);
  pthread_mutex_destroy(&file_lock);

  return EXIT_SUCCESS;
}
