//
// Created by zwb on 2020/12/12.
//

#include "optimize.h"

bool propagation_flag = false;

void OPT::optimize() {
	std::cout << "-----------optimize part---------\n";
	do {
		const_propagation_opt();
	} while (propagation_flag);
	condition_delete();
	
}

std::map<std::string, MID_CODE::mid_opnum> change_rec;

void OPT::const_propagation_opt() {
	propagation_flag = false;
	for (auto& mc : MID_CODE::middle_code_list) {
		if (mc.op_code == MID_CODE::DEAD) continue;
		if (mc.op_code == MID_CODE::ADD || mc.op_code == MID_CODE::SUB ||
			mc.op_code == MID_CODE::MUL || mc.op_code == MID_CODE::DIV) {
			MID_CODE::mid_opcode op = mc.op_code;
			bool const1 = IS_CONST(mc.op_num1.type);
			bool const2 = IS_CONST(mc.op_num2.type);
			int val1 = mc.op_num1.value;
			int val2 = mc.op_num2.value;
			if (const1 && const2) {
				int val = (op == MID_CODE::ADD) ? val1 + val2 :
					(op == MID_CODE::SUB) ? val1 - val2 :
					(op == MID_CODE::MUL) ? val1 * val2 :
					(op == MID_CODE::DIV) ? val1 / val2 : 0;
				if (mc.result.type == MID_CODE::INT_TEMP_VAR) {
					mc.result.type = MID_CODE::INT_TEMP_CONST;
				}
				else if (mc.result.type == MID_CODE::CHAR_TEMP_VAR) {
					mc.result.type = MID_CODE::CHAR_TEMP_CONST;
				}
				mc.result.value = val;
				MID_CODE::mid_opnum rec(mc.result.type,mc.result.value,mc.result.name);
				change_rec[rec.name] = rec;
				mc.op_code = MID_CODE::DEAD;
				propagation_flag = true;
			}
			else if (!const1 && const2) {
				if ((op == MID_CODE::SUB && val2 == 0) ||
					((op == MID_CODE::MUL || op == MID_CODE::DIV) && val2 == 1)) {
					mc.op_code = MID_CODE::DEAD;
					mc.op_num2.value = 0;
					MID_CODE::mid_opnum rec(mc.op_num1.type, mc.op_num1.value, mc.op_num1.name);
					change_rec[mc.result.name] = rec;
					propagation_flag = true;
				}
			}
			else if (const1 && !const2) {
				if (op == MID_CODE::MUL && val1 == 1) {
					mc.op_code = MID_CODE::DEAD;
					mc.op_num1.value = 0;
					propagation_flag = true;
					MID_CODE::mid_opnum rec(mc.op_num2.type, mc.op_num2.value, mc.op_num2.name);
					change_rec[mc.result.name] = rec;
				}
				else if ((op == MID_CODE::MUL || op == MID_CODE::DIV) && val1 == 0) {
					mc.op_code = MID_CODE::DEAD;
					mc.op_num1.value = 0;
					propagation_flag = true;
					MID_CODE::mid_opnum rec(MID_CODE::INT_CON, 0, "");
					change_rec[mc.result.name] = rec;
				}
				
			}
		}
	}
	for (auto& mc : MID_CODE::middle_code_list) {
		if (mc.op_num1.type != MID_CODE::NONE && change_rec.count(mc.op_num1.name) == 1) {
			mc.op_num1 = change_rec[mc.op_num1.name];
		}
		if (mc.op_num2.type != MID_CODE::NONE && change_rec.count(mc.op_num2.name) == 1) {
			mc.op_num2 = change_rec[mc.op_num2.name];
		}
		if (mc.result.type != MID_CODE::NONE && change_rec.count(mc.result.name) == 1) {
			mc.result = change_rec[mc.result.name];
		}
	}
	//change_rec.clear();
	
}

void OPT::condition_delete() {
	for (auto& mc : MID_CODE::middle_code_list) {
		MID_CODE::mid_opcode op = mc.op_code;
		bool const1 = IS_CONST(mc.op_num1.type);
		bool const2 = IS_CONST(mc.op_num2.type);
		int val1 = mc.op_num1.value;
		int val2 = mc.op_num2.value;
		if (op == MID_CODE::BEQ) {
			if (const1 && const2) {
				if (val1 == val2) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
					mc.op_num2.type = MID_CODE::NONE;
					
				}
			}
		}
		else if (op == MID_CODE::BNE) {
			if (const1 && const2) {
				if (val1 != val2) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
					mc.op_num2.type = MID_CODE::NONE;
				}
			}
		}
		else if (op == MID_CODE::BGEZ) {
			if (const1) {
				if (val1 >= 0) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
				}
			}
		}
		else if (op == MID_CODE::BGTZ) {
			if (const1) {
				if (val1 > 0) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
				}
			}
		}
		else if (op == MID_CODE::BLEZ) {
			if (const1) {
				if (val1 <= 0) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
				}
			}
		}
		else if (op == MID_CODE::BLTZ) {
			if (const1) {
				if (val1 < 0) {
					mc.op_code = MID_CODE::DELETE_LABEL;
					mc.op_num1.type = MID_CODE::NONE;
				}
			}
		}
	}
	for (auto it = MID_CODE::middle_code_list.begin(); it != MID_CODE::middle_code_list.end();it++) {
		MID_CODE::mid_opcode op = (*it).op_code;
		if (op == MID_CODE::DELETE_LABEL) {
			auto label = (*it).result.name;
			for (auto itt = it; itt != MID_CODE::middle_code_list.end(); itt++) {
				auto mc = (*itt);
				if (mc.op_code == MID_CODE::GEN_LABEL && mc.result.name == label) {
					(*itt).op_code = MID_CODE::DEAD;
					break;
				}
				(*itt).op_code = MID_CODE::DEAD;
			}
		}
	}
}