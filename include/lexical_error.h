#ifndef LEXICAL_ERROR_H
#define LEXICAL_ERROR_H

#include <stdexcept>
#include <string>
#include <location.hh>

class lexical_error : public std::exception {
public:
	lexical_error(const yy::location& location, const std::string& symbol)
		: _location{location}, message{build_message(symbol)} {}

	const char* what() const noexcept { return message.c_str(); }

	const yy::location& location() const { return _location; }

private:
	static std::string build_message(const std::string& symbol) {
		return "lexical error, unknown symbol " + symbol;
	}

	const yy::location _location;
	const std::string message;
};

#endif
