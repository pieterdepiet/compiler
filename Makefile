exec = compiler.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -Wall -g
lib = stdlib/lib.s
makefile = Makefile

$(exec): $(objects) $(lib) $(makefile)
	make lib
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c include/%.h $(makefile)
	gcc -c $(flags) $< -o $@

as: $(sources:.c=.s) $(makefile)

%.s: %.c include/%.h $(makefile)
	gcc -S -g $(flags) $< -o $@

aslink:
	gcc src/*.s $(flags) -o $(exec)

lib: $(lib)
	gcc -c stdlib/lib.s -o stdlib/lib.o

run:
	make
	./compiler.out ./examples/example

clean:
	-rm *.out
	-rm stdlib/*.o
	-rm src/*.o