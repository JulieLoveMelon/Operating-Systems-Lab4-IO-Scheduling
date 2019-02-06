iosched:iosched.o
	g++ iosched.o -o iosched

iosched.o:iosched.cpp
	g++ -std=c++11 -O2 iosched.cpp -c -o iosched.o

clean:
	rm iosched
	rm *.o
