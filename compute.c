#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <semaphore.h>
#define DIAMOND 0
#define SQUARE 1

typedef struct job job;
struct job {
	int x;
	int y;
	char method; //diamond or square
	int layer;
	int squareside_length;
};
int num_threads;

float *points;

job *job_que;
uint64_t job_que_length;

int *tasks_per_layer;
int task_count = 0;
sem_t task_count_mutex;

int layers = 6;
int current_layer = 0;
uint64_t side_length; //side_length of one side of the grid

float noise = 1;

//pthread_cond_t cond_parent_go;
//pthread_mutex_t mutex_parent;
//pthread_cond_t cond_child_go;
//pthread_mutex_t mutex_child;

void *worker(void* number);
int main(int argc,char **argsv){
	if(argc!=2) {
      //printf("%ld %ld\n",sizeof(float),sizeof(double) );
		printf("wrong number of arguments\n");
		exit(1);
	}
   srand(time(NULL));
	const char *str_size = argsv[1];
	num_threads = atoi(str_size)-1;
	/*pthread_mutex_init(&mutex_parent,NULL); //TODO check if returns -1 and print perror if so
	   pthread_cond_init(&cond_parent_go,NULL);
	   pthread_mutex_init(&mutex_child,NULL);
	   pthread_cond_init(&cond_child_go,NULL);*/
	side_length  = pow(2,layers)+1;
	job_que_length = side_length*side_length-4;
	job_que = malloc(sizeof(job)*(job_que_length));
   if(job_que == NULL){
      printf("Not enough memory\n");
      exit(1);

   }
	points = malloc(sizeof(double)*side_length*side_length);
   points[0] = 5;
   points[side_length-1] = 2;
   points[side_length*(side_length-1)+1] = 2;
   points[side_length*side_length-1] = -2;
	tasks_per_layer = malloc(sizeof(int)*layers);
	sem_init(&task_count_mutex,0,1);
   //printf("made it\n");
	int index = 0;
	for(int a =0; a<layers; a++) {
		int layer_index = 0;
		int squareside_length = (side_length-1)/pow(2,a);
		for(int b =0; b<side_length-1; b+=squareside_length) {
			for(int c = 0; c<side_length-1; c+=squareside_length) {
				job_que[index].x = b+squareside_length/2;
				job_que[index].y = c+squareside_length/2;
				job_que[index].method = DIAMOND;
				job_que[index].layer = a;
				job_que[index].squareside_length = squareside_length;
				index++;
				layer_index++;
			}
		}
		int row = 0;
		for(int b  = 0; b<side_length; b+=squareside_length/2) {
			for(int c =squareside_length/2*(row%2==0); c<side_length; c+=squareside_length) {
				job_que[index].x = c;
				job_que[index].y = b;
				job_que[index].method = SQUARE;
				job_que[index].layer = a;
				job_que[index].squareside_length = squareside_length;
				//printf("SQUARE %d %d\n",job_que[index].y,job_que[index].x);
				index++;
				layer_index++;
			}
			row++;
		}
		tasks_per_layer[a] = layer_index;
	}

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
		printf("tasks needed %d\n",tasks_per_layer[a] );
		while(task_count<tasks_per_layer[a]) ;
		task_count=0;
		/*pthread_cond_signal(&cond_child_go);
		          pthread_mutex_lock(&mutex_parent);
		   pthread_mutex_lock(&mutex_child);
		          pthread_cond_wait(&cond_parent_go,&mutex_parent);*/

	}
	for(int a =0; a<num_threads; a++) {
		pthread_join(tids[a],NULL);
	}
   FILE *f;
   f = fopen("output.dat","wb");
   fprintf(f, "%d\n",layers);
   for(int a =0;a<side_length*side_length;a++){
      fprintf(f, "%f\n",points[a]);
   }
   fclose(f);
	sem_destroy(&task_count_mutex);
	free(job_que);
	free(points);
	free(tasks_per_layer);

}

void *worker(void *number){
	int *thread_number;
	thread_number = number;
	//printf("%d\n",*thread_number);
	job current_job;
	int job_layer;
	int x;
	int y;
	char method;
	int squareside_length;
	for(int a =*thread_number; a<job_que_length; a+=num_threads) {
		current_job = job_que[a];
		x = current_job.x;
		y = current_job.y;
		method = current_job.method;
		job_layer = current_job.layer;
		squareside_length = current_job.squareside_length;
		//printf("%d\n",job_layer );
		while(job_layer>current_layer) ;

		/*if(a!=current_layer){
		   while(a)
		   running_count--;
		   pthread_cond_signal(&cond_parent_go);
		   printf("waiting... %d %d\n",current_layer,*thread_number );
		   pthread_cond_wait(&cond_child_go,&mutex_child);
		   printf("went... %d %d\n",current_layer,*thread_number );

		   }*/

		if(method==DIAMOND) {
			float avg = 0;
			avg+=points[x-squareside_length/2+(y-squareside_length/2)*side_length]/4;
			avg+=points[x+squareside_length/2+(y-squareside_length/2)*side_length]/4;
			avg+=points[x-squareside_length/2+(y+squareside_length/2)*side_length]/4;
			avg+=points[x+squareside_length/2+(y+squareside_length/2)*side_length]/4;
			points[x+y*side_length] = avg + (double)(rand()/(float)RAND_MAX)*noise;
		}else if(SQUARE) {
			float avg = 0;
			if(x == 0) {
            //printf("%d %d %d %d\n",x,y, squareside_length/2, side_length );
            avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;
			}else if(y == 0) {
				avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;

			}else if(x==side_length-1) {
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;

			}else if(y == side_length-1) {
				avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;

			}else{
				avg+=points[x+squareside_length/2+y*side_length]/4;
				avg+=points[x-squareside_length/2+y*side_length]/4;
				avg+=points[x+(y-squareside_length/2)*side_length]/4;
				avg+=points[x+(y+squareside_length/2)*side_length]/4;


			}
         points[x+side_length*y] = avg + (double)(rand()/RAND_MAX)*noise;
		}
		//printf("hello %d %d %d %d\n",job_layer,y,x,*thread_number );
		sem_wait(&task_count_mutex);
		task_count++;
		sem_post(&task_count_mutex);
	}
	pthread_exit(0);
}
