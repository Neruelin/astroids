
clean:
	rm -rf ./build

build: asteroids.c lib.c
	rm -rf ./build || true; mkdir ./build || true;
	gcc asteroids.c -o ./build/asteroids -lm -lGL -lglut

run: build
	./build/asteroids
