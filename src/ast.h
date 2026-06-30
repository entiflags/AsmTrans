#pragma once
#include <string>
#include <memory>
#include <vector>

enum class ast_node_type
{
	program,
	assignment,
	binary_op,
	number,
	variable,
	If
};

struct ast_node
{
	ast_node_type type;
	virtual ~ast_node() = default;
	explicit ast_node(ast_node_type t) : type(t) {}
};
struct number_node : public ast_node
{
	int value;
	explicit number_node(int val) : ast_node(ast_node_type::number), value(val) {}
};
struct variable_node : public ast_node
{
	std::string name;
	explicit variable_node(std::string nm) : ast_node(ast_node_type::variable), name(std::move(nm)) {}
};
struct binary_op_node : public ast_node
{
	char op;
	std::unique_ptr<ast_node> left;
	std::unique_ptr<ast_node> right;
	binary_op_node(char operation, std::unique_ptr<ast_node> l, std::unique_ptr<ast_node> r)
		: ast_node(ast_node_type::binary_op), op(operation), left(std::move(l)), right(std::move(r)) {}
};
struct assignment_node : public ast_node
{
	std::string var_name;
	std::unique_ptr<ast_node> value;
	assignment_node(std::string name, std::unique_ptr<ast_node> val)
		: ast_node(ast_node_type::assignment), var_name(std::move(name)), value(std::move(val)) {}
};
struct if_node : public ast_node
{
	std::unique_ptr<ast_node> condition;
	std::vector<std::unique_ptr<ast_node>> body;

	if_node(std::unique_ptr<ast_node> cond, std::vector<std::unique_ptr<ast_node>> b)
		: ast_node(ast_node_type::If), condition(std::move(cond)), body(std::move(b)) {}
};
struct program_node : public ast_node
{
	std::vector<std::unique_ptr<ast_node>> statements;
	program_node() : ast_node(ast_node_type::program) {}
};

