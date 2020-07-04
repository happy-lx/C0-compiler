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


//extern std::vector<midcode*> midcode_list;//中间代码表
//typedef struct
//{
//    fourth_type type;//四元式的类型
//    char op1[MaxTokenLength];
//    char op2[MaxTokenLength];
//    char op3[MAXTEMPLENGTH];
//}midcode;


void merge_const_value();
void delete_temp_value();
void const_replace();
void delete_useless_jmp();
void merge_same_exp();


void merge_same_exp()
{
    //合并局部公共子表达式
    //修改形如 + a b t1
    //         + a b t2

    //目标     + a b t1
    //         = t2 t1

    //要求a与t1不相等，b与t1不相等，然后两个表达式之间 a ， b  ， t1 的值没有被修改过
    int j;
    int k;
    for(int i=0;i<midcode_list.size();i++)
    {
        if(midcode_list[i]->type == F_PLUS || midcode_list[i]->type == F_MINUS || midcode_list[i]->type == F_MULTIPY || midcode_list[i]->type == F_DIVD)
        {
            if(strcmp(midcode_list[i]->op1,midcode_list[i]->op3) != 0 && strcmp(midcode_list[i]->op2,midcode_list[i]->op3))
            {
                //从当前开始，在这一个基本块内寻找公共子表达式
                j = i + 1;
                while(j < midcode_list.size() && (midcode_list[j]->type != PARA || midcode_list[j]->type != NONVOID_FUNC_CALL || midcode_list[j]->type != VOID_FUNC_CALL || midcode_list[j]->type != RET || midcode_list[j]->type != FUNCC_END || midcode_list[j]->type != JEP || midcode_list[j]->type != JNP || midcode_list[j]->type != JMP))
                {
                    if(midcode_list[j]->type == midcode_list[i]->type && strcmp(midcode_list[i]->op1,midcode_list[j]->op1) == 0 && strcmp(midcode_list[i]->op2,midcode_list[j]->op2) == 0)
                    {
                        //现在需要查找i 和 j之间的四元式看看是否有对a，b，t1值的修改
                        k = i + 1;
                        for(;k<j;k++)
                        {
                            if(midcode_list[k]->type == NEG || midcode_list[k]->type == F_SCANF || midcode_list[k]->type == F_ASSIGN)
                            {
                                if(strcmp(midcode_list[k]->op1,midcode_list[i]->op1) == 0 || strcmp(midcode_list[k]->op1,midcode_list[i]->op2) == 0 || strcmp(midcode_list[k]->op1,midcode_list[i]->op3) == 0)
                                {
                                    break;
                                }
                            }
                            if(midcode_list[k]->type == F_PLUS || midcode_list[k]->type == F_MINUS || midcode_list[k]->type == F_MULTIPY || midcode_list[k]->type == F_DIVD || midcode_list[k]->type == ARRA_ASSIGN_R)
                            {
                                if(strcmp(midcode_list[k]->op3,midcode_list[i]->op1) == 0 || strcmp(midcode_list[k]->op3,midcode_list[i]->op2) == 0 || strcmp(midcode_list[k]->op3,midcode_list[i]->op3) == 0)
                                {
                                    break;
                                }
                            }

                        }
                        if(k == j)
                        {
                            //说明不是中途退出的，也就是说检查到了最后没有对 a , b , t1的修改，即可以进行局部公共子表达式的替换
                            midcode_list[j]->type = F_ASSIGN;
                            strcpy(midcode_list[j]->op1,midcode_list[j]->op3);
                            strcpy(midcode_list[j]->op2,midcode_list[i]->op3);
                            strcpy(midcode_list[j]->op3,"");
                        }
                    }
                    j++;
                }
            }

        }
    }
}

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
            if(midcode_list[i]->type == F_ASSIGN && midcode_list[i]->op1[0] == '$')
            {
                if((i + 1)<length && midcode_list[i+1]->type == F_ASSIGN && strcmp(midcode_list[i]->op1,midcode_list[i+1]->op2) == 0)
                {
                    strcpy(midcode_list[i]->op1,midcode_list[i+1]->op1);
                    midcode_list.erase(midcode_list.begin()+i+1);//删掉第二个赋值
                    length--;
                }
            }else if(midcode_list[i]->type == NONVOID_FUNC_CALL && midcode_list[i]->op3[0] == '$')
            {
                if((i+1)<length && midcode_list[i+1]->type == F_ASSIGN && strcmp(midcode_list[i]->op3,midcode_list[i+1]->op2) == 0)
                {
                    strcpy(midcode_list[i]->op3,midcode_list[i+1]->op1);
                    midcode_list.erase(midcode_list.begin()+i+1);
                    length--;
                }

            }else if(midcode_list[i]->op3[0] == '$')
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
        case F_BT:
        case F_BAET:
        case F_LT:
        case F_LAET:
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
    int len = midcode_list.size();
    do
    {
        len = midcode_list.size();

        const_replace();//优化3 常量替换

        merge_const_value();//优化1 常量加减乘除改赋值

        delete_temp_value();//优化2 删除冗余局部变量

        delete_useless_jmp();//优化4 删除无用的跳转和标号

        merge_same_exp();//优化5 合并局部公共子表达式

    }while(len != midcode_list.size());
}

//int main()
//{
//    fp_source = fopen("./测试程序V2/test5.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误 , test11有死循环
//                                                    //暂时约定一下每一个方法都要有return，不然得对void类型没有return做特殊处理，即为func_end添加处理事务
//    ASTNODE* result = program();
//
//    build_table(result);
//
//    typecheck(result);
//
//    printf("\nSemantic Analyze Complete...\n\n");
//
//    gen_code(result);
//
//    printtable();
//
//    printf("\nmidcode :\n");
//
//    if(opticlock)
//    {
//        optimizer();
//    }
//
//    output_midcode();
//
//    fp_result = fopen("./result.asm","w");
//
//    gen_mips();
//
//    printf("\nmips code already generated...\n");
//
//    return 0;
//}
