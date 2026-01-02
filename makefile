main.out: main.cpp
	g++ -Wall -std=c++14 -o main.out main.cpp

clean:
	rm main.out

run: main.out
	./main.out
