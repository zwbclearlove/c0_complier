//
// Created by zwb on 2020/11/10.
//

#include "middle_code.h"

std::vector<MID_CODE::middle_code> MID_CODE::middle_code_list;
std::ofstream MID_CODE::mid_code;
bool MID_CODE::need_condition_rec;
std::vector<MID_CODE::middle_code> MID_CODE::condition_rec;


std::map<MID_CODE::mid_opcode, std::string> MID_CODE::mid_opcode_rec = {
        {MID_CODE::ADD,"ADD"},
        {MID_CODE::SUB,"SUB"},
        {MID_CODE::MUL,"MUL"},
        {MID_CODE::DIV,"DIV"},
        {MID_CODE::NEG,"NEG"},
        {MID_CODE::READ,"READ"},
        {MID_CODE::WRITE,"WRITE"},
        {MID_CODE::CONST,"CONST"},
        {MID_CODE::VAR,"VAR"},
        {MID_CODE::PARA,"PARA"},
        {MID_CODE::ARRAY,"ARRAY"},
        {MID_CODE::ARRAY2,"ARRAY2"},
        {MID_CODE::ASSIGN,"ASSIGN"},
        {MID_CODE::ARR_ASSIGN,"ARR_ASSIGN"},
        {MID_CODE::ARR_INIT,"ARR_INIT"},
        {MID_CODE::USE_ARR,"USE_ARR"},
        {MID_CODE::MAIN_START,"MAIN_START"},
        {MID_CODE::MAIN_END,"MAIN_END"},
        {MID_CODE::WRITE_NEW_LINE,"WRITE_NEW_LINE"},
        {MID_CODE::GEN_LABEL,"GEN_LABEL"},
        {MID_CODE::JUMP,"JUMP"},
        {MID_CODE::BEQ,"BEQ"},
        {MID_CODE::BNE,"BNE"},
        {MID_CODE::BGEZ,"BGEZ"},
        {MID_CODE::BGTZ, "BGTZ"},
        {MID_CODE::BLEZ, "BLEZ"},
        {MID_CODE::BLTZ, "BLTZ"},
        {MID_CODE::FUNC_START,"FUNC_START"},
        {MID_CODE::FUNC_END,"FUNC_END"},
        {MID_CODE::RETURN,"RETURN"},
        {MID_CODE::PUSH,"PUSH"},
        {MID_CODE::CALL,"CALL"},
        {MID_CODE::GET_RET,"GET_RET"}
};

std::map<MID_CODE::mid_opnum_type, std::string> mid_opnum_type_rec = {
        {MID_CODE::CHAR_CON,"CHAR_CON"},
        {MID_CODE::INT_CON,"INT_CON"},
        {MID_CODE::CHAR_TEMP_VAR,"CHAR_TEMP_VAR"},
        {MID_CODE::INT_TEMP_VAR,"INT_TEMP_VAR"},
        {MID_CODE::STR_CON,"STR_CON"},
        {MID_CODE::CHAR_GLOBAL_VAR,"CHAR_GLOBAL_VAR"},
        {MID_CODE::CHAR_GLOBAL_CONST,"CHAR_GLOBAL_CONST"},
        {MID_CODE::INT_GLOBAL_VAR,"INT_GLOBAL_VAR"},
        {MID_CODE::INT_GLOBAL_CONST,"INT_GLOBAL_CONST"},
        {MID_CODE::CHAR_PARTIAL_VAR,"CHAR_PARTIAL_VAR"},
        {MID_CODE::CHAR_PARTIAL_CONST,"CHAR_PARTIAL_CONST"},
        {MID_CODE::INT_PARTIAL_VAR,"INT_PARTIAL_VAR"},
        {MID_CODE::INT_PARTIAL_CONST,"INT_PARTIAL_CONST"},
        {MID_CODE::INT_PARTIAL_ARRAY,"INT_PARTIAL_ARRAY"},
        {MID_CODE::CHAR_PARTIAL_ARRAY,"CHAR_PARTIAL_ARRAY"},
        {MID_CODE::INT_GLOBAL_ARRAY,"INT_GLOBAL_ARRAY"},
        {MID_CODE::CHAR_GLOBAL_ARRAY,"CHAR_GLOBAL_ARRAY"},
        {MID_CODE::INT_PARTIAL_ARRAY2,"INT_PARTIAL_ARRAY2"},
        {MID_CODE::CHAR_PARTIAL_ARRAY2,"CHAR_PARTIAL_ARRAY2"},
        {MID_CODE::INT_GLOBAL_ARRAY2,"INT_GLOBAL_ARRAY2"},
        {MID_CODE::CHAR_GLOBAL_ARRAY2,"CHAR_GLOBAL_ARRAY2"},
        {MID_CODE::LABEL,"LABEL"},
        {MID_CODE::FUNC_NAME,"FUNC_NAME"},
        {MID_CODE::INT_PARA,"PARA INT"},
        {MID_CODE::CHAR_PARA,"PARA CHAR"}
};

MID_CODE::mid_opnum MID_CODE::create_new_tmp_var(Basic_type tp) {
    static int tmp_var_no = 0;
    identifier_info ii;
    switch (tp) {
        case INT_:
            ii.name = "t" + std::to_string(tmp_var_no++);
            ii.basic_type = INT_;
            ii.type = TMP_;
            ii.value = 0;
            ii.location = PARTIAL_;
            insert_ident(PARTIAL_,ii.name,ii);
            return MID_CODE::mid_opnum(MID_CODE::INT_TEMP_VAR,0,ii.name);
        case CHAR_: case CHAR_VAR: case CHAR_CONS: case CHAR_FUNC:
            ii.name = "t" + std::to_string(tmp_var_no++);
            ii.basic_type = CHAR_;
            ii.type = TMP_;
            ii.value = 0;
            ii.location = PARTIAL_;
            insert_ident(PARTIAL_,ii.name,ii);
            return MID_CODE::mid_opnum(MID_CODE::CHAR_TEMP_VAR,0,ii.name);
        default:
            break;
    }
    return MID_CODE::mid_opnum();
}

MID_CODE::mid_opnum MID_CODE::create_new_label() {
    static int label_no = 0;
    std::string name = "label_" + std::to_string(label_no++);
    return MID_CODE::mid_opnum(MID_CODE::LABEL,0,name);
}

void MID_CODE::add_mid_code(MID_CODE::mid_opcode op) {
    MID_CODE::middle_code middleCode;
    middleCode.op_code = op;
    middleCode.op_num1.type = NONE;
    middleCode.op_num2.type = NONE;
    middleCode.result.type = NONE;
    middle_code_list.push_back(middleCode);
    if (need_condition_rec) {
        condition_rec.push_back(middleCode);
    }
}

void MID_CODE::add_mid_code(MID_CODE::mid_opcode op,const MID_CODE::mid_opnum& src,const MID_CODE::mid_opnum& dst) {
    MID_CODE::middle_code middleCode;
    middleCode.op_code = op;
    middleCode.op_num1 = src;
    middleCode.result = dst;
    middleCode.op_num2.type = NONE;
    middle_code_list.push_back(middleCode);
    if (need_condition_rec) {
        condition_rec.push_back(middleCode);
    }
}

void MID_CODE::add_mid_code(MID_CODE::mid_opcode op,const MID_CODE::mid_opnum& result) {
    MID_CODE::middle_code middleCode;
    middleCode.op_code = op;
    middleCode.result = result;
    middleCode.op_num1.type = NONE;
    middleCode.op_num2.type = NONE;
    middle_code_list.push_back(middleCode);
    if (need_condition_rec) {
        condition_rec.push_back(middleCode);
    }
}

void MID_CODE::add_mid_code(MID_CODE::mid_opcode op, const MID_CODE::mid_opnum& op_num1,
        const MID_CODE::mid_opnum& op_num2,const MID_CODE::mid_opnum& res) {
    MID_CODE::middle_code middleCode;
    middleCode.op_code = op;
    middleCode.op_num1 = op_num1;
    middleCode.op_num2 = op_num2;
    middleCode.result = res;
    middle_code_list.push_back(middleCode);
    if (need_condition_rec) {
        condition_rec.push_back(middleCode);
    }
}

MID_CODE::mid_opnum_type MID_CODE::get_type(Location l,Basic_type bt, Type t) {
    MID_CODE::mid_opnum_type ret = MID_CODE::INT_CON;
    if (l == PARTIAL_) {
        if (bt == INT_) {
            if (t == VAR_) {
                ret = MID_CODE::INT_PARTIAL_VAR;
            } else if (t == CONST_) {
                ret = MID_CODE::INT_PARTIAL_CONST;
            } else if (t == ARRAY_) {
                ret = MID_CODE::INT_PARTIAL_ARRAY;
            } else if (t == ARRAY2_) {
                ret = MID_CODE::INT_PARTIAL_ARRAY2;
            } else if (t == PARAM_) {
                ret = MID_CODE::INT_PARA;
            }
        } else if (bt == CHAR_) {
            if (t == VAR_) {
                ret = MID_CODE::CHAR_PARTIAL_VAR;
            } else if (t == CONST_) {
                ret = MID_CODE::CHAR_PARTIAL_CONST;
            } else if (t == ARRAY_) {
                ret = MID_CODE::CHAR_PARTIAL_ARRAY;
            } else if (t == ARRAY2_) {
                ret = MID_CODE::CHAR_PARTIAL_ARRAY2;
            } else if (t == PARAM_) {
                ret = MID_CODE::CHAR_PARA;
            }
        }
    } else {
        if (bt == INT_) {
            if (t == VAR_) {
                ret = MID_CODE::INT_GLOBAL_VAR;
            } else if (t == CONST_) {
                ret = MID_CODE::INT_GLOBAL_CONST;
            } else if (t == ARRAY_) {
                ret = MID_CODE::INT_GLOBAL_ARRAY;
            } else if (t == ARRAY2_) {
                ret = MID_CODE::INT_GLOBAL_ARRAY2;
            } else if (t == PARAM_) {
                ret = MID_CODE::INT_PARA;
            }
        } else if (bt == CHAR_) {
            if (t == VAR_) {
                ret = MID_CODE::CHAR_GLOBAL_VAR;
            } else if (t == CONST_) {
                ret = MID_CODE::CHAR_GLOBAL_CONST;
            } else if (t == ARRAY_) {
                ret = MID_CODE::CHAR_GLOBAL_ARRAY;
            } else if (t == ARRAY2_) {
                ret = MID_CODE::CHAR_GLOBAL_ARRAY2;
            } else if (t == PARAM_) {
                ret = MID_CODE::CHAR_PARA;
            }
        }
    }
    return ret;
}

MID_CODE::mid_opnum::mid_opnum(Location l,Basic_type bt, Type t, int value, std::string name) {
    this->type = get_type(l,bt,t);
    this->value = value;
    this->value2 = 0;
    this->name = name;
}

MID_CODE::mid_opnum::mid_opnum(Location l,Basic_type bt, Type t, int value, int value2, std::string name) {
    this->type = get_type(l,bt,t);
    this->value = value;
    this->value2 = value2;
    this->name = name;
}


MID_CODE::mid_opnum::mid_opnum(identifier_info *ii) {
    this->type = get_type(ii->location,ii->basic_type,ii->type);
    this->value = 0;
    this->value2 = 0;
    this->name = ii->name;
}

std::string get_info(MID_CODE::mid_opnum opnum) {
    if (IS_CONST(opnum.type)) {
        return std::to_string(opnum.value);
    }
    else {
        return opnum.name;
    }
}

void MID_CODE::mid_code_output(int mode) {
    if (mode == 0) {
        mid_code.open("./middle_code.txt");
    }
    else if (mode == 1) {
        mid_code.open("./opt_middle_code.txt");
    }
    
    for (MID_CODE::middle_code i : middle_code_list) {
        if (i.op_code == MUL || i.op_code == DIV || i.op_code == ADD || i.op_code == SUB) {
            /*mid_code << mid_opcode_rec[i.op_code] << " ";
            
            if (IS_CONST(i.op_num1.type)) {
                mid_code << i.op_num1.value << " ";
            } else {
                mid_code << i.op_num1.name << " ";
            }
            if (IS_CONST(i.op_num2.type)) {
                mid_code << i.op_num2.value << " ";
            } else {
                mid_code << i.op_num2.name << " ";
            }
            if (IS_CONST(i.result.type)) {
                mid_code << i.result.value << " \n";
            } else {
                mid_code << i.result.name << " \n";
            }*/
            std::string op = (i.op_code == ADD) ? " + " :
                (i.op_code == SUB) ? " - " :
                (i.op_code == MUL) ? " * " :
                (i.op_code == DIV) ? " / " : " + ";
            mid_code << get_info(i.result) << " = " << get_info(i.op_num1) << op << get_info(i.op_num2) << "\n";
        } else if (i.op_code == MAIN_END || i.op_code == MAIN_START || i.op_code == WRITE_NEW_LINE) {
            mid_code << mid_opcode_rec[i.op_code] << " \n";
        } else if (i.op_code == FUNC_START || i.op_code == FUNC_END) {
            mid_code << mid_opcode_rec[i.op_code] << " " << i.result.name << " \n";
        } else if (i.op_code == RETURN) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.result) << "\n";
        } else if (i.op_code == ASSIGN || i.op_code == NEG) {
            /*mid_code << mid_opcode_rec[i.op_code] << " ";
            if (IS_CONST(i.op_num1.type)) {
                mid_code << i.op_num1.value << " ";
            } else {
                mid_code << i.op_num1.name << " ";
            }
            if (IS_CONST(i.result.type)) {
                mid_code << i.result.value << " \n";
            } else {
                mid_code << i.result.name << " \n";
            }*/
            std::string op = (i.op_code == ASSIGN) ? "= " :
                (i.op_code == SUB) ? "= -" : "=";
            mid_code << get_info(i.result) << op << get_info(i.op_num1) << "\n";
        } else if (i.op_code == READ || i.op_code == WRITE) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.result) << "\n";
        } else if (i.op_code == CONST || i.op_code == VAR || i.op_code == PARA) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << mid_opnum_type_rec[i.result.type] << " ";
            mid_code << i.result.name << " ";
            mid_code << i.result.value << " \n";
        } else if (i.op_code == ARRAY) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << mid_opnum_type_rec[i.result.type] << " ";
            mid_code << i.result.name << " ";
            mid_code << "size : " << i.result.value << " \n";
        } else if (i.op_code == ARRAY2) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << mid_opnum_type_rec[i.result.type] << " ";
            mid_code << i.result.name << " ";
            mid_code << "size : " << i.result.value << " " << i.result.value2 << " \n";
        } else if (i.op_code == ARR_ASSIGN || i.op_code == ARR_INIT) {
            //mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << i.op_num1.name << "[";
            mid_code << get_info(i.op_num2) << "]";
            mid_code << " = " << get_info(i.result) << std::endl;
        } else if (i.op_code == USE_ARR) {
            //mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.result) << " = ";
            mid_code << i.op_num1.name << "[";
            mid_code << get_info(i.op_num2) << "]";
            mid_code << std::endl;
        } else if (i.op_code == GEN_LABEL) {
            mid_code << i.result.name << " : " << std::endl;
        } else if (i.op_code == JUMP) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << i.result.name << std::endl;
        } else if (i.op_code == BNE || i.op_code == BEQ) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.op_num1) << " ";
            mid_code << get_info(i.op_num2) << " ";
            mid_code << i.result.name << "\n";
        } else if (i.op_code == BLEZ || i.op_code == BLTZ || i.op_code == BGEZ || i.op_code == BGTZ) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.op_num1) << " ";
            mid_code << i.result.name << "\n";
        } else if (i.op_code == PUSH) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.result) << "\n";
        } else if (i.op_code == CALL) {
            mid_code << mid_opcode_rec[i.op_code] << " " << i.result.name << " \n";
        } else if (i.op_code == GET_RET) {
            mid_code << mid_opcode_rec[i.op_code] << " ";
            mid_code << get_info(i.result) << "\n";
        }
    }
    mid_code.close();
}
