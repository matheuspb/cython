#ifndef AST_H
#define AST_H

#include <list>
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
};

class block : public node {
public:
	block() = default;
	explicit block(node* line) : node{}, lines{line} {}
	void add_line(node* line) { lines.push_back(line); }

private:
	std::list<node*> lines;
};

class type : public node {
public:
	enum _type { _int, _float, _char, _void, _bool };

	type() = default;
	explicit type(_type t) : node{}, _t{t} {}
	void add_dimension(unsigned int size) { dimensions.push_back(size); }
	bool compatible(type second) {
		if (_t == _int || _t == _float || _t == _bool) {
			if (second.t() == _int || second.t() == _float || second.t() == _bool)
				return true;
		}
		return false;
	}
	bool compatible() {
			if (_t == _int || _t == _float || _t == _bool)
				return true;
			return false;
	}
	type* cast(type second, operation oper) {
		switch (oper) {
			case minus :
			case times :
			case div :
			case plus :
			case exp :
				if (second.t() == _float || t() == _float)
					return new type(_float);
				else return new type(_int);

			case _and :
			case _or :
			case gt :
			case lt :
			case ge :
			case le :
			case eq :
			case ne :
			case _not :
				return new type(_bool);

			case uminus :
				return new type(t());

			default:
				return new type(_void);
		}
	}
	_type t() const { return _t; }

private:
	_type _t{_void};
	std::list<unsigned int> dimensions;
};

class binary_operation : public node {
public:
	binary_operation(operation op, type t, node* left, node* right)
		: node{}, op{op}, _t{t}, left{left}, right{right} {}
	type t() const { return _t; }

private:
	operation op;
	type _t;
	node* left;
	node* right;
};

class unary_operation : public node {
public:
	unary_operation(operation op, type t, node* operand)
		: node{}, op{op}, _t{t}, operand{operand} {}
	type t() const { return _t; }

private:
	type _t;
	operation op;
	node* operand;
};

class name : public node {
public:
	name() = default;
	explicit name(std::string identifier) : node{}, _identifier{identifier} {}
	void add_offset(node* offset) { offsets.push_back(offset); }
	std::string identifier() const { return _identifier; }

private:
	std::string _identifier;
	std::list<node*> offsets;
};

class assignment : public node {
public:
	assignment() = default;
	assignment(name variable, node* expression)
		: node{}, variable{variable}, expression{expression} {}

private:
	name variable;
	node* expression;
};

class elif_stmt : public node {
public:
	elif_stmt(node* cond, block elif_block)
		: node{}, cond{cond}, elif_block{elif_block} {}

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

private:
	node* condition;
	block code;
};

class return_stmt : public node {
public:
	return_stmt() = default;
	return_stmt(node* expression) : expression{expression} {}

private:
	node* expression;
};

class int_l : public node {
public:
	explicit int_l(int value) : node{}, value{value} {}

private:
	int value;
};

class float_l : public node {
public:
	explicit float_l(double value) : node{}, value{value} {}

private:
	double value;
};

class string_l : public node {
public:
	explicit string_l(std::string str) : node{}, str{str} {}

private:
	std::string str;
};

class bool_l : public node {
public:
	explicit bool_l(bool b) : node{}, b{b} {}

private:
	bool b;
};

class arg : public node {
public:
	arg() = default;
	arg(std::string identifier, type t, bool reference)
		: node{}, identifier{identifier}, _t{t}, reference{reference} {}
	type t() const { return _t; }

private:
	std::string identifier;
	type _t;
	bool reference{false};
};

class declaration : public node {
public:
	declaration(std::string name, type t, node* expression)
		: node{}, name{name}, _t{t}, expression{expression} {}
	type t() const { return _t; }

private:
	std::string name;
	type _t;
	node* expression;
};

class func : public node {
public:
	func(std::string name, std::list<arg> args, type t, block code)
		: node{}, name{name}, args{args}, _t{t}, code{code} {}
	func(std::string name, type t, block code)
		: node{}, name{name}, _t{t}, code{code} {}
	type t() const { return _t; }

private:
	std::string name;
	std::list<arg> args;
	type _t;
	block code;
};

class func_call : public node {
public:
	func_call(std::string name, std::list<node*> parameters)
		: node{}, name{name}, parameters{parameters} {}
	func_call(std::string name) : node{}, name{name} {}

private:
	std::string name;
	std::list<node*> parameters;
};

}  // namespace ast

#endif
