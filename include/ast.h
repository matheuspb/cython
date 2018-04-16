#ifndef AST_H
#define AST_H

#include <list>

namespace ast {

enum operation {
	plus, minus, times, div, exp, _and, _or,
	gt, lt, ge, le, eq, ne, _not, uminus
};

class node {
public:
	node() = default;
};

class block: public node {
public:
	block() = default;
	explicit block(node* line): node{}, lines{line} {}
	void add_line(node* line) { lines.push_back(line); }

private:
	std::list<node*> lines;
};

class binary_operation: public node {
public:
	binary_operation(operation _op, node* _left, node* _right):
		node{}, op{_op}, left{_left}, right{_right} {}

private:
	operation op;
	node* left;
	node* right;
};

class unary_operation: public node {
public:
	unary_operation(operation _op, node* _operand):
		node{}, op{_op}, operand{_operand} {}

private:
	operation op;
	node* operand;
};

}  // namespace ast

#endif
