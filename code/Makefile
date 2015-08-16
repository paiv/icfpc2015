
.PHONY: all clean

all: play_icfp2015

play_icfp2015: $(wildcard src/*.cpp) $(wildcard src/*.hpp)
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release ../src
	cd build && make
	cp build/play_icfp2015 ./

clean:
	rm -r play_icfp2015 build
