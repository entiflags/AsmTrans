#include <print>
#include <string_view>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

const char* token_type_to_string(token_type type)
{
	switch (type)
	{
	case token_type::identifier: return "identifier";
	case token_type::number: return "number";
	case token_type::assign: return "assign";
	case token_type::plus: return "operator";
	case token_type::mul: return "operator";
	case token_type::div: return "operator";
	case token_type::colon: return "colon";
	case token_type::newline: return "newline";
	case token_type::indent: return "indent";
	case token_type::detent: return "detent";
	case token_type::equal: return "equal";
	case token_type::notequal: return "notequal";
	case token_type::less: return "less";
	case token_type::greater: return "greater";
	case token_type::eof: return "eof";
	default: return "unknown";
	}
}

int main()
{
	std::string_view code = R"(x = 10
if x == 10:
	y = 20
	z = 30
)";
	std::printf("code:\n%.*s\n\n", static_cast<int>(code.size()), code.data());
	lexer lexer(code);
	std::vector<token> tokens = lexer.tokenize();
	for (const auto& token : tokens)
		std::printf("[%-15s] -> \"%s\"\n", token_type_to_string(token.type), token.value.c_str());
	parser parser(tokens);
	std::unique_ptr<ast_node> ast_root = parser.parse();
	codegen codegen;
	std::string asm_output = codegen.generate(ast_root.get(), asm_flavor::att);
	std::printf("compilation result\n%s\n", asm_output.c_str());
	return 0;
}