all:
	mkdir -p build && cd build && cmake .. && make && ctest
test:
	cd build && ./sql-test
clean:
	rm -rf build
