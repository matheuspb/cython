#ifndef ST_H
#define ST_H

#include <ast.h>
#include <errors.h>
#include <list>
#include <unordered_map>

namespace st {

/* stub class to be inherited by the classes of the symbol table entries */
class st_entry {
public:
	virtual ~st_entry() = default;
};

class variable : public st_entry {
public:
	explicit variable(ast::type t) : st_entry{}, t{t} {}
	void initialize() { initialized = true; }
	bool is_initialized() { return initialized; }
	const ast::type& type() const { return t; }

private:
	ast::type t;
	bool initialized{false};
};

class function : public st_entry {
public:
	explicit function(ast::func* declaration)
		: st_entry{}, declaration{declaration} {}

	const ast::func* declaration;
};

class symbol_table {
public:
	symbol_table() = default;
	explicit symbol_table(symbol_table* parent) : parent{parent} {}

	/* searches recursively through the STs for a given identifier */
	st_entry* lookup(const std::string& name) const {
		try {
			return symbols.at(name);
		} catch (std::out_of_range) {
			return parent != nullptr ? parent->lookup(name) : nullptr;
		}
	}

	bool insert_variable(const std::string& name, ast::type t) {
		if (symbols.find(name) == symbols.end()) {
			symbols[name] = new variable(t);
			return true;
		} else {
			return false;
		}
	}

	void initialize_variable(const std::string& name) {
		dynamic_cast<variable*>(lookup(name))->initialize();
	}

	bool is_declared(const std::string& name) const {
		return lookup(name) != nullptr;
	}

	bool is_initialized(const std::string& name) const {
		return dynamic_cast<variable*>(lookup(name))->is_initialized();
	}

	bool insert_function(const std::string& name, ast::func* declaration) {
		if (symbols.find(name) == symbols.end()) {
			symbols[name] = new function(declaration);
			return true;
		} else {
			return false;
		}
	}

	const ast::type& get_type(const std::string& name) const {
		return dynamic_cast<variable*>(lookup(name))->type();
	}

	const ast::type& get_function_return_type(const std::string& name) const {
		return dynamic_cast<function*>(lookup(name))->declaration->t();
	}

	symbol_table* const parent{nullptr};

private:
	std::unordered_map<std::string, st_entry*> symbols;
};

}  // namespace st

#endif
