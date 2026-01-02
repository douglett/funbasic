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
		MEMTYPE type; int i; string s;
	};
	Tokenizer tok;
	size_t lpos = 0;
	map<string, Memory> memory;

	int run() {
		int err = 0;
		lpos = 0;
		while (lpos < tok.lines.size()) {
			err = runline();
			if (err != 0)
				break;
		}
		return err;
	}

	int runline() {
		string cmd = tokens().size() ? tokens()[0] : "noop";
		string errormsg;

		// parse line
		if (cmd == "noop")
			lpos++;
		else if (cmd == "dim") {
			expect("identifier", 1);
			expect("match", 2, "=");
			expect("eol", 4);
			if (accept("number", 3))
				memory[tokens().at(1)] = { Memory::NUM, stoi(tokens().at(3)) };
			else if (accept("string", 3))
				memory[tokens().at(1)] = { Memory::STRING, 0, stripliteral(tokens().at(3)) };
			else
				throw runtime_error("type error");
			lpos++;
		}
		else
			errormsg = "unknown command [" + cmd + "]";
		
		// handle errors
		if (errormsg.length())
			fprintf(stderr, "Error: %s\n", errormsg.c_str()),
			fprintf(stderr, "	Line %d\n", (int)lpos + 1);
		return errormsg.length() ? 1 : 0;
	}

	const Tokenizer::TokenLine& line() {
		if (lpos >= tok.lines.size())
			throw runtime_error("line out-of-range. line " + to_string(lpos));
		return tok.lines[lpos];
	}
	const vector<string>& tokens() {
		return line().tokens;
	}

	int expect(const string& cmd, size_t pos, const string& match = "") {
		if (!accept(cmd, pos, match))
			throw runtime_error("expected " + cmd
				+ "(line " + to_string(lpos + 1) + ", pos " + to_string(pos) + ")");
		return 1;
	}
	int accept(const string& cmd, size_t pos, const string& match = "") {
		if (pos >= tokens().size()) {
			if (cmd == "eol")
				return 1;
			throw runtime_error("token out-of-range. line " + to_string(lpos) + ", pos " + to_string(pos));
		}
		auto& tok = tokens().at(pos);
		if (cmd == "identifier")
			return isidentifier(tok);
		else if (cmd == "match")
			return tok == match;
		else if (cmd == "number")
			return isnumber(tok);
		else if (cmd == "string")
			return isstrliteral(tok);
		else
			throw runtime_error("unknown accept command [" + cmd + "]");
		return 0;
	}

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
	printf("hello world\n");

	Tokenizer tok;
	tok.parsef("example.asm");
	tok.show();

	Runtime run;
	run.tok = tok;
	run.run();
}