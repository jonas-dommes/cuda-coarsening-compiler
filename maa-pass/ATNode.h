#ifndef ATNODE_H
#define ATNODE_H

#include "InstrStats.h"

#include <llvm/IR/Instructions.h>

#include <utility>

using namespace llvm;

enum class instr_t {
	NONE = 0, ADD, SUB, MUL, DIV, REM, SHL, SHR, OR, AND, XOR, CALL, LOAD, STORE, PHI, GETELEPTR, EXT
};

enum class val_t {
	NONE = 0, ARG, CONST_INT, CUDA_REG
};

const std::string instr_t_str[] = {
	"NONE", "ADD", "SUB", "MUL", "DIV", "REM", "SHL", "SHR", "OR", "AND", "XOR", "CALL", "LOAD", "STORE", "PHI", "GETELEPTR", "EXT", "UNDEF"
};

const std::string val_t_str[] {
	"NONE", "ARG", "CONST_INT", "CUDA_REG"
};


class ATNode {
public:

	Value* value;
	InstrStats* instr_stats;
	ATNode* parent;
	std::vector<ATNode*> children;
	instr_t instr_type;
	val_t value_type;
	int int_val;
	StringRef name;

	// Constructor
	ATNode(Value* value, InstrStats* instr_stats, ATNode* parent);

// METHODS
	void insertInstruction(Instruction* I);
	void handleCallStr();
	void set_instr_type(Instruction* I);

	void printErrsNode();
	std::string access_pattern_to_string();
	std::string access_pattern_instr();
	std::string access_patter_value();
	std::string op_to_string();
	std::string val_t_to_string();
	std::string instr_t_to_string();

};


#endif
