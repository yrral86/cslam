CFLAGS = -lglut -lm -lGLU -lGL -lpthread -g -pthread
OBJS = \
	buffer \
	lazygl \
	particle \
	random \
	sensor \
	swarm \
	ziggurat

OBJ_FILES = 	\
		buffer.o \
		lazygl.o \
		particle.o \
		random.o \
		sensor.o \
		swarm.o \
		ziggurat.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o slam slam.c $(OBJ_FILES) urg_driver/*.o $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) slam
