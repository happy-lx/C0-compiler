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
#include<algorithm>

static int num_str = 1;//用来表明当前是处理到了哪一个字符串，(字符串仅在printf中出现)
std::vector<std::pair<char*,int> > refn;//引用计数记录
char buf1[10];
char buf2[10];
char buf3[10];
int temp1;
int temp2;
int temp3;


void lw(char* reg_name,char*  vari_name);//将一个变量放入某一个寄存器中
void sw(char* reg_name,char*  vari_name);//将某一个寄存器中的内容存给某一个变量
int find_addr(char* vari_name);//找到某一0个变量关于fp的偏移地址，如果这个变量是在top符号表中，则返回-1
int gettotalsize(char* func_name);//对一个函数，计算具体的栈的大小
void gen_data();//生成data段的描述，具体内容是全局变量和printf的字符串
void gen_text();//生成代码段的描述

//新增的用来解决寄存器分配的方法
//总思路：进入函数时先清空refn，然后计算这个函数里面每个变量被引用的次数，然后对refn排序，前8个分别分配给s0-s7
//然后把参数变量和全局变量load进来。
//然后每次针对一个四元式翻译的时候，当用到变量的时候，先看是不是在这八个寄存器里，在就直接用，不在就load到临时寄存器t0-t7中
void init();//将refn清空
void cntpp(char*);//对某一个变量引用计数加一，如果是常量就不处理
int find_refn(char*);//对某一个变量找在哪个寄存器里，这个函数是对cntpp的辅助函数
int find_in_refn(char*);//这个才是找具体哪个变量在在哪个寄存器里面，如果下标超过7也返回-1
bool cmp(std::pair<char*,int> num1 , std::pair<char*,int> num2);//排序的时候的比较函数
void save_reg();//保存s0到s7
void return_reg();//还原s0到s7

void init()
{
    refn.clear();
}
void cntpp(char* input_str)
{
    if(is_const_value(input_str))
    {
        return;
    }
    int loc = find_refn(input_str);
    if(loc == -1)
    {
        //这个变量是第一次出现
        refn.push_back(std::make_pair(input_str,1));
        return;
    }
    //这个变量已经出现过了
    refn[loc].second++;
}
int find_refn(char* input_str)
{
    for(int i=0;i<refn.size();i++)
    {
        if(strcmp(input_str,refn[i].first) == 0)
        {
            return i;
        }
    }
    return -1;
}
int find_in_refn(char* input_str)
{
    for(int i=0;i<refn.size() && i<maxregnum ;i++)
    {
        //取size和8之间较小的那个，在这个范围内循环
        if(strcmp(input_str,refn[i].first) == 0)
        {
            return i;
        }
    }
    return -1;

}

bool cmp(std::pair<char*,int> num1 , std::pair<char*,int> num2)
{
    return num1.second > num2.second;

}

void save_reg()
{

    for(int i=0;i<maxregnum ;i++)
    {
        fprintf(fp_result,"\tsw\t$s%d,%d($sp)\n",i,-(i+1)*4);
    }
    fprintf(fp_result,"\tsubi\t$sp,$sp,%d\n",4*maxregnum);

}
void return_reg()
{
    for(int i=0;i<maxregnum;i++)
    {
        fprintf(fp_result,"\tlw\t$s%d,%d($sp)\n",maxregnum-i-1,(i)*4);
    }
    fprintf(fp_result,"\taddi\t$sp,$sp,%d\n",4*maxregnum);

}

bool is_const_value(char* input)
{
    if(input[0] >= '0' && input[0] <= '9' || input[0] == '\'' || (input[0] == '-' && input[1] >= '0' && input[1] <='9'))
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
            return -(result->addr - para_num)*4+(maxregnum)*4;
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
    int j;
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
                    temp1 = find_in_refn(midcode_list[i]->op1);
                    if(temp1 == -1)
                    {
                        fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                        lw("$t1",midcode_list[i]->op1);//把这个变量放到$t1
                        fprintf(fp_result,"\tsw\t$t1,0($sp)\n");//sw $t1 ,0($sp)
                    }
                    else
                    {
                        sprintf(buf1,"$s%d",temp1);
                        fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                        fprintf(fp_result,"\tsw\t%s,0($sp)\n",buf1);//sw $t1 ,0($sp)
                    }


                }
                break;
            case VOID_FUNC_CALL:
                save_reg();
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                return_reg();
                para_num = find_table(midcode_list[i]->op1)->paranum;
                fprintf(fp_result,"\tli\t$t1,%d\n",para_num*4);
                fprintf(fp_result,"\tadd\t$sp,$sp,$t1\n");
                break;
            case NONVOID_FUNC_CALL://返回值是在v1里面
                save_reg();
                fprintf(fp_result,"\tsubi\t$sp,$sp,4\n");//subi $sp,$sp,4
                fprintf(fp_result,"\tjal\t%s\n",midcode_list[i]->op1);
                return_reg();
                para_num = find_table(midcode_list[i]->op1)->paranum;
                fprintf(fp_result,"\tli\t$t1,%d\n",para_num*4);
                fprintf(fp_result,"\tadd\t$sp,$sp,$t1\n");

                temp1 = find_in_refn(midcode_list[i]->op3);
                if(temp1 == -1)
                {
                    sw("$v1",midcode_list[i]->op3);
                }
                else
                {
                    fprintf(fp_result,"\tmove\t$s%d,$v1\n",temp1);
                }
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
                //计算本个函数中的引用计数
                init();
                j = i+1;
                while(midcode_list[j]->type != FUNCC_END)
                {
                    switch(midcode_list[j]->type)
                    {
                    case PARA:
                        cntpp(midcode_list[j]->op1);
                        break;
                    case RET:
                        if(strcmp(midcode_list[j]->op1,"") != 0)
                            cntpp(midcode_list[j]->op1);
                        break;
                    case NONVOID_FUNC_CALL:
                        cntpp(midcode_list[j]->op3);
                        break;
                    case F_PLUS:
                    case F_MINUS:
                    case F_MULTIPY:
                    case F_DIVD:
                        cntpp(midcode_list[j]->op1);
                        cntpp(midcode_list[j]->op2);
                        cntpp(midcode_list[j]->op3);
                        break;
                    case ARRA_ASSIGN_L:
                    case ARRA_ASSIGN_R:
                        cntpp(midcode_list[j]->op2);
                        cntpp(midcode_list[j]->op3);
                        break;
                    case F_BT:
                    case F_BAET:
                    case F_LT:
                    case F_LAET:
                    case F_EQ:
                    case F_NEQ:
                    case F_ASSIGN:
                    case NEG:
                        cntpp(midcode_list[j]->op1);
                        cntpp(midcode_list[j]->op2);
                        break;
                    case F_PRINTF:
                        if(strcmp(midcode_list[j]->op2,"") != 0)
                            cntpp(midcode_list[j]->op2);
                        break;
                    case F_SCANF:
                        cntpp(midcode_list[j]->op1);
                        break;

                    }

                    j++;
                }

                //计数完了再排序
                std::sort(refn.begin(),refn.end(),cmp);

                //再生成一个函数开始标号
                fprintf(fp_result,"%s:\n",midcode_list[i]->op1);
                //处理函数调用进入时的事务
                fprintf(fp_result,"\tsw\t$ra,0($sp)\n");
                fprintf(fp_result,"\tsw\t$fp,-4($sp)\n");
                fprintf(fp_result,"\tadd\t$fp,$sp,$0\n");
                size_of_stack = gettotalsize(cur->name);
                fprintf(fp_result,"\tsubi\t$sp,$sp,%d\n",size_of_stack);

                //5月16日修改 需要把所有的局部常量放到栈中
                if(!opticlock)
                {
                    for(int i=0;i<MaxBukSize;i++)
                    {
                        p = cur->bucket[i];
                        while(p)
                        {
                            if(p->type == TConst)
                            {
                                if(p->kind == Char)
                                {
                                    fprintf(fp_result,"\tli\t$t5,%d\n",p->val.ch_val);
                                }else
                                {
                                    fprintf(fp_result,"\tli\t$t5,%d\n",p->val.int_val);
                                }
                                fprintf(fp_result,"\tsw\t$t5,%d($fp)\n",find_addr(p->name));
                            }
                            p = p->next;
                        }
                    }
                }

                //在栈形成之后查看这些寄存器，把参数和全局变量放入到寄存器中
                for(int i=0;i<refn.size() && i<maxregnum ;i++)
                {
                    if(find_addr(refn[i].first) == -1)
                    {
                        //全局变量
                        sprintf(buf1,"$s%d",i);
                        lw(buf1,refn[i].first);
                        continue;
                    }
                    entry* temp = find_table(refn[i].first);
                    if(temp->type == TArg)
                    {
                        fprintf(fp_result,"\tlw\t$s%d,%d($fp)\n",i,find_addr(refn[i].first));
                    }
                    if(!opticlock)
                    {
                        if(temp->type == TConst)
                        {
                            fprintf(fp_result,"\tlw\t$s%d,%d($fp)\n",i,find_addr(refn[i].first));
                        }
                    }
                }
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
                                temp1 = find_in_refn(midcode_list[i]->op1);
                                if(temp1 == -1)
                                {
                                    lw("$v1",midcode_list[i]->op1);
                                }
                                else
                                {
                                    fprintf(fp_result,"\tmove\t$v1,$s%d\n",temp1);
                                }
                            }
                        }
                        fprintf(fp_result,"\tadd\t$sp,$fp,$0\n");
                        fprintf(fp_result,"\tlw\t$ra,0($sp)\n");
                        fprintf(fp_result,"\taddi\t$sp,$sp,4\n");//addi $sp,$sp,4
                        fprintf(fp_result,"\tlw\t$fp,-8($sp)\n");//fp = $sp - 8处的内容
                        fprintf(fp_result,"\tjr\t$ra\n");
                }
                break;
            case FUNCC_END://5月22日添加，可以让void类型的函数无return
                if(strcmp(cur->name,"main") != 0)
                {
                    entry* temp = find_table(cur->name);
                    if(temp->type == TFunc && temp->kind == Void)
                    {
                        fprintf(fp_result,"\tadd\t$sp,$fp,$0\n");
                        fprintf(fp_result,"\tlw\t$ra,0($sp)\n");
                        fprintf(fp_result,"\taddi\t$sp,$sp,4\n");//addi $sp,$sp,4
                        fprintf(fp_result,"\tlw\t$fp,-8($sp)\n");//fp = $sp - 8处的内容
                        fprintf(fp_result,"\tjr\t$ra\n");
                    }
                }
                break;
            case LABLE:
                fprintf(fp_result,"%s:\n",midcode_list[i]->op1);
                break;
            case JMP:
                fprintf(fp_result,"\tj\t%s\n",midcode_list[i]->op1);
                break;
            case JNP:
                //对于if(-1) 和 while(-1) 的时候会出现四元式 5/18日修改
                //jnp     Lable2,   ,-1
                //jnp     Lable2,   ,t
                if(is_const_value(midcode_list[i]->op3))
                {
                    temp = atoi(midcode_list[i]->op3);
                    if(temp == 0)
                    {
                        //条件不满足需要跳转
                        fprintf(fp_result,"\tj\t%s\n",midcode_list[i]->op1);

                    }
                }else//可能是 jnp lable2 , , a || jnp lable2 , , $t1
                {
                    int temp1 = find_in_refn(midcode_list[i]->op3);
                    if(temp1 == -1)
                    {
                        lw("$t4",midcode_list[i]->op3);
                        sprintf(buf1,"$t4");
                    }
                    else
                    {
                        sprintf(buf1,"$s%d",temp1);
                    }
                    fprintf(fp_result,"\tbeq\t%s,$0,%s\n",buf1,midcode_list[i]->op1);
                }
                break;
            case JEP:
                //对于if(-1) 和 while(-1) 的时候会出现四元式 5/18日修改
                //jnp     Lable2,   ,-1
                //jep     Lable2,   ,-1
                if(is_const_value(midcode_list[i]->op3))
                {
                    temp = atoi(midcode_list[i]->op3);
                    if(temp != 0)
                    {
                        //条件满足需要跳转
                        fprintf(fp_result,"\tj\t%s\n",midcode_list[i]->op1);
                    }
                }else//可能是 jep lable2 , , a || jep lable2 , , $t1
                {
                    int temp1 = find_in_refn(midcode_list[i]->op3);
                    if(temp1 == -1)
                    {
                        lw("$t4",midcode_list[i]->op3);
                        sprintf(buf1,"$t4");
                    }
                    else
                    {
                        sprintf(buf1,"$s%d",temp1);
                    }
                    fprintf(fp_result,"\tbne\t%s,$0,%s\n",buf1,midcode_list[i]->op1);
                }

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
                    sprintf(buf1,"$t1");
                }
                else
                {
                    int temp1 = find_in_refn(midcode_list[i]->op1);
                    if(temp1 == -1)
                    {
                        lw("$t1",midcode_list[i]->op1);
                        sprintf(buf1,"$t1");
                    }
                    else
                    {
                        sprintf(buf1,"$s%d",temp1);
                    }

                }

                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op2);
                    sprintf(buf2,"$t2");
                }
                else
                {
                    int temp2 = find_in_refn(midcode_list[i]->op2);
                    if(temp2 == -1)
                    {
                        lw("$t2",midcode_list[i]->op2);
                        sprintf(buf2,"$t2");
                    }
                    else
                    {
                        sprintf(buf2,"$s%d",temp2);
                    }
                }

                switch(midcode_list[i]->type)
                {
                case F_BT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbgt\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tble\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                case F_BAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbge\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tblt\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                case F_LT:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tblt\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbge\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                case F_LAET:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tble\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbgt\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                case F_EQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbeq\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbne\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                case F_NEQ:
                    if(midcode_list[i+1]->type == JEP)
                        fprintf(fp_result,"\tbne\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    else
                        fprintf(fp_result,"\tbeq\t%s,%s,%s\n",buf1,buf2,midcode_list[i+1]->op1);
                    break;
                }

                i++;//跳过下一条语句
                break;
            case F_PLUS:
            case F_MINUS:
            case F_MULTIPY:
            case F_DIVD:
                //t1 = a + b
                if(is_const_value(midcode_list[i]->op1))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op1);
                    sprintf(buf1,"$t1");
                }else
                {
                    int temp1 = find_in_refn(midcode_list[i]->op1);
                    if(temp1 == -1)
                    {
                        lw("$t1",midcode_list[i]->op1);
                        sprintf(buf1,"$t1");
                    }
                    else
                    {
                        sprintf(buf1,"$s%d",temp1);
                    }
                }
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op2);
                    sprintf(buf2,"$t2");
                }
                else
                {
                    int temp2 = find_in_refn(midcode_list[i]->op2);
                    if(temp2 == -1)
                    {
                        lw("$t2",midcode_list[i]->op2);
                        sprintf(buf2,"$t2");
                    }
                    else
                    {
                        sprintf(buf2,"$s%d",temp2);
                    }
                }

                temp3 = find_in_refn(midcode_list[i]->op3);
                if(temp3 == -1)
                {
                    lw("$t3",midcode_list[i]->op3);
                    sprintf(buf3,"$t3");
                }
                else
                {
                    sprintf(buf3,"$s%d",temp3);
                }

                switch(midcode_list[i]->type)
                {
                case F_PLUS:
                    fprintf(fp_result,"\tadd\t%s,%s,%s\n",buf3,buf1,buf2);
                    break;
                case F_MINUS:
                    fprintf(fp_result,"\tsub\t%s,%s,%s\n",buf3,buf1,buf2);
                    break;
                case F_MULTIPY:
                    fprintf(fp_result,"\tmul\t%s,%s,%s\n",buf3,buf1,buf2);
                    break;
                case F_DIVD:
                    fprintf(fp_result,"\tdiv\t%s,%s\n",buf1,buf2);
                    fprintf(fp_result,"\tmflo\t%s\n",buf3);
                    break;
                }
                if(buf3[1] == 't')
                {
                    sw(buf3,midcode_list[i]->op3);
                }

                break;
            case F_ASSIGN:
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t2,%s\n",midcode_list[i]->op2);
                    sprintf(buf2,"$t2");
                }else
                {
                    temp2 = find_in_refn(midcode_list[i]->op2);
                    if(temp2 == -1)
                    {
                        lw("$t2",midcode_list[i]->op2);
                        sprintf(buf2,"$t2");
                    }
                    else
                    {
                        sprintf(buf2,"$s%d",temp2);
                    }

                }
                temp1 = find_in_refn(midcode_list[i]->op1);
                if(temp1 == -1)
                {
                    sw(buf2,midcode_list[i]->op1);
                }else
                {
                    fprintf(fp_result,"\tmove\t$s%d,%s\n",temp1,buf2);
                }


                break;
            case ARRA_ASSIGN_R:
                // t = a[1]
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    temp1 = find_in_refn(midcode_list[i]->op2);
                    if(temp1 == -1)
                    {
                        lw("$t1",midcode_list[i]->op2);
                    }
                    else
                    {
                        fprintf(fp_result,"\tmove\t$t1,$s%d\n",temp1);
                    }
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

                temp3 = find_in_refn(midcode_list[i]->op3);
                if(temp3 == -1)
                {
                    sw("$t2",midcode_list[i]->op3);
                }else
                {
                    fprintf(fp_result,"\tmove\t$s%d,$t2\n",temp3);
                }

                break;
            case ARRA_ASSIGN_L:
                //a[1] = t
                if(is_const_value(midcode_list[i]->op2))
                {
                    fprintf(fp_result,"\tli\t$t1,%s\n",midcode_list[i]->op2);
                }else
                {
                    temp1 = find_in_refn(midcode_list[i]->op2);
                    if(temp1 == -1)
                    {
                        lw("$t1",midcode_list[i]->op2);
                    }
                    else
                    {
                        fprintf(fp_result,"\tmove\t$t1,$s%d\n",temp1);
                    }
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
                    sprintf(buf2,"$t2");
                }else
                {
                    temp2 = find_in_refn(midcode_list[i]->op3);
                    if(temp2 == -1)
                    {
                        lw("$t2",midcode_list[i]->op3);
                        sprintf(buf2,"$t2");
                    }else
                    {
                        sprintf(buf2,"$s%d",temp2);
                    }

                }

                fprintf(fp_result,"\tsw\t%s,0($t3)\n",buf2);
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
                        temp2 = find_in_refn(midcode_list[i]->op2);
                        if(temp2 == -1)
                        {
                            lw("$a0",midcode_list[i]->op2);
                        }else
                        {
                            fprintf(fp_result,"\tmove\t$a0,$s%d\n",temp2);
                        }
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
                    temp1 = find_in_refn(midcode_list[i]->op1);
                    if(temp1 == -1)
                    {
                        sw("$v0",midcode_list[i]->op1);
                    }else
                    {
                        fprintf(fp_result,"\tmove\t$s%d,$v0\n",temp1);
                    }
                }else
                {
                    fprintf(fp_result,"\tli\t$v0,5\n");
                    fprintf(fp_result,"\tsyscall\n");
                    temp1 = find_in_refn(midcode_list[i]->op1);
                    if(temp1 == -1)
                    {
                        sw("$v0",midcode_list[i]->op1);
                    }else
                    {
                        fprintf(fp_result,"\tmove\t$s%d,$v0\n",temp1);
                    }
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
                temp1 = find_in_refn(midcode_list[i]->op1);
                if(temp1 == -1)
                {
                    sw("$t1",midcode_list[i]->op1);
                }else
                {
                    fprintf(fp_result,"\tmove\t$s%d,$t1\n",temp1);
                }
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

//int main()
//{
//    fp_source = fopen("./测试程序/1.c","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误
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
