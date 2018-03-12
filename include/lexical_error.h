#ifndef LEXICAL_ERROR_H
#define LEXICAL_ERROR_H

#include <stdexcept>
#include <string>

class lexical_error : public std::exception {
public:
	lexical_error(int line, const std::string& symbol)
		: _line{line}, message{build_message(symbol)} {}

	const char* what() const noexcept { return message.c_str(); }

	int line() const { return _line; }

private:
	static std::string build_message(const std::string& symbol) {
		return "lexical error, unknown symbol " + symbol;
	}

	const int _line;
	const std::string message;
};

#endif
