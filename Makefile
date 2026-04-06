default: link

reset: delete | all

all: compileglad | link

link: compilemain
	g++ bin/*.o -g -Llib -lglfw3 -lopengl32 -lgdi32 -o bin/program

compileglad:
	g++ include/glad/glad.c -g -c -Iinclude -o bin/glad.o

compilemain:
	g++ -g -c -Iinclude src/main.cpp -o bin/main.o

clean:
	rm bin/main.o
	rm bin/program.exe

delete:
	rm bin/*