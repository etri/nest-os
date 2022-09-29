APP_TARGET=resnet18
LOAD_TARGET=nnload

APP_OBJ_FILES=$(addsuffix .o, $(APP_TARGET)) resnet18_lib.o cque.o
APP_LDFLAGS=-lnest -lpthread -ldemo

LOAD_OBJ_FILES=nnload.o
LOAD_LDFLAGS=-lnest 

include ../../Makefile.app
