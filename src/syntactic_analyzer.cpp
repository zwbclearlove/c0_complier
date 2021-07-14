//
// Created by zwb on 2020/10/26.
//

#include "syntactic_analyzer.h"

identifier_info current_function;
bool curr_func_with_ret;

void sentence();
void sentence_list();
Basic_type call_return_func_sentence();

unsigned int unsigned_integer() {
    LEX::Symbol first_symbol = LEX::pre_read(1)[0];
    unsigned int ret = 0;
    if (IS_INT_CON(first_symbol)) {
        LEX::get_symbol(); // unsigned integer
        ret = LEX::num;
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<unsigned integer>" << std::endl;
        LEX::out << "<无符号整数>" << std::endl;
    }

    return ret;
}

int integer() {
    LEX::Symbol first_symbol = LEX::pre_read(1)[0];
    int sign = 1;
    int ret = 1;
    if (IS_SIGN_TK(first_symbol)) {
        LEX::get_symbol(); // + / -
        sign = (LEX::curr_symbol == LEX::PLUS) ? 1 : -1;
        ret = (int)unsigned_integer();
    } else if (IS_INT_CON(first_symbol)) {
        ret = (int)unsigned_integer();
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<integer>" << std::endl;
        LEX::out << "<整数>" << std::endl;
    }

    return sign * ret;
}

char character() {
    LEX::Symbol first_symbol = LEX::pre_read(1)[0];
    char ret = '\0';
    if (IS_CHAR_CONS(first_symbol)) {
        LEX::get_symbol(); // character
        ret = LEX::token[0];
    }
    //std::cout << "<character>" << std::endl;
    return ret;
}

std::string a_string() {
    std::string ret;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::STR_CON) {
        LEX::get_symbol();
        ret = LEX::token;
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<a_string>" << std::endl;
        LEX::out << "<字符串>" << std::endl;
    }

    return ret;
}

Basic_type constant(int& res) {
    Basic_type ret = INT_;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::INT_CON || IS_SIGN_TK(symbols[0])) {
        res = integer();
        ret = INT_;
    } else if (symbols[0] == LEX::CHAR_CON) {
        res = character();
        ret = CHAR_;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<constant>" << std::endl;
        LEX::out << "<常量>" << std::endl;
    }

    return ret;
}

Basic_type factor(MID_CODE::mid_opnum &opn) {
    Basic_type ret = INT_;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::IDENTIFIER_CON) {
        symbols = LEX::pre_read(2);
        if (symbols.empty()) {} //error
        if (symbols[1] == LEX::LPARENT) {
            ret = call_return_func_sentence();
            MID_CODE::mid_opnum op = MID_CODE::create_new_tmp_var(ret);
            MID_CODE::add_mid_code(MID_CODE::GET_RET,op);
            opn.type = op.type;
            opn.name = op.name;
            opn.value = op.value;
        } else if (symbols[1] == LEX::LBRACK) {
            LEX::get_symbol(); // iden
            identifier_info *p1 = look_up_global_ident(LEX::token);
            identifier_info *p2 = look_up_partial_ident(LEX::token);
            if (p1 == nullptr && p2 == nullptr) {
                log_error(IDEN_NO_DEFINITION,LEX::curr_symbol_line);
            } else if (p2 != nullptr) {
                p1 = p2;
            }
            MID_CODE::mid_opnum dst(p1);
            ret = (p1->basic_type == INT_) ? INT_ : CHAR_VAR;
            LEX::get_symbol(); // [
            MID_CODE::mid_opnum index1;
            Basic_type bt = expression(index1);//todo
            if (bt != INT_) {
                log_error(ARRAY_INDEX_TYPE_DISPATCH, LEX::curr_symbol_line);
            }
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::RBRACK) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
            }
            //get_symbol(); // ]
            symbols = LEX::pre_read(1);
            if (symbols.empty()) {} //error
            if (symbols[0] == LEX::LBRACK) { // ARRAY2
                LEX::get_symbol(); // [
                MID_CODE::mid_opnum index2;
                bt = expression(index2);//todo
                if (bt != INT_) {
                    log_error(ARRAY_INDEX_TYPE_DISPATCH, LEX::curr_symbol_line);
                }
                symbols = LEX::pre_read(1);
                if (symbols[0] == LEX::RBRACK) {
                    LEX::get_symbol();
                } else {
                    log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
                }
                //get_symbol(); // ]
                MID_CODE::mid_opnum tmp = MID_CODE::create_new_tmp_var(p1->basic_type);
                MID_CODE::mid_opnum index = MID_CODE::create_new_tmp_var(INT_);
                MID_CODE::mid_opnum size = MID_CODE::mid_opnum(MID_CODE::INT_CON,p1->value2,"");
                MID_CODE::mid_opnum re = MID_CODE::create_new_tmp_var(p1->basic_type);
                MID_CODE::add_mid_code(MID_CODE::MUL,index1,size,tmp);
                MID_CODE::add_mid_code(MID_CODE::ADD,tmp,index2,index);
                MID_CODE::add_mid_code(MID_CODE::USE_ARR,dst,index,re);
                opn.type = re.type;
                opn.value = re.value;
                opn.name = re.name;
                opn.value2 = re.value2;
            } else { //ARRAY
                MID_CODE::mid_opnum tmp = MID_CODE::create_new_tmp_var(p1->basic_type);
                MID_CODE::add_mid_code(MID_CODE::USE_ARR,dst,index1,tmp);
                opn.type = tmp.type;
                opn.value = tmp.value;
                opn.name = tmp.name;
                opn.value2 = tmp.value2;
            }
        } else {
            LEX::get_symbol(); //IDEN
            identifier_info *p1 = look_up_global_ident(LEX::token);
            identifier_info *p2 = look_up_partial_ident(LEX::token);
            if (p1 == nullptr && p2 == nullptr) {
                log_error(IDEN_NO_DEFINITION,LEX::curr_symbol_line);
            } else if (p2 != nullptr) {
                p1 = p2;
            }
            opn.type = MID_CODE::get_type(p1->location,p1->basic_type,p1->type);
            opn.value = p1->value;
            opn.value2 = p1->value2;
            opn.name = p1->name;
            ret = (p1->basic_type == INT_) ? INT_ : CHAR_;
        }
    } else if (IS_SIGN_TK(symbols[0]) || IS_INT_CON(symbols[0])) {
        //int t = integer();
        opn.type = MID_CODE::INT_CON;
        opn.value = integer();
        ret = INT_;
    } else if (IS_CHAR_CONS(symbols[0])) {
        //character();
        opn.type = MID_CODE::CHAR_CON;
        opn.value = character();
        ret = CHAR_CONS;
    } else if (symbols[0] == LEX::LPARENT) {
        LEX::get_symbol(); // (
        MID_CODE::mid_opnum op;
        expression(op);
        if (op.type == MID_CODE::CHAR_CON) opn.type = MID_CODE::INT_CON;
        else if (op.type == MID_CODE::CHAR_TEMP_VAR) opn.type = MID_CODE::INT_TEMP_VAR;
        else if (op.type == MID_CODE::CHAR_PARA) opn.type = MID_CODE::INT_PARA;
        else if (op.type == MID_CODE::CHAR_PARTIAL_VAR) opn.type = MID_CODE::INT_PARTIAL_VAR;
        else if (op.type == MID_CODE::CHAR_GLOBAL_VAR) opn.type = MID_CODE::INT_GLOBAL_VAR;
        else if (op.type == MID_CODE::CHAR_PARTIAL_CONST) opn.type = MID_CODE::INT_PARTIAL_CONST;
        else if (op.type == MID_CODE::CHAR_GLOBAL_CONST) opn.type = MID_CODE::INT_GLOBAL_CONST;
        else opn.type = op.type;
        opn.name = op.name;
        opn.value = op.value;
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
        }
        //get_symbol(); // )
        ret = INT_;
    } else {
        //error
    }
    std::cout << "factor print " << opn.value << "\n";
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<factor>" << std::endl;
        LEX::out << "<因子>" << std::endl;
    }

    return ret;
}

Basic_type term(MID_CODE::mid_opnum &opn) {
    Basic_type ret = INT_;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (IS_EXPS_FIRST(symbols[0])) {
        MID_CODE::mid_opnum factor1;
        ret = factor(factor1);
        while (true) {
            symbols = LEX::pre_read(1);
            if (symbols.empty()) break;
            if (IS_MULT_TK(symbols[0])) {
                LEX::get_symbol(); // * /
                LEX::Symbol sign = LEX::curr_symbol;
                MID_CODE::mid_opnum factor2;
                Basic_type bt = factor(factor2);
                /*if (bt == CHAR_VAR && ret == CHAR_VAR) {
                    ret = CHAR_VAR;
                } else {
                    ret = INT_;
                }*/
                ret = INT_;
                //todo : add one middle code new = factor1 * / factor2
                MID_CODE::mid_opnum res = MID_CODE::create_new_tmp_var(ret);
                MID_CODE::mid_opcode sign_code = MID_CODE::MUL;
                if (sign == LEX::MULTI) sign_code = MID_CODE::MUL;
                else if (sign == LEX::DIV) sign_code = MID_CODE::DIV;
                MID_CODE::add_mid_code(sign_code,factor1,factor2,res);
                factor1 = res;
            } else break;
        }
        opn.name = factor1.name;
        opn.type = factor1.type;
        opn.value = factor1.value;
    } else {
        //error
    }
    std::cout << "term print " << opn.value << "\n";
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<term>" << std::endl;
        LEX::out << "<项>" << std::endl;
    }

    return ret;
}

Basic_type expression(MID_CODE::mid_opnum &opn) {
    Basic_type ret = INT_;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (IS_EXPS_FIRST(symbols[0])) {
        bool is_neg = false;
        if (IS_SIGN_TK(symbols[0])) {
            LEX::get_symbol();
            if (LEX::curr_symbol == LEX::MINUS) {
                is_neg = true;
            }
        }
        MID_CODE::mid_opnum term1;
        ret = term(term1);
        if (is_neg) {
            MID_CODE::mid_opnum neg = MID_CODE::create_new_tmp_var(INT_);
            MID_CODE::add_mid_code(MID_CODE::NEG,term1,neg);
            term1 = neg;
        }
        while (true) {
            symbols = LEX::pre_read(1);
            if (symbols.empty()) break;
            if (IS_SIGN_TK(symbols[0])) {
                LEX::get_symbol(); // + -
                LEX::Symbol sign = LEX::curr_symbol;
                MID_CODE::mid_opnum term2;
                Basic_type bt = term(term2);
                /*if (bt == CHAR_VAR && ret == CHAR_VAR) {
                    ret = CHAR_VAR;
                } else {
                    ret = INT_;
                }*/
                ret = INT_;
                //todo : add one middle code new = term1 + - term2
                MID_CODE::mid_opnum res = MID_CODE::create_new_tmp_var(ret);
                MID_CODE::mid_opcode sign_code = MID_CODE::ADD;
                if (sign == LEX::PLUS) sign_code = MID_CODE::ADD;
                else if (sign == LEX::MINUS) sign_code = MID_CODE::SUB;
                MID_CODE::add_mid_code(sign_code,term1,term2,res);
                term1 = res;
            } else break;
        }
        opn.name = term1.name;
        opn.type = term1.type;
        opn.value = term1.value;
    } else {
        //error
    }
    if (ret == CHAR_VAR || ret == CHAR_FUNC || ret == CHAR_CONS) {
        ret = CHAR_;
    }

    std::cout << "expression print " << opn.value << "\n";
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<expression>" << std::endl;
        LEX::out << "<表达式>" << std::endl;
    }

    if (DEBUG) {
        std::cout << "expression type : " << ret << std::endl;
    }
    return ret;
}

void constant_assign(LEX::Symbol symbol, Location l) {
    LEX::Symbol first_symbol = LEX::pre_read(1)[0];
    if (IS_IDEN_TK(first_symbol)) {
        LEX::get_symbol(); //identifier
        identifier_info tmp;
        tmp.name = LEX::token;
        tmp.type = CONST_;
        tmp.location = l;
        first_symbol = LEX::pre_read(1)[0];
        if (first_symbol == LEX::ASSIGN) {
            LEX::get_symbol(); // =
            if (symbol == LEX::INT_TK) {
                tmp.basic_type = INT_;
                tmp.value = integer();
            } else if (symbol == LEX::CHAR_TK) {
                tmp.basic_type = CHAR_;
                tmp.value = character();
            }
            if (insert_ident(l,tmp.name,tmp) == -1) {
                log_error(IDEN_REDEFINITION,LEX::curr_symbol_line);
            }
            MID_CODE::add_mid_code(MID_CODE::CONST,
                                   MID_CODE::mid_opnum(l,tmp.basic_type,tmp.type,tmp.value,tmp.name));
        }
    }
}

void constant_definition(Location l) {
    LEX::Symbol first_symbol = LEX::pre_read(1)[0];
    LEX::Symbol type = first_symbol;
    if (IS_TYPE_TK(first_symbol)) {
        LEX::get_symbol(); // int / char
        constant_assign(type, l);
        while (true) {
            first_symbol = LEX::pre_read(1)[0];
            if (first_symbol == LEX::COMMA) {
                LEX::get_symbol(); // ,
                constant_assign(type, l);
            } else break;
        }
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<constant_definition>" << std::endl;
        LEX::out << "<常量定义>" << std::endl;
    }

}

void constant_description(Location l) {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); // const
    if (LEX::curr_symbol == LEX::CONST_TK) {
        constant_definition(l);
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
        //get_symbol();//;
    }
    while (true) {
        std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        LEX::Symbol first_symbol = symbols[0];
        if (first_symbol == LEX::CONST_TK) {
            LEX::get_symbol(); // const
            constant_definition(l);
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::SEMICN) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
            }
            //get_symbol();//;
        } else break;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<constant_description>" << std::endl;
        LEX::out << "<常量说明>" << std::endl;
    }

}

std::vector<int> linear_array_assign(Basic_type basicType, unsigned int length) {
    Basic_type bt;
    int n;
    std::vector<int> ans;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::LBRACE) {
        LEX::get_symbol(); // {
        int num_token = 1;
        bt = constant(n);//todo
        ans.push_back(n);
        if (bt != basicType) {
            log_error(INVALID_TYPE_IN_ARRAY_INIT,LEX::curr_symbol_line);
        }
        while (true) {
            symbols = LEX::pre_read(1);
            if (symbols.empty()) break;
            if (symbols[0] == LEX::COMMA) {
                LEX::get_symbol(); // ,
                bt = constant(n);//todo
                ans.push_back(n);
                if (bt != basicType) {
                    log_error(INVALID_TYPE_IN_ARRAY_INIT,LEX::curr_symbol_line);
                }
                num_token++;
            } else break;
        }
        if (num_token != length) {
            log_error(INVALID_NUM_IN_ARRAY_INIT,LEX::curr_symbol_line);
        }
        LEX::get_symbol(); // }
    } else {
        //error
    }
    return ans;
}

std::vector<std::vector<int>> two_dimension_array_assign(Basic_type basicType, unsigned int x, unsigned int y) {
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    std::vector<std::vector<int>> ans;
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::LBRACE) {
        LEX::get_symbol(); // {
        int num_token = 1;
        ans.push_back(linear_array_assign(basicType,y));
        while (true) {
            symbols = LEX::pre_read(1);
            if (symbols.empty()) break;
            if (symbols[0] == LEX::COMMA) {
                LEX::get_symbol(); // ,
                ans.push_back(linear_array_assign(basicType,y));
                num_token++;
            } else break;
        }
        if (num_token != x) {
            log_error(INVALID_NUM_IN_ARRAY_INIT,LEX::curr_symbol_line);
        }
        LEX::get_symbol(); // }
    } else {
        //error
    }
    return ans;
}

bool var_assign(LEX::Symbol symbol, Location l) {
    bool define_with_init = false;
    Basic_type curr_var_type = (symbol == LEX::INT_TK) ? INT_ : CHAR_;
    Basic_type bt;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    std::vector<int> array_value;
    std::vector<std::vector<int>> array2_value;
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::IDENTIFIER_CON) {
        identifier_info tmp;
        LEX::get_symbol(); //IDEN
        tmp.name = LEX::token;
        tmp.basic_type = (symbol == LEX::INT_TK) ? INT_ : CHAR_;
        tmp.type = VAR_;
        tmp.value = 0;
        tmp.value2 = 0;
        symbols = LEX::pre_read(1);
        if (symbols.empty()) {} //error
        if (symbols[0] == LEX::ASSIGN) {
            LEX::get_symbol(); // =
            define_with_init = true;
            bt = constant(tmp.value);
            tmp.value2 = 1;
            if (bt != curr_var_type) {
                log_error(INVALID_TYPE_IN_ARRAY_INIT,LEX::curr_symbol_line);
            }
            //MID_CODE::add_mid_code(MID_CODE::VAR,MID_CODE::mid_opnum(l,tmp.basic_type,tmp.type,tmp.value,tmp.name));
        } else if (symbols[0] == LEX::LBRACK) {
            LEX::get_symbol(); // [
            unsigned int x = unsigned_integer(); // x
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::RBRACK) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
            }
            //get_symbol(); // ]
            tmp.type = ARRAY_;
            tmp.value = (int)x;
            symbols = LEX::pre_read(1);
            if (symbols.empty()) {} //error
            if (symbols[0] == LEX::ASSIGN) {
                LEX::get_symbol(); // =
                define_with_init = true;
                array_value = linear_array_assign(curr_var_type,x);
            } else if (symbols[0] == LEX::LBRACK) {
                LEX::get_symbol(); // [
                unsigned int y = unsigned_integer(); // y
                symbols = LEX::pre_read(1);
                if (symbols[0] == LEX::RBRACK) {
                    LEX::get_symbol();
                } else {
                    log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
                }
                //get_symbol(); // ]
                tmp.type = ARRAY2_;
                tmp.value2 = (int)y;
                symbols = LEX::pre_read(1);
                if (symbols.empty()) {} //error
                if (symbols[0] == LEX::ASSIGN) {
                    LEX::get_symbol(); // =
                    define_with_init = true;
                    array2_value = two_dimension_array_assign(curr_var_type,x,y);
                }
            }
        }
        if (insert_ident(l,tmp.name,tmp) == -1) {
            log_error(IDEN_REDEFINITION,LEX::curr_symbol_line);
        }
        if (tmp.type == VAR_) {
            MID_CODE::add_mid_code(MID_CODE::VAR,
                                   MID_CODE::mid_opnum(l,tmp.basic_type,tmp.type,tmp.value,tmp.value2,tmp.name));
        } else if (tmp.type == ARRAY_) {
            MID_CODE::mid_opnum array(l,tmp.basic_type,tmp.type,tmp.value,tmp.value2,tmp.name);
            MID_CODE::add_mid_code(MID_CODE::ARRAY,array);
            if (define_with_init) {
                for (int i = 0; i < array_value.size(); i++) {
                    MID_CODE::mid_opnum index(MID_CODE::INT_CON,i,"");
                    MID_CODE::mid_opnum value(MID_CODE::INT_CON,array_value[i],"");
                    if (l == GLOBAL_)
                        MID_CODE::add_mid_code(MID_CODE::ARR_INIT,array,index,value);
                    else
                        MID_CODE::add_mid_code(MID_CODE::ARR_ASSIGN,array,index,value);
                }
            }
        } else if (tmp.type == ARRAY2_) {
            MID_CODE::mid_opnum array2(l,tmp.basic_type,tmp.type,tmp.value,tmp.value2,tmp.name);
            MID_CODE::add_mid_code(MID_CODE::ARRAY2,array2);
            if (define_with_init) {
                for (int i = 0; i < array2_value.size(); i++) {
                    for (int j = 0; j < array2_value[i].size(); j++) {
                        MID_CODE::mid_opnum index(MID_CODE::INT_CON,i*tmp.value2+j,"");
                        MID_CODE::mid_opnum value(MID_CODE::INT_CON,array2_value[i][j],"");
                        if (l == GLOBAL_)
                            MID_CODE::add_mid_code(MID_CODE::ARR_INIT,array2,index,value);
                        else
                            MID_CODE::add_mid_code(MID_CODE::ARR_ASSIGN,array2,index,value);
                    }
                }
            }
        }
    } else {}//error
    return define_with_init;
}

void var_definition(Location l) {
    LEX::get_symbol(); // int / char
    LEX::Symbol symbol = LEX::curr_symbol;
    bool define_with_init = var_assign(symbol, l);
    while (true) {
        std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::COMMA) {
            LEX::get_symbol(); // ,
            if (var_assign(symbol, l) != define_with_init) {
                //error
            }
        } else break;
    }
    if (define_with_init) {
        if (SYNTACTIC_OUTPUT) {
            std::cout << "<var_definition_with_init>" << std::endl;
            LEX::out << "<变量定义及初始化>" << std::endl;
        }

    } else {
        if (SYNTACTIC_OUTPUT) {
            std::cout << "<var_definition_without_init>" << std::endl;
            LEX::out << "<变量定义无初始化>" << std::endl;
        }

    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<var_definition>" << std::endl;
        LEX::out << "<变量定义>" << std::endl;
    }

}

void var_description(Location l) {
    std::vector<LEX::Symbol> symbols;
    var_definition(l);
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::SEMICN) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
    }
    //get_symbol(); // ;
    while (true) {
        symbols = LEX::pre_read(3);
        if (symbols.empty()) return;
        if (IS_TYPE_TK(symbols[0]) && IS_IDEN_TK(symbols[1]) && !IS_LEFT_SB(symbols[2])) {
            var_definition(l);
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::SEMICN) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
            }
            //get_symbol(); // ;
        } else break;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<var_description>" << std::endl;
        LEX::out << "<变量说明>" << std::endl;
    }

}

std::string declaration_head(MID_CODE::mid_opnum& func) {
    std::string func_name;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (IS_TYPE_TK(symbols[0])) {
        identifier_info ii;
        LEX::get_symbol(); // int / char
        ii.basic_type = (LEX::curr_symbol == LEX::INT_TK)? INT_ : CHAR_;
        func.type = (LEX::curr_symbol == LEX::INT_TK)? MID_CODE::INT_TYPE_FUNC : MID_CODE::CHAR_TYPE_FUNC;
        LEX::get_symbol(); // IDEN
        ii.name = LEX::token;
        func.name = LEX::token;
        ii.type = FUNC_;
        insert_ident(PARTIAL_,ii.name,ii);
        if (insert_ident(GLOBAL_, ii.name, ii) == -1) {
            log_error(IDEN_REDEFINITION,LEX::curr_symbol_line);
        }
        func_name = LEX::token;
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<declaration_head>" << std::endl;
        LEX::out << "<声明头部>" << std::endl;
    }

    return func_name;
}

std::vector<identifier_info> para_list() {
    std::vector<identifier_info> plist;
    while (true) {
        std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::RPARENT) {
            break;
        } else if (IS_TYPE_TK(symbols[0])) {
            identifier_info tmp;
            MID_CODE::mid_opnum para;
            LEX::get_symbol(); // int / char
            tmp.basic_type = (LEX::curr_symbol == LEX::INT_TK) ? INT_ : CHAR_;
            para.type = (LEX::curr_symbol == LEX::INT_TK) ? MID_CODE::INT_PARA : MID_CODE::CHAR_PARA;
            LEX::get_symbol(); // IDEN
            tmp.name = LEX::token;
            para.name = LEX::token;
            tmp.type = PARAM_;
            tmp.value = -1;
            if (insert_ident(PARTIAL_, LEX::token, tmp) == -1) {
                log_error(IDEN_REDEFINITION,LEX::curr_symbol_line);
            } else {
                plist.push_back(tmp);
            }
            MID_CODE::add_mid_code(MID_CODE::PARA,para);
        }
        symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::COMMA) {
            LEX::get_symbol();
            continue;
        } else if (symbols[0] == LEX::RPARENT) {
            break;
        } else {
            break;
        }
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<para_list>" << std::endl;
        LEX::out << "<参数表>" << std::endl;
    }

    return plist;
}

void condition(MID_CODE::mid_opnum label, bool is_condition_sentence) {
    std::vector<LEX::Symbol> symbols;
    MID_CODE::mid_opnum op1;
    LEX::Symbol relation_sign = LEX::EQL;
    Basic_type bt1 = expression(op1);//todo
    symbols = LEX::pre_read(1);
    /*if (IS_SENTENCE_FIRST(symbols[0])) sentence();
    else {} //error*/
    if (IS_REL_SIGN(symbols[0])) {
        LEX::get_symbol(); // relation sign
        relation_sign = LEX::curr_symbol;
    }
    MID_CODE::mid_opnum op2;
    Basic_type bt2 = expression(op2);//todo
    if (bt1 != INT_ || bt2 != INT_) {
        log_error(CONDITION_TYPE_DISPATCH,LEX::curr_symbol_line);
    }
    MID_CODE::mid_opnum tmp;
    switch (relation_sign) {
        case LEX::EQL: // ==
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BNE,op1,op2,label);
            else
                MID_CODE::add_mid_code(MID_CODE::BEQ,op1,op2,label);
            break;
        case LEX::NEQ: // !=
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BEQ,op1,op2,label);
            else
                MID_CODE::add_mid_code(MID_CODE::BNE,op1,op2,label);
            break;
        case LEX::LSS: // <
            tmp = MID_CODE::create_new_tmp_var(INT_);
            MID_CODE::add_mid_code(MID_CODE::SUB,op1,op2,tmp);
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BGEZ, tmp, label);
            else
                MID_CODE::add_mid_code(MID_CODE::BLTZ, tmp, label);
            break;
        case LEX::LEQ: // <=
            tmp = MID_CODE::create_new_tmp_var(INT_);
            MID_CODE::add_mid_code(MID_CODE::SUB,op1,op2,tmp);
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BGTZ, tmp, label);
            else
                MID_CODE::add_mid_code(MID_CODE::BLEZ, tmp, label);
            break;
        case LEX::GRE: // >
            tmp = MID_CODE::create_new_tmp_var(INT_);
            MID_CODE::add_mid_code(MID_CODE::SUB,op1,op2,tmp);
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BLEZ, tmp, label);
            else
                MID_CODE::add_mid_code(MID_CODE::BGTZ, tmp, label);
            break;
        case LEX::GEQ: // >=
            tmp = MID_CODE::create_new_tmp_var(INT_);
            MID_CODE::add_mid_code(MID_CODE::SUB,op1,op2,tmp);
            if (is_condition_sentence)
                MID_CODE::add_mid_code(MID_CODE::BLTZ, tmp, label);
            else
                MID_CODE::add_mid_code(MID_CODE::BGEZ, tmp, label);
            break;
        default:
            break;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<condition>" << std::endl;
        LEX::out << "<条件>" << std::endl;
    }

}

MID_CODE::mid_opcode reverse(MID_CODE::mid_opcode cur) {
    switch (cur) {
        case MID_CODE::BEQ :
            return MID_CODE::BNE;
        case MID_CODE::BNE:
            return MID_CODE::BEQ;
        case MID_CODE::BGEZ:
            return MID_CODE::BLTZ;
        case MID_CODE::BGTZ:
            return MID_CODE::BLEZ;
        case MID_CODE::BLEZ:
            return MID_CODE::BGTZ;
        case MID_CODE::BLTZ:
            return MID_CODE::BGEZ;
        default:
            return MID_CODE::DEAD;
    }
    return MID_CODE::DEAD;
}

void while_sentence() {
    std::vector<LEX::Symbol> symbols;
    if (OPTIMIZE) {
        LEX::get_symbol(); // while
        LEX::get_symbol(); // (
        MID_CODE::mid_opnum loop = MID_CODE::create_new_label();
        MID_CODE::mid_opnum end = MID_CODE::create_new_label();
        MID_CODE::need_condition_rec = true;
        condition(end, true);
        MID_CODE::need_condition_rec = false;
        std::vector<MID_CODE::middle_code> rec = MID_CODE::condition_rec;
        MID_CODE::condition_rec.clear();
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_RPARENT, LEX::curr_symbol_line);
        }
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, loop);

        sentence();
        for (auto& mc : rec) {
            if (IS_RELATION_MID_CODE(mc.op_code)) {
                mc.op_code = reverse(mc.op_code);
                mc.result = loop;
            }
            MID_CODE::middle_code_list.push_back(mc);
        }
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, end);
    }
    else {
        LEX::get_symbol(); // while
        LEX::get_symbol(); // (
        MID_CODE::mid_opnum loop = MID_CODE::create_new_label();
        MID_CODE::mid_opnum end = MID_CODE::create_new_label();
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, loop);
        condition(end, true);
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_RPARENT, LEX::curr_symbol_line);
        }
        //get_symbol(); // )

        sentence();
        MID_CODE::add_mid_code(MID_CODE::JUMP, loop);
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, end);
    }
}

unsigned int step() {
    unsigned int ret = unsigned_integer();
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<step>" << std::endl;
        LEX::out << "<步长>" << std::endl;
    }
    return ret;
}

void for_sentence() {
    if (OPTIMIZE) {
        LEX::get_symbol(); // for
        LEX::get_symbol(); // (
        LEX::get_symbol(); // IDEN
        identifier_info* p1 = look_up_global_ident(LEX::token);
        identifier_info* p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }
        if (p1 != nullptr && p1->type == CONST_) {
            log_error(ASSIGN_TO_CONSTANT, LEX::curr_symbol_line);
        }
        MID_CODE::mid_opnum dst(p1);
        LEX::get_symbol(); // =
        MID_CODE::mid_opnum op;
        expression(op);//todo
        MID_CODE::add_mid_code(MID_CODE::ASSIGN, op, dst);
        std::vector<LEX::Symbol> symbols;
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
        //get_symbol(); // ;
        MID_CODE::mid_opnum loop = MID_CODE::create_new_label();
        MID_CODE::mid_opnum end = MID_CODE::create_new_label();
        
        MID_CODE::need_condition_rec = true;
        condition(end, true);
        MID_CODE::need_condition_rec = false;
        std::vector<MID_CODE::middle_code> rec = MID_CODE::condition_rec;
        MID_CODE::condition_rec.clear();
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
        //get_symbol(); // ;
        LEX::get_symbol(); // IDEN
        p1 = look_up_global_ident(LEX::token);
        p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }
        if (p1 != nullptr && p1->type == CONST_) {
            log_error(ASSIGN_TO_CONSTANT, LEX::curr_symbol_line);
        }
        LEX::get_symbol(); // =
        LEX::get_symbol(); // IDEN
        p1 = look_up_global_ident(LEX::token);
        p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }

        LEX::get_symbol(); // + / -
        bool sign = (LEX::curr_symbol == LEX::PLUS);
        MID_CODE::mid_opnum s;
        s.type = MID_CODE::INT_CON;
        s.value = (int)step();
        MID_CODE::mid_opnum p(p1);

        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_RPARENT, LEX::curr_symbol_line);
        }
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, loop);

        //get_symbol(); // )
        sentence();
        if (sign)
            MID_CODE::add_mid_code(MID_CODE::ADD, p, s, p);
        else
            MID_CODE::add_mid_code(MID_CODE::SUB, p, s, p);
        for (auto& mc : rec) {
            if (IS_RELATION_MID_CODE(mc.op_code)) {
                mc.op_code = reverse(mc.op_code);
                mc.result = loop;
            }
            MID_CODE::middle_code_list.push_back(mc);
        }
        MID_CODE::condition_rec.clear();
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, end);
    }
    else {
        LEX::get_symbol(); // for
        LEX::get_symbol(); // (
        LEX::get_symbol(); // IDEN
        identifier_info* p1 = look_up_global_ident(LEX::token);
        identifier_info* p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }
        if (p1 != nullptr && p1->type == CONST_) {
            log_error(ASSIGN_TO_CONSTANT, LEX::curr_symbol_line);
        }
        MID_CODE::mid_opnum dst(p1);
        LEX::get_symbol(); // =
        MID_CODE::mid_opnum op;
        expression(op);//todo
        MID_CODE::add_mid_code(MID_CODE::ASSIGN, op, dst);
        std::vector<LEX::Symbol> symbols;
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
        //get_symbol(); // ;
        MID_CODE::mid_opnum loop = MID_CODE::create_new_label();
        MID_CODE::mid_opnum end = MID_CODE::create_new_label();
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, loop);
        condition(end, true);
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
        //get_symbol(); // ;
        LEX::get_symbol(); // IDEN
        p1 = look_up_global_ident(LEX::token);
        p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }
        if (p1 != nullptr && p1->type == CONST_) {
            log_error(ASSIGN_TO_CONSTANT, LEX::curr_symbol_line);
        }
        LEX::get_symbol(); // =
        LEX::get_symbol(); // IDEN
        p1 = look_up_global_ident(LEX::token);
        p2 = look_up_partial_ident(LEX::token);
        if (p1 == nullptr && p2 == nullptr) {
            log_error(IDEN_NO_DEFINITION, LEX::curr_symbol_line);
        }
        else if (p2 != nullptr) {
            p1 = p2;
        }

        LEX::get_symbol(); // + / -
        bool sign = (LEX::curr_symbol == LEX::PLUS);
        MID_CODE::mid_opnum s;
        s.type = MID_CODE::INT_CON;
        s.value = (int)step();
        MID_CODE::mid_opnum p(p1);

        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        }
        else {
            log_error(EXPECT_RPARENT, LEX::curr_symbol_line);
        }
        //get_symbol(); // )
        sentence();
        if (sign)
            MID_CODE::add_mid_code(MID_CODE::ADD, p, s, p);
        else
            MID_CODE::add_mid_code(MID_CODE::SUB, p, s, p);
        MID_CODE::add_mid_code(MID_CODE::JUMP, loop);
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, end);
    }
}

void loop_sentence() {
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::WHILE_TK) {
        while_sentence();
    } else if (symbols[0] == LEX::FOR_TK) {
        for_sentence();
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<loop_sentence>" << std::endl;
        LEX::out << "<循环语句>" << std::endl;
    }

}

void condition_sentence() {
    std::vector<LEX::Symbol> symbols;
    if (symbols.empty()) {} //error
    LEX::get_symbol(); // if
    LEX::get_symbol(); // (
    MID_CODE::mid_opnum label1 = MID_CODE::create_new_label();
    condition(label1,true);
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )
    symbols = LEX::pre_read(1);
    if (IS_SENTENCE_FIRST(symbols[0])) sentence();
    else {} //error
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::ELSE_TK) {
        MID_CODE::mid_opnum label2 = MID_CODE::create_new_label();
        MID_CODE::add_mid_code(MID_CODE::JUMP, label2);
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, label1);
        LEX::get_symbol(); // else
        symbols = LEX::pre_read(1);
        if (IS_SENTENCE_FIRST(symbols[0])) sentence();
        else {} //error
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, label2);
    } else {
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, label1);
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<condition_sentence>" << std::endl;
        LEX::out << "<条件语句>" << std::endl;
    }

}

void default_sentence() {
    LEX::get_symbol(); //default
    LEX::get_symbol(); //:
    sentence();
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<default_sentence>" << std::endl;
        LEX::out << "<缺省>" << std::endl;
    }
    /*std::vector<Symbol> symbols;
    symbols = pre_read(1);
    if (symbols.empty()) {}
    if (symbols[0] == DEFAULT_TK) {


    } else {
        log_error(EXPECT_DEFAULT_SENTENCE,curr_symbol_line);
    }*/
}

void case_sentence(Basic_type case_type,
                   const MID_CODE::mid_opnum& switch_num,const MID_CODE::mid_opnum& end_label) {
    std::vector<LEX::Symbol> symbols;
    Basic_type bt;
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {}
    MID_CODE::mid_opnum label = MID_CODE::create_new_label();
    if (symbols[0] == LEX::CASE_TK) {
        LEX::get_symbol(); //case
        int p;
        bt = constant(p);//todo
        if (bt != case_type) {
            log_error(CONSTANT_TYPE_DISPATCH, LEX::curr_symbol_line);
        }
        MID_CODE::mid_opnum cons;
        cons.type = MID_CODE::INT_CON;
        cons.value = p;
        LEX::get_symbol(); //:
        MID_CODE::add_mid_code(MID_CODE::BNE,switch_num, cons,label);
        sentence();
        MID_CODE::add_mid_code(MID_CODE::JUMP, end_label);
        MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, label);
        if (SYNTACTIC_OUTPUT) {
            std::cout << "<case_sentence>" << std::endl;
            LEX::out << "<情况子语句>" << std::endl;
        }

    }
}

void switch_table(Basic_type case_type,
                  const MID_CODE::mid_opnum& switch_num,const MID_CODE::mid_opnum& end_label) {
    std::vector<LEX::Symbol> symbols;
    case_sentence(case_type,switch_num,end_label);
    while (true) {
        symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::CASE_TK) {
            case_sentence(case_type,switch_num,end_label);
        } else break;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<switch_table>" << std::endl;
        LEX::out << "<情况表>" << std::endl;
    }

}

void switch_sentence() {
    std::vector<LEX::Symbol> symbols;
    bool expect_default = false;
    LEX::get_symbol(); // switch
    LEX::get_symbol(); // (
    MID_CODE::mid_opnum switch_num;//todo
    Basic_type switch_type = expression(switch_num);//todo
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )
    LEX::get_symbol(); // {
    MID_CODE::mid_opnum end = MID_CODE::create_new_label();
    switch_table(switch_type,switch_num,end);
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {}
    if (symbols[0] == LEX::DEFAULT_TK) {
        default_sentence();
    } else {
        expect_default = true;
    }
    MID_CODE::add_mid_code(MID_CODE::GEN_LABEL, end);
    LEX::get_symbol(); // }
    if (expect_default) {
        log_error(EXPECT_DEFAULT_SENTENCE,LEX::curr_symbol_line);
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<switch_sentence>" << std::endl;
        LEX::out << "<情况语句>" << std::endl;
    }

}

void scan_sentence() {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); //scanf
    LEX::get_symbol(); // (
    LEX::get_symbol(); // IDEN
    identifier_info *p1 = look_up_global_ident(LEX::token);
    identifier_info *p2 = look_up_partial_ident(LEX::token);
    if (p1 == nullptr && p2 == nullptr) {
        log_error(IDEN_NO_DEFINITION,LEX::curr_symbol_line);
    } else if (p2 != nullptr) {
        p1 = p2;
    }
    if (p1 != nullptr && p1->type == CONST_) {
        log_error(ASSIGN_TO_CONSTANT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )
    MID_CODE::mid_opnum dst(p1);
    MID_CODE::add_mid_code(MID_CODE::READ,dst);

    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<scan_sentence>" << std::endl;
        LEX::out << "<读语句>" << std::endl;
    }

}

void print_sentence() {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); // printf
    LEX::get_symbol(); // (
    MID_CODE::mid_opnum op;//todo
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::STR_CON) {
        MID_CODE::mid_opnum str(MID_CODE::STR_CON,0,a_string());
        MID_CODE::add_mid_code(MID_CODE::WRITE,str);
        symbols = LEX::pre_read(1);
        if (symbols.empty()) {} //error
        if (symbols[0] == LEX::COMMA) {
            LEX::get_symbol();
            expression(op); //todo
            MID_CODE::add_mid_code(MID_CODE::WRITE,op);
        }
    } else if (IS_EXPS_FIRST(symbols[0])) {
        expression(op); //todo
        std::cout << "print " << op.value << "\n";
        MID_CODE::add_mid_code(MID_CODE::WRITE,op);
    } else {
        //error
    }
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); //)
    MID_CODE::mid_opnum line;
    MID_CODE::add_mid_code(MID_CODE::WRITE_NEW_LINE,line);
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<print_sentence>" << std::endl;
        LEX::out << "<写语句>" << std::endl;
    }

}

void return_sentence() {
    Basic_type ret = VOID_;
    bool have_lp = false;
    bool no_exp = true;
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); //return
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::LPARENT) {
        LEX::get_symbol(); // (
        have_lp = true;
        symbols = LEX::pre_read(1);
        if (IS_EXPS_FIRST(symbols[0])) {
            no_exp = false;
            MID_CODE::mid_opnum return_value;//todo
            ret = expression(return_value);
            MID_CODE::add_mid_code(MID_CODE::RETURN,return_value);
            MID_CODE::mid_opnum func;
            func.name = current_function.name;
            func.type = MID_CODE::FUNC_NAME;
            func.value = current_function.value;
            MID_CODE::add_mid_code(MID_CODE::FUNC_END,func);//
        }
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
        }
        //get_symbol(); // )
    }
    if (!have_lp) {
        MID_CODE::add_mid_code(MID_CODE::RETURN);
        if (current_function.name == "main") {
            MID_CODE::add_mid_code(MID_CODE::MAIN_END);//
        } else {
            MID_CODE::mid_opnum func;
            func.name = current_function.name;
            func.type = MID_CODE::FUNC_NAME;
            func.value = current_function.value;
            MID_CODE::add_mid_code(MID_CODE::FUNC_END,func);//
        }
    }
    if (current_function.basic_type == VOID_) {
        if (have_lp) {
            log_error(RETURN_EXP_IN_VOID_FUNC,LEX::curr_symbol_line);
        }
    } else {
        if (!have_lp || no_exp || ret != current_function.basic_type) {
            log_error(RETURN_TYPE_DISPATCH,LEX::curr_symbol_line);
        }
    }
    curr_func_with_ret = true;
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<return_sentence>" << std::endl;
        LEX::out << "<返回语句>" << std::endl;
    }

}

void assign_sentence() {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); // IDEN
    identifier_info *p1 = look_up_global_ident(LEX::token);
    identifier_info *p2 = look_up_partial_ident(LEX::token);
    if (p1 == nullptr && p2 == nullptr) {
        log_error(IDEN_NO_DEFINITION,LEX::curr_symbol_line);
    } else if (p2 != nullptr) {
        p1 = p2;
    }
    if (p1 != nullptr && p1->type == CONST_) {
        log_error(ASSIGN_TO_CONSTANT,LEX::curr_symbol_line);
    }
    MID_CODE::mid_opnum dst(p1);
    symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::ASSIGN) {
        LEX::get_symbol(); // =
        MID_CODE::mid_opnum op;
        expression(op);
        MID_CODE::add_mid_code(MID_CODE::ASSIGN,op,dst);
    } else if (symbols[0] == LEX::LBRACK) {
        LEX::get_symbol(); // [
        MID_CODE::mid_opnum index1;
        Basic_type bt = expression(index1);
        if (bt != INT_) {
            log_error(ARRAY_INDEX_TYPE_DISPATCH, LEX::curr_symbol_line);
        }
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RBRACK) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
        }
        //get_symbol(); // ]
        symbols = LEX::pre_read(1);
        if (symbols.empty()) {} //error
        if (symbols[0] == LEX::ASSIGN) {
            LEX::get_symbol(); // =
            MID_CODE::mid_opnum value;
            expression(value);//todo
            MID_CODE::add_mid_code(MID_CODE::ARR_ASSIGN,dst,index1,value);
        } else if (symbols[0] == LEX::LBRACK) {
            LEX::get_symbol(); // [
            MID_CODE::mid_opnum index2;
            bt = expression(index2);//todo
            if (bt != INT_) {
                log_error(ARRAY_INDEX_TYPE_DISPATCH, LEX::curr_symbol_line);
            }
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::RBRACK) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_RBRACK,LEX::curr_symbol_line);
            }
            //get_symbol(); // ]
            symbols = LEX::pre_read(1);
            if (symbols.empty()) {} //error
            if (symbols[0] == LEX::ASSIGN) {
                LEX::get_symbol(); // =
                MID_CODE::mid_opnum value;
                MID_CODE::mid_opnum index = MID_CODE::create_new_tmp_var(p1->basic_type);
                MID_CODE::mid_opnum tmp = MID_CODE::create_new_tmp_var(INT_);
                MID_CODE::mid_opnum size = MID_CODE::mid_opnum(MID_CODE::INT_CON,p1->value2,"");
                expression(value);//todo
                MID_CODE::add_mid_code(MID_CODE::MUL,index1,size,tmp);
                MID_CODE::add_mid_code(MID_CODE::ADD,tmp,index2,index);
                MID_CODE::add_mid_code(MID_CODE::ARR_ASSIGN,dst,index,value);
            } else {
                //error
            }
        }
        else {
            //error
        }
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<assign_sentence>" << std::endl;
        LEX::out << "<赋值语句>" << std::endl;
    }

}

void value_list(const std::string& func_name) {
    bool flag = true;
    if (func_name.empty()) {
        flag = false;
    }
    std::vector<identifier_info> plist = func_para_list[func_name];
    std::vector<Basic_type> vlist;
    while (true) {
        std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::RPARENT) {
            break;
        } else if (IS_EXPS_FIRST(symbols[0])) {
            MID_CODE::mid_opnum para;//todo
            vlist.push_back(expression(para));//todo
            MID_CODE::add_mid_code(MID_CODE::PUSH,para);
        }
        symbols = LEX::pre_read(1);
        if (symbols.empty()) break;
        if (symbols[0] == LEX::COMMA) {
            LEX::get_symbol();
            continue;
        } else if (symbols[0] == LEX::RPARENT) {
            break;
        } else {
            break;
        }
    }
    if (flag) {
        int l1 = plist.size(), l2 = vlist.size();
        if (l1 != l2) {
            log_error(FUNC_PARA_NUM_DISPATCH,LEX::curr_symbol_line);
        } else {
            for (int i = 0; i < l1; i++) {
                if (plist[i].basic_type != vlist[i]) {
                    log_error(FUNC_PARA_TYPE_DISPATCH,LEX::curr_symbol_line);
                }
            }
        }
    }

    if (SYNTACTIC_OUTPUT) {
        std::cout << "<value_list>" << std::endl;
        LEX::out << "<值参数表>" << std::endl;
    }

}

Basic_type call_return_func_sentence() {
    Basic_type ret = INT_;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::IDENTIFIER_CON) {
        LEX::get_symbol(); // iden
        if (look_up_global_ident(LEX::token) == nullptr) {
            log_error(FUNC_NO_DEFINITION,LEX::curr_symbol_line);
        }
        identifier_info * curr_func = look_up_ident(GLOBAL_, LEX::token); //error
        if (curr_func != nullptr && (curr_func->basic_type == INT_ || curr_func->basic_type == CHAR_)) {
            ret = (curr_func->basic_type == INT_) ? INT_ : CHAR_FUNC;
            LEX::get_symbol(); // (
            value_list(curr_func->name);
            MID_CODE::mid_opnum func;
            func.name = curr_func->name;
            func.type = MID_CODE::FUNC_NAME;
            func.value = curr_func->value;
            MID_CODE::add_mid_code(MID_CODE::CALL,func);
            symbols = LEX::pre_read(1);
            if (symbols[0] == LEX::RPARENT) {
                LEX::get_symbol();
            } else {
                log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
            }
            //get_symbol(); // )
        } else {
            //error skip till
        }
    } else {
        //error
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<call_return_func_sentence>" << std::endl;
        LEX::out << "<有返回值函数调用语句>" << std::endl;
    }

    return ret;
}

void call_function_sentence() {
    bool return_func = false;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    if (symbols[0] == LEX::IDENTIFIER_CON) {
        identifier_info* curr_func = nullptr;
        LEX::get_symbol(); // iden
        if (look_up_global_ident(LEX::token) == nullptr) {
            log_error(FUNC_NO_DEFINITION,LEX::curr_symbol_line);
        } else {
            curr_func = look_up_ident(GLOBAL_, LEX::token); //error
        }
        if (curr_func != nullptr && (curr_func->basic_type == INT_ || curr_func->basic_type == CHAR_)) {
            return_func = true;
        }
        LEX::get_symbol(); // (
        if (curr_func) {
            value_list(curr_func->name);
        } else {
            value_list("");
        }
        MID_CODE::mid_opnum func;
        func.name = curr_func->name;
        func.type = MID_CODE::FUNC_NAME;
        func.value = curr_func->value;
        MID_CODE::add_mid_code(MID_CODE::CALL,func);
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::RPARENT) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
        }
        //get_symbol(); // )
    } else {
        //error
    }
    if (return_func) {
        if (SYNTACTIC_OUTPUT) {
            std::cout << "<call_return_func_sentence>" << std::endl;
            LEX::out << "<有返回值函数调用语句>" << std::endl;
        }

    } else {
        if (SYNTACTIC_OUTPUT) {
            std::cout << "<call_non_return_func_sentence>" << std::endl;
            LEX::out << "<无返回值函数调用语句>" << std::endl;
        }

    }
}

void sentence() {
    bool need_semicn = false;
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) {} //error
    switch (symbols[0]) {
        case LEX::WHILE_TK:
        case LEX::FOR_TK:
            loop_sentence();
            break;
        case LEX::IF_TK:
            condition_sentence();
            break;
        case LEX::SWITCH_TK:
            switch_sentence();
            break;
        case LEX::IDENTIFIER_CON:
            symbols = LEX::pre_read(2);
            if (symbols.empty()) {} //error
            if (symbols[1] == LEX::LPARENT) {
                call_function_sentence();
            } else if (symbols[1] == LEX::ASSIGN || symbols[1] == LEX::LBRACK) {
                assign_sentence();
            } else {
                //error
            }
            need_semicn = true;
            //get_symbol(); // ;
            break;
        case LEX::SCANF_TK:
            scan_sentence();
            need_semicn = true;
            //get_symbol(); // ;
            break;
        case LEX::PRINTF_TK:
            print_sentence();
            need_semicn = true;
            //get_symbol(); // ;
            break;
        case LEX::RETURN_TK:
            return_sentence();
            need_semicn = true;
            //get_symbol(); // ;
            break;
        case LEX::LBRACE:
            LEX::get_symbol(); // {
            sentence_list();
            LEX::get_symbol(); // }
            break;
        case LEX::SEMICN:
            LEX::get_symbol(); // ;
            break;
        default:
            break;
    }
    if (need_semicn) {
        symbols = LEX::pre_read(1);
        if (symbols[0] == LEX::SEMICN) {
            LEX::get_symbol();
        } else {
            log_error(EXPECT_SEMICN, LEX::curr_symbol_line);
        }
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<sentence>" << std::endl;
        LEX::out << "<语句>" << std::endl;
    }
}

void sentence_list() {
    while (true) {
        std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
        if (symbols.empty()) return;
        if (IS_SENTENCE_FIRST(symbols[0])) {
            sentence();
        } else break;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<sentence_list>" << std::endl;
        LEX::out << "<语句列>" << std::endl;
    }

}

void compound_sentence() {
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) return;
    LEX::Symbol first_symbol = symbols[0];
    if (first_symbol == LEX::CONST_TK) {
        constant_description(PARTIAL_);
    }
    symbols = LEX::pre_read(3);
    if (symbols.empty()) return;
    if (IS_TYPE_TK(symbols[0]) && IS_IDEN_TK(symbols[1]) && !IS_LEFT_SB(symbols[2])) {
        var_description(PARTIAL_);
    }
    sentence_list();
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<compound_sentence>" << std::endl;
        LEX::out << "<复合语句>" << std::endl;
    }

}

void return_func_definition() {
    std::vector<LEX::Symbol> symbols;
    MID_CODE::mid_opnum func;
    std::string func_name = declaration_head(func);
    LEX::get_symbol(); // (
    MID_CODE::add_mid_code(MID_CODE::FUNC_START,func);
    std::vector<identifier_info> plist = para_list();
    identifier_info *ii = look_up_global_ident(func_name);
    ii->value = plist.size();
    func.value = ii->value;
    func_para_list[ii->name] = plist;
    current_function.basic_type = ii->basic_type;
    current_function.name = ii->name;
    current_function.type = ii->type;
    current_function.value = ii->value;
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )

    LEX::get_symbol(); // {
    compound_sentence();
    clear_partial_table(current_function.name);
    LEX::get_symbol(); // }
    MID_CODE::add_mid_code(MID_CODE::FUNC_END,func);
    if (!curr_func_with_ret) {
        log_error(EXPECT_RETURN_SENTENCE,LEX::curr_symbol_line);
    } else {
        curr_func_with_ret = false;
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<return_func_definition>" << std::endl;
        LEX::out << "<有返回值函数定义>" << std::endl;
    }

}

void non_return_func_definition() {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); // void
    LEX::get_symbol(); // IDEN
    MID_CODE::mid_opnum func;
    identifier_info ii;
    ii.name = LEX::token;
    func.name = LEX::token;
    ii.basic_type = VOID_;
    func.type = MID_CODE::VOID_TYPE_FUNC;
    ii.type = FUNC_;
    insert_ident(PARTIAL_,ii.name,ii);
    LEX::get_symbol(); // (
    MID_CODE::add_mid_code(MID_CODE::FUNC_START,func);
    std::vector<identifier_info> plist = para_list();
    ii.value = plist.size();
    func.value = ii.value;
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )
    if (insert_ident(GLOBAL_, ii.name, ii) == -1) {
        log_error(IDEN_REDEFINITION,LEX::curr_symbol_line);
    } else {
        current_function.basic_type = ii.basic_type;
        current_function.name = ii.name;
        current_function.type = ii.type;
        current_function.value = ii.value;
        func_para_list[ii.name] = plist;
    }

    LEX::get_symbol(); // {
    compound_sentence();
    LEX::get_symbol(); // }
    MID_CODE::add_mid_code(MID_CODE::FUNC_END,func);
    clear_partial_table(current_function.name);
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<non_return_func_definition>" << std::endl;
        LEX::out << "<无返回值函数定义>" << std::endl;
    }

}

void function_description() {
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) return;
    if (IS_TYPE_TK(symbols[0])) {
        return_func_definition();
    } else if (IS_VOID_TK(symbols[0])) {
        non_return_func_definition();
    }
    while (true) {
        symbols = LEX::pre_read(3);
        if (symbols.empty()) break;
        if (IS_FUNC_HEAD(symbols[0]) && IS_IDEN_TK(symbols[1]) && IS_LEFT_SB(symbols[2])) {
            if (IS_TYPE_TK(symbols[0])) {
                return_func_definition();
            } else if (IS_VOID_TK(symbols[0])) {
                non_return_func_definition();
            }
        } else break;
    }
}

void main_function() {
    std::vector<LEX::Symbol> symbols;
    LEX::get_symbol(); // void
    LEX::get_symbol(); // main
    LEX::get_symbol(); // (
    symbols = LEX::pre_read(1);
    if (symbols[0] == LEX::RPARENT) {
        LEX::get_symbol();
    } else {
        log_error(EXPECT_RPARENT,LEX::curr_symbol_line);
    }
    //get_symbol(); // )
    current_function.basic_type = VOID_;
    current_function.name = "main";
    current_function.type = FUNC_;
    current_function.value = 0;
    func_para_list["main"] = std::vector<identifier_info>();
    insert_ident(GLOBAL_, "main", current_function);
    LEX::get_symbol(); // {
    MID_CODE::add_mid_code(MID_CODE::MAIN_START);
    compound_sentence();
    MID_CODE::add_mid_code(MID_CODE::MAIN_END);
    LEX::get_symbol(); // }
    clear_partial_table(current_function.name);
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<main_function>" << std::endl;
        LEX::out << "<主函数>" << std::endl;
    }
}

void program() {
    std::vector<LEX::Symbol> symbols = LEX::pre_read(1);
    if (symbols.empty()) return;
    LEX::Symbol first_symbol = symbols[0];
    if (first_symbol == LEX::CONST_TK) {
        constant_description(GLOBAL_);
    }
    symbols = LEX::pre_read(3);
    if (symbols.empty()) return;
    if (IS_TYPE_TK(symbols[0]) && IS_IDEN_TK(symbols[1]) && !IS_LEFT_SB(symbols[2])) {
        var_description(GLOBAL_);
    }
    symbols = LEX::pre_read(3);
    if (symbols.empty()) return;
    if (IS_FUNC_HEAD(symbols[0]) && IS_IDEN_TK(symbols[1]) && IS_LEFT_SB(symbols[2])) {
        function_description();
    }
    symbols = LEX::pre_read(3);
    if (symbols.empty()) return;
    if (IS_VOID_TK(symbols[0]) && IS_MAIN_TK(symbols[1]) && IS_LEFT_SB(symbols[2])) {
        main_function();
    }
    if (SYNTACTIC_OUTPUT) {
        std::cout << "<program>" << std::endl;
        LEX::out << "<程序>" << std::endl;
    }
}