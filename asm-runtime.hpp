#pragma once
#include "tokenhelpers.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <string>
#include <vector>
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
	// struct Memory {
	// 	enum MEMTYPE { NUM = 0, STR, ARR };
	// 	MEMTYPE type; int num; string str; vector<int> arr;
	// };
	// map<string, Memory> memory;

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
		// unknown
		else
			error("unknown command [" + cmd + "]");

		// OK
		return 0;
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
	// string memtostr(const Memory& mem) {
	// 	switch (mem.type) {
	// 		case Memory::NUM:  return to_string(mem.num);
	// 		case Memory::STR:  return mem.str;
	// 		case Memory::ARR:  return "array(" + to_string(mem.arr.size()) + ")";
	// 	}
	// 	return error("memtostr, unknown type"), "";
	// }
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
