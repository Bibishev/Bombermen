PROJECT_NAME = bombermen
CC = g++

SRC_FILES = \
    src/main.cpp \
    src/bomb/bomb.cpp \
    src/bomber/bomber.cpp \
    src/mob/mob.cpp \
    src/baf/baf.cpp \
    src/wiu/button.cpp \
    src/map/map.cpp

CFLAGS = -Wall -std=c++11 -O2 -g
INCLUDE_PATHS = -I"C:/raylib/raylib/src" -I"src" -I"src/bomb" -I"src/bomber" -I"src/mob" -I"src/baf" -I"src/wiu" -I"src/map"
LDLIBS = -L"C:/raylib/raylib/src" -lraylib -lopengl32 -lgdi32 -lwinmm -lm

all:
	$(CC) -o $(PROJECT_NAME) $(SRC_FILES) $(CFLAGS) $(INCLUDE_PATHS) $(LDLIBS)

run: all
	./$(PROJECT_NAME)

clean:
	del $(PROJECT_NAME).exe

.PHONY: all clean run