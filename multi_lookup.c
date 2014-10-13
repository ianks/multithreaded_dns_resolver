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

int NUM_FILES;

/* Each thread reads it own txt file to avoid race conditions */
void* read_file(void* filename)
{
  char hostname[SBUFSIZE];

  /* Read File and Process*/
  FILE* file = fopen((char*) filename, "r");

  while (fscanf(file, INPUTFS, hostname) > 0) {
    printf("%s\n", hostname);
  }

  fclose(file);
  return NULL;
}

int req_pool(char* filenames[])
{
  printf("%d", (NUM_FILES));
  pthread_t req_threads[NUM_FILES];

  /* Thread pool for reading files */
  int thread_id;
  int i;
  for (i = 0; i < NUM_FILES; i++) {
    char* filename = filenames[i];
    thread_id = pthread_create(&(req_threads[i]), NULL, read_file, (void*) filename);
  }

  /* Wait for threads to finish */
  /* This may not be neccesary as we fill the queue other threads can read */
  for (i = 0; i < NUM_FILES; i++)
    pthread_join(req_threads[i], NULL);

  return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
  NUM_FILES = argc - 2;

  /* Check Arguments */
  if(argc < MINARGS) {
    fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
    fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);

    return EXIT_FAILURE;
  }

  char* filenames[NUM_FILES];
  int i;
  for (i = 1; i < (argc - 1); i++)
    filenames[i - 1] = argv[i];

  req_pool(filenames);
}
