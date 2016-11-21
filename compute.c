#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define DIAMOND 0
#define SQUARE 1

typedef struct job job;
struct job {
   int x;
   int y;
   char method; //diamond or square
   int layer;
};

int gate  = 0; //controls thread execution


job *job_que;
int num_threads;
int layers = 8;
void *worker(void* number);
int main(int argc,char **argsv){
	if(argc!=2) {
		exit(1);
	}
	const char *str_size = argsv[1];
	num_threads = atoi(str_size)-1;


	pthread_t tids[num_threads];
	pthread_attr_t attrs[num_threads];
   int tnumber[num_threads];
	for (int i = 0; i < num_threads; i++) {
		pthread_attr_init(&attrs[i]);
      tnumber[i] = i;
      pthread_create(&tids[i],&attrs[i],worker,&tnumber[i]);
	}
   for(int a =0;a<num_threads;a++){
      pthread_join(tids[a],NULL);
   }


}

void *worker(void *number){
   int *thread_number;
   thread_number = number;
   printf("hello %d\n",*thread_number );
   pthread_exit(0);
}
