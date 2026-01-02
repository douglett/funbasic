#include <iostream>
#include <string>
#include <vector>
#include <fstream>
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
		enum MEMTYPE { NUM = 0, STRING };
		MEMTYPE type; int num; string str;
	};
	Tokenizer tok;
	size_t lpos = 0, pos = 0;
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

		// no-operation
		if (cmd == "noop")
			lpos++;
		// dim new variable
		else if (cmd == "dim") {
			expect("identifier");
			auto name = last();
			expect("match", "=");
			if (memory.count(name))
				error("redim of " + name);
			if (accept("number"))
				memory[name] = { Memory::NUM, stoi(last()) };
			else if (accept("string"))
				memory[name] = { Memory::STRING, 0, stripliteral(last()) };
			else
				error("type error");
			expect("eol");
			lpos++;
		}
		// set variable
		else if (cmd == "let") {
			expect("identifier");
			auto& name = last();
			if (accept("match", "=")) {
				if (accept("number") && getmem(name).type == Memory::NUM)
					getmem(name).num = stoi(last());
				else if (accept("string") && getmem(name).type == Memory::STRING)
					getmem(name).str = stripliteral(last());
				else
					error("type error");
				expect("eol");
				lpos++;
			}
			else if (accept("match", "+") || accept("match", "-") || accept("match", "*") || accept("match", "/")) {
				auto& op = last();
				expect("match", "=");
				int num = 0;
				if (accept("number") && getmem(name).type == Memory::NUM)
					num = stoi(last());
				else if (accept("identifier") && getmem(name).type == Memory::NUM && getmem(last()).type == Memory::NUM)
					num = getmem(last()).num;
				else
					error("type error");
				if      (op == "+")  getmem(name).num += num;
				else if (op == "-")  getmem(name).num -= num;
				else if (op == "*")  getmem(name).num *= num;
				else if (op == "/")  getmem(name).num /= num;
				expect("eol");
				lpos++;
			}
			else
				error("operator error");
		}
		// jump
		else if (cmd == "goto") {
			expect("identifier");
			auto& label = last();
			jumpto(label);
		}
		// conditional
		else if (cmd == "if") {
			expect("identifier");
			auto& name = last();
			if (getmem(name).type != Memory::NUM)
				error("type error");
			expect("match", "then");
			expect("match", "goto");
			expect("identifier");
			auto& label = last();
			if (getmem(name).num)
				jumpto(label);
			else
				lpos++;
		}
		// print variable to console
		else if (cmd == "print") {
			for (size_t i = 1; i < tokens().size(); i++) {
				auto& tok = tokens().at(i);
				if (isnumber(tok))
					printf("%d ", stoi(tok));
				else if (isstrliteral(tok))
					printf("%s ", stripliteral(tok).c_str());
				else if (isidentifier(tok)) {
					auto& var = getmem(tok);
					switch (var.type) {
						case Memory::NUM:     printf("%d ", var.num);  break;
						case Memory::STRING:  printf("%s ", var.str.c_str());  break;
					}
				}
				else
					error("bad argument");
			}
			printf("\n");
			lpos++;
		}
		// label
		else if (isidentifier(cmd) && accept("match", ":"))
			lpos++;  // also noop
		// unknown
		else
			error("unknown command [" + cmd + "]");

		// OK
		return 0;
	}

	int error(const string& msg, int pos = -1) {
		throw runtime_error(msg + " (line " + to_string(lpos + 1) + (pos > -1 ? ", " + to_string(pos) : "") + ")");
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
	const Tokenizer::TokenLine& line() {
		if (lpos >= tok.lines.size())
			error("line out-of-range");
		return tok.lines[lpos];
	}
	const vector<string>& tokens() {
		return line().tokens;
	}
	int expect(const string& cmd, const string& match = "") {
		if (!accept(cmd, match))
			error("expected " + cmd);
		return 1;
	}
	int accept(const string& cmd, const string& match = "") {
		if (pos >= tokens().size()) {
			if (cmd == "eol")
				return 1;
			error("token out-of-range");
		}
		auto& tok = tokens().at(pos);
		if (cmd == "identifier")
			return isidentifier(tok) && ++pos;
		else if (cmd == "match")
			return tok == match && ++pos;
		else if (cmd == "number")
			return isnumber(tok) && ++pos;
		else if (cmd == "string")
			return isstrliteral(tok) && ++pos;
		else
			error("unknown accept command [" + cmd + "]");
		return 0;
	}
	const string& last() {
		static const string EOL = "<eol>";
		if (pos - 1 >= tokens().size())
			return EOL;
		return tokens().at(pos - 1);
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
};


int main() {
	printf("funbasic parser online!\n\n");

	Tokenizer tok;
	tok.parsef("test.asm");
	tok.show();
	printf("\n");

	Runtime run;
	run.tok = tok;
	run.run();
}