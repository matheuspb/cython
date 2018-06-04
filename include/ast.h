#ifndef AST_H
#define AST_H

#include <location.hh>
#include <string>
#include <vector>

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

enum _type { _int, _float, _char, _void, _bool };

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
		for (auto line : lines)
			line->verify_function_calls();
	}

private:
	std::vector<node*> lines;
};

class type {
public:
	type() = default;
	explicit type(_type t) : _t{t} {}

	void add_dimension(unsigned int size) { dimensions.push_back(size); }

	bool compatible(type second) {
		if (_t == _int || _t == _float || _t == _bool) {
			if (second.t() == _int || second.t() == _float ||
				second.t() == _bool)
				return true;
		}
		return false;
	}

	bool compatible() {
		if (_t == _int || _t == _float || _t == _bool)
			return true;
		return false;
	}

	bool compat_assign(type second) {
		if (_t == second.t())
			return true;
		if (_t == _int || _t == _float || _t == _bool)
			if (second.t() == _int || second.t() == _float ||
				second.t() == _bool)
				return true;
		return false;
	}

	type* cast(type second, operation oper) {
		switch (oper) {
		case minus:
		case times:
		case div:
		case plus:
		case exp:
			if (second.t() == _float || t() == _float)
				return new type(_float);
			else
				return new type(_int);

		case _and:
		case _or:
		case gt:
		case lt:
		case ge:
		case le:
		case eq:
		case ne:
		case _not:
			return new type(_bool);

		case uminus:
			return new type(t());

		default:
			return new type(_void);
		}
	}

	_type t() const { return _t; }

private:
	_type _t{_int};
	std::vector<unsigned int> dimensions;
};

class expr : public node {
public:
	virtual type t() const = 0;
};

class binary_operation : public expr {
public:
	binary_operation(operation op, node* left, node* right, type t)
		: op{op}, _t{t}, left{left}, right{right} {}
	type t() const { return _t; }

	void verify_function_calls() const {
		left->verify_function_calls();
		right->verify_function_calls();
	}

private:
	operation op;
	type _t;
	node* left;
	node* right;
};

class unary_operation : public expr {
public:
	unary_operation(operation op, node* operand, type t)
		: op{op}, _t{t}, operand{operand} {}
	type t() const { return _t; }

	void verify_function_calls() const { operand->verify_function_calls(); }

private:
	operation op;
	type _t;
	node* operand;
};

class name : public expr {
public:
	name() = default;
	explicit name(std::string identifier, type t)
		: _identifier{identifier}, _t(t) {}
	void add_offset(node* offset) { offsets.push_back(offset); }

	std::string identifier() const { return _identifier; }
	type t() const { return _t; }

	void verify_function_calls() const {
		for (auto offset : offsets)
			offset->verify_function_calls();
	}

private:
	std::string _identifier;
	type _t;
	std::vector<node*> offsets;
};

class assignment : public expr {
public:
	assignment() = default;
	assignment(name variable, node* expression, type t)
		: variable{variable}, expression{expression}, _t(t) {}
	type t() const { return _t; }

	void verify_function_calls() const { expression->verify_function_calls(); }

private:
	name variable;
	node* expression;
	type _t;
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
		node* cond, block if_block, std::vector<elif_stmt> elif_stmts,
		block else_block)
		: node{}
		, cond{cond}
		, if_block{if_block}
		, elif_stmts{elif_stmts}
		, else_block{else_block} {}

	void verify_function_calls() const {
		cond->verify_function_calls();
		if_block.verify_function_calls();
		for (auto elif : elif_stmts)
			elif.verify_function_calls();
		else_block.verify_function_calls();
	}

private:
	node* cond;
	block if_block;
	std::vector<elif_stmt> elif_stmts;
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

class int_l : public expr {
public:
	explicit int_l(int value) : value{value} { _t = *new type(_int); }
	type t() const { return _t; }

	void verify_function_calls() const {}

private:
	type _t;
	int value;
};

class float_l : public expr {
public:
	explicit float_l(double value) : value{value} { _t = *new type(_float); }
	type t() const { return _t; }

	void verify_function_calls() const {}

private:
	type _t;
	double value;
};

class string_l : public expr {
public:
	explicit string_l(std::string str) : str{str} { _t = *new type(_char); }
	type t() const { return _t; }

	void verify_function_calls() const {}

private:
	type _t;
	std::string str;
};

class bool_l : public expr {
public:
	explicit bool_l(bool b) : b{b} { _t = *new type(_bool); }
	type t() const { return _t; }

	void verify_function_calls() const {}

private:
	type _t;
	bool b;
};

class arg : public expr {
public:
	arg() = default;
	arg(std::string identifier, type t, bool reference)
		: identifier{identifier}, _t{t}, reference{reference} {}
	type t() const { return _t; }

	void verify_function_calls() const {}

private:
	std::string identifier;
	type _t;
	bool reference{false};
};

class declaration : public expr {
public:
	declaration(std::string name, type t, node* expression)
		: name{name}, _t{t}, expression{expression} {}
	type t() const { return _t; }

	void verify_function_calls() const {
		if (expression)
			expression->verify_function_calls();
	}

private:
	std::string name;
	type _t;
	node* expression;
};

class func : public node {
public:
	func(std::string name, std::vector<arg> args, type t, block code)
		: name{name}, args{args}, _t{t}, code{code} {}
	func(std::string name, type t, block code)
		: name{name}, _t{t}, code{code} {}

	type t() const { return _t; }

	void verify_function_calls() const { code.verify_function_calls(); }

	const std::vector<arg> args;

private:
	std::string name;
	type _t;
	block code;
};

class func_call : public expr {
public:
	func_call(
		std::string name, std::vector<expr*> parameters, yy::location location)
		: expr{}, name{name}, parameters{parameters}, location{location} {}
	func_call(std::string name, yy::location location)
		: expr{}, name{name}, location{location} {}

	const std::string& func_name() const { return name; }

	/* check if the called function exists in the symbol table */
	void verify_function_calls() const;

	type t() const { /*TODO fix stub impl*/
		return type();
	}

private:
	std::string name;
	std::vector<expr*> parameters;
	yy::location location;
};

}  // namespace ast

#endif
