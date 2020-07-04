#include "global.h"
#include "lex.h"
#include "grammer.h"
#include "symbol.h"
#include "semantic.h"
#include "midcode.h"
#include "mips.h"
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

static int num_str = 1;//用来表明当前是处理到了哪一个字符串，(字符串仅在printf中出现)
FILE* fp_result;
FILE * fp_source;
FILE * fp_log;

void lw(char* reg_name,char*  vari_name);//将一个变量放入某一个寄存器中
void sw(char* reg_name,char*  vari_name);//将某一个寄存器中的内容存给某一个变量
int find_addr(char* vari_name);//找到某一个变量关于fp的偏移地址，如果这个变量是在top符号表中，则返回-1
int gettotalsize(char* func_name);//对一个函数，计算具体的栈的大小
void gen_mips();//生成mips汇编
void gen_data();//生成data段的描述，具体内容是全局变量和printf的字符串
void gen_text();//生成代码段的描述
bool is_const_value(char* input);//判断是否是一个常量，如para 1 | para 'c'

bool is_const_value(char* input)
{
    if(input[0] >= '0' && input[0] <= '9' || input[0] == '\'')
    {
        return true;
    }
    return false;
}

void lw(char* reg_name,char*  vari_name)
{
    //根据一个变量的名字将其加载到寄存器中
    //1.调用find_addr找到这个变量的地址
    //2.如果返回-1则是全局变量直接索引其名字,可以是int可以是char ，先la $t1 ，name ，lw reg_name , 0($t1)
    //3.如果不是-1则生成lw reg_name , addr(fp)

    int addr = find_addr(vari_name);
    if(addr == -1)
    {
        //说明此时是索引的全局变量
        fprintf(fp_result,"\tla\t$t1,%s\n",vari_name);
        fprintf(fp_result,"\tlw\t%s,0($t1)\n",reg_name);
    }else
    {
        fprintf(fp_result,"\tlw\t%s,%d($fp)\n",reg_name,addr);
    }

}

void sw(char* reg_name,char*  vari_name)
{
    //将寄存器的内容还原到内容中改变量的位置

    int addr = find_addr(vari_name);
    if(addr == -1)
    {
        //说明是一个全局变量
        fprintf(fp_result,"\tla\t$t2,%s\n",vari_name);
        fprintf(fp_result,"\tsw\t%s,0($t2)\n",reg_name);
    }else
    {
        fprintf(fp_result,"\tsw\t%s,%d($fp)\n",reg_name,addr);
    }
}

int find_addr(char* vari_name)
{
    //根据变量的名字来找在符号表的位置
    entry* result = find_table(vari_name);

    if(in == FALSE)
    {
        //说明是在全局符号表中找到的
        return -1;
    }else
    {
        //说明是在局部符号表中找到的
        entry* temp = find_table(cur->name);//找到说明当前函数的符号表项
        int para_num = temp->paranum;

        if(result->addr < para_num)
        {
            //这个变量是一个参数
            return -(result->addr - para_num)*4;
        }else
        {
            //这个变量是一个局部变量
            return -(result->addr - para_num)*4 - 8;
        }

    }
}
int gettotalsize(char* func_name)
{
    //计算函数的栈空间大小
    entry* fun = find_table(func_name);

    return 4 + (cur->totaladdr - fun->paranum)*4;
}
void gen_data()
{
    //产生data段
    //1.首先所有的全局变量都需要有一个声明，name : .word ...
    //2.如果是数组的话，name: .space arraysize
    //3.这些都是利用top符号表
    //4.还需要遍历中间代码表，找到printf对应的字符串
    entry* temp;
    int i;

    for(i=0;i<MaxBukSize;i++)
    {
        temp = top->bucket[i];
        while(temp)
        {
            if(temp->type == TConst && temp->kind == Char)
            {
                fprintf(fp_result,"\t%s:\t.word \'%c\'\n",temp->name,temp->val.ch_val);
            }else if(temp->type == TConst && temp->kind == Integer)
            {
                fprintf(fp_result,"\t%s:\t.word %d\n",temp->name,temp->val.int_val);
            }else if(temp->type == TVariable)
            {
                fprintf(fp_result,"\t%s:\t.word %d\n",temp->name,0);
            }else if(temp->type == TArray)
            {
                fprintf(fp_result,"\t%s:\t.space %d\n",temp->name,(temp->array_size)*4);
            }

            temp = temp->next;
        }
    }

    //遍历中间代码表，将所有printf语句中的字符串写进来
    for(i=0;i<midcode_list.size();i++)
    {
        if(midcode_list[i]->type == F_PRINTF)
        {
            if(strcmp(midcode_list[i]->op1,"") != 0)
            {
                //表示此时printf有字符串参数
                fprintf(fp_result,"\tmsg%d:\t.asciiz \"%s\"\n",num_str++,midcode_list[i]->op1);
            }
        }
    }
    num_str = 1;//重新将num_str赋值为1，之后每处理完一条printf语句就加一

}

void gen_text()
{
    //生成代码段
    int i;
    int para_num;
    entry* p;
    symboltable* q;
    int size_of_stack;
    int temp;

    for(i=0;i<midcode_list.size();i++)
    {
        //遍历每一条中间代码生成对应的mips
        switch(midcode_list[i]->type)
        {
            case PARA:
                if(is_const_value(midcode_list[i]->op1))
                {
                    //PARA 1 | PARA 'C'
                    fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op1);//li $t1,'c' | li $t1,1
                    fprintf(fp_result,"\tsw\t$t1,0($sp)\n");//sw $t1 ,0($sp)
                }else
                {
                    //PARA a
                    fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                    lw("$t1",midcode_list[i]->op1);//把这个变量放到$t1
                    fprintf(fp_result,"\tsw\t$t1,0($sp)\n");//sw $t1 ,0($sp)
                }
                break;
            case VOID_FUNC_CALL:
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                para_num = find_table(midcode_list[i]->op1)->paranum;
                fprintf(fp_result,"\tli\t$t1,%d\n",para_num*4);
                fprintf(fp_result,"\tadd\t$sp,$sp,$t1\n");
                break;
            case NONVOID_FUNC_CALL://返回值是在v1里面
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                para_num = find_table(midcode_list[i]->op1)->paranum;
                fprintf(fp_result,"\tli\t$t1,%d\n",para_num*4);
                fprintf(fp_result,"\tadd\t$sp,$sp,$t1\n");
                sw("$v1",midcode_list[i]->op3);
                break;
            case FUNC_BEGIN:
                //先设置符号表cur指针
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
                //再生成一个函数开始标号
                fprintf(fp_result,"%s:\n",midcode_list[i]->op1);
                //处理函数调用进入时的事务
                fprintf(fp_result,"\tsw\t$ra,0($sp)\n");
                fprintf(fp_result,"\tsw\t$fp,-4($sp)\n");
                fprintf(fp_result,"\tadd\t$fp,$sp,$0\n");
                size_of_stack = gettotalsize(cur->name);
                fprintf(fp_result,"\tsubi\t$sp,$sp,%d\n",size_of_stack);
                break;
            case RET:
                if(strcmp(cur->name,"main") != 0)
                {
                        if(strcmp(midcode_list[i]->op1,"") != 0)
                        {
                            //说明还有返回值
                            if(is_const_value(midcode_list[i]->op1))
                            {
                                //是一个常量 return 1
                                fprintf(fp_result,"\tli\t$v1,%s\n",midcode_list[i]->op1);
                            }else
                            {
                                lw("$v1",midcode_list[i]->op1);
                            }
                        }
                        fprintf(fp_result,"\tadd\t$sp,$fp,$0\n");
                        fprintf(fp_result,"\tlw\t$ra,0($sp)\n");
                        fprintf(fp_result,"\taddi\t$sp,$sp,4\n");//addi $sp,$sp,4
                        fprintf(fp_result,"\tlw\t$fp,-8($sp)\n");//fp = $sp - 8处的内容
                        fprintf(fp_result,"\tjr\t$ra\n");
                }
                break;
            case LABLE:
                fprintf(fp_result,"%s:\n",midcode_list[i]->op1);
                break;
            case JMP:
                fprintf(fp_result,"\tj\t%s\n",midcode_list[i]->op1);
                break;
            case JNP:
            case JEP:
                break;
            case F_BT:
            case F_BAET:
            case F_LT:
            case F_LAET:
            case F_EQ:
            case F_NEQ:
                //> 要根据后面一条四元式来决定调到哪里
                if(is_const_value(midcode_list[i]->op1))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op1);
                }
                else
                {
                    lw("$t1",midcode_list[i]->op1);
                }

                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op2);
                }
                else
                {
                    lw("$t2",midcode_list[i]->op2);
                }

                switch(midcode_list[i]->type)
                {
                case F_BT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbgt\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tble\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                case F_BAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbge\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tblt\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                case F_LT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tblt\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbge\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                case F_LAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tble\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbgt\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                case F_EQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbeq\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbne\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                case F_NEQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbne\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbeq\t$t1,$t2,%s\n",midcode_list[i+1]->op1);
                    break;
                }

                break;
            case F_PLUS:
            case F_MINUS:
            case F_MULTIPY:
            case F_DIVD:
                //t1 = a + b
                if(is_const_value(midcode_list[i]->op1))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op1);
                }else
                {
                    lw("$t1",midcode_list[i]->op1);
                }
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op2);
                }
                else
                {
                    lw("$t2",midcode_list[i]->op2);
                }

                switch(midcode_list[i]->type)
                {
                case F_PLUS:
                    fprintf(fp_result,"\tadd\t$t3,$t1,$t2\n");
                    sw("$t3",midcode_list[i]->op3);
                    break;
                case F_MINUS:
                    fprintf(fp_result,"\tsub\t$t3,$t1,$t2\n");
                    sw("$t3",midcode_list[i]->op3);
                    break;
                case F_MULTIPY:
                    fprintf(fp_result,"\tmul\t$t3,$t1,$t2\n");
                    sw("$t3",midcode_list[i]->op3);
                    break;
                case F_DIVD:
                    fprintf(fp_result,"\tdiv\t$t1,$t2\n");
                    fprintf(fp_result,"\tmflo\t$t3\n");
                    sw("$t3",midcode_list[i]->op3);
                    break;
                }
                break;
            case F_ASSIGN:
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    lw("$t1",midcode_list[i]->op2);
                }

                sw("$t1",midcode_list[i]->op1);
                break;
            case ARRA_ASSIGN_R:
                // t = a[1]
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    lw("$t1",midcode_list[i]->op2);
                }

                fprintf(fp_result,"\tsll\t$t1,$t1,2\n");

                temp = find_addr(midcode_list[i]->op1);
                if(temp == -1)
                {
                    //说明是全局
                    fprintf(fp_result,"\tla\t$t2,%s\n",midcode_list[i]->op1);
                }else
                {
                    fprintf(fp_result,"\tla\t$t2,%d($fp)\n",temp);
                }

                fprintf(fp_result,"\tsub\t$t3,$t2,$t1\n");
                fprintf(fp_result,"\tlw\t$t2,0($t3)\n");

                sw("$t2",midcode_list[i]->op3);
                break;
            case ARRA_ASSIGN_L:
                //a[1] = t
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    lw("$t1",midcode_list[i]->op2);
                }

                fprintf(fp_result,"\tsll\t$t1,$t1,2\n");

                temp = find_addr(midcode_list[i]->op1);
                if(temp == -1)
                {
                    //说明是全局
                    fprintf(fp_result,"\tla\t$t2,%s\n",midcode_list[i]->op1);
                }else
                {
                    fprintf(fp_result,"\tla\t$t2,%d($fp)\n",temp);
                }
                fprintf(fp_result,"\tsub\t$t3,$t2,$t1\n");

                if(is_const_value(midcode_list[i]->op3))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op3);
                }else
                {
                    lw("$t2",midcode_list[i]->op3);
                }

                fprintf(fp_result,"\tsw\t$t2,0($t3)\n");
                break;
            case F_PRINTF:
                if(strcmp(midcode_list[i]->op1,"") != 0)
                {
                    //需要输出字符串
                    //输出的是第num_str++号字符串
                    fprintf(fp_result,"\tli\t$v0,4\n");
                    fprintf(fp_result,"\tla\t$a0,msg%d\n",num_str++);
                    fprintf(fp_result,"\tsyscall\n");
                }

                if(strcmp(midcode_list[i]->op2,"") != 0)
                {
                    if(is_const_value(midcode_list[i]->op2))
                    {
                        fprintf(fp_result,"\tli\t$a0,%s\n",midcode_list[i]->op2);
                    }else
                    {
                        lw("$a0",midcode_list[i]->op2);
                    }
                    if(strcmp(midcode_list[i]->op3,"int") == 0)
                    {
                        fprintf(fp_result,"\tli\t$v0,1\n");
                    }else
                    {
                        fprintf(fp_result,"\tli\t$v0,11\n");
                    }
                    fprintf(fp_result,"\tsyscall\n");
                }
                break;
            case F_SCANF:
                p = find_table(midcode_list[i]->op1);
                if(p->kind == Char)
                {
                    fprintf(fp_result,"\tli\t$v0,12\n");
                    fprintf(fp_result,"\tsyscall\n");
                    sw("$v0",midcode_list[i]->op1);
                }else
                {
                    fprintf(fp_result,"\tli\t$v0,5\n");
                    fprintf(fp_result,"\tsyscall\n");
                    sw("$v0",midcode_list[i]->op1);
                }
                break;
            case NEG:
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    lw("$t1",midcode_list[i]->op2);
                }
                fprintf(fp_result,"\tsub\t$t1,$0,$t1\n");
                sw("$t1",midcode_list[i]->op1);
                break;
        }
    }
}

void gen_mips()
{
    fprintf(fp_result,".data\n");
    gen_data();
    fprintf(fp_result,".text\n");
    fprintf(fp_result,"\tj\tmain\n");
    gen_text();
}

int main()
{
    fp_source = fopen("./测试程序V2/test3.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误
                                                    //暂时约定一下每一个方法都要有return，不然得对void类型没有return做特殊处理，即为func_end添加处理事务
    ASTNODE* result = program();

    build_table(result);

    typecheck(result);

    printf("\nSemantic Analyze Complete...\n\n");

    gen_code(result);

    printtable();

    printf("\nmidcode :\n");

    output_midcode();

    fp_result = fopen("./result.asm","w");

    gen_mips();

    printf("\nmips code already generated...\n");

    return 0;
}
