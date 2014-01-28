CFLAGS = -lglut -lm -lGLU -lGL -g `pkg-config clutter-1.0 --libs --cflags`
OBJS = \
	filter \
	fuzzy_controller \
	motor \
	robot \
	sensor

OBJ_FILES = 	\
		filter.o \
		fuzzy_controller.o \
		motor.o \
		robot.o \
		sensor.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o simulation simulation.c $(OBJ_FILES) $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) simulation
