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

job *job_que;
int num_threads;
int layers = 8;
int current_layer = 0;

int wait_count = 0;
//pthread_cond_t cond_parent_go;
//pthread_mutex_t mutex_parent;
//pthread_cond_t cond_child_go;
//pthread_mutex_t mutex_child;

void *worker(void* number);
int main(int argc,char **argsv){
	if(argc!=2) {
      printf("wrong number of arguments\n");
		exit(1);
	}
	const char *str_size = argsv[1];
	num_threads = atoi(str_size)-1;
   fflush(stdout);
	/*pthread_mutex_init(&mutex_parent,NULL); //TODO check if returns -1 and print perror if so
	pthread_cond_init(&cond_parent_go,NULL);
	pthread_mutex_init(&mutex_child,NULL);
	pthread_cond_init(&cond_child_go,NULL);*/

	pthread_t tids[num_threads];
	pthread_attr_t attrs[num_threads];
	int tnumber[num_threads];
	for (int i = 0; i < num_threads; i++) {
		pthread_attr_init(&attrs[i]);
		tnumber[i] = i;
		pthread_create(&tids[i],&attrs[i],worker,&tnumber[i]);
	}
	for(int a =0; a<layers; a++) {
		current_layer = a;
      while(wait_count!=num_threads);
      wait_count=0;
      /*pthread_cond_signal(&cond_child_go);
		pthread_mutex_lock(&mutex_parent);
      pthread_mutex_lock(&mutex_child);
		pthread_cond_wait(&cond_parent_go,&mutex_parent);*/

	}
	for(int a =0; a<num_threads; a++) {
		pthread_join(tids[a],NULL);
	}


}

void *worker(void *number){

	/*
	   TODO read from job que
	 */
    int *thread_number;
    thread_number = number;
	for(int a =0; a<layers; a++) {
      if(a!=current_layer)wait_count++;
      while(a!=current_layer);

      /*if(a!=current_layer){
         while(a)
         running_count--;
         pthread_cond_signal(&cond_parent_go);
         printf("waiting... %d %d\n",current_layer,*thread_number );
         pthread_cond_wait(&cond_child_go,&mutex_child);
         printf("went... %d %d\n",current_layer,*thread_number );

      }*/

      printf("hello %d %d\n",current_layer,*thread_number );
	}
   wait_count++;
	pthread_exit(0);
}
