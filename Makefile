CFLAGS = -lglut -lm -lGLU -lGL -lpthread -g -pthread -DLINUX -Wall
OBJS = \
	boxmuller \
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
	swarm

OBJ_FILES = 	\
		boxmuller.o \
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
		swarm.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o slam slam.c $(OBJ_FILES) urg_driver/*.o $(CFLAGS)
	gcc -o replay replay.c $(OBJ_FILES) urg_driver/*.o $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) slam replay
