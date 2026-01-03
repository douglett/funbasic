#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

struct Tokenizer {
	struct TokenLine { vector<string> tokens; string comment, line; };
	vector<TokenLine> lines;
	
	int parsef(const string& fname) {
		// open file
		fstream fs(fname, ios::in);
		if (!fs.is_open())
			return fprintf(stderr, "Error: cannot open file: %s\n", fname.c_str()), 1;

		// parse lines
		string s, t;
		while (getline(fs, s)) {
			TokenLine tok = { {}, "", s };
			for (size_t i = 0; i < s.length(); i++) {
				// line comment
				if (s[i] == ';') {
					addtok(tok, t);
					tok.comment = s.substr(i);
					break;
				}
				// space separator
				else if (isspace(s[i]))
					addtok(tok, t);
				// add string literal
				else if (s[i] == '"') {
					addtok(tok, t);
					size_t j = i;
					for ( ; j < s.length(); j++) {
						t += s[j];
						if (s[j] == '"' && j > i)  break;
					}
					if (j >= s.length())
						return fprintf(stderr, "Error: unterminated string, line: %d\n", (int)i), 1;
					i = j;
				}
				// special character
				else if (!isalphanum(s[i]))
					addtok(tok, t), t = s[i], addtok(tok, t);
				// letter / number
				else
					t += s[i];
			}
			addtok(tok, t);
			lines.push_back(tok);
		}

		// ok
		return 0;
	}

	void show() const {
		for (size_t i = 0; i < lines.size(); i++) {
			auto& line   = lines[i];
			auto& tokens = line.tokens;
			printf("%02d  %02d  ", (int)i+1, (int)tokens.size());
			for (auto& tok : tokens)
				printf("%s ", tok.c_str());
			printf("%s\n", line.comment.c_str());
		}
	}

	// helpers
	static void addtok(TokenLine& line, string& s) {
		if (!s.length())  return;
		line.tokens.push_back(s);
		s = "";
	}
	static int isalphac(char c) {
		return isalpha(c) || c == '_';
	}
	static int isalphanum(char c) {
		return isalnum(c) || c == '_';
	}
};


struct Runtime {
	struct Memory {
		enum MEMTYPE { NUM = 0, STR };
		MEMTYPE type; int num; string str;
	};
	Tokenizer tok;
	vector<string> lastlist;
	size_t lpos = 0, pos = 0;
	int flagw_modulename = 0;
	string modulename;
	map<string, Memory> memory;

	// --- Parsing ---
	int run() {
		lpos = 0;
		while (lpos < tok.lines.size())
			runline();
		return 0;
	}

	int runline() {
		string cmd = tokens().size() ? tokens()[0] : "noop";
		string errormsg;
		pos = 1;

		if (cmd != "noop" && cmd != "module" && modulename.size() == 0 && !flagw_modulename) {
			warn("module name expected on first line");
			flagw_modulename = 1;
		}

		// no-operation
		if (cmd == "noop") {
			expect("$eol");
			lpos++;
		}
		// module name
		else if (cmd == "module") {
			if (modulename.length())
				error("module name redefined");
			expect("$identifier");
			modulename = last();
			expect("$eol");
			lpos++;
		}
		// dim new variable
		else if (cmd == "dim") {
			expect("$identifier");
			auto name = last();
			expect("=");
			if (memory.count(name))
				error("redim of " + name);
			if (accept("$number"))
				memory[name] = { Memory::NUM, strtoint(last()) };
			else if (accept("$string"))
				memory[name] = { Memory::STR, 0, stripliteral(last()) };
			else
				error("type error");
			expect("$eol");
			lpos++;
		}
		// set variable
		else if (cmd == "let") {
			// variable
			expect("$identifier");
			auto name = last();
			auto& var = getmem(name);
			// operator
			poperator();
			auto op = last();
			// argument
			Memory arg;
			if (accept("$number"))
				arg = { Memory::NUM, strtoint(last()) };
			else if (accept("$string"))
				arg = { Memory::STR, 0, stripliteral(last()) };
			else if (accept("$identifier"))
				arg = getmem(last());
			else
				error("expected argument");
			if (var.type != arg.type)
				error("type error");
			// operation
			switch (var.type) {
				case Memory::NUM:
					if      (op ==  "=")  var.num  = arg.num;
					else if (op == "+=")  var.num += arg.num;
					else if (op == "-=")  var.num -= arg.num;
					else if (op == "*=")  var.num *= arg.num;
					else if (op == "/=")  var.num /= arg.num;
					else    error("operator invalid on number");
					break;
				case Memory::STR:
					if   (op == "=")  var.str = arg.str;
					else error("operator invalid on string");
					break;
			}
			// end line
			expect("$eol");
			lpos++;
		}
		// jump
		else if (cmd == "goto") {
			expect("$identifier");
			auto label = last();
			expect("$eol");
			jumpto(label);
		}
		// conditional
		else if (cmd == "if") {
			expect("$identifier");
			auto name = last();
			if (getmem(name).type != Memory::NUM)
				error("type error");
			expect("then");
			expect("goto");
			expect("$identifier");
			auto label = last();
			expect("$eol");
			if (getmem(name).num)
				jumpto(label);
			else
				lpos++;
		}
		// print variable to terminal
		else if (cmd == "print") {
			for (size_t i = 1; i < tokens().size(); i++) {
				auto& tok = tokens().at(i);
				if (isnumber(tok))
					printf("%d ", strtoint(tok));
				else if (isstrliteral(tok))
					printf("%s ", stripliteral(tok).c_str());
				else if (isidentifier(tok)) {
					auto& var = getmem(tok);
					switch (var.type) {
						case Memory::NUM:  printf("%d ", var.num);  break;
						case Memory::STR:  printf("%s ", var.str.c_str());  break;
					}
				}
				else
					error("bad argument");
			}
			printf("\n");
			lpos++;
		}
		// get input from terminal
		else if (cmd == "input") {
			expect("$identifier");
			auto name = last();
			if (getmem(name).type != Memory::STR)
				error("type error");
			expect("$eol");
			getline(cin, getmem(name).str);
			lpos++;
		}
		// label
		else if (isidentifier(cmd) && accept(":")) {
			expect("$eol");
			lpos++;  // also noop
		}
		// unknown
		else
			error("unknown command [" + cmd + "]");

		// OK
		return 0;
	}

	int poperator() {
		vector<string> operators = { "=", "+ =", "- =", "* =", "/ =" };
		for (auto op : operators)
			if (accept(op))
				return 1;
		return error("expected operator");
	}

	int error(const string& msg) const {
		throw runtime_error(msg + " (line " + to_string(lpos + 1) + ", " + to_string(pos) + ")");
	}
	int warn(const string& msg) const {
		return printf("Warning: %s (line %d, %d)\n", msg.c_str(), (int)lpos + 1, (int)pos), 0;
	}

	// --- Runtime State ---
	int jumpto(const string& label) {
		for (size_t i = 0; i < tok.lines.size(); i++) {
			auto& tokens = tok.lines[i].tokens;
			if (tokens.size() == 2 && tokens.at(0) == label && tokens.at(1) == ":")
				return lpos = i;
		}
		return error("goto unknown label: [" + label + "]");
	}
	Memory& getmem(const string& name) {
		if (!memory.count(name))
			error("unknown variable name [" + name + "]");
		return memory.at(name);
	}

	// --- Parsing Helpers ---
	const Tokenizer::TokenLine& line() const {
		if (lpos >= tok.lines.size())
			error("line out-of-range");
		return tok.lines[lpos];
	}
	const vector<string>& tokens() const {
		return line().tokens;
	}
	int strtoint(const string& str) {
		try { return stoi(str); }
		catch (invalid_argument& e) { return error("strtoint: bad argument [" + str + "]"); }
	}
	string memtostr(const Memory& mem) {
		switch (mem.type) {
			case Memory::STR:  return mem.str;
			default:
			case Memory::NUM:  return to_string(mem.num);
		}
	}
	int expect(const string& cmd) {
		if (!accept(cmd))
			error("expected [" + cmd + "]");
		return 1;
	}
	int accept(const string& cmd) {
		lastlist = {};
		int p = pos;
		auto cmdlist = splitws(cmd);
		if (cmdlist.size() == 0)
			error("command list empty [" + cmd + "]");
		for (auto& cm : cmdlist) {
			if (!acceptone(cm))
				return pos = p, 0;
			lastlist.push_back(cm == "$eol" ? "<eol>" : tokens().at(pos - 1));
		}
		return 1;
	}
	int acceptone(const string& cmd) {
		if (pos >= tokens().size()) {
			if (cmd == "$eol")
				return 1;
			error("token out-of-range");
		}
		auto& tok = tokens().at(pos);
		if (cmd == "$identifier")
			return isidentifier(tok) && ++pos;
		else if (cmd == "$number")
			return isnumber(tok) && ++pos;
		else if (cmd == "$string")
			return isstrliteral(tok) && ++pos;
		else if (cmd == tok)
			return ++pos;
		return 0;
	}
	string last() {
		return joinvs(lastlist, "");
	}

	// --- Token Helpers ---
	static int isidentifier(const string& s) {
		if (s.length() == 0)  return 0;
		if (!Tokenizer::isalphac(s[0])) return 0;
		for (auto c : s)
			if (!Tokenizer::isalphanum(c))  return 0;
		return 1;
	}
	static int isnumber(const string& s) {
		if (s.length() == 0)  return 0;
		for (auto c : s)
			if (!isdigit(c))  return 0;
		return 1;
	}
	static int isstrliteral(const string& s) {
		return s.length() >= 2 && s.front() == '"' && s.back() == '"';
	}
	static string stripliteral(const string& s) {
		if (!isstrliteral(s))  return s;
		return s.substr(1, s.length() - 2);
	}
	static vector<string> splitws(const string& str) {
		vector<string> vs;
		string s;
		stringstream ss(str);
		while (ss >> s)
			vs.push_back(s);
		return vs;
	}
	static string joinvs(const vector<string>& vs, const string& glue = " ") {
		string str;
		int first = 1;
		for (auto& s : vs)
			if (first)  str += s, first = 0;
			else  str += glue + s; 
		return str;
	}
};


int main() {
	printf("funbasic parser online!\n\n");

	Tokenizer tok;
	// tok.parsef("asm/test.asm");
	tok.parsef("asm/maths.asm");
	// tok.parsef("asm/doug1.asm");
	tok.show();
	printf("\n");

	Runtime run;
	run.tok = tok;
	run.run();
}