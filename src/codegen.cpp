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
		if (current_flavor == asm_flavor::nasm)
		{
			asm_out += "    mov     eax," + std::to_string(const_value) + "\n";
		}
		else
		{
			asm_out += "    movl    $" + std::to_string(const_value) + ",%eax\n";
		}
		return;
	}
	if (node->type == ast_node_type::variable)
	{
		auto var_node = static_cast<variable_node*>(node);
		if (current_flavor == asm_flavor::nasm)
		{
			asm_out += "    mov     eax,[" + var_node->name + "]\n";
		}
		else
		{
			asm_out += "    movl    " + var_node->name + "(%rip),%eax\n";
		}
		return;
	}
	if (node->type == ast_node_type::If)
	{
		auto ifnode = static_cast<if_node*>(node);
		static int label_counter = 0;
		std::string end_label = ".L_END_IF_" + std::to_string(label_counter++);

		bool is_comparison = false;
		char comp_op = ' ';
		binary_op_node* comp_node = nullptr;

		if (ifnode->condition->type == ast_node_type::binary_op)
		{
			comp_node = static_cast<binary_op_node*>(ifnode->condition.get());
			if (comp_node->op == 'e' || comp_node->op == 'n' || comp_node->op == 'l' || comp_node->op == 'g')
			{
				is_comparison = true;
				comp_op = comp_node->op;
			}
		}

		if (is_comparison && comp_node)
		{
			compile_node(comp_node->right.get(), asm_out);
			asm_out += (current_flavor == asm_flavor::nasm) ? "    push    rax\n" : "    pushq   %rax\n";
			compile_node(comp_node->left.get(), asm_out);
			asm_out += (current_flavor == asm_flavor::nasm) ? "    pop     rbx\n" : "    popq    %rbx\n";

			if (current_flavor == asm_flavor::nasm)
			{
				asm_out += "    cmp     eax,ebx\n";

				if (comp_op == 'e')
				{
					asm_out += "    jne     " + end_label + "\n";
				}
				if (comp_op == 'n')
				{
					asm_out += "    je      " + end_label + "\n";
				}
				if (comp_op == 'l')
				{
					asm_out += "    jge     " + end_label + "\n";
				}
				if (comp_op == 'g')
				{
					asm_out += "    jle     " + end_label + "\n";
				}
			}
			else
			{
				asm_out += "    cmpl    %ebx,%eax\n";

				if (comp_op == 'e')
				{
					asm_out += "    jne     " + end_label + "\n";
				}
				if (comp_op == 'n')
				{
					asm_out += "    je      " + end_label + "\n";
				}
				if (comp_op == 'l')
				{
					asm_out += "    jge     " + end_label + "\n";
				}
				if (comp_op == 'g')
				{
					asm_out += "    jle     " + end_label + "\n";
				}
			}
		}
		else
		{
			compile_node(ifnode->condition.get(), asm_out);
			if (current_flavor == asm_flavor::nasm)
			{
				asm_out += "    cmp    eax,0\n    je     " + end_label + "\n";
			}
			else
			{
				asm_out += "    cmpl    $0,%eax\n    je     " + end_label + "\n";
			}
		}
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
		if (current_flavor == asm_flavor::nasm)
		{
			asm_out += "    ; saving result into var " + assign_node->var_name + "\n";
			asm_out += "    mov     [" + assign_node->var_name + "],eax\n";
		}
		else
		{
			asm_out += "    # saving result into var " + assign_node->var_name + "\n";
			asm_out += "    movl    %eax," + assign_node->var_name + "(%rip)\n";
		}
	}
	if (node->type == ast_node_type::binary_op)
	{
		auto op_node = static_cast<binary_op_node*>(node);
		auto [right_is_const, right_val] = try_fold_constants(op_node->right.get());
		if (right_is_const)
		{
			compile_node(op_node->left.get(), asm_out);
			if (current_flavor == asm_flavor::nasm)
			{
				switch (op_node->op)
				{
				case '+':asm_out += "    add    eax," + std::to_string(right_val) + "\n";break;
				case '-':asm_out += "    sub    eax," + std::to_string(right_val) + "\n";break;
				case '*':asm_out += "    imul   eax," + std::to_string(right_val) + "\n";break;
				}
			}
			else
			{
				switch (op_node->op)
				{
				case '+':asm_out += "    addl    $" + std::to_string(right_val) + ",%eax\n";break;
				case '-':asm_out += "    subl    $" + std::to_string(right_val) + ",%eax\n";break;
				case '*':asm_out += "    imull   $" + std::to_string(right_val) + ",%eax\n";break;
				}
			}
		}
		else
		{
			compile_node(op_node->right.get(), asm_out);
			asm_out += (current_flavor == asm_flavor::nasm) ? "    push   rax\n" : "    pushq   %rax\n";
			compile_node(op_node->left.get(), asm_out);
			asm_out += (current_flavor == asm_flavor::nasm) ? "    pop    rbx\n" : "    popq    %rbx\n";
			if (current_flavor == asm_flavor::nasm)
			{
				switch (op_node->op)
				{
				case '+':asm_out += "    add    eax,ebx\n";break;
				case '-':asm_out += "    sub    eax,ebx\n";break;
				case '*':asm_out += "    imul   eax,ebx\n";break;
				}
			}
			else
			{
				switch (op_node->op)
				{
				case '+':asm_out += "    addl    %ebx,%eax\n";break;
				case '-':asm_out += "    subl    %ebx,%eax\n";break;
				case '*':asm_out += "    imull   %ebx,%eax\n";break;
				}
			}
		}
	}
}
std::string codegen::generate(ast_node* root, asm_flavor flavor)
{
	current_flavor = flavor;

	std::string asm_code;
	if (current_flavor == asm_flavor::nasm)
	{
		asm_code = "section .text\nglobal main\nmain:\n";
	}
	else
	{
		asm_code = ".text\n.globl main\nmain:\n";
	}
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