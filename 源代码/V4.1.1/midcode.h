#ifndef _MIDCODE_H_
#define _MIDCODE_H_

#include "global.h"
#include "lex.h"
#include "grammer.h"
#include "symbol.h"
#include "semantic.h"
#include<vector>

#define MAXTEMPLENGTH 15

typedef enum{
    PARA , FUNC_BEGIN , FUNCC_END , RET , LABLE , JMP , JNP , JEP , VOID_FUNC_CALL ,NONVOID_FUNC_CALL,
    F_PLUS , F_MINUS , F_MULTIPY , F_DIVD , ARRA_ASSIGN_R , ARRA_ASSIGN_L ,
    F_BT , F_BAET , F_LT , F_LAET , F_EQ , F_NEQ , F_ASSIGN , F_PRINTF , F_SCANF,
    NEG
}fourth_type;

typedef struct
{
    fourth_type type;//四元式的类型
    char op1[MaxTokenLength];
    char op2[MaxTokenLength];
    char op3[MAXTEMPLENGTH];
}midcode;

extern std::vector<midcode*> midcode_list;//中间代码表

void gen_code(ASTNODE* tree);
void output_midcode();

#endif // _MIDCODE_H_
