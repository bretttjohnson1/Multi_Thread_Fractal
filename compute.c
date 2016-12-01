#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/time.h>
#include <utmpx.h>
#include <unistd.h>
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
float *red_vals;
float *blue_vals;
float *green_vals;
float *smoothed_points;

job *job_que;
uint64_t job_que_length;

int *tasks_per_layer;
int *task_count;
sem_t task_count_mutex;

int layers = 8;
int current_layer = 0;
uint64_t side_length; //side_length of one side of the grid

float noise = .1; //multiplied to random value. more noise = more variation

char smoothing = 1;

sem_t *thread_sem;
sem_t parent_sem;

char datamode = 0;

int sum(int *list,int num_threads);
void *smooth_worker(void *number);
void *worker(void* number);
int main(int argc,char **argsv){
   if(argc==5){
      const char *thread_num_str = argsv[1];
		num_threads = atoi(thread_num_str);
		const char *layers_str = argsv[2];
		layers = atoi(layers_str);
		const char *noise_str = argsv[3];
		sscanf(noise_str,"%f",&noise);
      datamode = atoi(argsv[4]);
   }
	if(argc==4) {
		const char *thread_num_str = argsv[1];
		num_threads = atoi(thread_num_str);
		const char *layers_str = argsv[2];
		layers = atoi(layers_str);
		const char *noise_str = argsv[3];
		sscanf(noise_str,"%f",&noise);
	}else if(argc==3) {
		const char *thread_num_str = argsv[1];
		num_threads = atoi(thread_num_str);
		const char *layers_str = argsv[2];
		layers = atoi(layers_str);
	}else if(argc==2) {
		const char *thread_num_str = argsv[1];
		num_threads = atoi(thread_num_str);
	}else{
		printf("Usage ./compute.o number_of_threads OR\n");
		printf("Usage ./compute.o number_of_threads layers OR\n");
		printf("Usage ./compute.o number_of_threads layers noise OR\n");
      printf("Usage ./compute.o number_of_threads layers noise datamode(bool)\n");
		exit(1);
	}
	struct timeval begin,end;
	gettimeofday(&begin, NULL);

	side_length  = pow(2,layers)+1;
	job_que_length = side_length*side_length-4;
	job_que = malloc(sizeof(job)*(job_que_length));
	if(job_que == NULL) {
		printf("Malloc err\n");
		exit(1);

	}
	points = malloc(sizeof(float)*side_length*side_length);
	red_vals = malloc(sizeof(float)*side_length*side_length);
	green_vals = malloc(sizeof(float)*side_length*side_length);
	blue_vals = malloc(sizeof(float)*side_length*side_length);


	points[0] = 0;
	points[side_length-1] = 0;
	points[side_length*(side_length-1)] = 0;
	points[side_length*side_length-1] = 0;

	tasks_per_layer = malloc(sizeof(int)*layers*2);
	sem_init(&task_count_mutex,0,1);
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
				index++;
				layer_index++;
			}
		}
		tasks_per_layer[a*2] = layer_index;
		layer_index = 0;
		int row = 0;
		for(int b  = 0; b<side_length; b+=squareside_length/2) {
			for(int c =squareside_length/2*(row%2==0); c<side_length; c+=squareside_length) {
				job_que[index].x = c;
				job_que[index].y = b;
				job_que[index].method = SQUARE;
				job_que[index].layer = a;
				index++;
				layer_index++;
			}
			row++;
		}
		tasks_per_layer[a*2+1] = layer_index;
	}

	sem_init(&parent_sem,0,0);
	thread_sem  = malloc(sizeof(sem_t)*(num_threads));
	for(int a = 0; a<num_threads-1; a++)
		sem_init(&thread_sem[a],0,0);

	task_count = malloc(sizeof(int)*num_threads);

	pthread_t tids[num_threads];
	pthread_attr_t attrs[num_threads];
	int tnumber[num_threads];
	if(!datamode) printf("Computing Fractals...\n");
	for (int i = 0; i < num_threads; i++) {
		pthread_attr_init(&attrs[i]);
		tnumber[i] = i;
		pthread_create(&tids[i],&attrs[i],worker,&tnumber[i]);
	}
	for(int a =0; a<layers*2; a++) {
		current_layer = a;
		for(int b = 0; b<num_threads; b++)
			sem_post(thread_sem+b);
		while(sum(task_count,num_threads)<tasks_per_layer[a]) {
			sem_wait(&parent_sem);
		}

		for(int b = 0; b<num_threads; b++)
			task_count[b]=0;

		if(!datamode) if(a%2) printf("Layer %d done\n",(a/2)+1);
		fflush(stdout);
	}
	for(int a =0; a<num_threads; a++) {
		pthread_join(tids[a],NULL);
	}
	struct timeval sbegin,send;
	gettimeofday(&sbegin,NULL);

	if(smoothing) {
		if(!datamode) printf("Smoothing...\n");
		smoothed_points = malloc(sizeof(float)*side_length*side_length);


		for (int i = 0; i < num_threads; i++) {
			pthread_attr_init(&attrs[i]);
			pthread_create(&tids[i],&attrs[i],smooth_worker,&tnumber[i]);
		}

		for(int a =0; a<num_threads; a++) {
			pthread_join(tids[a],NULL);
		}
		free(points);
	}else{
		smoothed_points = points;
	}
	gettimeofday(&send,NULL);

	if(!datamode) printf("Smoothing took %f seconds\n",send.tv_sec-sbegin.tv_sec+(send.tv_usec-sbegin.tv_usec)/1000000.0f);


	gettimeofday(&end,NULL);
	if(!datamode) printf("Total took %f seconds\n",end.tv_sec-begin.tv_sec+(end.tv_usec-begin.tv_usec)/1000000.0f);
	if(datamode) printf("%f\n",end.tv_sec-begin.tv_sec+(end.tv_usec-begin.tv_usec)/1000000.0f);

	if(!datamode) printf("Writing...\n" );
	if(!datamode) {
		FILE *f;
		f = fopen("output.dat","wb");
		fprintf(f, "%d\n",layers);
		for(int a =0; a<side_length*side_length; a++) {
			fprintf(f, "%f\n",smoothed_points[a]);
		}
		fclose(f);
		f = fopen("red.dat","wb");
		for(int a =0; a<side_length*side_length; a++) {
			fprintf(f, "%f\n",red_vals[a]);
		}
		fclose(f);
		f = fopen("green.dat","wb");
		for(int a =0; a<side_length*side_length; a++) {
			fprintf(f, "%f\n",green_vals[a]);
		}
		fclose(f);
		f = fopen("blue.dat","wb");
		for(int a =0; a<side_length*side_length; a++) {
			fprintf(f, "%f\n",blue_vals[a]);
		}
		fclose(f);
	}

	free(red_vals);

	free(green_vals);
	free(blue_vals);
	free(task_count);

	free(thread_sem);
	sem_destroy(&task_count_mutex);
	free(job_que);
	free(smoothed_points);
	free(tasks_per_layer);
	return 0;
}

void *worker(void *number){
	struct timeval time_seed;
	gettimeofday(&time_seed,NULL);
	unsigned int seed = time_seed.tv_usec;
	int *thread_number;
	thread_number = number;
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
		squareside_length = (side_length-1)/pow(2,job_layer);

		if(2*job_layer+method>current_layer)
			sem_post(&parent_sem);
		while(2*job_layer+method>current_layer) {
			//printf("CORE: %d TID: %d \n", sched_getcpu(), *thread_number);
			sem_wait(&thread_sem[*thread_number]);
		}


		if(method==DIAMOND) {
			float avg = 0;
			float red_avg = 0;
			float green_avg = 0;
			float blue_avg = 0;
			avg+=points[x-squareside_length/2+(y-squareside_length/2)*side_length]/4;
			avg+=points[x+squareside_length/2+(y-squareside_length/2)*side_length]/4;
			avg+=points[x-squareside_length/2+(y+squareside_length/2)*side_length]/4;
			avg+=points[x+squareside_length/2+(y+squareside_length/2)*side_length]/4;

			red_avg+=red_vals[x-squareside_length/2+(y-squareside_length/2)*side_length]/4;
			red_avg+=red_vals[x+squareside_length/2+(y-squareside_length/2)*side_length]/4;
			red_avg+=red_vals[x-squareside_length/2+(y+squareside_length/2)*side_length]/4;
			red_avg+=red_vals[x+squareside_length/2+(y+squareside_length/2)*side_length]/4;

			green_avg+=green_vals[x-squareside_length/2+(y-squareside_length/2)*side_length]/4;
			green_avg+=green_vals[x+squareside_length/2+(y-squareside_length/2)*side_length]/4;
			green_avg+=green_vals[x-squareside_length/2+(y+squareside_length/2)*side_length]/4;
			green_avg+=green_vals[x+squareside_length/2+(y+squareside_length/2)*side_length]/4;

			blue_avg+=blue_vals[x-squareside_length/2+(y-squareside_length/2)*side_length]/4;
			blue_avg+=blue_vals[x+squareside_length/2+(y-squareside_length/2)*side_length]/4;
			blue_avg+=blue_vals[x-squareside_length/2+(y+squareside_length/2)*side_length]/4;
			blue_avg+=blue_vals[x+squareside_length/2+(y+squareside_length/2)*side_length]/4;

			points[x+y*side_length] = avg + (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);

			red_vals[x+y*side_length] = red_avg + (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);

			blue_vals[x+y*side_length] = blue_avg + (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);

			green_vals[x+y*side_length] = green_avg + (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);


		}else if(SQUARE) {
			float avg = 0;
			float red_avg = 0;
			float green_avg = 0;
			float blue_avg = 0;
			if(x == 0) {
				avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;

				red_avg+=red_vals[x+squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x+(y-squareside_length/2)*side_length]/3;
				red_avg+=red_vals[x+(y+squareside_length/2)*side_length]/3;

				green_avg+=green_vals[x+squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x+(y-squareside_length/2)*side_length]/3;
				green_avg+=green_vals[x+(y+squareside_length/2)*side_length]/3;

				blue_avg+=blue_vals[x+squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x+(y-squareside_length/2)*side_length]/3;
				blue_avg+=blue_vals[x+(y+squareside_length/2)*side_length]/3;

			}else if(y == 0) {
				avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;

				red_avg+=red_vals[x+squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x-squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x+(y+squareside_length/2)*side_length]/3;

				green_avg+=green_vals[x+squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x-squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x+(y+squareside_length/2)*side_length]/3;

				blue_avg+=blue_vals[x+squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x-squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x+(y+squareside_length/2)*side_length]/3;

			}else if(x==side_length-1) {
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;
				avg+=points[x+(y+squareside_length/2)*side_length]/3;

				red_avg+=red_vals[x-squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x+(y-squareside_length/2)*side_length]/3;
				red_avg+=red_vals[x+(y+squareside_length/2)*side_length]/3;

				green_avg+=green_vals[x-squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x+(y-squareside_length/2)*side_length]/3;
				green_avg+=green_vals[x+(y+squareside_length/2)*side_length]/3;

				blue_avg+=blue_vals[x-squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x+(y-squareside_length/2)*side_length]/3;
				blue_avg+=blue_vals[x+(y+squareside_length/2)*side_length]/3;

			}else if(y == side_length-1) {
				avg+=points[x+squareside_length/2+y*side_length]/3;
				avg+=points[x-squareside_length/2+y*side_length]/3;
				avg+=points[x+(y-squareside_length/2)*side_length]/3;

				red_avg+=red_vals[x+squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x-squareside_length/2+y*side_length]/3;
				red_avg+=red_vals[x+(y-squareside_length/2)*side_length]/3;

				green_avg+=green_vals[x+squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x-squareside_length/2+y*side_length]/3;
				green_avg+=green_vals[x+(y-squareside_length/2)*side_length]/3;

				blue_avg+=blue_vals[x+squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x-squareside_length/2+y*side_length]/3;
				blue_avg+=blue_vals[x+(y-squareside_length/2)*side_length]/3;

			}else{
				avg+=points[x+squareside_length/2+y*side_length]/4;
				avg+=points[x-squareside_length/2+y*side_length]/4;
				avg+=points[x+(y-squareside_length/2)*side_length]/4;
				avg+=points[x+(y+squareside_length/2)*side_length]/4;

				red_avg+=red_vals[x+squareside_length/2+y*side_length]/4;
				red_avg+=red_vals[x-squareside_length/2+y*side_length]/4;
				red_avg+=red_vals[x+(y-squareside_length/2)*side_length]/4;
				red_avg+=red_vals[x+(y+squareside_length/2)*side_length]/4;

				green_avg+=green_vals[x+squareside_length/2+y*side_length]/4;
				green_avg+=green_vals[x-squareside_length/2+y*side_length]/4;
				green_avg+=green_vals[x+(y-squareside_length/2)*side_length]/4;
				green_avg+=green_vals[x+(y+squareside_length/2)*side_length]/4;

				blue_avg+=blue_vals[x+squareside_length/2+y*side_length]/4;
				blue_avg+=blue_vals[x-squareside_length/2+y*side_length]/4;
				blue_avg+=blue_vals[x+(y-squareside_length/2)*side_length]/4;
				blue_avg+=blue_vals[x+(y+squareside_length/2)*side_length]/4;

			}
			points[x+side_length*y] = avg +  (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);
			red_vals[x+side_length*y] = red_avg +  (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);
			green_vals[x+side_length*y] = green_avg +  (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);
			blue_vals[x+side_length*y] = blue_avg +  (double)((rand_r(&seed)-RAND_MAX/2)/(float)RAND_MAX)*2*noise*(layers-job_layer)*(layers-job_layer)*(layers-job_layer);

		}
		task_count[*thread_number]++;

	}
	sem_post(&parent_sem);
	pthread_exit(0);
}
void * smooth_worker(void * number){
	int *thread_number;
	thread_number = number;
	int len =(side_length)*(side_length);
	if(smoothing) {
		for(int h = (*thread_number); h<len; h+=num_threads) {
			int a = h/side_length;
			int b = h%side_length;
			float average = 0;
			float count = 0;
			for(int i = a-3; i<=a+3; i++) {
				for(int j = b-3; j<=b+3; j++) {
					if(i>=0 && j>=0 && i<side_length && j<side_length) {
						average+=points[i+j*side_length];
						count++;
					}
				}
				smoothed_points[h] = average/count;
			}
		}
	}

	pthread_exit(0);
}
int sum(int *list,int len){
	int tot = 0;
	for(int a =0; a<len; a++)
		tot+=list[a];
	return tot;
}
