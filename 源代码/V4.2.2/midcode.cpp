#include "global.h"
#include "lex.h"
#include "grammer.h"
#include "symbol.h"
#include "semantic.h"
#include "midcode.h"
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stack>

midcode* midcodeconstructor(fourth_type input);
char* getLable();
char* getTemp();
void emit_midcode(fourth_type in_type,char* in_op1,char* in_op2,char* in_op3);

static int lable_count = 0;//用以标号的生成
static int temp_count = 0;//用以临时变量的生成
std::stack<char*> result_stack;//结果放入这个栈中，便于上面的节点得到下面的节点的结果
std::vector<midcode*> midcode_list;//中间代码表

char* fourth_str[] ={"PARA","Func_begin","Func_end","return","Lable",
                    "jmp","jnp","jep","call","call","+","-","*","/","[]=R","[]=L",">",">=",
                    "<","<=","==","!=","=","printf","scanf","Neg"};

midcode* midcodeconstructor(fourth_type input)
{
    midcode* result = (midcode*)malloc(sizeof(midcode));
    result->type = input;
    return result;
}

char* getLable()
{
    //获得一个新的编号
    char* temp = (char*)malloc(sizeof(char)*MAXTEMPLENGTH);

    sprintf(temp,"Lable%d",lable_count++);

    return temp;
}

char* getTemp()
{
    char* temp = (char*)malloc(sizeof(char)*MAXTEMPLENGTH);

    sprintf(temp,"$t%d",temp_count++);

    //2020/5/02修改 将新建的临时变量加入到符号表中,不知道是不是正确
    insert_vari(temp,Integer);

    return temp;
}

void emit_midcode(fourth_type in_type,char* in_op1,char* in_op2,char* in_op3)
{
    midcode* result = midcodeconstructor(in_type);

    strcpy(result->op1,in_op1);
    strcpy(result->op2,in_op2);
    strcpy(result->op3,in_op3);

    midcode_list.push_back(result);
}

void gen_code(ASTNODE* tree)
{
    //需要处理的情况如下
    //1.首先当进入一个函数的时候需要把符号表指针更新到对应的位置,进入这个函数和退出这个函数要生成标识
    //2.对于+|-|*|/|<|<=|>|>=|==|!= 情况，首先看是不是生成的无用节点，如果是则去往右生成，然后再看这个节点的token是不是-
    //如果是-号的话则从结果栈弹出栈顶，再加一个四元式(NEG,,,)然后把结果push到栈中，如果这个节点不是无用节点
    //则遍历左右节点，最后生成对应的四元式，结果入栈
    //3.id
    //4.array
    //5.constnum
    //6.constchar
    //7.有返回值的函数调用
    //8.无返回值的函数调用
    //9.if
    //10.while
    //11.for
    //12.printf
    //13.scanf
    //14.return
    //15.参数调用
    //16.赋值语句

    symboltable* temp;//用来设置符号表的位置
    entry* p;//找表项
    char* result1 ;//用来获取子节点的计算结果
    char* result2 ;//用来获取子节点的计算结果
    char* temp_vari;
    char* temp_lable1;
    char* temp_lable2;

    if(tree != NULL && tree->type != CONSTDECLARE && tree->type != VARIDECLARE)
    {
        if(tree->type == FUNCDECLARE)
        {
            //函数类型
            //首先设置符号表的位置
            temp = top->funchild;
            while(temp)
            {
                if(strcmp(tree->childen[0]->attr.name,temp->name) == 0)
                {
                    cur = temp;
                    break;
                }
                temp = temp->sibling;
            }

            //然后生成函数的开始
            emit_midcode(FUNC_BEGIN,tree->childen[0]->attr.name,"","");
            gen_code(tree->childen[2]);
            emit_midcode(FUNCC_END,tree->childen[0]->attr.name,"","");

            gen_code(tree->sibling);
            return;
        }else if(tree->type == MAINTYPE)
        {
            cur = top->mainchild;

            emit_midcode(FUNC_BEGIN,"main","","");
            gen_code(tree->childen[0]);
            emit_midcode(FUNCC_END,"main","","");

            return ;
        }else if(tree->type == OPTYPE)
        {
            //先看看是不是生成的无用节点
            if(tree->useful == FALSE)
            {
                gen_code(tree->childen[1]);

                if(tree->ttype == MINUS)
                {
                    result1 = result_stack.top();//得到孩子节点的计算结果的临时变量
                    result_stack.pop();

                    temp_vari = getTemp();//存放本次结果的变量
                    emit_midcode(NEG,temp_vari,result1,"");//生成四元式
                    result_stack.push(temp_vari);//结果入栈
                }
                return;
            }

            //现在可以确定不是无用节点 a<b | a+b
            gen_code(tree->childen[0]);
            gen_code(tree->childen[1]);

            result2 = result_stack.top();
            result_stack.pop();
            result1 = result_stack.top();
            result_stack.pop();
            temp_vari = getTemp();

            switch(tree->ttype)
            {
                case PLUS:
                    emit_midcode(F_PLUS,result1,result2,temp_vari);
                    break;
                case MINUS:
                    emit_midcode(F_MINUS,result1,result2,temp_vari);
                    break;
                case MULTIP:
                    emit_midcode(F_MULTIPY,result1,result2,temp_vari);
                    break;
                case DIVD:
                    emit_midcode(F_DIVD,result1,result2,temp_vari);
                    break;
                case LT:
                    emit_midcode(F_LT,result1,result2,temp_vari);
                    break;
                case LTAE:
                    emit_midcode(F_LAET,result1,result2,temp_vari);
                    break;
                case BT:
                    emit_midcode(F_BT,result1,result2,temp_vari);
                    break;
                case BTAE:
                    emit_midcode(F_BAET,result1,result2,temp_vari);
                    break;
                case EQ:
                    emit_midcode(F_EQ,result1,result2,temp_vari);
                    break;
                case NEQ:
                    emit_midcode(F_NEQ,result1,result2,temp_vari);
                    break;
                default :
                    break;
            }

            result_stack.push(temp_vari);
            return;
        }else if(tree->type == IDTYPE)
        {
            //肯定是叶子节点
            result_stack.push(tree->attr.name);
            return;
        }else if(tree->type == CONSTNUM)
        {
            //肯定是叶子节点，是一个数字,复用一下result1
            result1 = (char*)malloc(sizeof(char)*MaxTokenLength);

            itoa(tree->attr.val,result1,10);
            result_stack.push(result1);
            return;
        }else if(tree->type == CONSTCHAR)
        {
            result1 = (char*)malloc(sizeof(char)*4);
            result1[0] = '\'';
            result1[1] = tree->attr.ch;
            result1[2] = '\'';
            result1[3] = '\0';

            result_stack.push(result1);
            return;
        }else if(tree->type == CALL)
        {
            //先把参数的代码生成
            //然后查符号表，看一下当前函数是什么类型的返回值，如果有返回值用(call,,,t1),无返回值用(call,,,)
            gen_code(tree->childen[1]);
            p = find_table(tree->childen[0]->attr.name);

            if(p->kind == Void)
            {
                //无返回值的函数调用
                emit_midcode(VOID_FUNC_CALL,tree->childen[0]->attr.name,"","");
            }else
            {
                //有返回值的函数调用
                temp_vari = getTemp();
                emit_midcode(NONVOID_FUNC_CALL,tree->childen[0]->attr.name,"",temp_vari);
                result_stack.push(temp_vari);
            }

            return;
        }else if(tree->type == VALUE_ARG)
        {
            //参数
            //先生成表达式的代码
            gen_code(tree->childen[0]);

            result1 = result_stack.top();
            result_stack.pop();

            emit_midcode(PARA,result1,"","");

            gen_code(tree->sibling);
            return;
        }else if(tree->type == IF_STME)
        {
            //if(a==b){}else{}
            gen_code(tree->childen[0]);//条件 t1=(a==b)

            result1 = result_stack.top();
            result_stack.pop();

            temp_lable1 = getLable();//else部分
            emit_midcode(JNP,temp_lable1,"",result1);//不满足条件跳到lable1即else处
            gen_code(tree->childen[1]);//then部分

            temp_lable2 = getLable();//出口
            emit_midcode(JMP,temp_lable2,"","");
            emit_midcode(LABLE,temp_lable1,"","");

            gen_code(tree->childen[2]);//else部分
            emit_midcode(LABLE,temp_lable2,"","");

            return;

        }else if(tree->type == ARRAYTYPE)
        {
            //一定是作为factor出现的
            //需要把array的代码生成赋值给临时变量
            //先把[]的内容算出来
            gen_code(tree->childen[1]);

            result1 = result_stack.top();
            result_stack.pop();

            temp_vari = getTemp();

            emit_midcode(ARRA_ASSIGN_R,tree->childen[0]->attr.name,result1,temp_vari);

            result_stack.push(temp_vari);
            return ;

        }else if(tree->type == WHILE_STMT)
        {
            //do{}while()
            temp_lable1 = getLable();
            emit_midcode(LABLE,temp_lable1,"","");

            gen_code(tree->childen[0]);//do部分

            gen_code(tree->childen[1]);//条件

            result1 = result_stack.top();
            result_stack.pop();

            emit_midcode(JEP,temp_lable1,"",result1);//条件如果成立，则调到do部分

            return;

        }else if(tree->type == FOR_STMT)
        {
            //for(i=0;i<length;i=i+1)
            gen_code(tree->childen[1]);
            result1 = result_stack.top();
            result_stack.pop();

            emit_midcode(F_ASSIGN,tree->childen[0]->attr.name,result1,"");//i=0;

            temp_lable1 = getLable();//出口
            temp_lable2 = getLable();//循环体部分开始
            emit_midcode(LABLE,temp_lable2,"","");//条件之前的lable
            gen_code(tree->childen[2]);//条件
            result1 = result_stack.top();
            result_stack.pop();

            emit_midcode(JNP,temp_lable1,"",result1);//不满足条件跳转到出口
            gen_code(tree->childen[6]);//循环体部分

            if(tree->ttype == PLUS)
            {
                //步长为+
                temp_vari = getTemp();
                result1 = (char*)malloc(sizeof(char)*MaxTokenLength);
                itoa(tree->childen[5]->attr.val,result1,10);
                emit_midcode(F_PLUS,tree->childen[4]->attr.name,result1,temp_vari);
                emit_midcode(F_ASSIGN,tree->childen[3]->attr.name,temp_vari,"");
            }else
            {
                //步长为-
                temp_vari = getTemp();
                result1 = (char*)malloc(sizeof(char)*MaxTokenLength);
                itoa(tree->childen[5]->attr.val,result1,10);
                emit_midcode(F_MINUS,tree->childen[4]->attr.name,result1,temp_vari);
                emit_midcode(F_ASSIGN,tree->childen[3]->attr.name,temp_vari,"");
            }
            emit_midcode(JMP,temp_lable2,"","");//无条件跳转回去再检查条件
            emit_midcode(LABLE,temp_lable1,"","");//出口

            return;

        }else if(tree->type == ASSIGN_STMT)
        {
            //赋值语句要看被赋值的对象是id还是数组
            //如果是数组就要(ARRA_ASSIGN_L,,,),如果不是就(=,,,)
            gen_code(tree->childen[1]);
            result1 = result_stack.top();
            result_stack.pop();

            if(tree->childen[0]->type == ARRAYTYPE)
            {
                //数组被赋值
                gen_code(tree->childen[0]->childen[1]);
                result2 = result_stack.top();
                result_stack.pop();
                emit_midcode(ARRA_ASSIGN_L,tree->childen[0]->childen[0]->attr.name,result2,result1);

            }else
            {
                //id被赋值
                emit_midcode(F_ASSIGN,tree->childen[0]->attr.name,result1,"");
            }

            return ;

        }else if(tree->type == PRINTF_STMT)//printf("%d",a)
        {
            if(tree->childen[1])
            {
                //如果有a的话
                gen_code(tree->childen[1]);
                result1 = result_stack.top();
                result_stack.pop();
                if(tree->childen[1]->kind == Integer)
                    result2 = "int";
                else
                    result2 = "char";


            }else
            {
                result1 = "";
                result2 = "";
            }


            if(tree->childen[0])
            {
                //如果有string的话
                emit_midcode(F_PRINTF,tree->childen[0]->attr.str,result1,result2);

            }else
            {
                emit_midcode(F_PRINTF,"",result1,result2);
            }

            return;

        }else if(tree->type == SCANF_STMT)
        {
            ASTNODE* t = tree->childen[0];

            while(t)
            {
                emit_midcode(F_SCANF,t->attr.name,"","");
                t = t->sibling;
            }

            return ;

        }else if(tree->type == RETURN_STMT)
        {
            if(tree->childen[0])
            {
                //如果有返回值
                gen_code(tree->childen[0]);
                result1 = result_stack.top();
                result_stack.pop();
                emit_midcode(RET,result1,"","");
            }else
            {
                emit_midcode(RET,"","","");
            }
            return;

        }


        for(int i=0;i<MAXCHILDREN;i++)
        {
            gen_code(tree->childen[i]);
        }
        gen_code(tree->sibling);
    }

}

void output_midcode()
{
    for(int i=0;i<midcode_list.size();i++)
    {
        printf("%-10s\t%-5s\t%-5s\t%-5s\n",fourth_str[(midcode_list[i]->type)],midcode_list[i]->op1,midcode_list[i]->op2,midcode_list[i]->op3);
    }
}

//int main()
//{
//    fp_source = fopen("./测试程序/3.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误
//
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
//    return 0;
//}

