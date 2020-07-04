#include "global.h"
#include "lex.h"
#include "grammer.h"
#include "symbol.h"
#include "semantic.h"
#include "midcode.h"
#include "mips.h"
#include "optimizer.h"
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

FILE* fp_result;
FILE * fp_source;
FILE * fp_log;

//extern std::vector<midcode*> midcode_list;//中间代码表
//typedef struct
//{
//    fourth_type type;//四元式的类型
//    char op1[MaxTokenLength];
//    char op2[MaxTokenLength];
//    char op3[MAXTEMPLENGTH];
//}midcode;


void optimizer();
void merge_const_value();
void delete_temp_value();
void const_replace();
void delete_useless_jmp();

void merge_const_value()//合并常量 ,将四元式中的 add 1 2 t1 || add 'c' 'b' t1
{
    for(int i=0;i<midcode_list.size();i++)
    {
        if(midcode_list[i]->type == F_PLUS || midcode_list[i]->type == F_MINUS || midcode_list[i]->type == F_MULTIPY || midcode_list[i]->type == F_DIVD)
        {
            //如果是+ | - | * | /
            if(is_const_value(midcode_list[i]->op1) && is_const_value(midcode_list[i]->op2))
            {
                if(midcode_list[i]->op1[0] == '\'')
                {
                    //两个都是char的加减乘除
                    char temp1 = midcode_list[i]->op1[1];
                    char temp2 = midcode_list[i]->op1[2];
                    char result = 0;
                    switch(midcode_list[i]->type)
                    {
                    case F_PLUS:
                        result = temp1 + temp2;
                        break;
                    case F_MINUS:
                        result = temp1 - temp2;
                        break;
                    case F_MULTIPY:
                        result = temp1 * temp2;
                        break;
                    case F_DIVD:
                        result = temp1 / temp2;
                        break;
                    }
                    midcode_list[i]->type = F_ASSIGN;
                    strcpy(midcode_list[i]->op1,midcode_list[i]->op3);
                    strcpy(midcode_list[i]->op3,"");
                    sprintf(midcode_list[i]->op2,"\'%c\'",result);

                }else
                {
                    //两个都是int的加减乘除
                    int temp1 = atoi(midcode_list[i]->op1);
                    int temp2 = atoi(midcode_list[i]->op2);
                    int result = 0;

                    switch(midcode_list[i]->type)
                    {
                    case F_PLUS:
                        result = temp1 + temp2;
                        break;
                    case F_MINUS:
                        result = temp1 - temp2;
                        break;
                    case F_MULTIPY:
                        result = temp1 * temp2;
                        break;
                    case F_DIVD:
                        result = temp1 / temp2;
                        break;
                    }
                    midcode_list[i]->type = F_ASSIGN;
                    strcpy(midcode_list[i]->op1,midcode_list[i]->op3);
                    strcpy(midcode_list[i]->op3,"");
                    sprintf(midcode_list[i]->op2,"%d",result);
                }
            }
        }
    }
}
void delete_temp_value()
{
    //消除形如 +|-|*|/               3       2       $t6
    //         =               t       $t6
    //目标：   +|-|*|/               3       2        t

    //以及形如 =               $t6     5
    //         =               t       $t6
    //目标     =               t       5

    //call            t               $t7
    //=               result  $t7
    //目标： call            t               result
    int length = midcode_list.size();

    for(int i=0;i<length;i++)
    {
        if(midcode_list[i]->type == F_PLUS || midcode_list[i]->type == F_MINUS || midcode_list[i]->type == F_MULTIPY || midcode_list[i]->type == F_DIVD || midcode_list[i]->type == F_ASSIGN || midcode_list[i]->type == NONVOID_FUNC_CALL)
        {
            if(midcode_list[i]->type == F_ASSIGN)
            {
                if((i + 1)<length && midcode_list[i+1]->type == F_ASSIGN && strcmp(midcode_list[i]->op1,midcode_list[i+1]->op2) == 0)
                {
                    strcpy(midcode_list[i]->op1,midcode_list[i+1]->op1);
                    midcode_list.erase(midcode_list.begin()+i+1);//删掉第二个赋值
                    length--;
                }
            }else if(midcode_list[i]->type == NONVOID_FUNC_CALL)
            {
                if((i+1)<length && midcode_list[i+1]->type == F_ASSIGN && strcmp(midcode_list[i]->op3,midcode_list[i+1]->op2) == 0)
                {
                    strcpy(midcode_list[i]->op3,midcode_list[i+1]->op1);
                    midcode_list.erase(midcode_list.begin()+i+1);
                    length--;
                }

            }else
            {
                //+|-|*|/
                if((i + 1)<length && midcode_list[i+1]->type == F_ASSIGN && strcmp(midcode_list[i]->op3,midcode_list[i+1]->op2) == 0)
                {
                    strcpy(midcode_list[i]->op3,midcode_list[i+1]->op1);
                    midcode_list.erase(midcode_list.begin()+i+1);//删掉赋值
                    length--;
                }
            }
        }
    }

}

void const_replace()
{
    //对于程序中的常量，直接用值来代替其变量
    //const int a = 1;
    //add a , b , c
    //目标 add 1 , b , c
    entry* temp1 = NULL;
    entry* temp2 = NULL;
    symboltable* q;

    for(int i=0;i<midcode_list.size();i++)
    {
        switch(midcode_list[i]->type)
        {
        case PARA:
            if(!is_const_value(midcode_list[i]->op1))
            {
                temp1 = find_table(midcode_list[i]->op1);
                if(temp1->type == TConst)
                {
                    if(temp1->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op1,"%d",temp1->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op1,"\'%c\'",temp1->val.ch_val);
                    }
                }
            }
            break;
        case FUNC_BEGIN:
            //设置符号表指针
            if(strcmp(midcode_list[i]->op1,"main") == 0)
            {
                cur = top->mainchild;
            }else
            {
                q = top->funchild;
                while(q)
                {
                    if(strcmp(q->name,midcode_list[i]->op1) == 0)
                    {
                        cur = q;
                        break;
                    }
                    q = q->sibling;
                }
            }
            break;
        case RET:
            if(strcmp(midcode_list[i]->op1,"") != 0 && !is_const_value(midcode_list[i]->op1))
            {
                temp1 = find_table(midcode_list[i]->op1);
                if(temp1->type == TConst)
                {
                    if(temp1->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op1,"%d",temp1->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op1,"\'%c\'",temp1->val.ch_val);
                    }
                }
            }
            break;
        case F_PLUS:
        case F_MINUS:
        case F_MULTIPY:
        case F_DIVD:
        case F_BAET:
        case F_BT:
        case F_LAET:
        case F_LT:
        case F_EQ:
        case F_NEQ:
            if(!is_const_value(midcode_list[i]->op1))
            {
                temp1 = find_table(midcode_list[i]->op1);
                if(temp1->type == TConst)
                {
                    if(temp1->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op1,"%d",temp1->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op1,"\'%c\'",temp1->val.ch_val);
                    }
                }
            }
            if(!is_const_value(midcode_list[i]->op2))
            {
                temp2 = find_table(midcode_list[i]->op2);
                if(temp2->type == TConst)
                {
                    if(temp2->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op2,"%d",temp2->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op2,"\'%c\'",temp2->val.ch_val);
                    }
                }
            }
            break;
        case ARRA_ASSIGN_R:
            if(!is_const_value(midcode_list[i]->op2))
            {
                temp2 = find_table(midcode_list[i]->op2);
                if(temp2->type == TConst)
                {
                    if(temp2->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op2,"%d",temp2->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op2,"\'%c\'",temp2->val.ch_val);
                    }
                }
            }
            break;
        case ARRA_ASSIGN_L:
            if(!is_const_value(midcode_list[i]->op2))
            {
                temp1 = find_table(midcode_list[i]->op2);
                if(temp1->type == TConst)
                {
                    if(temp1->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op2,"%d",temp1->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op2,"\'%c\'",temp1->val.ch_val);
                    }
                }
            }
            if(!is_const_value(midcode_list[i]->op3))
            {
                temp2 = find_table(midcode_list[i]->op3);
                if(temp2->type == TConst)
                {
                    if(temp2->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op3,"%d",temp2->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op3,"\'%c\'",temp2->val.ch_val);
                    }
                }
            }
            break;
        case F_ASSIGN:
        case F_PRINTF:
            if(strcmp(midcode_list[i]->op2,"") != 0 && !is_const_value(midcode_list[i]->op2))
            {
                temp2 = find_table(midcode_list[i]->op2);
                if(temp2->type == TConst)
                {
                    if(temp2->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op2,"%d",temp2->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op2,"\'%c\'",temp2->val.ch_val);
                    }
                }
            }
            break;
        case NEG:
            if(!is_const_value(midcode_list[i]->op2))
            {
                temp2 = find_table(midcode_list[i]->op2);
                if(temp2->type == TConst)
                {
                    if(temp2->kind == Integer)
                    {
                        sprintf(midcode_list[i]->op2,"%d",-temp2->val.int_val);
                    }else
                    {
                        sprintf(midcode_list[i]->op2,"\'%c\'",-temp2->val.ch_val);
                    }
                }
            }else
            {
                if(midcode_list[i]->op2[0] == '\'')
                {
                    char t = midcode_list[i]->op2[1];
                    sprintf(midcode_list[i]->op2,"\'%c\'",-t);
                }else
                {
                    int t = atoi(midcode_list[i]->op2);
                    sprintf(midcode_list[i]->op2,"%d",-t);
                }
            }

            midcode_list[i]->type = F_ASSIGN;


        }

    }
}

void delete_useless_jmp()
{
    //删除无用的跳转和标号 | 无else部分的if语句的冗余生成代码
    int length = midcode_list.size();
    for(int i=0;i<length;i++)
    {
        if(midcode_list[i]->type == JMP)
        {
            if((i+1)<length && (i+2)<length && midcode_list[i+1]->type == LABLE && midcode_list[i+2]->type == LABLE && strcmp(midcode_list[i]->op1,midcode_list[i+2]->op1) == 0)
            {
                midcode_list.erase(midcode_list.begin()+i+2);
                length--;
                midcode_list.erase(midcode_list.begin()+i);
                length--;
                i--;
            }
        }
    }

}
void optimizer()
{
    const_replace();//优化3 常量替换

    merge_const_value();//优化1 常量加减乘除改赋值

    delete_temp_value();//优化2 删除冗余局部变量

    delete_useless_jmp();//优化4 删除无用的跳转和标号
}

int main()
{
    fp_source = fopen("./测试程序V2/test5.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误
                                                    //暂时约定一下每一个方法都要有return，不然得对void类型没有return做特殊处理，即为func_end添加处理事务
    ASTNODE* result = program();

    build_table(result);

    typecheck(result);

    printf("\nSemantic Analyze Complete...\n\n");

    gen_code(result);

    printtable();

    printf("\nmidcode :\n");

//    optimizer();

    output_midcode();

    fp_result = fopen("./result.asm","w");

    gen_mips();

    printf("\nmips code already generated...\n");

    return 0;
}
