CFLAGS = -lglut -lm -lGLU -lGL -g `pkg-config clutter-1.0 --libs --cflags`
OBJS = \
	filter \
	fuzzy_controller \
	motor \
	particle \
	robot \
	sensor \
	scip/scipBase \
	scip/scipUtil \
	scip/scip1 \
	scip/scip2

OBJ_FILES = 	\
		filter.o \
		fuzzy_controller.o \
		motor.o \
		particle.o \
		robot.o \
		sensor.o \
		scip/scipBase.o \
		scip/scipUtil.o \
		scip/scip1.o \
		scip/scip2.o
all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o simulation simulation.c $(OBJ_FILES) $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) simulation
