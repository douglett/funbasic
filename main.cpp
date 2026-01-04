#include "tokenizer.hpp"
#include "runtime.hpp"
#include <iostream>
using namespace std;


int main() {
	printf("funbasic parser online!\n\n");

	Tokenizer tok;
	tok.parsef("asm/test.asm");
	// tok.parsef("asm/maths.asm");
	// tok.parsef("asm/doug1.asm");
	tok.show();
	printf("\n");

	Runtime run;
	run.tok = tok;
	run.run();
}