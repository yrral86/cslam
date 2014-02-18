CFLAGS = -lglut -lm -lGLU -lGL -lpthread -g -pthread
OBJS = \
	buffer \
	landmark \
	lazygl \
	particle \
	random \
	sensor \
	scip/scipBase \
	scip/scipUtil \
	scip/scip1 \
	scip/scip2 \
	swarm \
	ziggurat

OBJ_FILES = 	\
		buffer.o \
		landmark.o \
		lazygl.o \
		particle.o \
		random.o \
		sensor.o \
		scip/scipBase.o \
		scip/scipUtil.o \
		scip/scip1.o \
		scip/scip2.o \
		swarm.o \
		ziggurat.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o slam slam.c $(OBJ_FILES) urg_driver/*.o $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) slam
