OPENGL_ARGS=-lGL -lglut -lGLU

compute:
	gcc src/compute.c -o compute.o -lpthread -lm -g -std=c99
render:
	gcc src/render.c $(OPENGL_ARGS) -lm -o render.o -g
