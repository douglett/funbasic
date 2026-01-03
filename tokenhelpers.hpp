#pragma once
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
using namespace std;


struct TokenHelpers {
	// letters
	static int isalphac(char c) {
		return isalpha(c) || c == '_';
	}
	static int isalphanum(char c) {
		return isalnum(c) || c == '_';
	}

	// idendification
	static int isidentifier(const string& s) {
		if (s.length() == 0)  return 0;
		if (!isalphac(s[0])) return 0;
		for (auto c : s)
			if (!isalphanum(c))  return 0;
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

	// string tools
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
