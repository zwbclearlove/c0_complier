//
// Created by zwb on 2020/11/10.
//

#ifndef C0_COMPLIER_SYNTACTIC_ANALYZER_H
#define C0_COMPLIER_SYNTACTIC_ANALYZER_H

#include "complier.h"
#include "lexical_analyzer.h"
#include "symbol_table.h"
#include "middle_code.h"
#include "optimize.h"

#define SYNTACTIC_OUTPUT 0

#define IS_TYPE_TK(s) (s == LEX::INT_TK || s == LEX::CHAR_TK)
#define IS_IDEN_TK(s) (s == LEX::IDENTIFIER_CON)
#define IS_SIGN_TK(s) (s == LEX::PLUS || s == LEX::MINUS)
#define IS_MULT_TK(s) (s == LEX::MULTI || s == LEX::DIV)
#define IS_INT_CON(s) (s == LEX::INT_CON)
#define IS_CHAR_CONS(s) (s == LEX::CHAR_CON)
#define IS_LEFT_SB(s) (s == LEX::LPARENT)
#define IS_FUNC_HEAD(s) (s == LEX::INT_TK || s == LEX::CHAR_TK || s == LEX::VOID_TK)
#define IS_VOID_TK(s) (s == LEX::VOID_TK)
#define IS_MAIN_TK(s) (s == LEX::MAIN_TK)
#define IS_SENTENCE_FIRST(s) (s == LEX::FOR_TK  || s == LEX::WHILE_TK || s == LEX::IF_TK || s == LEX::IDENTIFIER_CON || s == LEX::SCANF_TK ||  \
                                s == LEX::PRINTF_TK || s == LEX::SWITCH_TK || s == LEX::SEMICN || s == LEX::RETURN_TK || s == LEX::LBRACE)
#define IS_REL_SIGN(s) (s == LEX::LSS || s == LEX::LEQ || s == LEX::GRE || s == LEX::GEQ || s == LEX::EQL || s== LEX::NEQ)
#define IS_EXPS_FIRST(s) (s == LEX::IDENTIFIER_CON || s == LEX::PLUS || s == LEX::MINUS || s == LEX::INT_CON || s == LEX::CHAR_CON || s == LEX::LPARENT)

void program();
void assign_sentence();
Basic_type expression(MID_CODE::mid_opnum &opn);


#endif //C0_COMPLIER_SYNTACTIC_ANALYZER_H
