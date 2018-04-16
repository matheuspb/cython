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
#include <ast.h>
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

/* non-terminal symbols */
%type <ast::block> inner_block block
%type <ast::node*> line declaration statement expression

/* precedence */
%right ASSIGN
%left OR
%left AND
%left NOT
%left GT LT GE LE EQ NE
%left PLUS MINUS
%left TIMES DIV
%right EXP
%left UMINUS

%%

program
	: program_
	| %empty
	;

program_
	: program_ declaration nl
	| program_ func_declaration
	| declaration nl
	| func_declaration
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
	: BEGIN_T inner_block END_T { $$ = $2; }
	;

inner_block
	: inner_block line { $1.add_line($2); $$ = $1; }
	| nl line { $$ = ast::block($2); }
	;

line
	: declaration nl { $$ = $1; }
	| statement nl { $$ = $1; }
	| expression nl { $$ = $1; }
	;

statement
	: if_stmt
	| for_stmt
	| while_stmt
	| return_stmt
	;

expression
	: expression PLUS expression {
		$$ = new ast::binary_operation(ast::plus, $1, $3);
	}
	| expression MINUS expression {
		$$ = new ast::binary_operation(ast::minus, $1, $3);
	}
	| expression TIMES expression {
		$$ = new ast::binary_operation(ast::times, $1, $3);
	}
	| expression DIV expression {
		$$ = new ast::binary_operation(ast::div, $1, $3);
	}
	| expression EXP expression {
		$$ = new ast::binary_operation(ast::exp, $1, $3);
	}
	| expression AND expression {
		$$ = new ast::binary_operation(ast::_and, $1, $3);
	}
	| expression OR expression {
		$$ = new ast::binary_operation(ast::_or, $1, $3);
	}
	| NOT expression { $$ = new ast::unary_operation(ast::_not, $2); }
	| MINUS expression %prec UMINUS {
		$$ = new ast::unary_operation(ast::uminus, $2);
	}
	| expression GT expression {
		$$ = new ast::binary_operation(ast::gt, $1, $3);
	}
	| expression LT expression {
		$$ = new ast::binary_operation(ast::lt, $1, $3);
	}
	| expression GE expression {
		$$ = new ast::binary_operation(ast::ge, $1, $3);
	}
	| expression LE expression {
		$$ = new ast::binary_operation(ast::le, $1, $3);
	}
	| expression EQ expression {
		$$ = new ast::binary_operation(ast::eq, $1, $3);
	}
	| expression NE expression {
		$$ = new ast::binary_operation(ast::ne, $1, $3);
	}
	| LPAREN expression RPAREN { $$ = $2; }
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
