%{
#include <iostream>
#include <cstdlib>
#include <string>
#include <lexical_error.h>
#include <parser.h>

#define yyterminate() \
		yypop_buffer_state(); \
		return yy::cython_parser::make_EOF_T(yyloc);
#define YY_DECL yy::cython_parser::symbol_type yylex()
#define YY_USER_ACTION yyloc.step(); yyloc.columns(yyleng);

static yy::location yyloc;
%}

%option noyywrap noinput nounput nodefault

%%

[ \t]	yyloc.step(); /* ignore whitespace */
\n+	{ /* ignores multiple new lines */
	yyloc.lines(yyleng); yyloc.step(); return yy::cython_parser::make_NL(yyloc);
}

#[^\n]*\n { /* single line comment */
	yyloc.lines(1); yyloc.step(); return yy::cython_parser::make_NL(yyloc);
}
\"\"\"[^\"]*\"\"\"	/* multi-line comment */

def		return yy::cython_parser::make_DEF(yyloc);
if		return yy::cython_parser::make_IF(yyloc);
elif	return yy::cython_parser::make_ELIF(yyloc);
else	return yy::cython_parser::make_ELSE(yyloc);
for		return yy::cython_parser::make_FOR(yyloc);
while	return yy::cython_parser::make_WHILE(yyloc);
do		return yy::cython_parser::make_DO(yyloc);
begin	return yy::cython_parser::make_BEGIN_T(yyloc);
end		return yy::cython_parser::make_END_T(yyloc);
return	return yy::cython_parser::make_RETURN(yyloc);
True	return yy::cython_parser::make_BOOL(true, yyloc);
False	return yy::cython_parser::make_BOOL(false, yyloc);

int		return yy::cython_parser::make_INT(yyloc);
float	return yy::cython_parser::make_FLOAT(yyloc);
char	return yy::cython_parser::make_CHAR(yyloc);
void	return yy::cython_parser::make_VOID(yyloc);

":"		return yy::cython_parser::make_COLON(yyloc);
";"		return yy::cython_parser::make_SEMICOLON(yyloc);
","		return yy::cython_parser::make_COMMA(yyloc);
"->"	return yy::cython_parser::make_ARROW(yyloc);
"&"		return yy::cython_parser::make_AMPERSEND(yyloc);

"+"		return yy::cython_parser::make_PLUS(yyloc);
"-"		return yy::cython_parser::make_MINUS(yyloc);
"*"		return yy::cython_parser::make_TIMES(yyloc);
"/"		return yy::cython_parser::make_DIV(yyloc);
"**"	return yy::cython_parser::make_EXP(yyloc);

and		return yy::cython_parser::make_AND(yyloc);
or		return yy::cython_parser::make_OR(yyloc);
not		return yy::cython_parser::make_NOT(yyloc);

"<"		return yy::cython_parser::make_LT(yyloc);
">"		return yy::cython_parser::make_GT(yyloc);
"<="	return yy::cython_parser::make_LE(yyloc);
">="	return yy::cython_parser::make_GE(yyloc);
"=="	return yy::cython_parser::make_EQ(yyloc);
"!="	return yy::cython_parser::make_NE(yyloc);

"("		return yy::cython_parser::make_LPAREN(yyloc);
")"		return yy::cython_parser::make_RPAREN(yyloc);
"["		return yy::cython_parser::make_LBRACKET(yyloc);
"]"		return yy::cython_parser::make_RBRACKET(yyloc);

"="		return yy::cython_parser::make_ASSIGN(yyloc);

[a-zA-Z_][a-zA-Z_0-9]* {
	return yy::cython_parser::make_IDENTIFIER(std::string(yytext), yyloc);
}
[0-9]+	return yy::cython_parser::make_INT_L(std::atoi(yytext), yyloc);
[0-9]+(\.[0-9]+)? {
	return yy::cython_parser::make_FLOAT_L(std::atof(yytext), yyloc);
}
\"[^\"]*\"	return yy::cython_parser::make_STRING_L(std::string(yytext), yyloc);
<<EOF>>	yyterminate();
.	throw lexical_error(yyloc, yytext);

%%
