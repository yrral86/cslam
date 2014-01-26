CFLAGS = -lglut -lm -lGLU -lGL -g `pkg-config clutter-1.0 --libs --cflags`
OBJS = \

OBJ_FILES = 	\

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o simulation simulation.c $(OBJ_FILES) $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) simulation
