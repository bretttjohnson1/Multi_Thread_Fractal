#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


typedef struct color color;

float PI= 3.1415f;
float offx=0.0f;
float offy=-.5f;
float offz=-6.0f;
char keys[256];
float phi=0;
float thet=0;
int window;
int layers;
float *points;

int points_len;
int side_length;
color *color_array;

struct color {
	float r;
	float g;
	float b;
};

void fail(){
	printf("Error %s \n",strerror(errno));
	fflush(stdout);
	exit(1);
}

void moveandrotate(){
	if(keys[32]) {
		offy-=.05+.5*keys[(int)'m'];
	}
	if(keys[(int)'c'])
		offy+=.05+.5*keys[(int)'m'];
	if(keys[(int)'w']) {
		offz+=.05+.5*keys[(int)'m'];
	}
	if(keys[(int)'s']) {
		offz-=.05+.5*keys[(int)'m'];
	}
	if(keys[(int)'d']) {
		offx-=.05+.5*keys[(int)'m'];
	}
	if(keys[(int)'a']) {
		offx+=.05+.5*keys[(int)'m'];
	}
	if(keys[(int)'j'])
		thet+=1+2*keys[(int)'m'];
	if(keys[(int)'l'])
		thet-=1+2*keys[(int)'m'];
	if(keys[(int)'i'])
		phi+=1+2*keys[(int)'m'];
	if(keys[(int)'k'])
		phi-=1+2*keys[(int)'m'];

	if(thet>=360) {
		thet-=360;
	}
   if(thet<=360) {
		thet+=360;
	}
	if(phi<=-360) {
		phi+=360;
	}
   if(phi>=360) {
		phi-=360;
	}

}

void init(int width, int height){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void draw(){
	//struct timeval begin,end;
	//gettimeofday(&begin, NULL);
	///gettimeofday(&end,NULL);
	//printf("data recieved in %lu ms\n",end.tv_usec-begin.tv_usec);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	moveandrotate();

	//struct timeval stop, start;
	//gettimeofday(&start, NULL);
	glTranslatef(offx,offy,offz);
	glScalef(.01,.01,.01);
   //printf("%f\n",cos(thet/360) );
   glRotatef(thet,0,1,0);
	glRotatef(phi,cos(thet/360*6.283185f),0,sin(thet/360*6.283185f));



	//gettimeofday(&begin, NULL);
	for(int a  = 0; a<side_length-1; a++)
		for(long b = 0; b<side_length-1; b++) {
			glColor3f(color_array[a+b*side_length].r,color_array[a+b*side_length].g,color_array[a+b*side_length].b);
			if(a%(side_length-1)!=0) {
				glBegin(GL_TRIANGLES);
				glVertex3f(b-side_length/2,points[a+b*side_length],a-side_length/2);
				glVertex3f(b+1-side_length/2,points[a+(b+1)*side_length],a-side_length/2);
				glVertex3f(b-side_length/2,points[a+1+b*side_length],(a+1)-side_length/2);
				glEnd();
				glBegin(GL_TRIANGLES);
				glVertex3f(b+1-side_length/2,points[a+(b+1)*side_length],a-side_length/2);
				glVertex3f(b-side_length/2,points[a+1+b*side_length],(a+1)-side_length/2);
				glVertex3f(b+1-side_length/2,points[a+1+(b+1)*side_length],a+1-side_length/2);
				glEnd();
			}

		}
	//gettimeofday(&end,NULL);
	//printf("points drawn in %lu ms\n",end.tv_usec-begin.tv_usec);


	// gettimeofday(&stop,NULL);


	///gettimeofday(&stop,NULL);
	glutSwapBuffers();

}
void resize(int width,int height){
	if(height==0)
		height=1;
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);

}
char polygon_mode = 1;
void keyPressed(unsigned char key, int x, int y){
	keys[key]=1;
	usleep(100);
	if(key == 'p') {
		if(polygon_mode == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
			polygon_mode = 1;
		}else if(polygon_mode == 1) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			polygon_mode = 0;
		}
	}
	if(key == 27) {
		glutDestroyWindow(window);
		exit(0);
	}

}
void keyUp(unsigned char key, int x, int y){
	keys[key]=0;
}

int main(int argc, char **argv){
	FILE *f;
	f = fopen("output.dat","r");
	char *p  = malloc(10);
	size_t len;
	getline(&p,&len,f);
	sscanf(p, "%d",&layers );
	printf("%d\n",layers );
	points_len = (pow(2,layers)+1)*(pow(2,layers)+1);
	points = malloc(points_len*sizeof(float));
	color_array = malloc(points_len*sizeof(color));
	side_length = pow(2,layers)+1;
	int a =0;
	while(getline(&p,&len,f)!=-1) {
		sscanf(p, "%6f",points+a);
		a++;
	}
	fclose(f);

   float red;
   float green;
   float blue;
   char *rd = malloc(10);
   char *gr = malloc(10);
   char *bl = malloc(10);
   FILE *red_file;
   FILE *green_file;
   FILE *blue_file;
   red_file = fopen("red.dat","r");
   green_file = fopen("green.dat","r");
   blue_file = fopen("blue.dat","r");
   float rmin = 0,gmin=0,bmin=0;
   float rmax = 0,gmax=0,bmax=0;

   a =0;
	while(getline(&rd,&len,red_file)!=-1 && getline(&gr,&len,green_file)!=-1 && getline(&bl,&len,blue_file)!=-1) {
		sscanf(rd, "%6f",&red);
      sscanf(gr, "%6f",&green);
      sscanf(bl, "%6f",&blue);
      if(red>rmax)rmax=red;
      if(red<rmin)rmin=red;
      if(green>gmax)gmax=green;
      if(green<gmin)gmin=green;
      if(blue>bmax)bmax=blue;
      if(blue<bmin)bmin=blue;

      color_array[a].r = red;
      color_array[a].g =  green;
      color_array[a].b =  blue;

		a++;
	}
   for(int a =0;a<points_len;a++){
      color_array[a].r +=fabs(rmin);
      color_array[a].r /=(rmax-rmin);

      color_array[a].g +=fabs(gmin);
      color_array[a].g /=(gmax-gmin);

      color_array[a].b +=fabs(bmin);
      color_array[a].b /=(bmax-bmin);
      //printf("%f\n", color_array[a].b);
   }
   free(rd);
   free(gr);
   free(bl);
   fclose(red_file);
   fclose(green_file);
   fclose(blue_file);

	int var = 1;
	glutInit(&var,argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(640,480);
	offz = -1-side_length/100;
	window = glutCreateWindow("DISPLAY");
	glutDisplayFunc(&draw);
	glutIdleFunc(&draw);
	glutFullScreen();
	glutReshapeFunc(&resize);
	glutKeyboardFunc(&keyPressed);
	glutKeyboardUpFunc(&keyUp);
	init(640,480);
	glutMainLoop();
	free(points);
}
