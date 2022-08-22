#include "Offset.h"

#include "llvm/ADT/StringRef.h"

#include <map>

using namespace llvm;


Offset :: Offset() : TidOffset {-1, -1, -1}, BidOffset {-1, -1, -1} {}


// Handle Ops
void Offset :: op_add(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] + b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] + b.BidOffset[i];
	}
}

void Offset :: op_sub(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] - b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] - b.BidOffset[i];
	}
}

void Offset :: op_mul(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] * b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] * b.BidOffset[i];
	}
}

void Offset :: op_div(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] / b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] / b.BidOffset[i];
	}
}

void Offset :: op_rem(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] % b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] % b.BidOffset[i];
	}
}

void Offset :: op_shl(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] << b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] << b.BidOffset[i];
	}
}

void Offset :: op_shr(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] >> b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] >> b.BidOffset[i];
	}
}

void Offset :: op_or(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] | b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] | b.BidOffset[i];
	}
}

void Offset :: op_and(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] & b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] & b.BidOffset[i];
	}
}

void Offset :: op_xor(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i] ^ b.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i] ^ b.BidOffset[i];
	}
}


// void Offset :: op_call(Offset a) {
//
// 	for (int i = 0; i < 3; i++) {
// 		this->TidOffset[i] = a.TidOffset[i];
// 		this->BidOffset[i] = a.BidOffset[i];
// 	}
// }

void Offset :: op_phi(Offset a, Offset b) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i];
	}
}

void Offset :: op_pass_up(Offset a) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = a.TidOffset[i];
		this->BidOffset[i] = a.BidOffset[i];
	}
}

// Handle Values
void Offset :: val_const_int(int val) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = val;
		this->BidOffset[i] = val;
	}
}

void Offset :: val_cuda_reg(StringRef call_str) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = 0;
		this->BidOffset[i] = 0;
	}

	std::map<char, unsigned int> char_map {{'x', 0}, {'y', 1}, {'z', 2}};

	std::pair<StringRef, StringRef> tmp = call_str.split('.');

	unsigned int dim = char_map[tmp.second.front()];

	if (tmp.first.equals("tid")) { // Thread Id

		this->TidOffset[dim] = 1;

	} else if (tmp.first.equals("ctaid")) { // Block Id

		this->BidOffset[dim] = 1;

	} else if (tmp.first.equals("ntid")) { // Block Dim

		if (dim <= 1 ) {
			this->BidOffset[dim] = 256;
		} else {
			this->BidOffset[dim] = 32;
		}

	} else if (tmp.first.equals("nctaid")) { // Grid Dim

		// this->BidOffset[char_map[tmp.second.front()]] = 256;

	}

}

void Offset :: val_inc() {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = 0;
		this->BidOffset[i] = 0;
	}
}

void Offset :: val_arg() {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] = 0;
		this->BidOffset[i] = 0;
	}
}

void Offset :: mul_by_dep(int* tid_dep, int* bid_dep) {

	for (int i = 0; i < 3; i++) {
		this->TidOffset[i] *= tid_dep[i];
		this->BidOffset[i] *= bid_dep[i];
	}
}


// Utlity
std::string Offset :: to_string() {

	std::string str = "TidOffset[";
	for (int& offset : this->TidOffset) {
		str.append(std::to_string(offset));
		str.append(", ");
	}
	str.pop_back();
	str.pop_back();

	str.append("]\nBidOffset[");
	for (int& offset : this->BidOffset) {
		str.append(std::to_string(offset));
		str.append(", ");
	}
	str.pop_back();
	str.pop_back();
	str.append("]\n");

	return str;
}
