compute:
	gcc compute.c -o compute.o -lpthread -lm -g -std=c99
render:
	gcc render.c -lm -lGL -lglut -lGLU  -o render.o -lGLEW -g
