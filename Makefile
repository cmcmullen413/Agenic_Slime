default: link

link: compileglad | compilemain
	g++ bin/*.o -g -Llib -lglfw3 -lopengl32 -lgdi32 -o bin/program

compileglad:
	gcc include/glad/glad.c -g -c -Iinclude -o bin/glad.o

compilemain:
	g++ -g -c -Iinclude src/main.cpp -o bin/main.o

clean:
	rm bin/*