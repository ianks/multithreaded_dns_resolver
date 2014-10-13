
#include<pthread.h>
#include<stdlib.h>
#define NUMTHRDS 4

// uncomment the comments to check how mutex affect the threads' execution

pthread_mutex_t count_mutex;
int count;
pthread_t callThd[NUMTHRDS];

void deposit(void *t)
{
	//pthread_mutex_lock(&count_mutex);
    count = count + 100;
    printf("thread number %d prev account balance and present accout balance %d %d\n\n",t,count-100,count);
	//pthread_mutex_unlock(&count_mutex);
	pthread_exit((void*) count);
}

void withdraw(void *t)
{
	//pthread_mutex_lock(&count_mutex);
    count = count - 50;
    printf("thread number %d prev account balance and present accout balance %d %d\n\n",t,count+50,count);
	//pthread_mutex_unlock(&count_mutex);
	pthread_exit((void*) count);
}

int main()
{
	count = 100;
	void *status;
	int i;
	pthread_mutex_init(&count_mutex, NULL);
	pthread_create(&callThd[0], NULL, deposit, (void *)0);
	pthread_create(&callThd[1], NULL, withdraw, (void *)1);
	pthread_create(&callThd[2], NULL, deposit, (void *)2);
	pthread_create(&callThd[3], NULL, withdraw, (void *)3);

	for(i=0;i<NUMTHRDS;i++)
	{
		pthread_join(callThd[i], &count);
	}
	//pthread_mutex_destroy(&count_mutex);
	pthread_exit(NULL);
	return 0;
}
