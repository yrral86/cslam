CFLAGS = -lglut -lm -lGLU -lGL
OBJS = \
	lazygl

OBJ_FILES = 	\
		lazygl.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o simulation simulation.c $(OBJ_FILES) $(CFLAGS) -export-dynamic

clean:
	rm -f *~ $(OBJ_FILES) simulation
