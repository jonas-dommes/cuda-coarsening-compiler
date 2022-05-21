#include <iostream>
#include <algorithm>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Instructions.h>

#include "../llvm-rpc-passes/Common.h"
#include "../llvm-rpc-passes/GridAnalysisPass.h"

#include "FunctionStats.h"

#include "InstrStats.h"

using namespace llvm;

void InstrStats::analyseInstr(Instruction *I, LoopInfo *LI, struct dependance_t dep_calls) {


	if (isa<StoreInst>(I)) {

		this->addr = I->getOperand(1);
		this->is_store = true;

	} else if (isa<LoadInst>(I)) {

		this->addr = I->getOperand(0);
		this->is_load = true;
	}

	this->getLoopDepth(I, LI);
	this->analyseDependence(I, dep_calls);
}

void InstrStats::printInstrStats() {

	if (this->is_load) {
		printf("\t\tLoad ");
	} else if (this->is_store) {
		printf("\t\tStore");
	}
	if (this->is_tid_dep) {
		printf("\tTID");
	}
	if (this->is_bid_dep) {
		printf("\tBID");
	}
	if (this->is_blocksize_dep) {
		printf("\tBSD");
	}
	if (this->is_gridsize_dep) {
		printf("\tGSD");
	}
	printf("\n");

	printf("\t\tLoop Depth: %d\n", this->loop_depth);
	printf("\t\tAddr: %p\n", this->addr);

}

unsigned int InstrStats::getLoopDepth(Instruction *I, LoopInfo *LI) {

	this->loop_depth = LI->getLoopDepth((I->getParent()));

	return this->loop_depth;
}

void InstrStats::analyseDependence(Instruction *I, struct dependance_t dep_calls) {

	std::set<Instruction*> workset;
	std::set<Instruction*> doneset;

	// errs() << "\nStarting Analysis of:\n\t" << *I << "\n";

	// Fill Addr Operand of load/store instr in worklist
	if (this->is_load) {
		workset.insert(cast<Instruction>(I->getOperand(0)));
			// errs() << *cast<Instruction>(I->getOperand(0)) << "\n";
	} else if (this->is_store) {
		workset.insert(cast<Instruction>(I->getOperand(1)));
	}

	// while there are instr in workset
	while (!workset.empty()) {
		// get and delete first instr from workset, add to doneset
		Instruction* instr = *workset.begin();
		workset.erase(instr);
		doneset.insert(instr);

		// check if instr is in dep_calls
		if (dep_calls.tid_calls.count(instr) > 0) {

			this->is_tid_dep = true;
			// errs() << "Is in tid_calls: " << *instr << "\n";
		}
		if (dep_calls.bid_calls.count(instr) > 0) {

			this->is_bid_dep = true;
			// errs() << "Is in bid_calls: " << *instr << "\n";
		}
		if (dep_calls.blocksize_calls.count(instr) > 0) {

			this->is_blocksize_dep = true;
			// errs() << "Is in blocksize_calls: " << *instr << "\n";
		}
		if (dep_calls.gridsize_calls.count(instr) > 0) {

			this->is_gridsize_dep = true;
			// errs() << "Is in gridsize_calls: " << *instr << "\n";
		}

		// errs() << *instr << " has Operands:\n";

		// add operands to workset if not in doneset
		for (auto &op : instr->operands()) {
			Instruction* op_instr = dyn_cast<Instruction>(op);

			// Continue if Operand is no Instruction
			if (op_instr == NULL) {
				continue;
			}

			if (doneset.count(op_instr) == 0) {

				workset.insert(op_instr);
				// errs() << "\t" << *op_instr << "\n";
			}
		}
		// handle phis?
	}

	// if (!(this->is_tid_dep || this->is_bid_dep || this->is_blocksize_dep || this->is_gridsize_dep)) {
	// 	errs() << "\n-----------------Instruction was not relevant-----------------\n" << *I << "\n\n";
	// }
}
