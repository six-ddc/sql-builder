all:
	mkdir -p build && cd build && cmake .. && make 
test:
	cd build && ./sql-test
clean:
	rm -rf build
