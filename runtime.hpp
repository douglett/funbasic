#pragma once
#include "tokenhelpers.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;


struct Runtime : TokenHelpers {
	struct Memory {
		enum MEMTYPE { NUM = 0, STR, ARR };
		MEMTYPE type; int num; string str; vector<int> arr;
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
			// variable
			expect("$identifier");
			auto name = last();
			if (memory.count(name))
				error("redim of " + name);
			// operator
			expect("=");
			// argument
			auto arg = pargument();
			memory[name] = arg;
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
			string op = poperator();
			// argument
			Memory arg = pargument();
			if (var.type != arg.type)
				error("type error");
			// operation
			switch (var.type) {
				case Memory::NUM:
					if      (op ==  "=")  var.num  = arg.num;
					// maths
					else if (op == "+=")  var.num += arg.num;
					else if (op == "-=")  var.num -= arg.num;
					else if (op == "*=")  var.num *= arg.num;
					else if (op == "/=")  var.num /= arg.num;
					// comparison
					else if (op == "==")  var.num = var.num == arg.num;
					else if (op == "!=")  var.num = var.num != arg.num;
					else    error("operator invalid on number");
					break;
				case Memory::STR:
					if   (op == "=")  var.str = arg.str;
					else error("operator invalid on string");
					break;
				case Memory::ARR:
					error("operator invalid on array");
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
		else if (cmd == "if" || cmd == "ifn") {
			expect("$identifier");
			auto name = last();
			if (getmem(name).type != Memory::NUM)
				error("type error");
			expect("then");
			expect("goto");
			expect("$identifier");
			auto label = last();
			expect("$eol");
			if (cmd == "if" && getmem(name).num)
				jumpto(label);
			else if (cmd == "ifn" && !getmem(name).num)
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
					printf("%s ", memtostr(var).c_str());
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
			auto& var = getmem(last());
			if (var.type != Memory::STR)
				error("type error");
			getline(cin, var.str);
			expect("$eol");
			lpos++;
		}
		// special array commands
		else if (cmd == "push") {
			expect("$identifier");
			auto& var = getmem(last());
			if (var.type != Memory::ARR)
				error("type error");
			accept(",");
			auto arg = pargument();
			if (arg.type == Memory::NUM)
				var.arr.push_back(arg.num);
			else if (arg.type == Memory::ARR)
				var.arr.insert(var.arr.end(), arg.arr.begin(), arg.arr.end());
			else
				error("type error");
			expect("$eol");
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

	string poperator() {
		vector<string> operators = { "= =", "! =", "+ =", "- =", "* =", "/ =", "=" };
		for (auto op : operators)
			if (accept(op))
				return op;
		error("expected operator");
		return "";
	}

	Memory pargument() {
		Memory arg;
		if (accept("$number"))
			arg = { Memory::NUM, strtoint(last()) };
		else if (accept("$string"))
			arg = { Memory::STR, 0, stripliteral(last()) };
		else if (accept("$identifier (")) {
			auto conversion = last(0);
			expect("$identifier");
			auto var2 = getmem(last());
			expect(")");
			if (conversion == "toint" && var2.type == Memory::STR)
				arg = { Memory::NUM, strtoint(var2.str, 0) };
			else
				error("bad conversion"); 
		}
		else if (accept("$identifier"))
			arg = getmem(last());
		else if (accept("[")) {
			arg = { Memory::ARR, 0, "", {} };
			while (!accept("]")) {
				if (accept("$eol"))
					error("unterminated array");
				Memory arg2 = pargument();
				if (arg2.type != Memory::NUM)
					error("type error");
				accept(",");  // commas optional
				arg.arr.push_back(arg2.num);
			}
		}
		else
			error("expected argument");
		return arg;
	}

	int error(const string& msg) const {
		throw runtime_error(msg + " (line " + to_string(lpos + 1) + ", " + to_string(pos + 1) + ")");
	}
	int warn(const string& msg) const {
		return printf("Warning: %s (line %d, %d)\n", msg.c_str(), int(lpos + 1), int(pos + 1)), 0;
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
	int strtoint(const string& str, int defaultnum) {
		try { return stoi(str); }
		catch (invalid_argument& e) { return defaultnum; }
	}
	string memtostr(const Memory& mem) {
		switch (mem.type) {
			case Memory::NUM:  return to_string(mem.num);
			case Memory::STR:  return mem.str;
			case Memory::ARR:  return "array(" + to_string(mem.arr.size()) + ")";
		}
		return error("memtostr, unknown type"), "";
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
			if (!matchnext(cm))
				return pos = p, 0;
			pos++;
			lastlist.push_back(cm == "$eol" ? "<eol>" : tokens().at(pos - 1));
		}
		return 1;
	}
	int matchnext(const string& cmd) {
		if (pos >= tokens().size())
			return cmd == "$eol" ? 1 : 0;
		auto& tok = tokens().at(pos);
		if (cmd == "$identifier")
			return isidentifier(tok);
		else if (cmd == "$number")
			return isnumber(tok);
		else if (cmd == "$string")
			return isstrliteral(tok);
		else if (cmd == tok)
			return 1;
		return 0;
	}
	string last() {
		return joinvs(lastlist, "");
	}
	string last(size_t i) {
		if (i >= lastlist.size())
			error("last out-of-range");
		return lastlist.at(i);
	}
};
