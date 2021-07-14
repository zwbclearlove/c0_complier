//
// Created by zwb on 2020/11/10.
//

#include "object_code.h"

std::ofstream OC::mips;
std::vector<std::string> OC::data_section;
std::vector<std::string> OC::text_section;
std::vector<std::string> OC::init_section;

std::map<std::string, OC::Record*> OC::reg_map;
std::string temp_used[10];
std::queue<int> all_reg_used;

int str_no = 0;
std::map<std::string, std::string> str_rec;
std::string curr_func_name;

int tmp_label_no = 0;

identifier_info* const_zero;

void OC::init() {
    int gp_offset = 0;
    reg_map["const_zero"] = new Record();
    reg_map["const_zero"]->reg = ZERO;
    reg_map["const_tmp1"] = new Record();
    reg_map["const_tmp2"] = new Record();
    reg_map["runtime_const_var"] = new Record();
    str_rec.clear();
    for (auto & p : global_table) {
        identifier_info cur = p.second;
        if (cur.type == VAR_) {
            reg_map["global_var_"+cur.name] = new Record(NONE, true, gp_offset);
            std::cout << "global_var_"+cur.name+"\n";
            gp_offset += 4;
            if (DEBUG) {
                std::cout << "GLOBAL VAR " << cur.name;
                std::cout << " at " << reg_map["global_var_"+cur.name]->gp_offset << std::endl;
            }
        }
        else if (cur.type == CONST_) {
            reg_map["global_const_"+cur.name] = new Record();
            std::cout << "global_const_"+cur.name+"\n";
            if (DEBUG) {
                std::cout << "GLOBAL CONST " << cur.name << std::endl;
            }
        }
        else if (cur.type == ARRAY_) {
            reg_map["global_array_"+cur.name] = new Record(NONE, true, gp_offset);
            gp_offset += 4 * cur.value;
            if (DEBUG) {
                std::cout << "GLOBAL ARRAY  " << cur.name << " size:" << cur.value;
                std::cout << " at " << reg_map["global_array_"+cur.name]->gp_offset << std::endl;
            }
        }
        else if (cur.type == ARRAY2_) {
            reg_map["global_array2_"+cur.name] = new Record(NONE, true, gp_offset);
            gp_offset += 4 * cur.value * cur.value2;
            if (DEBUG) {
                std::cout << "GLOBAL ARRAY2  " << cur.name << " size:" << cur.value << " " << cur.value2;
                std::cout << " at " << reg_map["global_array2_"+cur.name]->gp_offset << std::endl;
            }
        }
        else if (cur.type == FUNC_) {
            Record *record = new Record(NONE,true);
            reg_map["func_"+cur.name] = record;
            std::cout << "func_"+cur.name+"\n";
            if (cur.name == "main") record->stack_size += 4;
            else record->stack_size += 8;
            if (DEBUG) {
                std::cout << "in FUNC " << cur.name << " para num:" << cur.value << std::endl;
            }
            int sp_offset = 0;
            std::string s_alloc[8];
            for(auto & i : s_alloc) i = "";
            auto f_tbl = func_table[cur.name];//func table
            auto p_tbl = func_para_list[cur.name]; //para list
            for (auto & pa : p_tbl) {
                record->stack_size += 4;
                reg_map["func_"+cur.name+"_para_"+pa.name] = new Record();
                std::cout << "func_"+cur.name+"_para_"+pa.name+"\n";
            }
            for (auto & pp : f_tbl) {
                identifier_info cc = pp.second;
                if (cc.type == VAR_) {
                    Record* child = new Record();
                    child->sp_offset = sp_offset;
                    sp_offset += 4;
                    reg_map["func_"+cur.name+"_var_"+cc.name] = child;
                    if (DEBUG) {
                        std::cout << "FUNC " << cur.name << " VAR " << cc.name;
                        std::cout << " At " << reg_map["func_"+cur.name+"_var_"+cc.name]->sp_offset << std::endl;
                    }
                    record->stack_size += 4;
                    // distribute reg
                    for (int i = 0; i < 8; i++) {
                        if (s_alloc[i].empty()) {
                            s_alloc[i] = "func_"+cur.name+"_var_"+cc.name;
                            child->reg = Reg(s0 + i);
                            break;
                        }
                    }
                }
                else if (cc.type == CONST_) {
                    reg_map["func_"+cur.name+"_const_"+cc.name] = new Record();
                    if (DEBUG) {
                        std::cout << "FUNC " << cur.name << " CONST " << cc.name << std::endl;
                    }
                }
                else if (cc.type == ARRAY_) {
                    Record* child = new Record();
                    record->stack_size += 4 * cc.value;
                    child->sp_offset = sp_offset;
                    sp_offset += 4 * cc.value;
                    reg_map["func_"+cur.name+"_array_"+cc.name] = child;
                    if (DEBUG) {
                        std::cout << "FUNC " << cur.name << " ARRAY " << cc.name << " size:" << cc.value;
                        std::cout << " At " << reg_map["func_"+cur.name+"_array_"+cc.name]->sp_offset << std::endl;
                    }
                }
                else if (cc.type == ARRAY2_) {
                    Record* child = new Record();
                    record->stack_size += 4 * cc.value * cc.value2;
                    child->sp_offset = sp_offset;
                    sp_offset += 4 * cc.value * cc.value2;
                    reg_map["func_"+cur.name+"_array2_"+cc.name] = child;
                    if (DEBUG) {
                        std::cout << "FUNC " << cur.name << " ARRAY2 " << cc.name << " size:" << cc.value << " " << cc.value2;
                        std::cout << " At " << reg_map["func_"+cur.name+"_array2_"+cc.name]->sp_offset << std::endl;
                    }
                }
            }
            for (auto & pp : f_tbl) {
                identifier_info cc = pp.second;
                if (cc.type == TMP_) {
                    record->stack_size += 4;
                    reg_map["func_"+cur.name+"_tmp_"+cc.name] = new Record(NONE, false, 0, sp_offset);
                    if (DEBUG) {
                        std::cout << "FUNC " << cur.name << " TMP " << cc.name;
                        std::cout << " At " << reg_map["func_"+cur.name+"_tmp_"+cc.name]->sp_offset << std::endl;
                    }
                    sp_offset += 4;
                }
            }
            if (DEBUG) {
                int sub = record->stack_size - 4 * (p_tbl.size());
                if (cur.name == "main") {
                    std::cout << "FUNC " << cur.name << " ra rec ";
                    std::cout << " At " << sub-4 << std::endl;
                }
                else {
                    std::cout << "FUNC " << cur.name << " fp rec ";
                    std::cout << " At " << sub-8 << std::endl;
                    std::cout << "FUNC " << cur.name << " ra rec ";
                    std::cout << " At " << sub-4 << std::endl;
                }
            }
            for(int i = 0; i < 8; i++)
            {
                if(!s_alloc[i].empty())
                {
                    record->stack_size += 4; // save regs
                    record->func_used.push_back(Reg(s0 + i)); // rec reg used
                }
            }
            for (int i = 0; i < p_tbl.size(); i++) {
                reg_map["func_"+cur.name+"_para_"+p_tbl[i].name]->sp_offset = record->stack_size - 4 * (i + 1);
                if (DEBUG) {
                    std::cout << "FUNC " << cur.name << " PARA " << p_tbl[i].name;
                    std::cout << " At " << reg_map["func_"+cur.name+"_para_"+p_tbl[i].name]->sp_offset << std::endl;
                }
            }

            if (DEBUG) {
                std::cout << "FUNC " << cur.name << " para num:" << cur.value;
                std::cout << " stack size: " << reg_map["func_"+cur.name]->stack_size << std::endl;
            }
        }
    }
}

std::string itos(int n) {
    return std::to_string(n);
}

std::string get_name(const std::string& func, const MID_CODE::mid_opnum& opn) {
    std::string ret;
    if (opn.type == MID_CODE::INT_GLOBAL_VAR ||
        opn.type == MID_CODE::CHAR_GLOBAL_VAR)
        return "global_var_"+opn.name;
    else if (opn.type == MID_CODE::INT_GLOBAL_CONST ||
             opn.type == MID_CODE::CHAR_GLOBAL_CONST)
        return "global_const_"+opn.name;
    else if (opn.type == MID_CODE::INT_GLOBAL_ARRAY ||
             opn.type == MID_CODE::CHAR_GLOBAL_ARRAY)
        return "global_array_"+opn.name;
    else if (opn.type == MID_CODE::INT_GLOBAL_ARRAY2 ||
             opn.type == MID_CODE::CHAR_GLOBAL_ARRAY2)
        return "global_array2_"+opn.name;
    else if (opn.type == MID_CODE::INT_PARA ||
             opn.type == MID_CODE::CHAR_PARA) {
        return "func_"+func+"_para_"+opn.name;
    }
    else if(opn.type == MID_CODE::INT_PARTIAL_VAR ||
            opn.type == MID_CODE::CHAR_PARTIAL_VAR) {
        return "func_"+func+"_var_"+opn.name;
    }
    else if (opn.type == MID_CODE::INT_PARTIAL_CONST ||
             opn.type == MID_CODE::CHAR_PARTIAL_CONST) {
        return "func_"+func+"_const_"+opn.name;
    }
    else if (opn.type == MID_CODE::INT_PARTIAL_ARRAY ||
             opn.type == MID_CODE::CHAR_PARTIAL_ARRAY)
        return "func_"+func+"_array_"+opn.name;
    else if (opn.type == MID_CODE::INT_PARTIAL_ARRAY2 ||
             opn.type == MID_CODE::CHAR_PARTIAL_ARRAY2)
        return "func_"+func+"_array2_"+opn.name;
    else if (opn.type == MID_CODE::INT_TEMP_VAR ||
             opn.type == MID_CODE::CHAR_TEMP_VAR) {
        return "func_"+func+"_tmp_"+opn.name;
    }
    return "";
}

std::string get_reg_name(OC::Reg reg) {
    if(reg >= OC::t0 && reg <= OC::t9)
    {
        std::string ret("$t");
        ret.append(1, char('0' + reg - OC::t0));
        return ret;
    }
    if(reg >= OC::s0 && reg <= OC::s7)
    {
        std::string ret("$s");
        ret.append(1, char('0' + reg - OC::s0));
        return ret;
    }
    if(reg >= OC::a0 && reg <= OC::a3)
    {
        std::string ret("$a");
        ret.append(1, char('0' + reg - OC::a0));
        return ret;
    }
    if(reg == OC::ZERO)
        return std::string("$0");
    return "";
}

std::string address(std::string name) {
    if (OC::reg_map[name]->is_global)
        return itos(OC::reg_map[name]->gp_offset) + "($gp)";
    else
        return itos(OC::reg_map[name]->sp_offset) + "($fp)";
}

bool is_const(const std::string& name) {
    return (std::regex_search(name, std::regex("const")));
}

OC::Reg tmp_alloc(const std::string& name) {
    for (int i = 0; i < 10; i++) {
        if (temp_used[i].empty()) {
            temp_used[i] = name;
            OC::reg_map[name]->reg = OC::Reg(OC::t0 + i);
            all_reg_used.push(i);
            return OC::Reg(OC::t0 + i);
        }
    }
    int reg_index = all_reg_used.front();
    if (!is_const(name)) {
        all_reg_used.pop();
        all_reg_used.push(reg_index);
    }
    std::string get_out = temp_used[reg_index];
    if (!is_const(get_out)) {
        OC::text_section.emplace_back("sw " + get_reg_name(OC::Reg(OC::t0 + reg_index)) + ", " + address((get_out)));
    }
    OC::reg_map[name]->reg = OC::reg_map[get_out]->reg;
    OC::reg_map[get_out]->reg = OC::NONE;
    temp_used[reg_index] = name;
    return OC::Reg(OC::t0 + reg_index);
}

void OC::tmp_reg_refresh() {
    for (int i = 0; i < 10; i++) {
        if (!temp_used[i].empty()) {
            if (!is_const(temp_used[i])) {
                if(DEBUG)text_section.emplace_back("#store tmp var "+temp_used[i]);
                text_section.emplace_back("sw "+get_reg_name(Reg(t0+i))+", "+address(temp_used[i]));
            }
            reg_map[temp_used[i]]->reg = NONE;
            temp_used[i] = "";
        }
        all_reg_used = std::queue<int>();
    }
}
void OC::tmp_restore() {
    for (int i = 0; i < 10; i++) {
        if (!temp_used[i].empty()) {
            if (!is_const(temp_used[i])) {
                text_section.emplace_back("sw "+get_reg_name(Reg(t0+i))+", "+address(temp_used[i]));
            }
            reg_map[temp_used[i]]->reg = NONE;
            temp_used[i] = "";
        }
        all_reg_used = std::queue<int>();
    }
}

void OC::refresh(Reg r) {
    if (r >= t0 && r <= t9) {
        reg_map[temp_used[r-t0]]->reg = NONE;
        temp_used[r-t0] = "";
    }
}

void OC::translate() {
    OC::init();
    data_section.clear();
    init_section.clear();
    text_section.clear();
    data_section.emplace_back(".data");
    init_section.emplace_back(".text");
    //text_section.emplace_back("jal main");
    text_section.emplace_back("j main");
    text_section.emplace_back("exit:");
    text_section.emplace_back("li $v0, 10");
    text_section.emplace_back("syscall");

    for (auto & mc : MID_CODE::middle_code_list) {
        if (DEBUG) {
            //std::cout << MID_CODE::mid_opcode_rec[mc.op_code] << std::endl;
        }
        int sub;
        if (mc.op_code == MID_CODE::CONST) {

        }
        else if (mc.op_code == MID_CODE::VAR && mc.result.value2 == 1) {
            std::string var_name = get_name(curr_func_name,mc.result);
            if (DEBUG) {
                std::cout << var_name << " init with value " << mc.result.value << "\n";
            }
            if (mc.result.type == MID_CODE::INT_GLOBAL_VAR ||
                mc.result.type == MID_CODE::CHAR_GLOBAL_VAR) {
                init_section.emplace_back("li $v0, "+itos(mc.result.value));
                if (reg_map[var_name]->reg == NONE) {
                    init_section.emplace_back("sw $v0, "+address(var_name));
                } else {
                    init_section.emplace_back("move "+get_reg_name(reg_map[var_name]->reg)+", $v0");
                }
            } else {
                text_section.emplace_back("li $v0, "+itos(mc.result.value));
                if (reg_map[var_name]->reg == NONE) {
                    text_section.emplace_back("sw $v0, "+address(var_name));
                } else {
                    text_section.emplace_back("move "+get_reg_name(reg_map[var_name]->reg)+", $v0");
                }
            }
        }
        else if (mc.op_code == MID_CODE::PARA) {

        }
        else if (mc.op_code == MID_CODE::ADD) {
            if (IS_CONST(mc.op_num1.type) && IS_CONST(mc.op_num2.type)) {
                int ans = mc.op_num1.value + mc.op_num2.value;
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                }
                text_section.emplace_back("li "+get_reg_name(reg_map[res]->reg)+", "+itos(ans));
            } else if (IS_CONST(mc.op_num1.type) || IS_CONST(mc.op_num2.type)) {
                int cns = IS_CONST(mc.op_num1.type) ? mc.op_num1.value : mc.op_num2.value;
                std::string var = IS_CONST(mc.op_num1.type) ?
                                  get_name(curr_func_name,mc.op_num2) : get_name(curr_func_name, mc.op_num1);
                if (reg_map[var]->reg == NONE) {
                    Reg r = tmp_alloc(var);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(var));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[var]->reg == NONE) {
                        Reg r1 = tmp_alloc(var);
                        text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(var));
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+var + "+" + itos(cns) +"="+res);
                text_section.emplace_back("addiu "+get_reg_name(reg_map[res]->reg)+
                                          ", "+get_reg_name(reg_map[var]->reg)+", "+itos(cns));

            } else {
                std::string src1 = get_name(curr_func_name,mc.op_num1);
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                }
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            Reg r1 = tmp_alloc(src1);
                            text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+src1 + "+" + src2 +"="+res);
                text_section.emplace_back("addu "+get_reg_name(reg_map[res]->reg)+", "+
                                          get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            }
        }
        else if (mc.op_code == MID_CODE::SUB) {
            if (IS_CONST(mc.op_num1.type) && IS_CONST(mc.op_num2.type)) {
                int ans = mc.op_num1.value - mc.op_num2.value;
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                }
                text_section.emplace_back("li "+get_reg_name(reg_map[res]->reg)+", "+itos(ans));
            } else if (IS_CONST(mc.op_num2.type)) {
                int cns = mc.op_num2.value;
                std::string var = get_name(curr_func_name, mc.op_num1);
                if (reg_map[var]->reg == NONE) {
                    Reg r = tmp_alloc(var);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(var));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[var]->reg == NONE) {
                        Reg r1 = tmp_alloc(var);
                        text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(var));
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+var + "-" + itos(cns) +"="+res);
                text_section.emplace_back("subiu "+get_reg_name(reg_map[res]->reg)+
                                          ", "+get_reg_name(reg_map[var]->reg)+", "+itos(cns));
            } else if (IS_CONST(mc.op_num1.type)) {
                std::string src1 = "const_tmp1";
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r1 = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src2));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            tmp_alloc(src1);
                            text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+ itos(mc.op_num1.value) + "-" + src2 +"="+res);
                text_section.emplace_back("subu "+get_reg_name(reg_map[res]->reg)+", "+
                                          get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            } else {
                std::string src1 = get_name(curr_func_name,mc.op_num1);
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                }
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            Reg r1 = tmp_alloc(src1);
                            text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+src1 + "-" + src2 +"="+res);
                text_section.emplace_back("subu "+get_reg_name(reg_map[res]->reg)+", "+
                                          get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            }
        }
        else if (mc.op_code == MID_CODE::MUL) {
            if (IS_CONST(mc.op_num1.type) && IS_CONST(mc.op_num2.type)) {
                int ans = mc.op_num1.value * mc.op_num2.value;
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                }
                text_section.emplace_back("li "+get_reg_name(reg_map[res]->reg)+", "+itos(ans));
            } else if (IS_CONST(mc.op_num1.type) || IS_CONST(mc.op_num2.type)) {
                int cns = IS_CONST(mc.op_num1.type) ? mc.op_num1.value : mc.op_num2.value;
                std::string var = IS_CONST(mc.op_num1.type) ?
                                  get_name(curr_func_name,mc.op_num2) : get_name(curr_func_name, mc.op_num1);
                if (reg_map[var]->reg == NONE) {
                    Reg r = tmp_alloc(var);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(var));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[var]->reg == NONE) {
                        Reg r1 = tmp_alloc(var);
                        text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(var));
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+var + "*" + itos(cns) +"="+res);
                if ((cns & (cns - 1)) == 0) {
                    int x = 0;
                    while (cns > 1) {
                        cns >>= 1;
                        x++;
                    }
                    text_section.emplace_back("sll " + get_reg_name(reg_map[res]->reg) +
                        ", " + get_reg_name(reg_map[var]->reg) + ", " + itos(x));
                }
                else
                    text_section.emplace_back("mul "+get_reg_name(reg_map[res]->reg)+
                                          ", "+get_reg_name(reg_map[var]->reg)+", "+itos(cns));
            } else {
                std::string src1 = get_name(curr_func_name,mc.op_num1);
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                }
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            Reg r1 = tmp_alloc(src1);
                            text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+src1 + "*" + src2 +"="+res);
                text_section.emplace_back("mul "+get_reg_name(reg_map[res]->reg)+", "+
                                          get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            }
        }
        else if (mc.op_code == MID_CODE::DIV) {
            if (IS_CONST(mc.op_num1.type) && IS_CONST(mc.op_num2.type)) {
                int ans = mc.op_num1.value / mc.op_num2.value;
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                }
                text_section.emplace_back("li "+get_reg_name(reg_map[res]->reg)+", "+itos(ans));
            } else if (IS_CONST(mc.op_num2.type)) {
                int cns = mc.op_num2.value;
                std::string var = get_name(curr_func_name, mc.op_num1);
                if (reg_map[var]->reg == NONE) {
                    Reg r = tmp_alloc(var);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(var));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[var]->reg == NONE) {
                        Reg r1 = tmp_alloc(var);
                        text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(var));
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+var + "/" + itos(cns) +"="+res);
                if (cns > 0 && (cns & (cns - 1)) == 0) {
                    int x = 0;
                    while (cns > 1) {
                        cns >>= 1;
                        x++;
                    }
                    std::string tmp_label_yes = "tmp_label" + itos(tmp_label_no++);
                    std::string tmp_label_exit = "tmp_label" + itos(tmp_label_no++);
                    text_section.emplace_back("bgez " + get_reg_name(reg_map[var]->reg) + ", " +
                        tmp_label_yes);
                    text_section.emplace_back("neg " + get_reg_name(reg_map[var]->reg) + ", " +
                        get_reg_name(reg_map[var]->reg));
                    text_section.emplace_back("srl " + get_reg_name(reg_map[res]->reg) +
                        ", " + get_reg_name(reg_map[var]->reg) + ", " + itos(x));
                    text_section.emplace_back("neg " + get_reg_name(reg_map[res]->reg) + ", " +
                        get_reg_name(reg_map[res]->reg));
                    text_section.emplace_back("j " + tmp_label_exit);
                    text_section.emplace_back(tmp_label_yes + ":");
                    text_section.emplace_back("srl " + get_reg_name(reg_map[res]->reg) +
                        ", " + get_reg_name(reg_map[var]->reg) + ", " + itos(x));
                    text_section.emplace_back(tmp_label_exit + ":");
                } else
                    text_section.emplace_back("div "+get_reg_name(reg_map[res]->reg)+", "+get_reg_name(reg_map[var]->reg)+", "+itos(cns));
            } else if (IS_CONST(mc.op_num1.type)) {
                std::string src1 = "const_tmp1";
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r1 = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src2));
                }
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            tmp_alloc(src1);
                            text_section.emplace_back("li "+get_reg_name(r)+", "+itos(mc.op_num1.value));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+ itos(mc.op_num1.value) + "/" + src2 +"="+res);
                text_section.emplace_back("div " + get_reg_name(reg_map[src1]->reg) + ", " + get_reg_name(reg_map[src2]->reg));
                text_section.emplace_back("mflo " + get_reg_name(reg_map[res]->reg));
                //text_section.emplace_back("div "+get_reg_name(reg_map[res]->reg)+", "+get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            } else {
                std::string src1 = get_name(curr_func_name,mc.op_num1);
                std::string src2 = get_name(curr_func_name,mc.op_num2);
                std::string res = get_name(curr_func_name,mc.result);
                if (reg_map[src1]->reg == NONE) {
                    Reg r = tmp_alloc(src1);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                }
                if (reg_map[src2]->reg == NONE) {
                    Reg r = tmp_alloc(src2);
                    text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                }
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r = tmp_alloc(src1);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src1));
                    }
                    if (reg_map[src2]->reg == NONE) {
                        Reg r = tmp_alloc(src2);
                        text_section.emplace_back("lw "+get_reg_name(r)+", "+address(src2));
                        if (reg_map[src1]->reg == NONE) {
                            Reg r1 = tmp_alloc(src1);
                            text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                        }
                    }
                }
                if(DEBUG)text_section.emplace_back("#"+src1 + "/" + src2 +"="+res);
                text_section.emplace_back("div " + get_reg_name(reg_map[src1]->reg) + ", " + get_reg_name(reg_map[src2]->reg));
                text_section.emplace_back("mflo " + get_reg_name(reg_map[res]->reg));
                //text_section.emplace_back("div "+get_reg_name(reg_map[res]->reg)+", "+get_reg_name(reg_map[src1]->reg)+", "+get_reg_name(reg_map[src2]->reg));
            }
        }
        else if (mc.op_code == MID_CODE::NEG) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            std::string res = get_name(curr_func_name,mc.result);
            if (IS_CONST(mc.op_num1.type)) {
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                }
                text_section.emplace_back("li "+get_reg_name(reg_map[res]->reg)+", "+itos(-mc.op_num1.value));
            } else {
                if (reg_map[src1]->reg == NONE) {
                    Reg r1 = tmp_alloc(src1);
                    text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                }
                if (reg_map[res]->reg == NONE) {
                    tmp_alloc(res);
                    if (reg_map[src1]->reg == NONE) {
                        Reg r1 = tmp_alloc(src1);
                        text_section.emplace_back("lw "+get_reg_name(r1)+", "+address(src1));
                    }
                }
                text_section.emplace_back("neg "+get_reg_name(reg_map[res]->reg)+
                                          ", "+get_reg_name(reg_map[src1]->reg));
            }
        }
        else if (mc.op_code == MID_CODE::READ) {
            std::string read_var = get_name(curr_func_name,mc.result);
            std::cout << read_var << std::endl;
            if (mc.result.type == MID_CODE::INT_GLOBAL_VAR ||
                mc.result.type == MID_CODE::INT_PARTIAL_VAR) {
                text_section.emplace_back("li $v0, 5");
                text_section.emplace_back("syscall");
                if (reg_map[read_var]->reg == NONE) {
                    text_section.emplace_back("sw $v0, "+address(read_var));
                } else {
                    text_section.emplace_back("move "+get_reg_name(reg_map[read_var]->reg)+", $v0");
                }
            } else if (mc.result.type == MID_CODE::CHAR_GLOBAL_VAR ||
                       mc.result.type == MID_CODE::CHAR_PARTIAL_VAR) {
                text_section.emplace_back("li $v0, 12");
                text_section.emplace_back("syscall");
                if (reg_map[read_var]->reg == NONE) {
                    text_section.emplace_back("sw $v0, "+address(read_var));
                } else {
                    text_section.emplace_back("move "+get_reg_name(reg_map[read_var]->reg)+", $v0");
                }
            }
        }
        else if (mc.op_code == MID_CODE::WRITE) {
            if (mc.result.type == MID_CODE::INT_GLOBAL_CONST ||
                mc.result.type == MID_CODE::INT_PARTIAL_CONST ||
                mc.result.type == MID_CODE::INT_CON ||
                mc.result.type == MID_CODE::INT_TEMP_CONST) {
                text_section.emplace_back("li $a0, "+itos(mc.result.value));
                text_section.emplace_back("li $v0, 1");
                text_section.emplace_back("syscall");
            } else if (mc.result.type == MID_CODE::CHAR_GLOBAL_CONST ||
                       mc.result.type == MID_CODE::CHAR_PARTIAL_CONST ||
                       mc.result.type == MID_CODE::CHAR_CON || 
                       mc.result.type == MID_CODE::CHAR_TEMP_CONST) {
                text_section.emplace_back("li $a0, "+itos(mc.result.value));
                text_section.emplace_back("li $v0, 11");
                text_section.emplace_back("syscall");
            } else if (mc.result.type == MID_CODE::STR_CON) {
                std::string str = std::regex_replace(mc.result.name,std::regex("\\\\"),"\\\\");
                if (str_rec.count(str) == 0) {
                    std::string s = "str" + itos(str_no++);

                    str_rec[str] = s;
                    data_section.emplace_back(s+": .asciiz \""+str+"\"");
                }
                text_section.emplace_back("la $a0, " + str_rec[str]);
                text_section.emplace_back("li $v0, 4");
                text_section.emplace_back("syscall");
            } else if (mc.result.type == MID_CODE::CHAR_GLOBAL_VAR ||
                       mc.result.type == MID_CODE::CHAR_PARTIAL_VAR ||
                       mc.result.type == MID_CODE::CHAR_TEMP_VAR ||
                       mc.result.type == MID_CODE::CHAR_PARA) {
                std::string str = get_name(curr_func_name,mc.result);
                if (reg_map[str]->reg == NONE) {
                    text_section.emplace_back("lw $a0, "+address(str));
                } else {
                    text_section.emplace_back("move $a0, "+
                                              get_reg_name(reg_map[str]->reg));
                }
                text_section.emplace_back("li $v0, 11");
                text_section.emplace_back("syscall");
            } else {
                std::string str = get_name(curr_func_name,mc.result);
                if (reg_map[str]->reg == NONE) {
                    text_section.emplace_back("lw $a0, "+address(str));
                } else {
                    text_section.emplace_back("move $a0, "+
                                              get_reg_name(reg_map[str]->reg));
                }
                text_section.emplace_back("li $v0, 1");
                text_section.emplace_back("syscall");
            }
        }
        else if (mc.op_code == MID_CODE::ASSIGN) {
            std::string src1 = get_name(curr_func_name, mc.op_num1);
            std::string dst = get_name(curr_func_name, mc.result);
            if (reg_map[dst]->reg == NONE) {
                tmp_alloc(dst);
            }
            if (IS_CONST(mc.op_num1.type)) {
                text_section.emplace_back("li " + get_reg_name(reg_map[dst]->reg) + ", " + itos(mc.op_num1.value));
            }
            else {
                if (reg_map[src1]->reg == NONE) {
                    text_section.emplace_back("lw " + get_reg_name(reg_map[dst]->reg) + ", " + address(src1));
                }
                else {
                    text_section.emplace_back("move " + get_reg_name(reg_map[dst]->reg) + ", " + get_reg_name(reg_map[src1]->reg));
                }
            }
        }
        else if (mc.op_code == MID_CODE::WRITE_NEW_LINE) {
            text_section.emplace_back("li $a0, 10");
            text_section.emplace_back("li $v0, 11");
            text_section.emplace_back("syscall");
        }
        else if (mc.op_code == MID_CODE::ARR_ASSIGN) { //array[index] = value
            std::string index = get_name(curr_func_name,mc.op_num2);
            if (IS_CONST(mc.op_num2.type)) { // index
                index = "const_tmp1";
                if (reg_map[index]->reg == NONE) tmp_alloc(index);
                text_section.emplace_back("li "+get_reg_name(reg_map[index]->reg)+", "+itos(mc.op_num2.value));
            } else if (reg_map[index]->reg == NONE) {
                tmp_alloc(index);
                text_section.emplace_back("lw "+get_reg_name(reg_map[index]->reg)+", "+address(index));
            }
            std::string value = get_name(curr_func_name,mc.result);;
            if (IS_CONST(mc.result.type)) { // value
                value = "const_tmp2";
                if (reg_map[value]->reg == NONE) tmp_alloc(value);
                text_section.emplace_back("li "+get_reg_name(reg_map[value]->reg)+", "+itos(mc.result.value));
            } else if (reg_map[value]->reg == NONE) {
                tmp_alloc(value);
                text_section.emplace_back("lw "+get_reg_name(reg_map[value]->reg)+", "+address(value));
            }
            Reg run_tmp_reg = a0;
            text_section.emplace_back("sll "+get_reg_name(run_tmp_reg)+", "+get_reg_name(reg_map[index]->reg)+", 2");
            std::string array = get_name(curr_func_name,mc.op_num1);
            if (reg_map[array]->is_global) {
                text_section.emplace_back("add "+ get_reg_name(run_tmp_reg) +","+ get_reg_name(run_tmp_reg) +",$gp");
                text_section.emplace_back("sw "+get_reg_name(reg_map[value]->reg)+", "+
                                          itos(reg_map[array]->gp_offset)+"("+ get_reg_name(run_tmp_reg) +")");
            } else {
                text_section.emplace_back("add " + get_reg_name(run_tmp_reg) + "," + get_reg_name(run_tmp_reg) + ",$fp");
                text_section.emplace_back("sw " + get_reg_name(reg_map[value]->reg) + ", " +
                                          itos(reg_map[array]->sp_offset) + "(" + get_reg_name(run_tmp_reg) + ")");
            }
        }
        else if (mc.op_code == MID_CODE::ARR_INIT) { //array[index] = value
            std::string index = get_name(curr_func_name,mc.op_num2);
            if (IS_CONST(mc.op_num2.type)) { // index
                index = "const_tmp1";
                if (reg_map[index]->reg == NONE) tmp_alloc(index);
                init_section.emplace_back("li "+get_reg_name(reg_map[index]->reg)+", "+itos(mc.op_num2.value));
            } else if (reg_map[index]->reg == NONE) {
                tmp_alloc(index);
                init_section.emplace_back("lw "+get_reg_name(reg_map[index]->reg)+", "+address(index));
            }
            std::string value = get_name(curr_func_name,mc.result);;
            if (IS_CONST(mc.result.type)) { // value
                value = "const_tmp2";
                if (reg_map[value]->reg == NONE) tmp_alloc(value);
                init_section.emplace_back("li "+get_reg_name(reg_map[value]->reg)+", "+itos(mc.result.value));
            } else if (reg_map[value]->reg == NONE) {
                tmp_alloc(value);
                init_section.emplace_back("lw "+get_reg_name(reg_map[value]->reg)+", "+address(value));
            }
            Reg run_tmp_reg = a0;
            init_section.emplace_back("sll "+ get_reg_name(run_tmp_reg) +", "+get_reg_name(reg_map[index]->reg)+", 2");
            std::string array = get_name(curr_func_name,mc.op_num1);
            if (reg_map[array]->is_global) {
                init_section.emplace_back("add "+ get_reg_name(run_tmp_reg) +","+ get_reg_name(run_tmp_reg) +",$gp");
                init_section.emplace_back("sw "+get_reg_name(reg_map[value]->reg)+", "+itos(reg_map[array]->gp_offset)+"("+ get_reg_name(run_tmp_reg) +")");
            } else {
                init_section.emplace_back("add "+ get_reg_name(run_tmp_reg) +","+ get_reg_name(run_tmp_reg) +",$fp");
                init_section.emplace_back("sw "+get_reg_name(reg_map[value]->reg)+", "+itos(reg_map[array]->sp_offset)+"("+ get_reg_name(run_tmp_reg) +")");
            }
        }
        else if (mc.op_code == MID_CODE::USE_ARR) { // var = array[index]
            std::string var = get_name(curr_func_name,mc.result);
            if (reg_map[var]->reg == NONE) {
                tmp_alloc(var);
            }
            std::string index = get_name(curr_func_name,mc.op_num2);
            if (IS_CONST(mc.op_num2.type)) { // index
                index = "const_tmp1";
                if (reg_map[index]->reg == NONE) tmp_alloc(index);
                text_section.emplace_back("li "+get_reg_name(reg_map[index]->reg)+", "+itos(mc.op_num2.value));
            } else if (reg_map[index]->reg == NONE) {
                tmp_alloc(index);
                text_section.emplace_back("lw "+get_reg_name(reg_map[index]->reg)+", "+address(index));
            }
            Reg run_tmp_reg = a0;
            text_section.emplace_back("sll "+ get_reg_name(run_tmp_reg) +", "+get_reg_name(reg_map[index]->reg)+", 2");
            std::string array = get_name(curr_func_name,mc.op_num1);
            if (reg_map[array]->is_global) {
                text_section.emplace_back("add "+ get_reg_name(run_tmp_reg) +","+ get_reg_name(run_tmp_reg) +",$gp");
                text_section.emplace_back("lw "+get_reg_name(reg_map[var]->reg)+", "+itos(reg_map[array]->gp_offset)+"("+ get_reg_name(run_tmp_reg) +")");
            } else {
                text_section.emplace_back("add "+ get_reg_name(run_tmp_reg) +","+ get_reg_name(run_tmp_reg) +",$fp");
                text_section.emplace_back("lw "+get_reg_name(reg_map[var]->reg)+", "+itos(reg_map[array]->sp_offset)+"("+ get_reg_name(run_tmp_reg) +")");
            }
        }
        else if (mc.op_code == MID_CODE::GEN_LABEL) {
            tmp_reg_refresh();
            text_section.emplace_back(mc.result.name+":");
        }
        else if (mc.op_code == MID_CODE::JUMP) {
            tmp_reg_refresh();
            text_section.emplace_back("j "+mc.result.name);
        }
        else if (mc.op_code == MID_CODE::BEQ) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            std::string src2 = get_name(curr_func_name,mc.op_num2);
            if (IS_CONST(mc.op_num2.type)) {
                src2 = "const_tmp2";
                if (reg_map[src2]->reg == NONE) tmp_alloc(src2);
                text_section.emplace_back("li "+get_reg_name(reg_map[src2]->reg)+", "+itos(mc.op_num2.value));
            } else if (reg_map[src2]->reg == NONE) {
                tmp_alloc(src2);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src2]->reg)+", "+address(src2));
            }
            Reg r1 = reg_map[src1]->reg;
            Reg r2 = reg_map[src2]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("beq "+get_reg_name(r1)+", "+
                                      get_reg_name(r2)+", "+mc.result.name);

        }
        else if (mc.op_code == MID_CODE::BNE) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            std::string src2 = get_name(curr_func_name,mc.op_num2);
            if (IS_CONST(mc.op_num2.type)) {
                src2 = "const_tmp2";
                if (reg_map[src2]->reg == NONE) tmp_alloc(src2);
                text_section.emplace_back("li "+get_reg_name(reg_map[src2]->reg)+", "+itos(mc.op_num2.value));
            } else if (reg_map[src2]->reg == NONE) {
                tmp_alloc(src2);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src2]->reg)+", "+address(src2));
            }
            Reg r1 = reg_map[src1]->reg;
            Reg r2 = reg_map[src2]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("bne "+get_reg_name(r1)+", "+
                                      get_reg_name(r2)+", "+mc.result.name);
        }
        else if (mc.op_code == MID_CODE::BGEZ) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            Reg r1 = reg_map[src1]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("bgez "+get_reg_name(r1)+", "+mc.result.name);
        } else if (mc.op_code == MID_CODE::BGTZ) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            Reg r1 = reg_map[src1]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("bgtz "+get_reg_name(r1)+", "+mc.result.name);
        } else if (mc.op_code == MID_CODE::BLEZ) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            Reg r1 = reg_map[src1]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("blez "+get_reg_name(r1)+", "+mc.result.name);
        } else if (mc.op_code == MID_CODE::BLTZ) {
            std::string src1 = get_name(curr_func_name,mc.op_num1);
            if (IS_CONST(mc.op_num1.type)) {
                src1 = "const_tmp1";
                if (reg_map[src1]->reg == NONE) tmp_alloc(src1);
                text_section.emplace_back("li "+get_reg_name(reg_map[src1]->reg)+", "+itos(mc.op_num1.value));
            } else if (reg_map[src1]->reg == NONE) {
                tmp_alloc(src1);
                text_section.emplace_back("lw "+get_reg_name(reg_map[src1]->reg)+", "+address(src1));
            }
            Reg r1 = reg_map[src1]->reg;
            tmp_reg_refresh();
            text_section.emplace_back("bltz "+get_reg_name(r1)+", "+mc.result.name);
        }
        else if (mc.op_code == MID_CODE::RETURN) {
            if (mc.result.type != MID_CODE::NONE) {
                std::string ret = get_name(curr_func_name,mc.result);
                if (IS_CONST(mc.result.type)) {
                    text_section.emplace_back("li $v0, "+itos(mc.result.value));
                } else if (reg_map[ret]->reg == NONE) {
                    text_section.emplace_back("lw $v0, "+address(ret));
                } else {
                    text_section.emplace_back("move $v0, "+get_reg_name(reg_map[ret]->reg));
                }
            }
        }
        else if (mc.op_code == MID_CODE::GET_RET) {
            std::string var = get_name(curr_func_name,mc.result);
            if(DEBUG)text_section.emplace_back("#get ret to"+var);
            if (reg_map[var]->reg == NONE) {
                text_section.emplace_back("sw $v0, "+address(var));
            } else {
                text_section.emplace_back("move "+get_reg_name(reg_map[var]->reg)+", $v0");
            }
        }
        else if (mc.op_code == MID_CODE::PUSH) {
            text_section.emplace_back("subi $sp, $sp, 4");
            std::string para = get_name(curr_func_name,mc.result);
            if (IS_CONST(mc.result.type)) {
                para = "const_tmp1";
                if (reg_map[para]->reg == NONE) tmp_alloc(para);
                text_section.emplace_back("li "+get_reg_name(reg_map[para]->reg)+", "+itos(mc.result.value));
                text_section.emplace_back("sw "+get_reg_name(reg_map[para]->reg)+", "+"0($sp)");
                refresh(reg_map[para]->reg);
            } else {
                if (reg_map[para]->reg == NONE) {
                    tmp_alloc(para);
                    text_section.emplace_back("lw "+get_reg_name(reg_map[para]->reg)+", "+address(para));
                    text_section.emplace_back("sw "+get_reg_name(reg_map[para]->reg)+", "+"0($sp)");
                    refresh(reg_map[para]->reg);
                } else
                    text_section.emplace_back("sw "+get_reg_name(reg_map[para]->reg)+", "+"0($sp)");
            }
        }
        else if (mc.op_code == MID_CODE::CALL) {
            tmp_reg_refresh();
            text_section.emplace_back("jal "+mc.result.name);
        }
        else if (mc.op_code == MID_CODE::MAIN_START) {
            curr_func_name = "main";
            text_section.emplace_back("main:");
            sub = reg_map["func_main"]->stack_size;
            text_section.emplace_back("subi $sp, $sp,"+itos(sub));
            //text_section.emplace_back("sw $ra, "+itos(sub-4)+"($sp)");
            text_section.emplace_back("move $fp, $sp");
        }
        else if (mc.op_code == MID_CODE::MAIN_END) {
            sub = reg_map["func_main"]->stack_size;
            //text_section.emplace_back("lw $ra, " + itos(sub - 4) + "($sp)");
            text_section.emplace_back("addi $sp, $sp, " + itos(reg_map["func_main"]->stack_size));
            //text_section.emplace_back("jr $ra");
            text_section.emplace_back("j exit");
        }
        else if (mc.op_code == MID_CODE::FUNC_START) {
            curr_func_name = mc.result.name;
            text_section.emplace_back(curr_func_name+":");
            sub = reg_map["func_"+curr_func_name]->stack_size -
                  4 * func_para_list[curr_func_name].size();
            text_section.emplace_back("subi $sp, $sp,"+itos(sub));
            text_section.emplace_back("sw $ra, "+itos(sub-4)+"($sp)");
            text_section.emplace_back("sw $fp, "+itos(sub-8)+"($sp)");
            text_section.emplace_back("move $fp, $sp");
            auto func_used = reg_map["func_"+curr_func_name]->func_used;
            for (int i = 0; i < func_used.size(); i++) {
                if(DEBUG)text_section.emplace_back("#store local var");
                text_section.emplace_back("sw "+get_reg_name(func_used[i])+ ", " +
                                          itos(sub-4*(i+3))+"($fp)");
            }
        }
        else if (mc.op_code == MID_CODE::FUNC_END) {
            sub = reg_map["func_"+curr_func_name]->stack_size -
                  4 * func_para_list[curr_func_name].size();
            tmp_reg_refresh();
            auto func_used = reg_map["func_"+curr_func_name]->func_used;
            for (int i = 0; i < func_used.size(); i++) {
                if(DEBUG)text_section.emplace_back("#restore local var");
                text_section.emplace_back("lw "+get_reg_name(func_used[i])+ ", " +
                                          itos(sub-4*(i+3))+"($fp)");
            }
            text_section.emplace_back("lw $ra, "+itos(sub-4)+"($fp)");
            text_section.emplace_back("lw $fp, "+itos(sub-8)+"($fp)");


            text_section.emplace_back("addi $sp, $sp, "+
                                      itos(reg_map["func_"+curr_func_name]->stack_size));
            text_section.emplace_back("jr $ra");
        }
    }
    
}

void OC::mips_output(int mode) {
    if (mode == 0) {
        mips.open("mips.txt");
    }
    else if (mode == 1) {
        mips.open("opt_mips.txt");
    }
    
    for (auto & s : data_section) {
        mips << s << std::endl;
    }
    for (auto & s : init_section) {
        mips << s << std::endl;
    }
    for (auto & s : text_section) {
        mips << s << std::endl;
    }
    mips.close();
}