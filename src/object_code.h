//
// Created by zwb on 2020/11/10.
//

#ifndef C0_COMPLIER_OBJECT_CODE_H
#define C0_COMPLIER_OBJECT_CODE_H

#include "complier.h"
#include "symbol_table.h"
#include "middle_code.h"

#define IS_CONST(t) (t == MID_CODE::CHAR_CON || t == MID_CODE::INT_CON \
                    || t == MID_CODE::CHAR_GLOBAL_CONST || t == MID_CODE::INT_GLOBAL_CONST \
                    || t == MID_CODE::CHAR_PARTIAL_CONST || t == MID_CODE::INT_PARTIAL_CONST \
                    || t == MID_CODE::CHAR_TEMP_CONST || t == MID_CODE::INT_TEMP_CONST)


namespace OC {
    enum Reg {
        ZERO, v0, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, s0, s1, s2, s3, s4, s5, s6, s7, gp, sp, ra, a0, a1, a2, a3, NONE
    };

    struct Record {
        Reg reg;
        std::vector<Reg> func_used;
        bool is_global;
        int gp_offset;
        int sp_offset;
        int stack_size;
        Record(Reg _reg = NONE, bool ig = false, int go = 0, int so = 0, int ss = 0):
                reg(_reg), is_global(ig), gp_offset(go), sp_offset(so), stack_size(ss){}
    };

    extern std::map<std::string, Record*> reg_map;

    extern std::ofstream mips;
    extern std::vector<std::string> data_section;
    extern std::vector<std::string> text_section;
    extern std::vector<std::string> init_section;

    void init();
    void translate();
    void tmp_reg_refresh();
    void tmp_restore();
    void refresh(Reg r);
    void mips_output(int mode);
}




#endif //C0_COMPLIER_OBJECT_CODE_H
