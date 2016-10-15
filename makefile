chip8: chip8.c chip8.h
	gcc -std=c11 -o chip8 `pkg-config --cflags --libs sdl2` chip8.c

clean:
	rm chip8
