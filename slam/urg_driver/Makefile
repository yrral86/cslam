# Makefile for libs/c
# $Id: Makefile,v 4537e5141a86 2011/02/17 07:10:04 Satofumi $

# Compile options
CC = gcc
#CFLAGS = -O2 -Wall -Werror -W $(INCLUDES)
CFLAGS = -g -O0 -Wall -Werror -W $(INCLUDES)
INCLUDES =
LDFLAGS =
LDLIBS =

# Target
URG_C_LIB = liburg_c.a
TARGET = $(URG_C_LIB)

all : $(TARGET)

clean :
	$(RM) *.o $(TARGET)

depend :
	makedepend -Y -- $(INCLUDES) -- $(wildcard *.h *.c)

.PHONY : all clean depend
######################################################################
$(URG_C_LIB) : \
	$(URG_C_LIB)(urg_sensor.o) \
	$(URG_C_LIB)(urg_utils.o) \
	$(URG_C_LIB)(urg_debug.o) \
	$(URG_C_LIB)(urg_connection.o) \
	$(URG_C_LIB)(urg_serial.o) \
	$(URG_C_LIB)(urg_serial_utils.o) \
	$(URG_C_LIB)(urg_tcpclient.o) \
	$(URG_C_LIB)(urg_ring_buffer.o) \

urg_serial_utils.o : urg_serial_utils_windows.c urg_serial_utils_linux.c

# DO NOT DELETE

urg_connection.o: urg_connection.h urg_serial.h
urg_connection.o: urg_detect_os.h urg_ring_buffer.h
urg_connection.o: urg_tcpclient.h
urg_debug.o: urg_debug.h urg_sensor.h
urg_debug.o: urg_connection.h urg_serial.h
urg_debug.o: urg_detect_os.h urg_ring_buffer.h
urg_debug.o: urg_tcpclient.h
urg_ring_buffer.o: urg_ring_buffer.h
urg_sensor.o: urg_sensor.h urg_connection.h
urg_sensor.o: urg_serial.h urg_detect_os.h
urg_sensor.o: urg_ring_buffer.h urg_tcpclient.h
urg_sensor.o: urg_errno.h
urg_serial.o: urg_serial.h urg_detect_os.h
urg_serial.o: urg_ring_buffer.h urg_serial_linux.c
urg_serial_utils.o: urg_serial_utils.h urg_detect_os.h
urg_serial_utils.o: urg_serial_utils_linux.c
urg_serial_utils_linux.o: urg_serial_utils.h
urg_serial_utils_windows.o: urg_serial_utils.h
urg_serial_utils_windows.o: urg_detect_os.h
urg_serial_windows.o: urg_serial.h urg_detect_os.h
urg_serial_windows.o: urg_ring_buffer.h
urg_tcpclient.o: urg_detect_os.h urg_tcpclient.h
urg_tcpclient.o: urg_ring_buffer.h
urg_utils.o: urg_utils.h urg_sensor.h
urg_utils.o: urg_connection.h urg_serial.h
urg_utils.o: urg_detect_os.h urg_ring_buffer.h
urg_utils.o: urg_tcpclient.h urg_errno.h
