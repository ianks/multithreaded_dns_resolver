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
pthread_mutex_t file_lock;
pthread_cond_t queue_full;

int NUM_FILES;

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
    while (queue_is_full(&address_queue))
      pthread_cond_wait(&queue_full, &queue_lock);

    queue_push(&address_queue, hostname);

    /* This can be removed, just showing that cond_wait works */
    char* payload = queue_pop(&address_queue);
    queue_push(&address_queue, hostname);
    printf("%s\n", payload);
    pthread_mutex_unlock(&queue_lock);
  }

  fclose(file);
  pthread_exit(NULL);

  return NULL;
}

void* req_pool(void* files)
{
  char** filenames;
  filenames = (char**) files;
  pthread_t req_threads[NUM_FILES];

  /* Thread pool for reading files */
  int i;
  int thread_id;
  for (i = 0; i < NUM_FILES; i++) {
    char* filename = filenames[i];
    thread_id = pthread_create(&(req_threads[i]), NULL, read_file, (void*) filename);
    pthread_join(req_threads[i], NULL);
  }

  return NULL;
}

void* res_pool()
{
  return NULL;
}

int main(int argc, char* argv[])
{
  /* Initialize vars */
  NUM_FILES = argc - 2;
  queue_size = queue_init(&address_queue, 500);
  pthread_cond_init(&queue_full, NULL);
  pthread_mutex_init(&queue_lock, NULL);
  pthread_mutex_init(&file_lock, NULL);

  /* Check arguments */
  if(argc < MINARGS) {
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);

    return EXIT_FAILURE;
  }

  /* Extract the filenames from argv */
  char* filenames[NUM_FILES];
  int i;
  for (i = 1; i < (argc - 1); i++)
    filenames[i - 1] = argv[i];

  /* IDs for producer and consumer threads */
  pthread_t producer, consumer;

  /* Start producer thread pool */
  int producer_thread;
  producer_thread = pthread_create(&(producer), NULL, req_pool, (void*)filenames);

  /* Start consumer thread pool */
  int consumer_thread;
  consumer_thread = pthread_create(&(consumer), NULL, res_pool, NULL);

  /* Don't finish until producer and consumer threads return */
  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);

  /* Destroy the locks */
  pthread_mutex_destroy(&queue_lock);
  pthread_mutex_destroy(&file_lock);

  return EXIT_SUCCESS;
}
