########################################################
# application directory must include
# 'config' file that defines CC, CPP, DEBUG, OPENCV
#
# Makefile in application directory must include
# TARGET=xxx
# APP_OBJ_FILES=aaa.o bbb.o ...
# APP_LDFLAGS=-lnest -L$(REFLIB) -l$(lib) 
#
# Change the NEST_HOME variable for your setting
########################################################

# notice : NEST_HOME environment variable must be registered
# e.g. in your ~/.bashrc file, add the following line
# export NEST_HOME=/home/sheart95/nestos-1.1

include config

ifeq ($(DEBUG), 1)
OPT=-O0
else
OPT=-O3
endif

CFLAGS=-I. -I$(NEST_HOME) -I$(NEST_HOME)/include -I$(NEST_HOME)/src -g -std=c99 -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CFLAGS+=-D_GNU_SOURCE
CPPFLAGS=-I. -I$(NEST_HOME) -I$(NEST_HOME)/include -I$(NEST_HOME)/src -g -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC $(OPT)
CPPFLAGS+=-D_GNU_SOURCE
CPPFLAGS+= -std=c++11
LDFLAGS=-lm
LDFLAGS+=-lpthread #use pthread
LDFLAGS+=-L$(NEST_HOME)/lib

ifeq ($(OPENCV), 1)
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV

### support opencv 3.x flags
LDFLAGS+= `pkg-config --libs opencv` -lstdc++
COMMON+= `pkg-config --cflags opencv`

### support opencv 4.x flags #LDFLAGS+= `pkg-config --libs opencv4` -lstdc++ #COMMON+= `pkg-config --cflags opencv4`
else
LDFLAGS+= -lstdc++
endif

all: $(APP_TARGET) $(LOAD_TARGET)

$(APP_TARGET): $(APP_OBJ_FILES)
	$(CC) $(APP_OBJ_FILES) $(APP_LDFLAGS) $(LDFLAGS) -o $@ 

$(LOAD_TARGET): $(LOAD_OBJ_FILES)
	$(CC) $(LOAD_OBJ_FILES) $(LOAD_LDFLAGS) $(LDFLAGS) -o $@ 

.c.o:
	$(CC) -c $(COMMON) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) -c $(COMMON) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(APP_TARGET) $(APP_OBJ_FILES)
	rm -f $(LOAD_TARGET) $(LOAD_OBJ_FILES)
	rm -f *.stackdump

print:
	echo $(NEST_HOME)
