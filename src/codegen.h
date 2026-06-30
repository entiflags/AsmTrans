#pragma once
#include <string>
#include "ast.h"

class codegen
{
public:
	codegen() = default;
	std::string generate(ast_node* root);
private:
	std::pair<bool, int> try_fold_constants(ast_node* node);
	void compile_node(ast_node* node, std::string& asm_out);
};