#ifndef AST_H
#define AST_H

#include "llvm/ADT/APInt.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include <errors.h>
#include <location.hh>
#include <string>
#include <vector>

namespace ast {

using namespace llvm;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);

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
	virtual void verify_semantic() = 0;
	virtual Value* codegen() = 0;
};

class block : public node {
public:
	block() = default;
	explicit block(node* line) : node{}, lines{line} {}

	void add_line(node* line) { lines.push_back(line); }

	void verify_semantic() {
		for (auto line : lines)
			line->verify_semantic();
	}

	Value* codegen(){};

private:
	std::vector<node*> lines;
};

class type {
public:
	type() = default;
	explicit type(_type t) : _t{t} {}

	void add_dimension(unsigned int size) { dimensions.push_back(size); }

	bool compatible(const type& second) const {
		if (_t == _int || _t == _float || _t == _bool) {
			if (second.t() == _int || second.t() == _float ||
				second.t() == _bool)
				return true;
		}
		return false;
	}

	bool compatible() const {
		if (_t == _int || _t == _float || _t == _bool)
			return true;
		return false;
	}

	bool compat_assign(const type& second) const {
		if (_t == second.t())
			return true;
		if (_t == _int || _t == _float || _t == _bool)
			if (second.t() == _int || second.t() == _float ||
				second.t() == _bool)
				return true;
		return false;
	}

	type cast(const type& second, const operation& oper) const {
		switch (oper) {
		case minus:
		case times:
		case div:
		case plus:
		case exp:
			if (second.t() == _float || t() == _float)
				return type(_float);
			else
				return type(_int);

		case _and:
		case _or:
		case gt:
		case lt:
		case ge:
		case le:
		case eq:
		case ne:
		case _not:
			return type(_bool);

		case uminus:
			return type(t());

		default:
			return type(_void);
		}
	}

	_type t() const { return _t; }

private:
	_type _t{_int};
	std::vector<unsigned int> dimensions;
};

class expr : public node {
public:
	virtual const type& t() const = 0;
};

class binary_operation : public expr {
public:
	binary_operation(
		const operation& op, expr* left, expr* right, const yy::location& loc)
		: op{op}, left{left}, right{right}, loc{loc} {}

	const type& t() const { return _t; }

	void verify_semantic() {
		left->verify_semantic();
		right->verify_semantic();

		if (!left->t().compatible(right->t()))
			throw semantic_error(yy::location(), "invalid types for operation");
		_t = left->t().cast(right->t(), op);
	}

	Value* codegen() {
		Value* l = left->codegen();
		Value* r = right->codegen();

		if (!l || !r)
			return nullptr;

		if (t().t() == _float) {
			if (left->t().t() != _float)
				l = Builder.CreateSIToFP(
					l, Type::getDoubleTy(TheContext), "inttmp");
			if (right->t().t() != _float)
				r = Builder.CreateSIToFP(
					r, Type::getDoubleTy(TheContext), "inttmp");
		} else {
			// verificar se é int 32 msm
			if (left->t().t() == _float)
				l = Builder.CreateFPToSI(
					l, Type::getInt32Ty(TheContext), "inttmp");
			if (right->t().t() == _float)
				r = Builder.CreateFPToSI(
					r, Type::getInt32Ty(TheContext), "inttmp");
		}

		switch (op) {
		case minus:
			if (t().t() == _float)
				return Builder.CreateFSub(l, r, "subtmp");
			return Builder.CreateSub(l, r, "subtmp");
		case times:
			if (t().t() == _float)
				return Builder.CreateFMul(l, r, "multmp");
			return Builder.CreateMul(l, r, "multmp");
		case div:
			if (t().t() == _float)
				return Builder.CreateFDiv(l, r, "divtmp");
			return Builder.CreateSDiv(l, r, "divtmp");
		case plus:
			if (t().t() == _float)
				return Builder.CreateFAdd(l, r, "addtmp");
			return Builder.CreateAdd(l, r, "addtmp");
		case exp:
			// missing
			return nullptr;
		case _and:
			return Builder.CreateAnd(l, r, "andtmp");
		case _or:
			return Builder.CreateOr(l, r, "ortmp");
		case gt:
			return Builder.CreateICmpSGT(l, r, "cmpttmp");
		case lt:
			// Integer Compare Signed Lower Than
			return Builder.CreateICmpSLT(l, r, "cmpttmp");
		case ge:
			return Builder.CreateICmpSGE(l, r, "cmpttmp");
		case le:
			return Builder.CreateICmpSLE(l, r, "cmpttmp");
		case eq:
			return Builder.CreateICmpEQ(l, r, "cmpttmp");
		case ne:
			return Builder.CreateICmpNE(l, r, "cmpttmp");
		}
	}

private:
	operation op;
	type _t;
	expr* left;
	expr* right;
	yy::location loc;
};

class unary_operation : public expr {
public:
	unary_operation(const operation& op, expr* operand, const yy::location& loc)
		: op{op}, operand{operand}, loc{loc} {}
	const type& t() const { return _t; }

	void verify_semantic() {
		operand->verify_semantic();
		if (!operand->t().compatible())
			throw semantic_error(yy::location(), "invalid types for operation");
		_t = operand->t().cast(operand->t(), op);
	}

	Value* codegen() {
		Value* l = operand->codegen();

		switch (op) {
		case _not:
			if (operand->t().t() == _float)
				l = Builder.CreateFPToSI(
					l, Type::getInt32Ty(TheContext), "inttmp");
			return Builder.CreateNot(l, "nottmp");
		case uminus:
			if (t().t() == _float)
				return Builder.CreateFNeg(l, "umintmp");
			return Builder.CreateNeg(l, "umintmp");
		}
	}

private:
	operation op;
	type _t;
	expr* operand;
	yy::location loc;
};

class name : public expr {
public:
	name() = default;
	name(const std::string& identifier, const type& t)
		: _identifier{identifier}, _t(t) {}
	void add_offset(node* offset) { offsets.push_back(offset); }

	const std::string& identifier() const { return _identifier; }

	const type& t() const { return _t; }

	void verify_semantic() {
		for (auto offset : offsets)
			offset->verify_semantic();
	}

	Value* codegen() {}

private:
	std::string _identifier;
	type _t;
	std::vector<node*> offsets;
};

class assignment : public expr {
public:
	assignment() = default;
	assignment(const name& variable, expr* expression, const type& t)
		: variable{variable}, expression{expression}, _t(t) {}

	const type& t() const { return _t; }

	void verify_semantic() {
		expression->verify_semantic();
		if (!_t.compat_assign(expression->t()))
			throw semantic_error(
				yy::location(), "invalid type for assignment, "
								"expression and name type differ");
	}

	Value* codegen() {}

private:
	name variable;
	expr* expression;
	type _t;
};

class elif_stmt : public node {
public:
	elif_stmt(expr* cond, const block& elif_block)
		: node{}, cond{cond}, elif_block{elif_block} {}

	void verify_semantic() {
		cond->verify_semantic();
		elif_block.verify_semantic();
		if (!cond->t().compatible())
			throw semantic_error(
				yy::location(),
				"invalid type for if statement"
				", only int, float and bool can be used for if operations.");
	}

	Value* codegen() {}

private:
	expr* cond;
	block elif_block;
};

class if_stmt : public node {
public:
	if_stmt() = default;
	if_stmt(
		expr* cond, const block& if_block,
		const std::vector<elif_stmt>& elif_stmts, const block& else_block)
		: node{}
		, cond{cond}
		, if_block{if_block}
		, elif_stmts{elif_stmts}
		, else_block{else_block} {}

	void verify_semantic() {
		cond->verify_semantic();
		if_block.verify_semantic();
		for (auto elif : elif_stmts)
			elif.verify_semantic();
		else_block.verify_semantic();
		if (!cond->t().compatible())
			throw semantic_error(
				yy::location(),
				"invalid type for if statement"
				", only int, float and bool can be used for if operations.");
	}

	Value* codegen() {}

private:
	expr* cond;
	block if_block;
	std::vector<elif_stmt> elif_stmts;
	block else_block;
};

class for_stmt : public node {
public:
	for_stmt() = default;
	for_stmt(node* init, expr* condition, node* step, const block& code)
		: node{}, init{init}, condition{condition}, step{step}, code{code} {}

	void verify_semantic() {
		init->verify_semantic();
		condition->verify_semantic();
		step->verify_semantic();
		code.verify_semantic();
		if (!condition->t().compatible())
			throw semantic_error(
				yy::location(), "Invalid type for condition"
								" statement, only int, float and bool"
								" can be used for conditions.");
	}

	Value* codegen(){};

private:
	node* init;
	expr* condition;
	node* step;
	block code;
};

class while_stmt : public node {
public:
	while_stmt() = default;
	while_stmt(expr* condition, const block& code)
		: condition{condition}, code{code} {}

	void verify_semantic() {
		condition->verify_semantic();
		code.verify_semantic();
	}

	Value* codegen() {}

private:
	node* condition;
	block code;
};

class return_stmt : public node {
public:
	return_stmt() = default;
	explicit return_stmt(node* expression) : expression{expression} {}

	void verify_semantic() { expression->verify_semantic(); }

	Value* codegen() {}

private:
	node* expression;
};

class int_l : public expr {
public:
	explicit int_l(int value) : value{value} { _t = type(_int); }

	const type& t() const { return _t; }

	void verify_semantic() {}

	Value* codegen() { return ConstantInt::get(TheContext, APInt(value, 32)); }

private:
	type _t;
	int value;
};

class float_l : public expr {
public:
	explicit float_l(double value) : value{value} { _t = type(_float); }

	const type& t() const { return _t; }

	void verify_semantic() {}

	Value* codegen() { return ConstantFP::get(TheContext, APFloat(value)); }

private:
	type _t;
	double value;
};

class string_l : public expr {
public:
	explicit string_l(const std::string& str) : str{str} { _t = type(_char); }

	const type& t() const { return _t; }

	void verify_semantic() {}

	Value* codegen() {}

private:
	type _t;
	std::string str;
};

class bool_l : public expr {
public:
	explicit bool_l(bool b) : b{b} { _t = type(_bool); }

	const type& t() const { return _t; }

	void verify_semantic() {}

	Value* codegen() { return ConstantInt::get(TheContext, APInt(b, 32)); }

private:
	type _t;
	bool b;
};

class arg : public expr {
public:
	arg() = default;
	arg(const std::string& identifier, const type& t, bool reference)
		: identifier{identifier}, _t{t}, reference{reference} {}

	const type& t() const { return _t; }

	void verify_semantic() {}

	Value* codegen() {}

private:
	std::string identifier;
	type _t;
	bool reference{false};
};

class declaration : public expr {
public:
	declaration(const std::string& name, const type& t, expr* expression)
		: name{name}, _t{t}, expression{expression} {}

	const type& t() const { return _t; }

	void verify_semantic() {
		if (expression)
			expression->verify_semantic();
	}

	Value* codegen() {}

private:
	std::string name;
	type _t;
	expr* expression;
};

class func : public node {
public:
	func(
		const std::vector<arg>& args, const std::string& name, const type& t,
		const block& code)
		: args{args}, name{name}, _t{t}, code{code} {}
	func(const std::string& name, const type& t, const block& code)
		: name{name}, _t{t}, code{code} {}

	const type& t() const { return _t; }

	void verify_semantic() { code.verify_semantic(); }

	const std::vector<arg> args;

	Value* codegen() {}

private:
	std::string name;
	type _t;
	block code;
};

class func_call : public expr {
public:
	func_call(
		const std::string& name, const std::vector<expr*>& parameters,
		const yy::location& location)
		: expr{}, name{name}, parameters{parameters}, location{location} {}
	func_call(const std::string& name, const yy::location& location)
		: expr{}, name{name}, location{location} {}

	const std::string& func_name() const { return name; }

	/* check if the called function exists in the symbol table */
	void verify_semantic();

	Value* codegen() {}

	const type& t() const;

private:
	std::string name;
	std::vector<expr*> parameters;
	yy::location location;
};

}  // namespace ast

#endif
