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
  return NULL;
}

int req_pool(char* filenames[])
{
  pthread_t req_threads[NUM_FILES];

  /* Thread pool for reading files */
  int thread_id;
  int i;
  for (i = 0; i < NUM_FILES; i++) {
    char* filename = filenames[i];
    thread_id = pthread_create(&(req_threads[i]), NULL, read_file, (void*) filename);
    pthread_join(req_threads[i], NULL);
  }

  pthread_mutex_destroy(&queue_lock);
  pthread_exit(NULL);

  return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
  pthread_cond_init(&queue_full, NULL);
  pthread_mutex_init(&queue_lock, NULL);

  NUM_FILES = argc - 2;
  queue_size = queue_init(&address_queue, 5);

  /* Check Arguments */
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

  req_pool(filenames);
}
