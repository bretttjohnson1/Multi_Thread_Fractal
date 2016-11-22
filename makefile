all:
	gcc compute.c -o compute.o -lpthread -lm
	gcc render.c -lm -lGL -lglut -lGLU  -o render.o -g
