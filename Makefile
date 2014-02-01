CFLAGS = -lglut -lm -lGLU -lGL -g
OBJS = \
	lazygl \
	particle \
	random \
	sensor \
	scip/scipBase \
	scip/scipUtil \
	scip/scip1 \
	scip/scip2 \
	ziggurat

OBJ_FILES = 	\
		lazygl.o \
		particle.o \
		random.o \
		sensor.o \
		scip/scipBase.o \
		scip/scipUtil.o \
		scip/scip1.o \
		scip/scip2.o \
		ziggurat.o

all:
	for i in $(OBJS); do \
		(gcc -c -o $$i.o $$i.c $(CFLAGS)); \
	done
	gcc -o slam slam.c $(OBJ_FILES) $(CFLAGS)

clean:
	rm -f *~ $(OBJ_FILES) slam
