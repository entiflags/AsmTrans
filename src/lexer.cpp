#include "lexer.h"
#include <cctype>

std::vector<token> lexer::tokenize()
{
	std::vector<token> tokens;
	bool is_at_line_start = true;
	while (cursor < src.size())
	{
		if (is_at_line_start)
		{
			int current_indent = 0;
			while (peek() == ' ' || peek() == '\t')
			{
				if (advance() == '\t')
				{
					current_indent += 4;
				}
				else
				{
					current_indent++;
				}
			}
			if (peek() == '\n' || peek() == '#')
			{
				while (cursor < src.size() && peek() != '\n')
				{
					advance();
				}
				if (peek() == '\n')
				{
					advance();
				}
				continue;
			}
			int last_indent = indent_stack.back();
			if (current_indent > last_indent)
			{
				indent_stack.push_back(current_indent);
				tokens.push_back({ token_type::indent,"" });
			}
			else if (current_indent < last_indent)
			{
				while (!indent_stack.empty() && indent_stack.back() > current_indent)
				{
					indent_stack.pop_back();
					tokens.push_back({ token_type::detent,"" });
				}
			}
			is_at_line_start = false;
		}
		char current = peek();
		if (current == ' ' || current == '\t')
		{
			advance();
			continue;
		}
		if (current == '\n')
		{
			advance();
			tokens.push_back({ token_type::newline, "\\n" });
			is_at_line_start = true;
			continue;
		}
		if (std::isalpha(current) || current == '_')
		{
			std::string id;
			while (std::isalnum(peek()) || peek() == '_')
				id += advance();
			tokens.push_back({ token_type::identifier,id });
			continue;
		}
		if (std::isdigit(current))
		{
			std::string num;
			while (std::isdigit(peek()))
				num += advance();
			tokens.push_back({ token_type::number,num });
			continue;
		}
		switch (current)
		{
		case '=':
			advance();
			tokens.push_back({ token_type::assign,"=" });
			break;
		case '+':
			advance();
			tokens.push_back({ token_type::plus,"+" });
			break;
		case '-':
			advance();
			tokens.push_back({ token_type::minus,"-" });
			break;
		case '*':
			advance();
			tokens.push_back({ token_type::mul,"*" });
			break;
		case '/':
			advance();
			tokens.push_back({ token_type::div,"/" });
			break;
		case ':':
			advance();
			tokens.push_back({ token_type::colon,":" });
			break;
		default:
			advance();
			break;
		}
	}
	while (indent_stack.size() > 1)
	{
		indent_stack.pop_back();
		tokens.push_back({ token_type::detent,"" });
	}
	tokens.push_back({ token_type::eof,"" });
	return tokens;
}


