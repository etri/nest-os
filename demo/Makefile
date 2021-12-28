# if you need debugging, set it to 1, else 0
include config

export DEBUG
export CC
export CPP

LIBNAME=libdemo.a

ifeq ($(DEBUG), 1)
OPT=-O0
else
OPT=-O3
endif

CFLAGS=-I. -Iinclude -I../include -Ios -Inn -Icpu -Idemo -g -std=c99 -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CFLAGS+=-D_GNU_SOURCE
CPPFLAGS=-I. -Iinclude -I../include -Ios -Inn -Icpu -Idemo -g -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CPPFLAGS+=-D_GNU_SOURCE
CPPFLAGS+= -std=c++11
LDFLAGS=-lm
LDFLAGS+=-ldl
LDFLAGS+=-lpthread #use pthread

LDFLAGS+= -lrt
LDFLAGS+=-mavx 

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

OBJ_FILES= box.o image.o name.o \

LIB_OBJ_FILES=$(OBJ_FILES) \

LIB_OBJ_FILES2=$(OBJ_FILES) \

ifeq ($(OPENCV), 1)
OBJ_FILES+=image_opencv.o
else
endif

all: objcreate libmake

objcreate: $(OBJ_FILES)

.c.o:
	$(CC) -c $(COMMON) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) -c $(COMMON) $(CPPFLAGS) $< -o $@

clean:
	echo $(OBJ_FILES)
	rm -f $(OBJ_FILES) $(CPP_OBJ_FILES) *.o
	rm -f *.stackdump

libmake:
	ar rscv $(LIBNAME) $(LIB_OBJ_FILES) 
	mv $(LIBNAME) ../lib
