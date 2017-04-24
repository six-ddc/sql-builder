all:
	cd test && mkdir -p build && cd build && cmake .. && make && ctest
test: all
	cd test/build && ./sql-test
clean:
	rm -rf test/build
