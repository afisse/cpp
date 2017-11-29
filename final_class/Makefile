all:main.out

main.out: main.o cursor.o cursor_impl.o mongo_cursor.o
	g++ $^ -o $@

%.o: %.cpp %.hpp
	g++ -c $^

clean:
	rm -rf *.out *.o
