exec = csach.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -g

# Find if you have gcc or clang and uses it
CC = $(shell command -v gcc >/dev/null 2>&1 && echo "gcc" || echo "clang")

$(exec): $(objects)
	$(CC) $(objects) $(flags) -o $(exec)

%.o: %.c include/%.h
	$(CC) -c $(flags) $< -o $@

# System install
install:
	make
	cp ./csach.out /usr/local/bin/csach

# System uninstall
uninstall:
	rm /usr/local/bin/csach

# Clean the object files and the .out fle
clean:
	-rm *.out
	-rm src/*.o