#pragma once
#include <string>
#include <set>
#include "ast.h"

enum class asm_flavor
{
	att,
	nasm
};

class codegen
{
public:
	codegen() = default;
	std::string generate(ast_node* root, asm_flavor flavor = asm_flavor::nasm);
private:
	asm_flavor current_flavor;
	std::set<std::string> registered_variables;
	std::pair<bool, int> try_fold_constants(ast_node* node);
	void compile_node(ast_node* node, std::string& asm_out);
};