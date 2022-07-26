exec = compiler.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
headers = $(wildcard src/include/*.h)
flags = -Wall -g
lib = stdlib/lib.s
makefile = Makefile

$(exec): $(objects) $(lib) $(makefile)
	make lib
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c $(headers) $(makefile)
	gcc -c $(flags) $< -o $@

lib: $(lib)
	gcc -c stdlib/libas.s -o stdlib/libs.o
	gcc -c stdlib/lib.c -o stdlib/libc.o
	ld -r stdlib/libs.o stdlib/libc.o -o stdlib/lib.o

clean:
	-rm *.out
	-rm stdlib/*.o
	-rm src/*.o
	-rm program.o
	-rm program.s