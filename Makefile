CFLAGS = -lglut -lm -lGLU -lGL -g `pkg-config clutter-1.0 --libs --cflags`
OBJS = \
	filter \
	fuzzy_controller \
	lazygl \
	motor \
	particle \
	random \
	robot \
	sensor \
	scip/scipBase \
	scip/scipUtil \
	scip/scip1 \
	scip/scip2 \
	swarm \
	ziggurat

OBJ_FILES = 	\
		filter.o \
		fuzzy_controller.o \
		lazygl.o \
		motor.o \
		particle.o \
		random.o \
		robot.o \
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
	gcc -o record record.c $(OBJ_FILES) $(CFLAGS)
	gcc -o simulation simulation.c $(OBJ_FILES) $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) simulation record
