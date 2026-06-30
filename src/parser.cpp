#include "parser.h"
#include <stdexcept>

bool parser::match(token_type type)
{
	if (peek().type == type)
	{
		advance();
		return true;
	}
	return false;
}

std::unique_ptr<ast_node> parser::parse()
{
	auto program = std::make_unique<program_node>();

	while (peek().type != token_type::eof)
	{
		auto stmt = parse_statement();
		if (stmt)
		{
			program->statements.push_back(std::move(stmt));
		}
		else
		{
			if (peek().type != token_type::eof)
			{
				advance();
			}
		}
	}

	return program;
}

std::unique_ptr<ast_node> parser::parse_assignment()
{
	if (peek().type == token_type::identifier)
	{
		token id_token = peek();
		if (cursor + 1 < tokens.size() && tokens[cursor + 1].type == token_type::assign)
		{
			advance();// Название переменной
			advance();// =
			auto expr = parse_expression();
			return std::make_unique<assignment_node>(id_token.value, std::move(expr));
		}
	}
	return parse_expression();
}
std::unique_ptr<ast_node> parser::parse_statement()
{
	while (peek().type == token_type::newline)
	{
		advance();
	}
	if (peek().type == token_type::eof || peek().type == token_type::detent)
	{
		return nullptr;
	}

	if (peek().type == token_type::identifier && peek().value == "if")
	{
		advance();
		auto condition = parse_expression();

		if (peek().type == token_type::colon)
		{
			advance();
		}
		if (peek().type == token_type::newline)
		{
			advance();
		}

		std::vector<std::unique_ptr<ast_node>> body;

		if (peek().type == token_type::indent)
		{
			advance();
			while (peek().type != token_type::detent && peek().type != token_type::eof)
			{
				auto stmt = parse_statement();
				if (stmt)
				{
					body.push_back(std::move(stmt));
				}
			}
			if (peek().type == token_type::detent)
			{
				advance();
			}
		}
		return std::make_unique<if_node>(std::move(condition), std::move(body));
	}
	if (peek().type == token_type::identifier)
	{
		token id_token = peek();
		if (cursor + 1 < tokens.size() && tokens[cursor + 1].type == token_type::assign)
		{
			advance();
			advance();
			auto expr = parse_expression();
			if (peek().type == token_type::newline)
			{
				advance();
			}
			return std::make_unique<assignment_node>(id_token.value, std::move(expr));
		}
	}
	return nullptr;
}
std::unique_ptr<ast_node> parser::parse_expression()
{
	auto left = parse_term();
	while (peek().type == token_type::plus || peek().type == token_type::minus)
	{
		token op_token = advance();
		char op = op_token.value[0];
		auto right = parse_term();
		left = std::make_unique<binary_op_node>(op, std::move(left), std::move(right));
	}
	return left;
}
std::unique_ptr<ast_node> parser::parse_term()
{
	auto left = parse_primary();
	while (peek().type == token_type::mul || peek().type == token_type::div)
	{
		token op_token = advance();
		char op = op_token.value[0];
		auto right = parse_primary();
		left = std::make_unique<binary_op_node>(op, std::move(left), std::move(right));
	}
	return left;
}
std::unique_ptr<ast_node> parser::parse_primary()
{
	token current = peek();
	if (match(token_type::number))
	{
		return std::make_unique<number_node>(std::stoi(current.value));
	}
	if (match(token_type::identifier))
	{
		return std::make_unique<variable_node>(current.value);
	}
	return nullptr;// Синтаксическая ошибка
}