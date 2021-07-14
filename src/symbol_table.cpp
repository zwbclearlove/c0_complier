//
// Created by zwb on 2020/11/10.
//

#include "symbol_table.h"

std::map<std::string, identifier_info> global_table;
std::map<std::string, identifier_info> partial_table;
std::map<std::string, std::vector<identifier_info>> func_para_list;
std::map<std::string, std::map<std::string, identifier_info>> func_table;

void clear_partial_table(const std::string& func_name) {
    func_table[func_name] = partial_table;
    partial_table = std::map<std::string, identifier_info>();
}

identifier_info create(std::string name, Basic_type b, Type t, int value, int value2) {
    identifier_info tmp;
    std::transform(name.begin(),name.end(),name.begin(),tolower);
    tmp.name = name;
    tmp.basic_type = b;
    tmp.type = t;
    tmp.value = value;
    tmp.value2 = value2;
    return tmp;
}

identifier_info *look_up_global_ident(std::string identifier_name) {
    if (global_table.count(identifier_name) == 1) {
        return &global_table[identifier_name];
    }
    return nullptr;
}

int insert_global_ident(std::string identifier_name, identifier_info ii) {
    if (look_up_global_ident(identifier_name) == nullptr) {
        identifier_info ij = create(identifier_name,ii.basic_type,ii.type,ii.value,ii.value2);
        ij.location = GLOBAL_;
        global_table[identifier_name] = ij;
        return 0;
    }
    return -1;
}

identifier_info *look_up_partial_ident(std::string identifier_name) {
    if (partial_table.count(identifier_name) == 1) {
        return &partial_table[identifier_name];
    }
    return nullptr;
}

int insert_partial_ident(std::string identifier_name, identifier_info ii) {
    if (look_up_partial_ident(identifier_name) == nullptr) {
        identifier_info ij = create(identifier_name,ii.basic_type,ii.type,ii.value,ii.value2);
        ij.location = PARTIAL_;
        partial_table[identifier_name] = ij;
        return 0;
    }
    return -1;
}

int insert_ident(Location l, std::string identifier_name, identifier_info ii) {
    std::transform(identifier_name.begin(),identifier_name.end(),identifier_name.begin(),tolower);
    if (DEBUG) {
        std::cout << "insert to " << ((l == GLOBAL_) ? "GLOBAL" : "PARTIAL");
        std::cout << " name : " << identifier_name;
        std::cout << " b_type : " << ii.basic_type; // INT_ 0, CHAR_ 1, VOID_ 2
        std::cout << " type : " << ii.type; // CONST_ 0, VAR_ 1, ARRAY_ 2, ARRAY2_ 3, FUNC_ 4, PARAM_ 5, TMP_ 6
        std::cout << " value : " << ii.value;
        std::cout << " value2 : " << ii.value2 << std::endl;
    }

    if (l == GLOBAL_) {
        ii.location = GLOBAL_;
        return insert_global_ident(identifier_name,ii);
    } else {
        ii.location = PARTIAL_;
        return insert_partial_ident(identifier_name,ii);
    }
}

identifier_info *look_up_ident(Location l, std::string identifier_name) {
    std::transform(identifier_name.begin(),identifier_name.end(),identifier_name.begin(),tolower);

    if (l == GLOBAL_) {
        return look_up_global_ident(identifier_name);
    } else if (l == PARTIAL_) {
        return look_up_partial_ident(identifier_name);
    }
    return nullptr;
}