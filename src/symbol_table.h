//
// Created by zwb on 2020/11/10.
//

#ifndef C0_COMPLIER_SYMBOL_TABLE_H
#define C0_COMPLIER_SYMBOL_TABLE_H

#include "complier.h"
#include "lexical_analyzer.h"

enum Location {
    GLOBAL_, PARTIAL_
};

enum Basic_type {
    INT_, CHAR_, VOID_, CHAR_VAR, CHAR_FUNC, CHAR_CONS
};

enum Type {
    CONST_, VAR_, ARRAY_, ARRAY2_, FUNC_, PARAM_, TMP_
};

typedef struct {
    std::string name;
    Basic_type basic_type; // int char void
    Type type; // const, var, array, func, param
    int value; // const -> value, var -> null, array -> length, func -> para_num
    // array2 -> 1 size
    int value2; // array2 -> 2 size
    Location location;
} identifier_info;

extern std::map<std::string, identifier_info> global_table;
extern std::map<std::string, identifier_info> partial_table;
extern std::map<std::string, std::vector<identifier_info>> func_para_list;
extern std::map<std::string, std::map<std::string, identifier_info>> func_table;

void clear_partial_table(const std::string& func_name);

identifier_info create(std::string name, Basic_type b, Type t, int value, int value2);

int insert_ident(Location l, std::string identifier_name, identifier_info ii);
identifier_info *look_up_ident(Location l, std::string identifier_name);

int insert_global_ident(std::string identifier_name, identifier_info ii);
identifier_info *look_up_global_ident(std::string identifier_name);

int insert_partial_ident(std::string identifier_name, identifier_info ii);
identifier_info *look_up_partial_ident(std::string identifier_name);

#endif //C0_COMPLIER_SYMBOL_TABLE_H
