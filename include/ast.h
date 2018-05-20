#ifndef AST_H
#define AST_H

#include <list>
#include <location.hh>
#include <string>

namespace ast {

enum operation {
	plus,
	minus,
	times,
	div,
	exp,
	_and,
	_or,
	gt,
	lt,
	ge,
	le,
	eq,
	ne,
	_not,
	uminus
};

class node {
public:
	node() = default;
	virtual ~node() = default;

	/* do semantic verifications on function calls */
	virtual void verify_function_calls() const = 0;
};

class block : public node {
public:
	block() = default;
	explicit block(node* line) : node{}, lines{line} {}

	void add_line(node* line) { lines.push_back(line); }

	void verify_function_calls() const {
		for (auto line: lines)
			line->verify_function_calls();
	}

private:
	std::list<node*> lines;
};

class binary_operation : public node {
public:
	binary_operation(operation op, node* left, node* right)
		: node{}, op{op}, left{left}, right{right} {}

	void verify_function_calls() const {
		left->verify_function_calls();
		right->verify_function_calls();
	}

private:
	operation op;
	node* left;
	node* right;
};

class unary_operation : public node {
public:
	unary_operation(operation op, node* operand)
		: node{}, op{op}, operand{operand} {}

	void verify_function_calls() const { operand->verify_function_calls(); }

private:
	operation op;
	node* operand;
};

class name : public node {
public:
	name() = default;
	explicit name(std::string identifier) : node{}, _identifier{identifier} {}

	void add_offset(node* offset) { offsets.push_back(offset); }

	std::string identifier() const { return _identifier; }

	void verify_function_calls() const {
		for (auto offset: offsets)
			offset->verify_function_calls();
	}

private:
	std::string _identifier;
	std::list<node*> offsets;
};

class assignment : public node {
public:
	assignment() = default;
	assignment(name variable, node* expression)
		: node{}, variable{variable}, expression{expression} {}

	void verify_function_calls() const { expression->verify_function_calls(); }

private:
	name variable;
	node* expression;
};

class elif_stmt : public node {
public:
	elif_stmt(node* cond, block elif_block)
		: node{}, cond{cond}, elif_block{elif_block} {}

	void verify_function_calls() const {
		cond->verify_function_calls();
		elif_block.verify_function_calls();
	}

private:
	node* cond;
	block elif_block;
};

class if_stmt : public node {
public:
	if_stmt() = default;
	if_stmt(
		node* cond, block if_block, std::list<elif_stmt> elif_stmts,
		block else_block)
		: node{}
		, cond{cond}
		, if_block{if_block}
		, elif_stmts{elif_stmts}
		, else_block{else_block} {}

	void verify_function_calls() const {
		cond->verify_function_calls();
		if_block.verify_function_calls();
		for (auto elif: elif_stmts)
			elif.verify_function_calls();
		else_block.verify_function_calls();
	}

private:
	node* cond;
	block if_block;
	std::list<elif_stmt> elif_stmts;
	block else_block;
};

class for_stmt : public node {
public:
	for_stmt() = default;
	for_stmt(node* init, node* condition, node* step, block code)
		: node{}, init{init}, condition{condition}, step{step}, code{code} {}

	void verify_function_calls() const {
		init->verify_function_calls();
		condition->verify_function_calls();
		step->verify_function_calls();
		code.verify_function_calls();
	}

private:
	node* init;
	node* condition;
	node* step;
	block code;
};

class while_stmt : public node {
public:
	while_stmt() = default;
	while_stmt(node* condition, block code)
		: condition{condition}, code{code} {}

	void verify_function_calls() const {
		condition->verify_function_calls();
		code.verify_function_calls();
	}

private:
	node* condition;
	block code;
};

class return_stmt : public node {
public:
	return_stmt() = default;
	return_stmt(node* expression) : expression{expression} {}

	void verify_function_calls() const { expression->verify_function_calls(); }

private:
	node* expression;
};

class int_l : public node {
public:
	explicit int_l(int value) : node{}, value{value} {}

	void verify_function_calls() const {}

private:
	int value;
};

class float_l : public node {
public:
	explicit float_l(double value) : node{}, value{value} {}

	void verify_function_calls() const {}

private:
	double value;
};

class string_l : public node {
public:
	explicit string_l(std::string str) : node{}, str{str} {}

	void verify_function_calls() const {}

private:
	std::string str;
};

class bool_l : public node {
public:
	explicit bool_l(bool b) : node{}, b{b} {}

	void verify_function_calls() const {}

private:
	bool b;
};

class type : public node {
public:
	enum _type { _int, _float, _char, _void };

	type() = default;
	explicit type(_type t) : node{}, _t{t} {}

	void add_dimension(unsigned int size) { dimensions.push_back(size); }

	_type t() const { return _t; }

	void verify_function_calls() const {}

private:
	_type _t{_void};
	std::list<unsigned int> dimensions;
};

class arg : public node {
public:
	arg() = default;
	arg(std::string identifier, type t, bool reference)
		: node{}, identifier{identifier}, t{t}, reference{reference} {}

	void verify_function_calls() const {}

private:
	std::string identifier;
	type t;
	bool reference{false};
};

class declaration : public node {
public:
	declaration(std::string name, type t, node* expression)
		: node{}, name{name}, t{t}, expression{expression} {}

	void verify_function_calls() const { expression->verify_function_calls(); }

private:
	std::string name;
	type t;
	node* expression;
};

class func : public node {
public:
	func(std::string name, std::list<arg> args, type t, block code)
		: node{}, name{name}, args{args}, t{t}, code{code} {}
	func(std::string name, type t, block code)
		: node{}, name{name}, t{t}, code{code} {}

	void verify_function_calls() const { code.verify_function_calls(); }

private:
	std::string name;
	std::list<arg> args;
	type t;
	block code;
};

class func_call : public node {
public:
	func_call(
		std::string name, std::list<node*> parameters, yy::location location)
		: node{}, name{name}, parameters{parameters}, location{location} {}
	func_call(std::string name, yy::location location)
		: node{}, name{name}, location{location} {}

	const std::string& func_name() const { return name; }

	/* check if the called function exists in the symbol table */
	void verify_function_calls() const;

private:
	std::string name;
	std::list<node*> parameters;
	yy::location location;
};

}  // namespace ast

#endif
