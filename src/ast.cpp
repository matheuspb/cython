#include <ast.h>
#include <st.h>

extern st::symbol_table* current;

void ast::func_call::verify_function_calls() const {
	if (current->lookup(name) == nullptr)
		throw semantic_error(location, "calling unknown function " + name);

	for (auto parameter : parameters)
		parameter->verify_function_calls();
}
