//
// Created by zwb on 2020/12/12.
//

#ifndef C0_COMPLIER_OPTIMIZE_H
#define C0_COMPLIER_OPTIMIZE_H

#include "complier.h"
#include "middle_code.h"
#include "object_code.h"

#define OPTIMIZE 0
#define IS_CONST_TYPE(t) (t == MID_CODE::CHAR_CON || t == MID_CODE::INT_CON \
                    || t == MID_CODE::CHAR_GLOBAL_CONST || t == MID_CODE::INT_GLOBAL_CONST \
                    || t == MID_CODE::CHAR_PARTIAL_CONST || t == MID_CODE::INT_PARTIAL_CONST \
                    || t == MID_CODE::CHAR_TEMP_CONST || t == MID_CODE::INT_TEMP_CONST)


namespace OPT {

    void optimize();

    void peep_hole_opt();
    void dag_opt();
    void const_propagation_opt();
    void condition_delete();

}



#endif //C0_COMPLIER_OPTIMIZE_H
