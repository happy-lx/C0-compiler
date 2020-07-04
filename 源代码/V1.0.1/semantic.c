#include "symbol.h"
#include "global.h"
#include "lex.h"
#include "grammer.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

FILE * fp_source;
FILE * fp_log;

void typecheck(ASTNODE * tree)
{
    int i;
    symboltable* temp;//用来设置符号表的位置
    entry* p;//找表项
    ASTNODE* t;//用来遍历算参数的个数
    int acc = 0;//记录参数的个数

    if(tree != NULL && tree->type != CONSTDECLARE && tree->type != VARIDECLARE)
    {

        if(tree->type == OPTYPE)
        {
            //此时可以为+ - * / | < <= > >= == !=
            if(tree->useful == FALSE)
            {
                //表示是exp生成的无用节点
                typecheck(tree->childen[1]);

                tree->kind = tree->childen[1]->kind;
                return;
            }else
            {
                //此时表示都是有用的节点，不需要区分是运算符号还是比较符号，因为都是二元运算
                    typecheck(tree->childen[0]);
                    typecheck(tree->childen[1]);

                    if(tree->childen[0]->kind == tree->childen[1]->kind)
                    {
                        //表示是可以的
                        tree->kind = tree->childen[0]->kind;
                        return;
                    }else
                    {
                        //表示匹配发生错误
                        printf("Sementic Error at line %d : type conflict\n",tree->lineno);
                        exit(1);
                    }

            }

        }else if(tree->type == CALL)
        {//函数调用语句,需要检查参数的个数是否与调用时相匹配
            p = find_table(tree->childen[0]->attr.name);

            if(p)
            {
                //找到了这个表项
                tree->kind = p->kind;
                typecheck(tree->childen[1]);
                if(p->type != TFunc)
                {
                    printf("Semantic Error at line %d : invalid refering\n",tree->lineno);
                    exit(1);
                }

                if((p->paranum == 0 && tree->childen[1] == NULL) || (p->paranum == tree->childen[1]->attr.val))//参数个数匹配
                {
                    p->used = TRUE;//标记为定义过后使用了
                    return;
                }else
                {
                    printf("Semantic Error at line %d : function call matched wrong arg numbers\n",tree->lineno);
                    exit(1);
                }
            }else
            {
                printf("Semantic Error at line %d : function used but not declared\n",tree->lineno);
                exit(1);
            }

        }else if(tree->type == VALUE_ARG)
        {
            //暂时只看参数个数是否匹配
            t = tree;
            acc = 0;
            while(t)
            {
                acc++;
                t = t->sibling;
            }
            tree->attr.val = acc;
            return;

        }else if(tree->type == FUNCDECLARE)//为了不让function的id来干扰
        {
            //同时符号表也要设置成对应的符号表
            temp = top->funchild;
            while(temp)
            {
                if(strcmp(temp->name,tree->childen[0]->attr.name) == 0)
                {
                    cur = temp;
                    break;
                }
                temp = temp->sibling;
            }
            typecheck(tree->childen[2]);
            return;
        }else if(tree->type == MAINTYPE)
        {
            cur = top->mainchild;
            typecheck(tree->childen[0]);
            return;
        }else if(tree->type == IDTYPE)
        {
            p = find_table(tree->attr.name);

            if(p)
            {
                tree->kind = p->kind;

                if(p->type != TVariable && p->type != TConst && p->type != TArg)
                {
                    printf("Semantic Error at line %d : invalid refering \n",tree->lineno);
                    exit(1);
                }
                p->used = TRUE;
                return;
            }
            else
            {
                printf("Semantic Error at line %d : variable used but not declared\n",tree->lineno);
                exit(1);
            }
        }else if(tree->type == CONSTCHAR)
        {
            tree->kind = Char;
            return;
        }else if(tree->type == CONSTNUM)
        {
            tree->kind = Integer;
            return;
        }else if(tree->type == ASSIGN_STMT)
        {
            typecheck(tree->childen[0]);
            typecheck(tree->childen[1]);

            if(tree->childen[0]->kind == tree->childen[1]->kind)
            {
                //表示是可以的
                tree->kind = tree->childen[0]->kind;
                return;
            }else
            {
                //表示匹配发生错误
                printf("Sementic Error at line %d : type conflict\n",tree->lineno);
                exit(1);
            }
        }else if(tree->type == ARRAYTYPE)
        {
            p = find_table(tree->childen[0]->attr.name);

            if(p)
            {
                tree->kind = p->kind;
                typecheck(tree->childen[1]);

                if(p->type != TArray)
                {
                    printf("Semantic Error at line %d : invalid refering \n",tree->lineno);
                    exit(1);
                }

                if(tree->childen[1]->kind == Integer)
                {
                    if(tree->childen[1]->type == OPTYPE && tree->childen[1]->childen[1]->type == CONSTNUM)
                    {
                        if(tree->childen[1]->childen[1]->attr.val >= p->array_size)
                        {
                            printf("Semantic Error at line %d : array index out of bound\n",tree->lineno);
                            exit(1);
                        }
                    }

                }else
                {
                    printf("Semantic Error at line %d : index of array is not a integer\n",tree->lineno);
                    exit(1);
                }
                p->used = TRUE;
                return;
            }else
            {
                printf("Semantic Error at line %d : array used but not declared\n",tree->lineno);
                exit(1);
            }
        }


        for(i=0;i<MAXCHILDREN;i++)
        {
            typecheck(tree->childen[i]);
        }
        typecheck(tree->sibling);
    }
}

int main()
{
    fp_source = fopen("./测试程序/2.txt","r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误

    ASTNODE* result = program();

    build_table(result);

    typecheck(result);

    printtable();

    printf("\nSemantic Analyze Complete...\n");

    return 0;

}
