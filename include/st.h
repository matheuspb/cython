#ifndef ST_H
#define ST_H

#include <unordered_map>
#include <errors.h>
#include <ast.h>

namespace st {

/* stub class to be inherited by the classes of the symbol table entries */
class st_entry {
public:
	virtual ~st_entry() = default;
};

class variable : public st_entry {
public:
	variable(ast::type t) : st_entry{}, t{t} {}
	void initialize() { initialized = true; }
	bool is_initialized() { return initialized; }

private:
	ast::type t;
	bool initialized{false};
};

class symbol_table {
public:
	symbol_table() = default;
	symbol_table(symbol_table* parent) : parent{parent} {}

	/* searches recursively through the STs for a given identifier */
	st_entry* lookup(const std::string& name) const {
		try {
			return symbols.at(name);
		} catch (std::out_of_range) {
			return parent != nullptr ? parent->lookup(name) : nullptr;
		}
	}

	bool insert_variable(const std::string& name, ast::type t) {
		if (t.t() == ast::type::_void)
			throw semantic_error("cannot declare variable of type void");

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

	symbol_table * const parent{nullptr};

private:
	std::unordered_map<std::string, st_entry*> symbols;
};

}

#endif
