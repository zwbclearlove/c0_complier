#include "lexical_analyzer.h"
#include "syntactic_analyzer.h"
#include "error.h"
#include "middle_code.h"
#include "object_code.h"
#include "optimize.h"

int main() {
    LEX::out.open("output.txt");
    LEX::file.open("testfile.txt");
    err.open("error.txt");
    program();
    LEX::file.close();
    LEX::out.close();
    err.close();

    if (MID_CODE_OUTPUT && !error_in_program) {
        MID_CODE::mid_code_output(0);
    }
    if (OPTIMIZE && !error_in_program) {
        OPT::optimize();
        MID_CODE::mid_code_output(1);
    }
    if (!error_in_program) {
        OC::translate();
        OC::mips_output(0);
    }
    
    return 0;
}