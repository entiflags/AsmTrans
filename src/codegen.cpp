#include "codegen.h"

std::pair<bool, int> codegen::try_fold_constants(ast_node* node)
{
	if(!node) return{ false,0 };
	if (node->type == ast_node_type::number)
	{
		return{ true,static_cast<number_node*>(node)->value };
	}
	if (node->type == ast_node_type::binary_op)
	{
		auto op_node = static_cast<binary_op_node*>(node);

		auto [left_is_const, left_val] = try_fold_constants(op_node->left.get());
		auto [right_is_const, right_val] = try_fold_constants(op_node->right.get());

		if (left_is_const && right_is_const)
		{
			switch (op_node->op)
			{
			case'+':return { true,left_val + right_val };
			case'-':return { true,left_val - right_val };
			case'*':return { true,left_val * right_val };
			case'/':return { right_val != 0 ? std::pair{true,left_val / right_val} : std::pair{false,0} };
			}
		}
	}
	return{ false,0 };
}

void codegen::compile_node(ast_node* node, std::string& asm_out)
{
	if (!node) return;

	auto [is_const, const_value] = try_fold_constants(node);
	if (is_const)
	{
		asm_out += "    movl    $" + std::to_string(const_value) + ",%eax\n";
		return;
	}
	if (node->type == ast_node_type::variable)
	{
		auto var_node = static_cast<variable_node*>(node);
		asm_out += "    movl    " + var_node->name + "(%rip),%eax\n";
		return;
	}
	if (node->type == ast_node_type::If)
	{
		auto ifnode = static_cast<if_node*>(node);
		static int label_counter = 0;
		std::string end_label = ".L_END_IF_" + std::to_string(label_counter++);
		compile_node(ifnode->condition.get(), asm_out);
		asm_out += "    cmpl    $0,%eax\n";
		asm_out += "    je      " + end_label + "\n";
		asm_out += "    # IF BLOCK START\n";
		for (const auto& stmt : ifnode->body)
			compile_node(stmt.get(), asm_out);
		asm_out += "    # IF BLOCK END\n";
		asm_out += end_label + ":\n";
		return;
	}
	if (node->type == ast_node_type::assignment)
	{
		auto assign_node = static_cast<assignment_node*>(node);
		compile_node(assign_node->value.get(), asm_out);
		// нужно сделать это более хорошим способом
		asm_out += "    # saving result into var " + assign_node->var_name + "\n";
		asm_out += "    movl    %eax," + assign_node->var_name + "(%rip)\n";
	}
	if (node->type == ast_node_type::binary_op)
	{
		auto op_node = static_cast<binary_op_node*>(node);
		auto [right_is_const, right_val] = try_fold_constants(op_node->right.get());
		if (right_is_const)
		{
			compile_node(op_node->left.get(), asm_out);
			switch (op_node->op)
			{
			case '+':asm_out += "    addl    $"+std::to_string(right_val)+",%eax\n";break;
			case '-':asm_out += "    subl    $" + std::to_string(right_val) + ",%eax\n";break;
			case '*':asm_out += "    imull   $" + std::to_string(right_val) + ",%eax\n";break;
			}
		}
		else
		{
			compile_node(op_node->right.get(), asm_out);
			asm_out += "    pushq   %rax\n";
			compile_node(op_node->left.get(), asm_out);
			asm_out += "    popq    %rbx\n";
			switch (op_node->op)
			{
			case '+':asm_out += "    addl    %ebx,%eax\n";break;
			case '-':asm_out += "    subl    %ebx,%eax\n";break;
			case '*':asm_out += "    imull   %ebx,%eax\n";break;
			}
		}
	}
}
std::string codegen::generate(ast_node* root)
{
	std::string asm_code = ".text\n.globl main\nmain:\n";
	if (root && root->type == ast_node_type::program)
	{
		auto prog_node = static_cast<program_node*>(root);
		for (const auto& stmt : prog_node->statements)
			compile_node(stmt.get(), asm_code);
	}
	else
	{
		compile_node(root, asm_code);
	}
	asm_code += "    ret\n";
	return asm_code;
}