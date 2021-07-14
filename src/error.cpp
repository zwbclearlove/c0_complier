//
// Created by zwb on 2020/11/10.
//

#include "error.h"

bool error_in_program = false;
std::ofstream err;

std::map<ERROR_TYPE, std::string> err_code_rec = {
        {INT_OVERFLOW, "INT_OVERFLOW a"},
        {UNEXPECTED_CHAR_IN_INT, "UNEXPECTED_CHAR_IN_INT a"},
        {INVALID_CHAR_CON, "INVALID_CHAR_CON a"},
        {INVALID_STRING_CON, "INVALID_STRING_CON a"},
        {UNKNOWN_SYMBOL, "UNKNOWN_SYMBOL a"},
        {IDEN_REDEFINITION, "IDEN_REDEFINITION b"},
        {IDEN_NO_DEFINITION, "IDEN_NO_DEFINITION c"},
        {FUNC_NO_DEFINITION, "FUNC_NO_DEFINITION c"},
        {FUNC_PARA_NUM_DISPATCH, "FUNC_PARA_NUM_DISPATCH d"},
        {FUNC_PARA_TYPE_DISPATCH, "FUNC_PARA_TYPE_DISPATCH e"},
        {CONDITION_TYPE_DISPATCH, "CONDITION_TYPE_DISPATCH f"},
        {RETURN_TYPE_DISPATCH, "RETURN_TYPE_DISPATCH h"},
        {EXPECT_RETURN_SENTENCE, "EXPECT_RETURN_SENTENCE h"},
        {RETURN_EXP_IN_VOID_FUNC, "RETURN_EXP_IN_VOID_FUNC g"},
        {ARRAY_INDEX_TYPE_DISPATCH, "ARRAY_INDEX_TYPE_DISPATCH i"},
        {ASSIGN_TO_CONSTANT, "ASSIGN_TO_CONSTANT j"},
        {EXPECT_SEMICN, "EXPECT_SEMICN k"},
        {EXPECT_RPARENT, "EXPECT_RPARENT l"},
        {EXPECT_RBRACK, "EXPECT_RBRACK m"},
        {INVALID_NUM_IN_ARRAY_INIT, "INVALID_NUM_IN_ARRAY_INIT n"},
        {INVALID_TYPE_IN_ARRAY_INIT, "INVALID_TYPE_IN_ARRAY_INIT o"},
        {CONSTANT_TYPE_DISPATCH, "CONSTANT_TYPE_DISPATCH o"},
        {EXPECT_DEFAULT_SENTENCE, "EXPECT_DEFAULT_SENTENCE p"}
};

std::map<ERROR_TYPE, std::string> err_output = {
        {INT_OVERFLOW, "a"},
        {UNEXPECTED_CHAR_IN_INT, "a"},
        {INVALID_CHAR_CON, "a"},
        {INVALID_STRING_CON, "a"},
        {UNKNOWN_SYMBOL, "a"},
        {IDEN_REDEFINITION, "b"},
        {IDEN_NO_DEFINITION, "c"},
        {FUNC_NO_DEFINITION, "c"},
        {FUNC_PARA_NUM_DISPATCH, "d"},
        {FUNC_PARA_TYPE_DISPATCH, "e"},
        {CONDITION_TYPE_DISPATCH, "f"},
        {RETURN_TYPE_DISPATCH, "h"},
        {EXPECT_RETURN_SENTENCE, "h"},
        {RETURN_EXP_IN_VOID_FUNC, "g"},
        {ARRAY_INDEX_TYPE_DISPATCH, "i"},
        {ASSIGN_TO_CONSTANT, "j"},
        {EXPECT_SEMICN, "k"},
        {EXPECT_RPARENT, "l"},
        {EXPECT_RBRACK, "m"},
        {INVALID_NUM_IN_ARRAY_INIT, "n"},
        {INVALID_TYPE_IN_ARRAY_INIT, "o"},
        {CONSTANT_TYPE_DISPATCH, "o"},
        {EXPECT_DEFAULT_SENTENCE, "p"}
};


void log_error(ERROR_TYPE errorType, int lineNO) {
    error_in_program = true;
    std::cout << err_code_rec[errorType] << " in line " << lineNO << std::endl;
    err << lineNO << " " << err_output[errorType] << std::endl;
}
