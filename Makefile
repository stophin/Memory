
SRC_FILE=Memory.cpp tmemory.c memprotect.c
GCC_FLAG=-fpermissive
LIB_FILE=

all:
	g++ -g -O0  -o Memory ${GCC_FLAG} ${SRC_FILE} ${LIB_FILE}
mingw:
	g++ -g -O0 -D_MINGW_ -o Memory.exe ${GCC_FLAG} ${SRC_FILE} ${LIB_FILE}
	
clean:
	rm ./Memory.exe
	
run:
	./Memory
	
	
	
