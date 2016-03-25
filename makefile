all: sql.cpp
	g++ -std=c++11 -o libsql_builder.so -shared -fPIC -D_DEBUG sql.cpp
test: test.cpp all
	g++ -std=c++11 -g -ggdb -lsql_builder -L. -D_DEBUG -o test test.cpp
run: test
	./test
