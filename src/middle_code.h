//
// Created by zwb on 2020/11/10.
//

#ifndef C0_COMPLIER_MIDDLE_CODE_H
#define C0_COMPLIER_MIDDLE_CODE_H

#include "complier.h"
#include "lexical_analyzer.h"
#include "symbol_table.h"
#include "object_code.h"

#define MID_CODE_OUTPUT 1
#define IS_RELATION_MID_CODE(c) (c == MID_CODE::BEQ || c == MID_CODE::BNE || \
                                 c == MID_CODE::BGTZ || c == MID_CODE::BGEZ ||\
                                 c == MID_CODE::BLTZ || c == MID_CODE::BLEZ)



namespace MID_CODE {
    extern int tmp_var_no;

    extern std::ofstream mid_code;

    enum mid_opcode {
        CONST, VAR, PARA, ARRAY, ARRAY2,
        ADD, SUB, MUL, DIV, NEG, READ, WRITE,
        ASSIGN, ARR_ASSIGN, ARR_INIT, USE_ARR, WRITE_NEW_LINE,
        MAIN_START, MAIN_END, GEN_LABEL, JUMP,
        BEQ, BNE, BGEZ, BGTZ, BLEZ, BLTZ,
        FUNC_START, FUNC_END, RETURN, PUSH, CALL, GET_RET,
        DEAD, DELETE_LABEL
    };

    enum mid_opnum_type {
        NONE, CHAR_CON, INT_CON, CHAR_TEMP_VAR, INT_TEMP_VAR, STR_CON,
        CHAR_GLOBAL_VAR, CHAR_GLOBAL_CONST, INT_GLOBAL_VAR, INT_GLOBAL_CONST,
        CHAR_PARTIAL_VAR, CHAR_PARTIAL_CONST, INT_PARTIAL_VAR, INT_PARTIAL_CONST,
        CHAR_PARTIAL_ARRAY, CHAR_GLOBAL_ARRAY, INT_PARTIAL_ARRAY, INT_GLOBAL_ARRAY,
        CHAR_PARTIAL_ARRAY2, CHAR_GLOBAL_ARRAY2, INT_PARTIAL_ARRAY2, INT_GLOBAL_ARRAY2,
        CHAR_PARA, INT_PARA, LABEL, FUNC_NAME, INT_TYPE_FUNC, CHAR_TYPE_FUNC, VOID_TYPE_FUNC,

        CHAR_TEMP_CONST, INT_TEMP_CONST,
    };

    extern std::map<MID_CODE::mid_opcode, std::string> mid_opcode_rec;

    struct mid_opnum {
        mid_opnum_type type;
        int value;
        int value2;
        std::string name;
        mid_opnum(){}
        mid_opnum(mid_opnum_type type, int value, std::string name) {
            this->type = type;
            this->value = value;
            this->name = name;
        }
        mid_opnum(mid_opnum_type type, int value,int value2, std::string name) {
            this->type = type;
            this->value = value;
            this->value2 = value2;
            this->name = name;
        }
        mid_opnum(Location l,Basic_type bt, Type t, int value, std::string name);
        mid_opnum(Location l,Basic_type bt, Type t, int value, int value2, std::string name);
        mid_opnum(identifier_info* ii);
    };

    struct middle_code {
        mid_opcode op_code;
        mid_opnum op_num1;
        mid_opnum op_num2;
        mid_opnum result;
        middle_code() {}
    };

    extern std::vector<middle_code> middle_code_list;
    extern bool need_condition_rec;
    extern std::vector<middle_code> condition_rec;

    mid_opnum create_new_tmp_var(Basic_type tp);
    mid_opnum create_new_label();

    mid_opnum_type get_type(Location l,Basic_type bt, Type t);
    void add_mid_code(MID_CODE::mid_opcode op);
    void add_mid_code(MID_CODE::mid_opcode op,const MID_CODE::mid_opnum& src,const MID_CODE::mid_opnum& dst);
    void add_mid_code(MID_CODE::mid_opcode op,const MID_CODE::mid_opnum& result);
    void add_mid_code(MID_CODE::mid_opcode op, const MID_CODE::mid_opnum& op_num1,
                      const MID_CODE::mid_opnum& op_num2,const MID_CODE::mid_opnum& res);

    void mid_code_output(int mode);
}

#endif //C0_COMPLIER_MIDDLE_CODE_H
