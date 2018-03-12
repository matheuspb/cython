%{
#include <iostream>
#include <lexical_error.h>
%}

%option yylineno noyywrap noinput nounput nodefault

%%

[ \t]	/* ignore whitespace */
\n+	std::cout << "NL" << std::endl;  /* ignores multiple new lines */

#[^\n]*\n	/* single line comment */
\"\"\"[^\"]*\"\"\"\n?	/* multi-line comment */

def	std::cout << "DEF" << std::endl;
if	std::cout << "IF" << std::endl;
elif	std::cout << "ELIF" << std::endl;
else	std::cout << "ELSE" << std::endl;
for	std::cout << "FOR" << std::endl;
while	std::cout << "WHILE" << std::endl;
begin	std::cout << "BEGIN" << std::endl;
end	std::cout << "END" << std::endl;
return	std::cout << "RETURN" << std::endl;
True|False	std::cout << "BOOL" << std::endl;

int	std::cout << "INT" << std::endl;
float	std::cout << "FLOAT" << std::endl;
char	std::cout << "CHAR" << std::endl;
void	std::cout << "VOID" << std::endl;

":"	std::cout << "COLON" << std::endl;
";"	std::cout << "SEMICOLON" << std::endl;
","	std::cout << "COMMA" << std::endl;
"->"	std::cout << "ARROW" << std::endl;
"&"	std::cout << "AMPERSEND" << std::endl;

"+"	std::cout << "PLUS" << std::endl;
"-"	std::cout << "MINUS" << std::endl;
"*"	std::cout << "TIMES" << std::endl;
"/"	std::cout << "DIV" << std::endl;
"**"	std::cout << "EXP" << std::endl;

and	std::cout << "AND" << std::endl;
or	std::cout << "OR" << std::endl;
not	std::cout << "NOT" << std::endl;

"<"	std::cout << "LT" << std::endl;
">"	std::cout << "GT" << std::endl;
"<="	std::cout << "LE" << std::endl;
">="	std::cout << "GE" << std::endl;
"=="	std::cout << "EQ" << std::endl;
"!="	std::cout << "NE" << std::endl;

"("	std::cout << "LPAREN" << std::endl;
")"	std::cout << "RPAREN" << std::endl;
"["	std::cout << "LBRACKET" << std::endl;
"]"	std::cout << "RBRACKET" << std::endl;

"="	std::cout << "ASSIGN" << std::endl;

[a-zA-Z_][a-zA-Z_0-9]*	std::cout << "IDENTIFIER" << std::endl;
[0-9]+(\.[0-9]+)?	std::cout << "NUMBER_L" << std::endl;
\'[^\']?\'	std::cout << "CHAR_L" << std::endl;
\"[^\"]*\"	std::cout << "STRING_L" << std::endl;
<<EOF>>	yyterminate();
.	throw lexical_error(yylineno, yytext);

%%

int main(int argc, char** argv) {
	if (argc > 1)
		yyin = std::fopen(argv[1], "r");

	try {
		yylex();
	} catch (const lexical_error& e) {
		std::cout << "[Line " << e.line() << "] " << e.what() << std::endl;
	}
	yypop_buffer_state();  /* clean scanner memory */

	if (yyin != stdin)
		std::fclose(yyin);
}
