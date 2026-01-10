#include "tokenizer.hpp"
#include "asm-runtime.hpp"
#include <iostream>
using namespace std;


int main() {
	printf("funbasic parser online!\n\n");

	Tokenizer tok;
	tok.parsef("asm-asm/test.asm");
	tok.show();
	printf("\n");

	AsmRuntime run;
	run.tok = tok;
	run.run();
}