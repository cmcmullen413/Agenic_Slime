compileglad: include/glad/gladc
	gcc include/glad/gladc -g -c -o bin/glad.o

compilecpp: src/*.cpp
	