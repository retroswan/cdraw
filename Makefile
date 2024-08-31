include .env

CC?=gcc
CFLAGS_DEBUG?=-g -std=c99
CFLAGS_RELEASE?=-O2 -std=c99
CFLAGS?=${CFLAGS_DEBUG}
INCS?=
LIBS?=
PLATFORM?=Debug
RPATH?=-Wl,-rpath=./

SRC:=$(wildcard src/*.c src/**/*.c src/**/**/*.c src/**/**/**/*.c src/**/**/**/**/*.c)
OBJ:=$(patsubst src/%.c, src/%.o, $(SRC))

.PHONY=build
build:
	mkdir -p bin
	mkdir -p bin/${PLATFORM}
	${foreach file, ${SRC}, ${CC} ${CFLAGS} -c ${file} -o ${patsubst src/%.c, src/%.o, ${file}} ${INCS} &&} echo
	${CC} ${CFLAGS} ${OBJ} -o bin/${PLATFORM}/main ${LIBS} ${RPATH}

.PHONY=debug
debug:
	make build
	
.PHONY=release
release:
	make build CFLAGS="${CFLAGS_RELEASE}" PLATFORM="Release"

.PHONY=clean
clean:
	rm -f ${OBJ}
	rm -f bin/Debug/main
	rm -f bin/Release/main

.PHONY=shaders
shaders:
	cd bin/Debug/Content/Shaders/Source && ./compile.sh

.PHONY=valgrind
valgrind:
	valgrind --leak-check=full bin/Debug/main &> valgrind.txt
