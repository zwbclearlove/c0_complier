//
// Created by zwb on 2020/11/10.
//

#include "lexical_analyzer.h"
#include <iostream>
#include <queue>


std::ifstream LEX::file;
std::ofstream LEX::out;
char LEX::curr_char;
std::string LEX::token;
LEX::Symbol LEX::curr_symbol;
int code_line_no = 1;
int LEX::curr_symbol_line;//todo
int LEX::end_of_file = 0;
unsigned int LEX::num = 0;
std::vector<LEX::word_info> pre_read_queue;

std::map<std::string,LEX::Symbol> reserved_word = {
        {"const", LEX::CONST_TK}, {"int",LEX::INT_TK},
        {"char", LEX::CHAR_TK}, {"void", LEX::VOID_TK},
        {"main", LEX::MAIN_TK}, {"if", LEX::IF_TK},
        {"else",LEX::ELSE_TK}, {"switch", LEX::SWITCH_TK},
        {"case", LEX::CASE_TK}, {"default", LEX::DEFAULT_TK},
        {"while", LEX::WHILE_TK}, {"for", LEX::FOR_TK},
        {"scanf", LEX::SCANF_TK}, {"printf", LEX::PRINTF_TK},
        {"return", LEX::RETURN_TK}
};

std::map<std::string, LEX::Symbol> delimiter = {
        {"+", LEX::PLUS}, {"-", LEX::MINUS}, {"*",LEX::MULTI}, {"/", LEX::DIV},
        {"(", LEX::LPARENT}, {")", LEX::RPARENT}, {"[", LEX::LBRACK}, {"]", LEX::RBRACK},
        {"{", LEX::LBRACE}, {"}", LEX::RBRACE}, {";", LEX::SEMICN}, {",", LEX::COMMA},
        {":", LEX::COLON}, {"=", LEX::ASSIGN}, {"<", LEX::LSS}, {"<=", LEX::LEQ},
        {">", LEX::GRE}, {">=", LEX::GEQ}, {"==", LEX::EQL}, {"!=", LEX::NEQ}
};

int get_char() {
    char c;
    if ((c = LEX::file.get()) == EOF) return -1;
    LEX::curr_char = c;
    if (c == '\n') code_line_no++;// todo: 单词行号单个记录，保证不是读到\n就加行号
    return 0;
}

void clear_token() {
    LEX::token.clear();
}

void cat_token() {
    LEX::token += LEX::curr_char;
}

void error() {
    std::cout << "unknown word in line " << code_line_no << std::endl;
}

void retract() {
    if (LEX::curr_char =='\n') code_line_no--;
    LEX::file.unget();
}

LEX::Symbol is_reserved_word() {
    if (reserved_word.count(LEX::token)) {
        LEX::curr_symbol = reserved_word[LEX::token];
        return LEX::curr_symbol;
    }
    return LEX::OTHER;
}

LEX::Symbol is_delimiter() {
    if (delimiter.count(LEX::token)) {
        LEX::curr_symbol = delimiter[LEX::token];
        return LEX::curr_symbol;
    }
    return LEX::OTHER;
}

void symbol_output() {
    LEX::out << to_string(LEX::curr_symbol) << " " << LEX::token << std::endl;
    std::cout << to_string(LEX::curr_symbol) << " " << LEX::token << " in " << LEX::curr_symbol_line << std::endl;
}

int LEX::get_symbol(bool is_pre_read) {
    if (get_char() == -1) return -1;
    clear_token();
    while (IS_WHITE(curr_char))
        if (get_char() == -1) break;

    if (IS_ALPHA(curr_char)) {
        while (IS_ALPHA(curr_char) || IS_DIGIT(curr_char)) {
            cat_token();
            if (get_char() == -1) { end_of_file = 1; break; }
        }
        if (!end_of_file) retract();
        std::transform(token.begin(),token.end(),token.begin(),tolower);
        LEX::Symbol r = is_reserved_word();
        if (r == LEX::OTHER) curr_symbol = LEX::IDENTIFIER_CON;
        if (DEBUG) {
            std::cout << "read a identifier : " + token << std::endl;
        }
    }
    else if (IS_DIGIT(curr_char)) {
        //to do: 需要考虑无符号整数中出现字母的情况
        bool is_num = true;
        while (true) {
            if (IS_DIGIT(curr_char)) {
                cat_token();
            } else if (IS_ALPHA(curr_char)) {
                cat_token();
                is_num = false;
            } else {
                break;
            }
            if (get_char() == -1) { end_of_file = 1; break; }
        }
        if (!end_of_file) retract();
        if (is_num) {
            try {
                num = std::stoi(token);
            } catch (std::out_of_range&) {
                log_error(INT_OVERFLOW, code_line_no);
                curr_symbol = LEX::INT_CON;
                num = -1;
                return 1;
            }
        } else {
            log_error(UNEXPECTED_CHAR_IN_INT, code_line_no);
            num = -1;
            curr_symbol = LEX::INT_CON;
            return 1;
        }
        if (DEBUG) {
            std::cout << "read a int : " + token << std::endl;
        }
        curr_symbol = LEX::INT_CON;
    }
    else if (IS_CHAR_CON(curr_char)) {
        bool end_of_char = false;
        cat_token();
        while (true) {
            if (get_char() == -1) { end_of_file = 1; break; }
            if (curr_char == '\'') { end_of_char = true; cat_token(); break; }
            if (curr_char == '\n' || curr_char == '\r') break;
            cat_token();
        }
        //to do: 错误情况处理
        if (!end_of_char || token.size() != 3 || !IS_CHAR(token[1])) {
            log_error(INVALID_CHAR_CON, code_line_no);
            token = '\0';
            curr_symbol = LEX::CHAR_CON;
            return 0;
        }
        token = token.substr(1,1);
        curr_symbol = LEX::CHAR_CON;
        if (DEBUG) {
            std::cout << "read a char : " + token << std::endl;
        }
    }
    else if (IS_STR_CON(curr_char)) {
        bool is_string = true;
        bool end_of_str = false;
        while (true) {
            if (get_char() == -1) { end_of_file = 1; break; }
            if (!IS_STR_CHAR(curr_char)) is_string = false;
            if (curr_char == '"') { end_of_str = true; break; }
            if (curr_char == '\n' || curr_char == '\r') break;
            cat_token();
        }
        //to do: 错误情况处理
        //token.erase(token.begin());
        //token.erase(token.end());
        if (!end_of_str || !is_string || token.empty()) {
            log_error(INVALID_STRING_CON, code_line_no);
            token = "";
            curr_symbol = LEX::STR_CON;
            return 0;
        }

        if (DEBUG) {
            std::cout << "read a string : " + token << std::endl;
        }
        curr_symbol = LEX::STR_CON;
    }
    else if (IS_SINGLE_SEMI(curr_char)) {
        cat_token();
        is_delimiter();
        if (DEBUG) {
            std::cout << "read a delimiter : " + token << std::endl;
        }
    }
    else if (IS_SEMI_START(curr_char)) {
        cat_token();
        if (get_char() == -1) end_of_file = 1;
        if (!end_of_file) {
            if (curr_char == '=') cat_token();
            else retract();
        }
        LEX::Symbol r = is_delimiter();
        curr_symbol = r;
        if (DEBUG) {
            std::cout << "read a delimiter : " + token << std::endl;
        }
    } else if (curr_char == '!') {
        cat_token();
        if (get_char() == -1) return -1; // error
        if (curr_char == '=') cat_token();
        else {
            log_error(UNKNOWN_SYMBOL, code_line_no);
            curr_symbol = LEX::UNKNOWN_SYM;
            token = "";
            return 0;
        }
        is_delimiter();
        if (DEBUG) {
            std::cout << "read a delimiter : " + token << std::endl;
        }
    } else if (IS_WHITE(curr_char)) {
        return -1;
    } else {
        log_error(UNKNOWN_SYMBOL, code_line_no);
        curr_symbol = LEX::UNKNOWN_SYM;
        token = "";
        return 0;
    }
    if (!is_pre_read && LEXICAL_OUTPUT) symbol_output();
    return 0;
}

std::string LEX::to_string(LEX::Symbol symbol) {
    switch (symbol) {
        case LEX::OTHER: return "OTHER";
        case LEX::IDENTIFIER_CON: return "IDENFR";
        case LEX::INT_CON: return "INTCON";
        case LEX::CHAR_CON: return "CHARCON";
        case LEX::STR_CON: return "STRCON";
        case LEX::CONST_TK: return "CONSTTK";
        case LEX::INT_TK: return "INTTK";
        case LEX::CHAR_TK: return "CHARTK";
        case LEX::VOID_TK: return "VOIDTK";
        case LEX::MAIN_TK: return "MAINTK";
        case LEX::IF_TK: return "IFTK";
        case LEX::ELSE_TK: return "ELSETK";
        case LEX::SWITCH_TK: return "SWITCHTK";
        case LEX::CASE_TK: return "CASETK";
        case LEX::DEFAULT_TK: return "DEFAULTTK";
        case LEX::WHILE_TK: return "WHILETK";
        case LEX::FOR_TK: return "FORTK";
        case LEX::SCANF_TK: return "SCANFTK";
        case LEX::PRINTF_TK: return "PRINTFTK";
        case LEX::RETURN_TK: return "RETURNTK";
        case LEX::PLUS: return "PLUS";
        case LEX::MINUS: return "MINU";
        case LEX::MULTI: return "MULT";
        case LEX::DIV: return "DIV";
        case LEX::LSS: return "LSS";
        case LEX::LEQ: return "LEQ";
        case LEX::GRE: return "GRE";
        case LEX::GEQ: return "GEQ";
        case LEX::EQL: return "EQL";
        case LEX::NEQ: return "NEQ";
        case LEX::COLON: return "COLON";
        case LEX::ASSIGN: return "ASSIGN";
        case LEX::SEMICN: return "SEMICN";
        case LEX::COMMA: return "COMMA";
        case LEX::LPARENT: return "LPARENT";
        case LEX::RPARENT: return "RPARENT";
        case LEX::LBRACK: return "LBRACK";
        case LEX::RBRACK: return "RBRACK";
        case LEX::LBRACE: return "LBRACE";
        case LEX::RBRACE: return "RBRACE";
        case LEX::UNKNOWN_SYM: return "UNKNOWN_SYMBOL";
    }
    return "";
}

LEX::word_info new_word_info(LEX::Symbol symbol, std::string name, int line_no) {
    LEX::word_info t;
    t.symbol = symbol;
    t.token = std::string(name);
    t.line_no = line_no;
    return t;
}

LEX::Symbol LEX::pre_read() {
    /*if (!pre_read_queue.empty()) {
        return pre_read_queue.front().first;
    }*/
    if (get_symbol(true) == -1) return LEX::OTHER;
    pre_read_queue.push_back(new_word_info(curr_symbol, token, code_line_no));
    return curr_symbol;
}

std::vector<LEX::Symbol> LEX::pre_read(int pre_read_num) {
    int len = pre_read_queue.size();
    std::vector<LEX::Symbol> ans;
    if (len < pre_read_num) {
        for (int i = 0; i < pre_read_num - len; i++) {
            if (pre_read() == LEX::OTHER) return ans;
        }
    }
    for (int i = 0; i < pre_read_num; i++) {
        ans.push_back(pre_read_queue[i].symbol);
    }
    return ans;
}

int LEX::get_symbol() {
    if (pre_read_queue.empty()) {
        return get_symbol(false);
    }
    LEX::word_info t = pre_read_queue.front();
    curr_symbol = t.symbol;
    token = t.token;
    curr_symbol_line = t.line_no;
    pre_read_queue.erase(pre_read_queue.begin());
    if (LEXICAL_OUTPUT) symbol_output();
    return 0;
}