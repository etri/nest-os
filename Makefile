include config

############################################
#   TARGET LIST that you want to execute   #
############################################

#### DO NOT USE BELOW TARGETS (not completed yet)
TARGET2=daemon
TARGET3=dmkill

ALL_TARGET=$(TARGET2) $(TARGET3)

ifeq ($(DEBUG), 1)
OPT=-O0
else
OPT=-O3
endif

CFLAGS=-I. -Iinclude -Isrc -g -std=c99 -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CFLAGS+=-D_GNU_SOURCE
CPPFLAGS=-I. -Iinclude -Isrc -g -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CPPFLAGS+=-D_GNU_SOURCE
CPPFLAGS+= -std=c++11
LDFLAGS=-lm
LDFLAGS+=-lpthread #use pthread

YOLO_LDFLAG=-Los_yolo_stcnpu/yolov2/lib -ldemo

ifeq ($(OPENCV), 1)
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV

### support opencv 3.x flags
LDFLAGS+= `pkg-config --libs opencv` -lstdc++
COMMON+= `pkg-config --cflags opencv`

### support opencv 4.x flags
#LDFLAGS+= `pkg-config --libs opencv4` -lstdc++
#COMMON+= `pkg-config --cflags opencv4`
else
LDFLAGS+= -lstdc++
endif

OBJ_FILES= src/ntask.o src/nprocess.o src/server_npuos.o src/server_comm.o src/share_comm.o src/loader.o src/scheduler.o src/bit_handler.o src/nos_bit.o src/util_time.o src/info.o src/queue.o \

DAEMON_OBJ_FILES=$(addsuffix .o, $(TARGET2))
DMKILL_OBJ_FILES=$(addsuffix .o, $(TARGET3))

all: objcreate daemonobjcreate $(TARGET2) $(TARGET3) libmake
dmon: objcreate daemonobjcreate $(TARGET2)
kill: objcreate daemonobjcreate $(TARGET3)

$(TARGET): $(OBJ_FILES) $(CPP_OBJ_FILES) $(APP_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(CPP_OBJ_FILES) $(APP_OBJ_FILES) $(YOLO_LDFLAG) -o $@ $(LDFLAGS) 

$(TARGET2) : $(OBJ_FILES) $(CPP_OBJ_FILES) $(DAEMON_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(CPP_OBJ_FILES) $(DAEMON_OBJ_FILES) -o $@ $(LDFLAGS) 

$(TARGET3) : $(OBJ_FILES) $(CPP_OBJ_FILES) $(DMKILL_OBJ_FILES)
	$(CC) $(OBJ_FILES) $(CPP_OBJ_FILES) $(DMKILL_OBJ_FILES) -o $@ $(LDFLAGS) 

objcreate: $(OBJ_FILES)

daemonobjcreate: $(DAEMON_OBJ_FILES)

.c.o:
	$(CC) -c $(COMMON) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) -c $(COMMON) $(CPPFLAGS) $< -o $@

clean:
	echo $(OBJ_FILES)
	rm -f $(ALL_TARGET) $(OBJ_FILES) $(DAEMON_OBJ_FILES) $(DMKILL_OBJ_FILES)
	rm -f *.stackdump

appclean:
	make -C app -f Makefile.del rm

libmake :
	ar rscv libnest.a $(OBJ_FILES)
	mv libnest.a lib
