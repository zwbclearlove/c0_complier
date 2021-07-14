//
// Created by zwb on 2020/11/10.
//

#ifndef C0_COMPLIER_LEXICAL_ANALYZER_H
#define C0_COMPLIER_LEXICAL_ANALYZER_H

#include "complier.h"
#include "error.h"

#define LEXICAL_OUTPUT 1

#define IS_ALPHA(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c=='_'))
#define IS_WHITE(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r')
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_CHAR_CON(c) (c == '\'')
#define IS_STR_CON(c) (c == '"')
#define IS_CHAR(c) (c == '+' || c == '-' || c == '*' || c == '/' || IS_ALPHA(c) || IS_DIGIT(c))
#define IS_STR_CHAR(c) (c >= 32 && c <= 126)
#define IS_SINGLE_SEMI(c) (c == '+' || c == '-' || c == '*' || c == '/' || c == ';' \
                        || c == ':' || c == ',' || c == '(' || c == ')' \
                        || c == '{' || c == '}' || c == '[' || c == ']')
#define IS_SEMI_START(c) (c == '<' || c == '>' || c == '=')

namespace LEX {
    enum Symbol {
        OTHER, IDENTIFIER_CON, INT_CON, CHAR_CON, STR_CON,
        CONST_TK, INT_TK,CHAR_TK, VOID_TK, MAIN_TK, IF_TK,
        ELSE_TK,SWITCH_TK, CASE_TK, DEFAULT_TK, WHILE_TK,
        FOR_TK, SCANF_TK, PRINTF_TK, RETURN_TK, PLUS, MINUS,
        MULTI, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, COLON,
        ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK,
        RBRACK, LBRACE, RBRACE, UNKNOWN_SYM
    };

    typedef struct {
        Symbol symbol;
        std::string token;
        int line_no;
    } word_info;

    extern std::ifstream file;
    extern std::ofstream out;
    extern char curr_char;
    extern std::string token;
    extern Symbol curr_symbol;
    extern int curr_symbol_line;
    extern int end_of_file;
    extern unsigned int num;

    int get_symbol(bool is_pre_read);
    int get_symbol();
    Symbol pre_read();
    std::vector<Symbol> pre_read(int pre_read_num);
    std::string to_string(Symbol symbol);
}


#endif //C0_COMPLIER_LEXICAL_ANALYZER_H
