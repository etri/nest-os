# if you need debugging, set it to 1, else 0
include config
GPU=0

TARGET=nos
ALL_TARGET=$(TARGET)

ifeq ($(DEBUG), 1)
OPT=-O0
else
OPT=-O3
endif

CFLAGS=-I. -Iinclude -I../include -Isrc -Inpu -g -std=c99 -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CFLAGS+=-D_GNU_SOURCE
CFLAGS+=-DOPENCV `pkg-config --cflags opencv`

LDFLAGS=-lm
LDFLAGS+=-lpthread #use pthread
ifeq ($(GPU), 1)
LDFLAGS+= -L/usr/local/cuda/lib64 -lcuda -lcudart -lcublas -lcurand
endif


## DUE TO INCLUDE LIBRARY YOLOV2, THE FOLLOWING LIBFLAGS SHOULD BE INCLUDED
LDFLAGS+=-lstdc++ #use c++
LDFLAGS+=`pkg-config --libs opencv` #use opencv package
#ifeq ($(NPU), 1)
LDFLAGS+=-ldl
LDFLAGS+=-lrt
#LDFLAGS+=-mavx
#endif

OBJ_FILES= src/task.o src/os_sched.o src/queue.o src/npuos.o src/sock_handler.o src/share_comm.o src/bit_handler.o src/nn_object.o src/os_shm.o npu/npu.o
#OBJ_FILES+=src/util_time.o # not needed in darknet since it is already included

#NNOBJ_FILES= yolov2/lib/libyolov2.a darknet/libdarknet.a
NNOBJ_FILES=darknet/libdarknet_cpu.a

CPUOS_OBJ_FILES=$(addsuffix .o, $(TARGET))

all: objcreate $(TARGET) move

$(TARGET) : $(OBJ_FILES) $(NNOBJ_FILES) $(CPUOS_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(NNOBJ_FILES) $(CPUOS_OBJ_FILES) -o $@ $(LDFLAGS)

objcreate: $(OBJ_FILES)

move:
ifeq ($(GPU), 1)
	mv $(TARGET) darknet/nos_gpu
else
	mv $(TARGET) darknet/nos_cpu 
endif

.c.o:
	$(CC) -c $(COMMON) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) -c $(COMMON) $(CPPFLAGS) $< -o $@

clean:
	echo $(OBJ_FILES)
	rm -f $(ALL_TARGET) $(OBJ_FILES) *.o
	#make -C npu clean
	rm -f *.stackdump
