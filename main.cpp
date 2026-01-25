#include "tokenizer.hpp"
#include "asm-runtime.hpp"
#include <iostream>
using namespace std;


int main() {
	printf("fun-asm parser online!\n\n");

	Tokenizer tok;
	// tok.parsef("asm-asm/basics.asm");
	// tok.parsef("asm-asm/arrays.asm");
	tok.parsef("asm-asm/test.asm");
	tok.show();
	printf("\n");

	AsmRuntime run;
	run.tok = tok;
	run.run();
}