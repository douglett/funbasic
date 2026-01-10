main.out: main.cpp tokenhelpers.hpp tokenizer.hpp asm-runtime.hpp
	g++ -Wall -std=c++14 -o main.out main.cpp

clean:
	rm main.out

run: main.out
	./main.out
