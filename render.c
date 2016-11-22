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

float offx=1.5f;
float offy=0.0f;
float offz=-6.0f;
char keys[256];
float phi=0;
float thet=0;
int window;
int layers;
float *points;
int points_len;
int side_length;

void fail(){
	printf("Error %s \n",strerror(errno));
	fflush(stdout);
	exit(1);
}

void moveandrotate(){
	if(keys[32]) {
		offy-=.05+keys[(int)'m'];
	}
	if(keys[(int)'c'])
		offy+=.05+keys[(int)'m'];
	if(keys[(int)'w']) {
		offz+=.05+keys[(int)'m'];
	}
	if(keys[(int)'s']) {
		offz-=.05+keys[(int)'m'];
	}
	if(keys[(int)'d']) {
		offx-=.05+keys[(int)'m'];
	}
	if(keys[(int)'a']) {
		offx+=.05+keys[(int)'m'];
	}
	if(keys[(int)'j'])
		thet+=1;
	if(keys[(int)'l'])
		thet+=-1;
	if(keys[(int)'i'])
		phi+=1;
	if(keys[(int)'k'])
		phi-=1;
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
	glRotatef(thet,0,1,0);


	//gettimeofday(&begin, NULL);
   for(int a  = 0;a<side_length-1;a++)
	for(long b = 0; b<side_length-1; b++) {
      glColor3f(0,.5/(pow(1+1/(double)(layers*layers),-points[a+b*side_length])),.5/(pow(1+1/(double)(layers*layers),-points[a+b*side_length])));
		if(a%(side_length-1)!=0) {
			glBegin(GL_TRIANGLES);
			      glVertex3f(b,points[a+b*side_length],a);
               glVertex3f(b+1,points[a+(b+1)*side_length],a);
               glVertex3f(b,points[a+1+b*side_length],(a+1));
			glEnd();
			glBegin(GL_TRIANGLES);
            glVertex3f(b+1,points[a+(b+1)*side_length],a);
            glVertex3f(b,points[a+1+b*side_length],(a+1));
            glVertex3f(b+1,points[a+1+(b+1)*side_length],a+1);
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
void keyPressed(unsigned char key, int x, int y){
	keys[key]=1;
	usleep(100);
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
	side_length = pow(2,layers)+1;
	int a =0;
	while(getline(&p,&len,f)!=-1) {
		sscanf(p, "%6f",points+a);
		//printf("%f\n",fl );
		//printf("%f\n",*(points+a) );
		a++;
	}
	//free(p);
	fclose(f);
	printf("hello\n");

	int b = 1;
	glutInit(&b,argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(640,480);

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
