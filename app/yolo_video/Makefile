APP_TARGET=yolo_video
LOAD_TARGET=nnload

APP_OBJ_FILES=$(addsuffix .o, $(APP_TARGET))
APP_LDFLAGS=-lnest -ldemo

LOAD_OBJ_FILES=nnload.o
LOAD_LDFLAGS=-lnest 

include ../../Makefile.app
