CC=g++
LIB_DIR=./
LIBS=-L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lpthread
LIB=-lhcnetsdk
SILENT_LIB=-lsilenthcnetsdk
CFLAGS=-std=c++11 -m32 -Wall

all:
	$(CC) $(CFLAGS) -O2 src/main.cpp $(LIBS) $(SILENT_LIB) -o web-cam-bruteforcer

clean:
	rm web-cam-bruteforcer
