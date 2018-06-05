#include <ast.h>
#include <st.h>

extern st::symbol_table* current;

void ast::func_call::verify_semantic() {
	/* verify if the function exists */
	if (current->lookup(name) == nullptr)
		throw semantic_error(location, "calling unknown function " + name);

	for (auto parameter : parameters)
		parameter->verify_semantic();

	/* parameters type checking */
	const ast::func* func_declaration =
		dynamic_cast<st::function*>(current->lookup(name))->declaration;

	if (func_declaration->args.size() != parameters.size())
		throw semantic_error(location, "wrong number of parameters");

	for (auto i = 0u; i < parameters.size(); i++)
		if (func_declaration->args[i].t().t() != parameters[i]->t().t())
			throw semantic_error(location, "invalid parameter type");
}

ast::type ast::func_call::t() const {
	return current->get_function_return_type(name);
}
