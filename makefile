default:
	gcc -obuild/basilisktest.exe src/main.c src/basilisk/* -Iinclude/basilisk/ -Iinclude/gl/ -Iinclude/ -Llib -Wall -Wno-switch -lglfw3 -lgdi32 -lglad -llodepng -Wno-varargs