all: sql.cpp
	g++ -std=c++11 -o libsql_builder.so -shared -fPIC -D_DEBUG sql.cpp
test: test.cpp all
	g++ -std=c++11 -g -ggdb -lboost_system -lboost_test_exec_monitor -lsql_builder -L. -D_DEBUG -o test test.cpp
run: test
	./test
