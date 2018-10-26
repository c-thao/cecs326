all: link
	echo "Program files created."

link: compile
	echo "Linking files..."
	gcc -o fish fish.o -lrt -lpthread
	gcc -o pellet pellet.o -lrt -lpthread
	gcc -o swim_mill swim_mill.o -lrt -lpthread

compile:
	echo "Compiling c files..."
	gcc -c fish.c
	gcc -c pellet.c
	gcc -c swim_mill.c

clean:
	rm swim_mill.o fish.o pellet.o
