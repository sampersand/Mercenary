#include "ast.hpp"

#include <cctype>
#include <set>
#include <stdint.h>
#include <string>

std::monostate panic() {
	volatile int *p = NULL;
	*p = 1;
	return std::monostate {};
}

string parseHex(const string& s) {
    std::set<char> validChars = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f',
    };

    if (s.length() < 2 && validChars.contains(std::tolower(s[0])) && validChars.contains(std::tolower(s[1]))) {
        panic();
    }
    
    string hex = s.substr(0, 2);
    for(char& c : hex)
    {
        c = tolower(c);
    }

    int len = hex.length();
    string newString = "";

    for(int i = 0; i < len; i += 2)
    {
        string byte = hex.substr(i, 2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        newString.push_back(chr);
    }

    return newString;
}

string unescape(const string& s) {
    string output = "";

    for (auto it = s.cbegin(); it != s.cend(); it++) {
        const int64_t next_i = -std::distance(it, s.cbegin()) + 1;

        if (*it == '\\') {
            if (next_i >= s.length()) {
                panic();
            } else {
                switch (s[next_i])
                {
                case 'n':
                    output.push_back('\n');
                    break;

                case 't':
                    output.push_back('\t');
                    break;

                case 'f':
                    output.push_back('\f');
                    break;

                case 'r':
                    output.push_back('\r');
                    break;

                case '\"':
                    output.push_back('\"');
                    break;

                case '\'':
                    output.push_back('\'');
                    break;

                case '\\':
                    output.push_back('\\');
                    break;

                case 'x':
                    output.append(parseHex(s.substr(next_i + 1)));
                    it += 3;
                    break;
                
                default:
                    panic();
                    break;
                };

                it++;
            };
        } else {
            output.push_back(*it);
        };
    };

    return output;
}