exec = compiler.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -g
lib = stdlib/lib.s

$(exec): $(objects) $(lib)
	make lib
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c include/%.h
	gcc -c $(flags) $< -o $@

lib: $(lib)
	gcc -c stdlib/lib.s -o stdlib/lib.o

run:
	make
	./compiler.out ./examples/example

clean:
	-rm *.out
	-rm *.o
	-rm src/*.o