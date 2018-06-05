%language "c++"
%skeleton "lalr1.cc"

%define parser_class_name { cython_parser }
%define api.token.constructor
%define api.value.type variant
%define parse.error verbose

%locations

%code requires
{
#include <string>
#include <vector>
#include <errors.h>
#include <ast.h>
#include <st.h>
}

%code
{
extern FILE* yyin;
extern yy::cython_parser::symbol_type yylex();
extern void yypop_buffer_state();

/* current symbol table being used during parsing */
st::symbol_table* current = new st::symbol_table;
std::vector<ast::node*> program;
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
%type <ast::block> inner_block block else
%type <ast::expr*> expression atom_expr assignment func_call
%type <ast::node*> line declaration func_declaration
%type <ast::node*> statement if_stmt for_stmt while_stmt return_stmt
%type <std::vector<ast::elif_stmt>> elif
%type <ast::name> name
%type <ast::arg> arg
%type <ast::type> type
%type <std::vector<ast::arg>> args_list
%type <std::vector<ast::expr*>> parameters

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
	| nl program_
	;

program_
	: program_ declaration nl { program.push_back($2); }
	| program_ func_declaration { program.push_back($2); }
	| declaration nl { program.push_back($1); }
	| func_declaration { program.push_back($1); }
	;

start_scope
	: %empty { current = new st::symbol_table(current); }
	;

end_scope
	: %empty { current = current->parent; }
	;

declaration
	: IDENTIFIER COLON type {
		$$ = new ast::declaration($1, $3, nullptr);
		if ($3.t() == ast::_void)
			throw semantic_error(@1, "cannot declare variable of type void");
		if (!current->insert_variable($1, $3))
			throw semantic_error(@1, "variable " + $1 + " already declared");
	}
	| IDENTIFIER COLON type ASSIGN expression {
		$$ = new ast::declaration($1, $3, $5);
		if ($3.t() == ast::_void)
			throw semantic_error(@1, "cannot declare variable of type void");
		if (!current->insert_variable($1, $3))
			throw semantic_error(@1, "variable " + $1 + " already declared");
		current->initialize_variable($1);
	}
	;

func_declaration
	: DEF IDENTIFIER start_scope LPAREN args_list RPAREN
			ARROW type block end_scope nl {
		auto node = new ast::func($2, $5, $8, $9);
		$$ = node;
		if (!current->insert_function($2, node))
			throw semantic_error(@1, "function " + $2 + " already defined");
	}
	| DEF IDENTIFIER start_scope LPAREN RPAREN ARROW type block end_scope nl {
		auto node = new ast::func($2, $7, $8);
		$$ = node;
		if (!current->insert_function($2, node))
			throw semantic_error(@1, "function " + $2 + " already defined");
	}
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
	: if_stmt { $$ = $1; }
	| for_stmt { $$ = $1; }
	| while_stmt { $$ = $1; }
	| return_stmt { $$ = $1; }
	;

expression
	: expression PLUS expression {
		$$ = new ast::binary_operation(ast::plus, $1, $3, @2);
	}
	| expression MINUS expression {
		$$ = new ast::binary_operation(ast::minus, $1, $3, @2);
	}
	| expression TIMES expression {
		$$ = new ast::binary_operation(ast::times, $1, $3, @2);
	}
	| expression DIV expression {
		$$ = new ast::binary_operation(ast::div, $1, $3, @2);
	}
	| expression EXP expression {
		$$ = new ast::binary_operation(ast::exp, $1, $3, @2);
	}
	| expression AND expression {
		$$ = new ast::binary_operation(ast::_and, $1, $3, @2);
	}
	| expression OR expression {
		$$ = new ast::binary_operation(ast::_or, $1, $3, @2);
	}
	| NOT expression {
		$$ = new ast::unary_operation(ast::_not, $2, @1);
	}
	| MINUS expression %prec UMINUS {
		$$ = new ast::unary_operation(ast::uminus, $2, @1);
	}
	| expression GT expression {
		$$ = new ast::binary_operation(ast::gt, $1, $3, @2);
	}
	| expression LT expression {
		$$ = new ast::binary_operation(ast::lt, $1, $3, @2);
	}
	| expression GE expression {
		$$ = new ast::binary_operation(ast::ge, $1, $3, @2);
	}
	| expression LE expression {
		$$ = new ast::binary_operation(ast::le, $1, $3, @2);
	}
	| expression EQ expression {
		$$ = new ast::binary_operation(ast::eq, $1, $3, @2);
	}
	| expression NE expression {
		$$ = new ast::binary_operation(ast::ne, $1, $3, @2);
	}
	| LPAREN expression RPAREN { $$ = $2; }
	| assignment { $$ = $1; }
	| atom_expr { $$ = $1; }
	;

atom_expr
	: name {
		if (!current->is_initialized($1.identifier()))
			throw semantic_error(@1,
				"use of uninitialized variable " + $1.identifier());
		$$ = new ast::name($1);
	}
	| func_call { $$ = $1; }
	| INT_L { $$ = new ast::int_l($1); }
	| FLOAT_L { $$ = new ast::float_l($1); }
	| STRING_L { $$ = new ast::string_l($1); }
	| BOOL { $$ = new ast::bool_l($1); }
	;

assignment
	: name ASSIGN expression {
		$$ = new ast::assignment($1, $3, $1.t());
		current->initialize_variable($1.identifier());
	}
	;

func_call
	: IDENTIFIER LPAREN parameters RPAREN {
		$$ = new ast::func_call($1, $3, @1);
	}
	| IDENTIFIER LPAREN RPAREN { $$ = new ast::func_call($1, @1); }
	;

parameters
	: parameters COMMA expression { $1.push_back($3); }
	| expression { $$ = {$1}; }
	;

if_stmt
	: IF expression DO inner_block END_T {
		$$ = new ast::if_stmt(
			$2, $4, std::vector<ast::elif_stmt>(), ast::block());
	}
	| IF expression DO inner_block elif END_T {
		$$ = new ast::if_stmt($2, $4, $5, ast::block());
	}
	| IF expression DO inner_block else END_T {
		$$ = new ast::if_stmt($2, $4, std::vector<ast::elif_stmt>(), $5);
	}
	| IF expression DO inner_block elif else END_T {
		$$ = new ast::if_stmt($2, $4, $5, $6);
	}
	;

elif
	: elif ELIF expression inner_block {
		$1.push_back(ast::elif_stmt($3, $4));
		$$ = $1;
	}
	| ELIF expression inner_block {
		$$ = {ast::elif_stmt($2, $3)};
	}
	;

else
	: ELSE inner_block { $$ = $2; }
	;

for_stmt
	: FOR declaration SEMICOLON expression SEMICOLON expression block {
		$$ = new ast::for_stmt($2, $4, $6, $7);
	}
	| FOR expression SEMICOLON expression SEMICOLON expression block {
		$$ = new ast::for_stmt($2, $4, $6, $7);
	}
	;

while_stmt
	: WHILE expression block {
		$$ = new ast::while_stmt($2, $3); }
	;

return_stmt
	: RETURN expression { $$ = new ast::return_stmt($2); }
	;

args_list
	: args_list COMMA arg { $1.push_back($3); $$ = $1; }
	| arg { $$ = {$1}; }
	;

arg
	: IDENTIFIER COLON type {
		current->insert_variable($1, $3);
		current->initialize_variable($1);
		$$ = ast::arg($1, $3, false);
	}
	| IDENTIFIER AMPERSEND COLON type {
		// TODO reference (&) arguments
		current->insert_variable($1, $4);
		current->initialize_variable($1);
		$$ = ast::arg($1, $4, true);
	}
	;

type
	: type LBRACKET RBRACKET { $1.add_dimension(0); $$ = $1; }
	| type LBRACKET INT_L RBRACKET { $1.add_dimension($3); $$ = $1; }
	| INT { $$ = ast::type(ast::_int); }
	| FLOAT { $$ = ast::type(ast::_float); }
	| CHAR { $$ = ast::type(ast::_char); }
	| VOID { $$ = ast::type(ast::_void); }
	;

name
	: name LBRACKET expression RBRACKET { $1.add_offset($3); $$ = $1; }
	| IDENTIFIER {
		if (!current->is_declared($1))
			throw semantic_error(@1, "use of undeclared variable " + $1);
		$$ = ast::name($1, current->get_type($1));
	}
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

		for (auto n: program)
			n->verify_semantic();
	} catch (const lexical_error& e) {
		yypop_buffer_state();  // cleans scanner memory
		show_error(e.location(), e.what());
	} catch (const semantic_error& e) {
		yypop_buffer_state();  // cleans scanner memory
		show_error(e.location(), e.what());
	}

	if (yyin != stdin)
		std::fclose(yyin);
}
