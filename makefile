all:
	mkdir -p build && cd build && cmake .. && make && ctest
test: all
	cd build && ./sql-test
clean:
	rm -rf build
