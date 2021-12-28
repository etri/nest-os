APP_TARGET=yolo
#APP_TARGET=yolo_b
#APP_TARGET=yolo_c
#APP_TARGET=yolo_d
#APP_TARGET=yolo_e
LOAD_TARGET=nnload

APP_OBJ_FILES=$(addsuffix .o, $(APP_TARGET))
APP_LDFLAGS=-lnest -ldemo

LOAD_OBJ_FILES=nnload.o
LOAD_LDFLAGS=-lnest 

include ../../Makefile.app
