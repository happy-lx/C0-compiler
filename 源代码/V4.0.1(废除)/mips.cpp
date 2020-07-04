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
#include<queue>

static int num_str = 1;//用来表明当前是处理到了哪一个字符串，(字符串仅在printf中出现)
FILE* fp_result;
FILE * fp_source;
FILE * fp_log;

Reg Reg_info[maxregnum];
std::queue<Reg*> Q;

void lw(char* reg_name,char*  vari_name);//将一个变量放入某一个寄存器中
void sw(char* reg_name,char*  vari_name);//将某一个寄存器中的内容存给某一个变量
int find_addr(char* vari_name);//找到某一个变量关于fp的偏移地址，如果这个变量是在top符号表中，则返回-1
int gettotalsize(char* func_name);//对一个函数，计算具体的栈的大小
void gen_mips();//生成mips汇编
void gen_data();//生成data段的描述，具体内容是全局变量和printf的字符串
void gen_text();//生成代码段的描述
bool is_const_value(char* input);//判断是否是一个常量，如para 1 | para 'c'
void init_reg_info()//把reg_info里面的内容全部清空
void save_registers()//保存s0-s7寄存器在栈中 ，(在函数调用的时候)
void replace_registers()//还原s0-s7寄存器
int get_reg(char* input)//利用LRU策略取到一个寄存器
int in_reg(char* input)//通过变量名或者常量值来找寄存器描述符 如果找到了就返回下标没有就返回-1

void init_reg_info()
{
    //把reg_info里面的内容全部清空
    for(int i=0;i<maxregnum;i++)
    {
        Reg_info[i].id = i;
        Reg_info[i].busy = false;
        Reg_info[i].vari_info.clear();
    }
    Q = std::queue<Reg*>();//清空队列
}

void save_registers()
{
    //保存s0-s7寄存器在栈中 ，(在函数调用的时候)
    for(int i=0;i<maxregnum;i++)
    {
        fprintf(fp_result,"\tsw\t$s%d,%d($sp)\n",i,(-4*(i+1)));
    }
    fprintf(fp_result,"\tsubi\t$sp,$sp,32\n");//subi $sp,$sp,32  | 4*8=32 | 最后sp指向s7的保留位置
}
void replace_registers()
{
    //还原s0-s7寄存器
    //现在sp指向s7的位置
    for(int i=0;i<maxregnum;i++)
    {
        fprintf(fp_result,"\tlw\t$s%d,%d($sp)\n",(maxregnum-1-i),4*i);
    }
    fprintf(fp_result,"\taddi\t$sp,$sp,32\n");//addi $sp,$sp,32  | 4*8=32 | 最后sp指向最后一个参数的保留位置
}

int in_reg(char* input)
{
    for(int i=0;i<maxregnum;i++)
    {
        if(Reg_info[i].busy == true)
        {
            for(j=0;j<Reg_info[i].vari_info.size();j++)
            {
                if(strcmp(Reg_info[i].vari_info[j],input) == 0)
                {
                    return i;//找到是第i个寄存器内有这个变量或者常量
                }
            }
        }
    }
    return -1;//没找到
}
int get_reg()
{
    //利用FIFO策略取到一个寄存器
    //1.先找一遍有没有空的寄存器，有的话直接分配，设置busy和push到队列
    //2.如没有空的寄存器，则找队列头元素返回,同时这个寄存器内的变量全部溢出到内存中
    for(int i=0;i<maxregnum;i++)
    {
        if(Reg_info[i].busy == false)
        {
            Reg_info[i].busy == true;
            Q.push(&(Reg_info[i]));
            return i;
        }
    }

    Reg* head = Q.front();
    Q.pop();

    char reg_name_buf[10];
    sprintf(reg_name_buf,"$s%d",head->id);

    //这个寄存器内容溢出到内存中
    for(int i=0;i<head->vari_info.size();i++)
    {
        if(! is_const_value(head->vari_info[i]))
        {
            //是一个变量
            if(strlen(head->vari_info[i]) <= 2 || !(head->vari_info[i][0] == '$' && head->vari_info[i][1] == '$'))
            {
                sw(reg_name_buf,head->vari_info[i])//例如sw($s1,a)
            }
        }
    }
    Reg_info[head->id].vari_info.clear();
    Q.push(head);

    return head->id;

    //后续需要把对应的变量的名字给push到vector里面
}

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
            return -(result->addr - para_num)*4 + 32;//可能需要修改一下，加上8个保留寄存器的偏移地址
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
    int reg_num1;
    int reg_num2;
    int reg_num3;
    char reg_name_buf1[10];
    char reg_name_buf2[10];
    char reg_name_buf3[10];

    for(i=0;i<midcode_list.size();i++)
    {
        //遍历每一条中间代码生成对应的mips
        switch(midcode_list[i]->type)
        {
            case PARA:
                reg_num1 = in_reg(midcode_list[i]->op1);
                if(reg_num1 == -1)
                {
                    //说明这个变量或者常量不在寄存器中
                    reg_num1 = get_reg();
                    sprintf(reg_name_buf1,"$s%d",reg_num1);
                    if(is_const_value(midcode_list[i]->op1))
                    {
                        //PARA 1 | PARA 'C'
                        fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                        fprintf(fp_result,"\tli\t%s,%s\n",reg_name_buf1,midcode_list[i]->op1);
                        fprintf(fp_result,"\tsw\t%s,0($sp)\n",reg_name_buf1);
                    }else
                    {
                        //PARA a
                        fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                        lw(reg_name_buf1,midcode_list[i]->op1);
                        fprintf(fp_result,"\tsw\t%s,0($sp)\n",reg_name_buf1);
                    }
                    Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);

                }else
                {
                    //这个变量已经在寄存器中了
                    fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                    fprintf(fp_result,"\tsw\t$s%d,0($sp)\n",reg_num1);
                }

//                if(is_const_value(midcode_list[i]->op1))
//                {
//                    //PARA 1 | PARA 'C'
//                    fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
//                    fprintf(fp_result,"\tli\t%s,%s\n",reg_name_buf1,midcode_list[i]->op1);//li $s1,'c' | li $s1,1
//                    fprintf(fp_result,"\tsw\t$%s,0($sp)\n",reg_name_buf1);//sw $s1 ,0($sp)
//                }else
//                {
//                    //PARA a
//                    fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
//                    lw(reg_name_buf1,midcode_list[i]->op1);//把这个变量放到$s1
//                    fprintf(fp_result,"\tsw\t%s,0($sp)\n",reg_name_buf1);//sw $t1 ,0($sp)
//                }
                break;
            case VOID_FUNC_CALL:
                save_registers();
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                replace_registers();
                para_num = find_table(midcode_list[i]->op1)->paranum;
                fprintf(fp_result,"\tli\t$t1,%d\n",para_num*4);
                fprintf(fp_result,"\tadd\t$sp,$sp,$t1\n");
                break;
            case NONVOID_FUNC_CALL://返回值是在v1里面
                save_registers();
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                replace_registers();
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
                init_reg_info();
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
//                reg_num1 = get_reg(midcode_list[i]->op1);
//                sprintf(reg_name_buf1,"$s%d",reg_num1);
//
//                reg_num2 = get_reg(midcode_list[i]->op2);
//                sprintf(reg_name_buf2,"$s%d",reg_num2);
                //> a , b , t1
                reg_num1 = in_reg(midcode_list[i]->op1);
                reg_num2 = in_reg(midcode_list[i]->op2);
                if(reg_num1 == -1)
                {
                    if(reg_num2 == -1)
                    {
                        //第一 和 第二个操作数不在寄存器里
                        reg_num1 = get_reg();
                        reg_num2 = get_reg();
                        sprintf(reg_name_buf1,"$s%d",reg_num1);
                        sprintf(reg_name_buf2,"$s%d",reg_num2);

                        if(is_const_value(midcode_list[i]->op1))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op1);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op1);
                        }

                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num2,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf2,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                        Reg_info[reg_num2].vari_info.push_back(midcode_list[i]->op2);


                    }else
                    {
                        //第一个不在寄存器里，第二个在寄存器里
                        reg_num1 = get_reg();
                        sprintf(reg_name_buf1,"$s%d",reg_num1);
                        if(is_const_value(midcode_list[i]->op1))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op1);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op1);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                    }
                }else
                {
                    if(reg_num2 == -1)
                    {
                        //第一个在寄存器里，第二个不在
                        reg_num2 = get_reg();
                        sprintf(reg_name_buf2,"$s%d",reg_num2);
                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num2,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf2,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num2].vari_info.push_back(midcode_list[i]->op2);

                    }else
                    {
                        //都在寄存器里 无操作
                    }
                }

                switch(midcode_list[i]->type)
                {
                case F_BT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbgt\t$s%d,$s%d,%s\n",reg_num1,reg_num2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tble\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                case F_BAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbge\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tblt\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                case F_LT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tblt\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbge\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                case F_LAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tble\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbgt\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                case F_EQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbeq\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbne\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                case F_NEQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbne\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbeq\t$s%d,$s%d,%s\n",reg_num1,reg_num2,mmidcode_list[i+1]->op1);
                    break;
                }

                break;
            case F_PLUS:
            case F_MINUS:
            case F_MULTIPY:
            case F_DIVD:
                //$t1 = a + b
                reg_num1 = in_reg(midcode_list[i]->op1);
                reg_num2 = in_reg(midcode_list[i]->op2);
                if(reg_num1 == -1)
                {
                    if(reg_num2 == -1)
                    {
                        //第一 和 第二个操作数不在寄存器里
                        reg_num1 = get_reg();
                        reg_num2 = get_reg();
                        sprintf(reg_name_buf1,"$s%d",reg_num1);
                        sprintf(reg_name_buf2,"$s%d",reg_num2);

                        if(is_const_value(midcode_list[i]->op1))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op1);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op1);
                        }

                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num2,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf2,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                        Reg_info[reg_num2].vari_info.push_back(midcode_list[i]->op2);


                    }else
                    {
                        //第一个不在寄存器里，第二个在寄存器里
                        reg_num1 = get_reg();
                        sprintf(reg_name_buf1,"$s%d",reg_num1);
                        if(is_const_value(midcode_list[i]->op1))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op1);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op1);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                    }
                }else
                {
                    if(reg_num2 == -1)
                    {
                        //第一个在寄存器里，第二个不在
                        reg_num2 = get_reg();
                        sprintf(reg_name_buf2,"$s%d",reg_num2);
                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num2,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf2,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num2].vari_info.push_back(midcode_list[i]->op2);

                    }else
                    {
                        //都在寄存器里 无操作
                    }
                }

                reg_num3 = get_reg();
                Reg_info[reg_num3].vari_info.push_back(midcode_list[i]->op3);

                switch(midcode_list[i]->type)
                {
                case F_PLUS:
                    fprintf(fp_result,"\tadd\t$s%d,$s%d,$s%d\n",reg_num3,reg_num1,reg_num2);
                    break;
                case F_MINUS:
                    fprintf(fp_result,"\tsub\t$s%d,$s%d,$s%d\n",reg_num3,reg_num1,reg_num2);
                    break;
                case F_MULTIPY:
                    fprintf(fp_result,"\tmul\t$s%d,$s%d,$s%d\n",reg_num3,reg_num1,reg_num2);
                    break;
                case F_DIVD:
                    fprintf(fp_result,"\tdiv\t$s%d,$s%d\n",reg_num1,reg_num2);
                    fprintf(fp_result,"\tmflo\t$s%d\n",reg_num3);
                    break;
                }
                break;
            case F_ASSIGN:
                reg_num1 = in_reg(midcode_list[i]->op1);
                if(reg_num1 == -1)
                {
                    //这个值不在寄存器中
                    reg_num2 = in_reg(midcode_list[i]->op2);
                    if(reg_num2 == -1)
                    {
                        reg_num1 = get_reg();
                        sprintf(reg_name_buf1,"$s%d",reg_num1);

                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op2);
                    }else
                    {
                        Reg_info[reg_num2].vari_info.push_back(midcode_list[i]->op1);
                    }
                }else
                {
                    //这个值已经在寄存器中了，得把这个寄存器里面的内容给释放了，因为有可能这个寄存器里还有别的变量，如果直接load到这个寄存器中，别的变量的值就被覆盖了
                    sprintf(reg_name_buf1,"$s%d",reg_num1);
                    for(int i=0;i<Reg_info[reg_num1].vari_info.size();i++)
                    {
                        if(! is_const_value(Reg_info[reg_num1].vari_info[i]))
                        {
                            //是一个变量
                            if(strlen(Reg_info[reg_num1].vari_info[i]) <= 2 || !(Reg_info[reg_num1].vari_info[i][0] == '$' && Reg_info[reg_num1].vari_info[i][1] == '$'))
                            {
                                sw(reg_name_buf1,Reg_info[reg_num1].vari_info[i])//例如sw($s1,a)
                            }
                        }
                    }
                    Reg_info[reg_num1].vari_info.clear();

                    reg_num2 = in_reg(midcode_list[i]->op2);
                    if(reg_num2 == -1)
                    {
                        if(is_const_value(midcode_list[i]->op2))
                        {
                            fprintf(fp_result,"\tli\t$s%d,%s\n",reg_num1,midcode_list[i]->op2);
                        }else
                        {
                            lw(reg_name_buf1,midcode_list[i]->op2);
                        }
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op2);
                    }else
                    {
                        fprintf(fp_result,"\tmove\t$s%d,$s%d\n",reg_num1,reg_num2);
                        Reg_info[reg_num1].vari_info.push_back(midcode_list[i]->op1);
                    }

                }


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
    fp_source = fopen("./测试程序V2/testopti.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误
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
