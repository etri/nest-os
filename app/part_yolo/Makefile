APP_TARGET=part_yolo
LOAD_TARGET=nnload

APP_OBJ_FILES=$(addsuffix .o, $(APP_TARGET))
APP_LDFLAGS=-lnest -ldemo

LOAD_OBJ_FILES=$(addsuffix .o, $(LOAD_TARGET))
LOAD_LDFLAGS=-lnest 

include ../../Makefile.app
