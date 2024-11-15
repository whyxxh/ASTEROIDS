CFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

build:
	gcc *.c $(CFLAGS) -o GAME

run:
	./GAME

clean:
	rm GAME
