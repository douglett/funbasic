#pragma once
#include "tokenhelpers.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
using namespace std;


struct AsmRuntime : TokenHelpers {
	// parsing
	Tokenizer tok;
	vector<string> lastlist;
	size_t lpos = 0, pos = 0;
	int flagw_modulename = 0;
	// runtime
	string modulename;
	struct Memory {
		typedef shared_ptr<Memory> Memptr;
		enum MEMTYPE { NUM = 0, STR, ARR };
		MEMTYPE type; int num; string str; vector<int> arr;
	};
	typedef Memory::Memptr Memptr;
	vector<Memptr> stack;
	struct Frame {
		map<string, Memptr> variables;
		size_t returnpos;
	};
	Frame global;
	vector<Frame> framestack;

	// --- Parsing ---
	int run() {
		lpos = 0;
		while (lpos < tok.lines.size())
			runline();
		return 0;
	}

	int error(const string& msg) const {
		throw runtime_error(msg + " (line " + to_string(lpos + 1) + ", " + to_string(pos + 1) + ")");
	}
	int warn(const string& msg) const {
		return printf("Warning: %s (line %d, %d)\n", msg.c_str(), int(lpos + 1), int(pos + 1)), 0;
	}

	int runline() {
		string cmd = tokens().size() ? tokens()[0] : "noop";
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
			expect("$identifier $eol");
			modulename = last(0);
			lpos++;
		}
		// stack integer
		else if (cmd == "int") {
			expect("$number $eol");
			int i = strtoint(last(0));
			pushst(makeint(i));
			lpos++;
		}
		// stack string
		else if (cmd == "str") {
			expect("$string $eol");
			string s = stripliteral(last(0));
			pushst(makestr(s));
			lpos++;
		}
		// stack duplicate top
		else if (cmd == "dup") {
			expect("$eol");
			pushst(topst());
			lpos++;
		}
		// integer maths
		else if (cmd == "add" || cmd == "sub" || cmd == "mul" || cmd == "div") {
			expect("$eol");
			int a = getint(popst());
			int b = getint(popst());
			int i = 0;
			if      (cmd == "add")  i = b + a;
			else if (cmd == "sub")  i = b - a;
			else if (cmd == "mul")  i = b * a;
			else if (cmd == "div")  i = b / a;
			pushst(makeint(i));
			lpos++;
		}
		// print commands
		else if (cmd == "print" || cmd == "println") {
			expect("$eol");
			if      (cmd == "print")   printf("%s ",  memtostr(topst()).c_str());
			else if (cmd == "println") printf("%s\n", memtostr(topst()).c_str());
			lpos++;
		}
		// DIMension - create variable
		else if (cmd == "dim") {
			int local = accept("$");
			expect("$identifier");
			auto name = last();
			auto& frame = getframe(local);
			if (frame.variables.count(name))
				error("variable redefinition");
			if      (accept("$number")) frame.variables[name] = makeint(strtoint(last()));
			else if (accept("$string")) frame.variables[name] = makestr(stripliteral(last()));
			else    error("expected default");
			expect("$eol");
			lpos++;
		}
		// get variable
		else if (cmd == "get") {
			int local = accept("$");
			expect("$identifier $eol");
			auto name = last(0);
			auto& frame = getframe(local);
			if (!frame.variables.count(name))
				error("missing variable");
			pushst(frame.variables.at(name));
			lpos++;
		}
		else if (cmd == "set") {
			int local = accept("$");
			expect("$identifier $eol");
			auto name = last(0);
			auto& frame = getframe(local);
			if (!frame.variables.count(name))
				error("missing variable");
			if (frame.variables.at(name)->type != topst()->type)
				error("type mismatch");
			frame.variables.at(name) = popst();
			lpos++;
		}
		// unknown
		else
			error("unknown command [" + cmd + "]");

		// OK
		return 0;
	}

	// --- Runtime State ---
	static Memptr makeint(int num) {
		auto p = make_shared<Memory>();
		*p = { Memory::NUM, num };
		return p;
	}
	static Memptr makestr(const string& str) {
		auto p = make_shared<Memory>();
		*p = { Memory::STR, 0, str };
		return p;
	}
	int getint(Memptr p) {
		if (p->type != Memory::NUM)
			error("expected int");
		return p->num;
	}
	string& getstr(Memptr p) {
		if (p->type != Memory::STR)
			error("expected string");
		return p->str;
	}
	Memptr pushst(Memptr p) {
		stack.push_back(p);
		return p;
	}
	Memptr popst() {
		if (stack.size() == 0)
			error("pop from empty stack");
		auto p = stack.back();
		stack.pop_back();
		return p;
	}
	Memptr topst() {
		if (stack.size() == 0)
			error("top from empty stack");
		return stack.back();
	}
	Frame& getframe(int local = 0) {
		if (!local)
			return global;
		if (framestack.size() == 0)
			error("framestack is empty");
		return framestack.back();
	}
	int jumpto(const string& label) {
		for (size_t i = 0; i < tok.lines.size(); i++) {
			auto& tokens = tok.lines[i].tokens;
			if (tokens.size() == 2 && tokens.at(0) == label && tokens.at(1) == ":")
				return lpos = i;
		}
		return error("goto unknown label: [" + label + "]");
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
	string memtostr(Memptr p) {
		switch (p->type) {
			case Memory::NUM:  return to_string(p->num);
			case Memory::STR:  return p->str;
			case Memory::ARR:  return "array(" + to_string(p->arr.size()) + ")";
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
