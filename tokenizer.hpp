#pragma once
#include "tokenhelpers.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;


struct Tokenizer : TokenHelpers {
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
			// printf("%02d  (%02d)  ", (int)i+1, (int)tokens.size());
			printf("%02d  ", (int)i+1);
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
};
