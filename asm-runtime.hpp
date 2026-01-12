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
		enum MEMTYPE { NUM = 0, STR, ARR, TBL };
		MEMTYPE type; int num; string str; vector<Memptr> arr; map<string, Memptr> tbl;
	};
	typedef Memory::Memptr Memptr;
	vector<Memptr> stack;
	struct Frame {
		size_t returnpos;
		map<string, Memptr> variables;
	};
	Frame global;
	vector<Frame> framestack;

	// --- Parsing ---
	int run() {
		lpos = 0;
		while (lpos < tok.lines.size())
			if (runline())
				break;
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
		}
		// module name
		else if (cmd == "module") {
			if (modulename.length())
				error("module name redefined");
			expect("$identifier $eol");
			modulename = last(0);
		}
		// stack integer
		else if (cmd == "int") {
			expect("$number $eol");
			int i = strtoint(last(0));
			pushst(makeint(i));
		}
		// stack string
		else if (cmd == "str") {
			expect("$string $eol");
			string s = stripliteral(last(0));
			pushst(makestr(s));
		}
		// stack duplicate top
		else if (cmd == "dup") {
			expect("$eol");
			pushst(topst());
		}
		// drop top from stack
		else if (cmd == "drop") {
			expect("$eol");
			popst();
		}
		// integer maths
		else if (cmd == "add" || cmd == "sub" || cmd == "mul" || cmd == "div") {
			expect("$eol");
			int a = popst(Memory::NUM)->num;
			int b = popst(Memory::NUM)->num;
			int i = 0;
			if      (cmd == "add")  i = b + a;
			else if (cmd == "sub")  i = b - a;
			else if (cmd == "mul")  i = b * a;
			else if (cmd == "div" && a == 0)  error("division by zero");
			else if (cmd == "div")  i = b / a;
			pushst(makeint(i));
		}
		// print commands
		else if (cmd == "print" || cmd == "println") {
			expect("$eol");
			if      (cmd == "print")   printf("%s ",  memtostr(topst()).c_str());
			else if (cmd == "println") printf("%s\n", memtostr(topst()).c_str());
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
			else if (accept("[ ]"))     frame.variables[name] = makearr();
			else if (accept("{ }"))     frame.variables[name] = maketbl();
			else    error("expected default");
			expect("$eol");
		}
		// get variable
		else if (cmd == "get") {
			accept("$ $identifier") || expect("$identifier");
			auto name = last();
			expect("$eol");
			pushst(getvar(name));
		}
		// set variable
		else if (cmd == "set") {
			accept("$ $identifier") || expect("$identifier");
			auto name = last();
			expect("$eol");
			auto& p = getvar(name);
			p = popst(p->type);
		}
		// jump to position
		else if (cmd == "jmp") {
			expect("$identifier $eol");
			lpos = labelpos(last(0));
		}
		// call function - push stack frame & jump
		else if (cmd == "call") {
			expect("$identifier $eol");
			framestack.push_back({ lpos });
			lpos = labelpos(last(0));
		}
		// return function - drop stack frame & jump
		else if (cmd == "ret") {
			expect("$eol");
			lpos = getframe(1).returnpos;
			framestack.pop_back();
		}
		// push to array
		else if (cmd == "push") {
			expect("$eol");
			auto valp = popst();
			auto arrp = popst(Memory::ARR);
			if (arrp->arr.size() > 0 && arrp->arr.at(0)->type != valp->type)
				error("type mismatch in array");
			arrp->arr.push_back(valp);
		}
		// pop from array to stack top
		else if (cmd == "pop") {
			expect("$eol");
			auto arrp = popst(Memory::ARR);
			if (arrp->arr.size() == 0)
				error("pop from empty array");
			pushst(arrp->arr.back());
			arrp->arr.pop_back();
		}
		// array index
		else if (cmd == "indx") {
			expect("$eol");
			auto idxp = popst(Memory::NUM);
			auto arrp = popst(Memory::ARR);
			if (idxp->num < 0 || (size_t)idxp->num >= arrp->arr.size())
				error("index out of range");
			pushst(arrp->arr.at(idxp->num));
		}
		// table member
		else if (cmd == "memb") {
			if (accept("set $identifier $eol")) {
				auto name = last(1);
				auto valp = popst();
				auto tblp = popst(Memory::TBL);
				tblp->tbl[name] = valp;
			}
			else {
				expect("$identifier $eol");
				auto name = last(0);
				auto tblp = popst(Memory::TBL);
				if (!tblp->tbl.count(name))
					error("table member not found");
				pushst(tblp->tbl.at(name));
			}
		}
		// yield to sheduler (rest)
		else if (cmd == "yield") {
			expect("$eol");
			return lpos++, 1;
		}
		// label
		else if (isidentifier(cmd) && accept(":")) {
			expect("$eol");
		}
		// unknown
		else
			error("unknown command [" + cmd + "]");

		// OK
		lpos++;
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
	static Memptr makearr() {
		auto p = make_shared<Memory>();
		*p = { Memory::ARR };
		return p;
	}
	static Memptr maketbl() {
		auto p = make_shared<Memory>();
		*p = { Memory::TBL };
		return p;
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
	Memptr popst(Memory::MEMTYPE type) {
		auto p = popst();
		if (p->type != type)
			error("type mismatch");
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
	Memptr& getvar(string name) {
		int local = 0;
		if (name.length() && name.at(0) == '$')
			name = name.substr(1), local = 1;
		auto& frame = getframe(local);
		if (!frame.variables.count(name))
			error("missing variable");
		return frame.variables.at(name);
	}
	int labelpos(const string& label) {
		for (size_t i = 0; i < tok.lines.size(); i++) {
			auto& tokens = tok.lines[i].tokens;
			if (tokens.size() == 2 && tokens.at(0) == label && tokens.at(1) == ":")
				return i;
		}
		return error("unknown label: [" + label + "]");
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
			case Memory::TBL:  return "table(" + to_string(p->tbl.size()) + ")";
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
