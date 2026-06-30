#pragma once
#include <vector>
#include "lexer.h"
#include "ast.h"

class parser
{
public:
	explicit parser(std::vector<token> t) : tokens(std::move(t)), cursor(0) {}

	std::unique_ptr<ast_node> parse();
private:
	std::vector<token> tokens;
	size_t cursor;

	token peek() const
	{
		return tokens[cursor];
	}
	token advance()
	{
		return tokens[cursor++];
	}
	bool match(token_type type);

	std::unique_ptr<ast_node> parse_assignment();
	std::unique_ptr<ast_node> parse_statement();
	std::unique_ptr<ast_node> parse_expression();
	std::unique_ptr<ast_node> parse_term();
	std::unique_ptr<ast_node> parse_primary();
	std::unique_ptr<ast_node> parse_comparison();
};