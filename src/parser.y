%language "c++"
%skeleton "lalr1.cc"

%define parser_class_name { cython_parser }
%define api.token.constructor
%define api.value.type variant
%define parse.error verbose

%locations

%code requires
{
#include <lexical_error.h>
#include <string>
}

%code
{
extern FILE* yyin;
extern yy::cython_parser::symbol_type yylex();
extern void yypop_buffer_state();
}

/* terminal symbols */
%token <bool> BOOL "boolean"
%token <std::string> IDENTIFIER "identifier"
%token <double> FLOAT_L "float literal"
%token <int> INT_L "integer literal"
%token <std::string> STRING_L "string literal"

%token NL "new line"
%token EOF_T 0 "end of file"

%token DEF "def"
%token IF "if"
%token ELIF "elif"
%token ELSE "else"
%token FOR "for"
%token WHILE "while"
%token DO "do"
%token BEGIN_T "begin"
%token END_T "end"
%token RETURN "return"

%token INT "int"
%token FLOAT "float"
%token CHAR "char"
%token VOID "void"

%token COLON ":"
%token SEMICOLON ";"
%token COMMA ","
%token ARROW "->"
%token AMPERSEND "&"

%token PLUS "+"
%token MINUS "-"
%token TIMES "*"
%token DIV "/"
%token EXP "**"

%token AND "and"
%token OR "or"
%token NOT "not"

%token LT "<"
%token GT ">"
%token LE "<="
%token GE ">="
%token EQ "=="
%token NE "!="

%token LPAREN "("
%token RPAREN ")"
%token LBRACKET "["
%token RBRACKET "]"

%token ASSIGN "="

/* TODO non-terminal symbols */

/* TODO precedence */

%%

program
	: program declaration nl
	| program func_declaration
	| declaration nl
	| func_declaration
	| %empty
	;

declaration
	: IDENTIFIER COLON type
	| IDENTIFIER COLON type ASSIGN expression
	;

func_declaration
	: DEF IDENTIFIER LPAREN args_list RPAREN ARROW type block nl
	| DEF IDENTIFIER LPAREN RPAREN ARROW type block nl
	;

block
	: BEGIN_T inner_block END_T
	;

inner_block
	: inner_block line
	| nl line
	;

line
	: declaration nl
	| statement nl
	| expression nl
	;

statement
	: if_stmt
	| for_stmt
	| while_stmt
	| return_stmt
	;

expression
	: expression PLUS expression
	| expression MINUS expression
	| expression TIMES expression
	| expression DIV expression
	| expression EXP expression
	| expression AND expression
	| expression OR expression
	| NOT expression
	| MINUS expression
	| expression GT expression
	| expression LT expression
	| expression GE expression
	| expression LE expression
	| expression EQ expression
	| expression NE expression
	| LPAREN expression RPAREN
	| assignment
	| atom_expr
	;

atom_expr
	: name
	| func_call
	| INT_L
	| FLOAT_L
	| STRING_L
	| BOOL
	;

assignment
	: name ASSIGN expression
	;

func_call
	: IDENTIFIER LPAREN parameters RPAREN
	| IDENTIFIER LPAREN RPAREN
	;

parameters
	: parameters COMMA expression
	| expression
	;

if_stmt
	: IF expression DO inner_block END_T
	| IF expression DO inner_block elif END_T
	| IF expression DO inner_block else END_T
	| IF expression DO inner_block elif else END_T
	;

elif
	: elif ELIF expression inner_block
	| ELIF expression inner_block
	;

else
	: ELSE inner_block
	;

for_stmt
	: FOR declaration SEMICOLON expression SEMICOLON expression block
	| FOR expression SEMICOLON expression SEMICOLON expression block
	;

while_stmt
	: WHILE expression block
	;

return_stmt
	: RETURN expression
	;

args_list
	: args_list COMMA arg
	| arg
	;

arg
	: IDENTIFIER COLON type
	| IDENTIFIER AMPERSEND COLON type
	;

type
	: type LBRACKET RBRACKET
	| type LBRACKET INT_L RBRACKET
	| INT
	| FLOAT
	| CHAR
	| VOID
	;

name
	: name LBRACKET expression RBRACKET
	| IDENTIFIER
	;

nl
	: nl NL
	| NL
	;

%%

void show_error(const yy::location& l, const std::string &m) {
	std::cerr << "[Error at " << l << "] " << m << std::endl;
}

void yy::cython_parser::error(const location_type& l, const std::string &m) {
	show_error(l, m);
}

int main(int argc, char** argv) {
	if (argc > 1)
		yyin = std::fopen(argv[1], "r");

	try {
		yy::cython_parser p;
		p.parse();
	} catch (const lexical_error& e) {
		yypop_buffer_state();  // cleans scanner memory
		show_error(e.location(), e.what());
	}

	if (yyin != stdin)
		std::fclose(yyin);
}
