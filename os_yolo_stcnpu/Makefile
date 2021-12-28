# if you need debugging, set it to 1, else 0
include config

TARGET=nos
ALL_TARGET=$(TARGET)

ifeq ($(DEBUG), 1)
OPT=-O0
else
OPT=-O3
endif

CFLAGS=-I. -Iinclude -I../include -Isrc -Inpu -g -std=c99 -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CFLAGS+=-D_GNU_SOURCE

LDFLAGS=-lm
LDFLAGS+=-lpthread #use pthread

## DUE TO INCLUDE LIBRARY YOLOV2, THE FOLLOWING LIBFLAGS SHOULD BE INCLUDED
LDFLAGS+=-lstdc++ #use c++
LDFLAGS+=`pkg-config --libs opencv` #use opencv package
ifeq ($(NPU), 1)
LDFLAGS+=-lrt
endif

ifeq ($(NPU), 1)
CFLAGS+=-DSTCNPU_INCLUDED
endif

OBJ_FILES= src/task.o src/os_sched.o src/queue.o src/npuos.o src/sock_handler.o src/share_comm.o src/bit_handler.o src/nn_object.o src/util_time.o src/os_shm.o npu/npu.o 

NNOBJ_FILES= yolov2/lib/libyolov2.a

CPUOS_OBJ_FILES=$(addsuffix .o, $(TARGET))

all: objcreate $(TARGET) move

$(TARGET) : $(OBJ_FILES) $(NNOBJ_FILES) $(CPUOS_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(NNOBJ_FILES) $(CPUOS_OBJ_FILES) -o $@ $(LDFLAGS)

objcreate: $(OBJ_FILES)

move:
	mv $(TARGET) yolov2

.c.o:
	$(CC) -c $(COMMON) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) -c $(COMMON) $(CPPFLAGS) $< -o $@

clean:
	echo $(OBJ_FILES)
	rm -f $(ALL_TARGET) $(OBJ_FILES) *.o
	#make -C npu clean
	rm -f *.stackdump
