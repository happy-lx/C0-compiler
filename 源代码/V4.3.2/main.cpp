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

FILE * fp_result;
FILE * fp_source;
FILE * fp_log;

int opticlock = 1;

char dir_str[maxdirlen];
char opti_str[10];

int main()
{
    printf("Please input the absolute path of your source file:");
    gets(dir_str);
    printf("\nWould you like to optimize the final code?[default:Y](Y/n):");
    gets(opti_str);

    if(opti_str[0] == 'n')
        opticlock = 0;

    fp_source = fopen(dir_str,"r");//4 | 7 | 9 有问题，这里没有考虑类型转换 ，8 | 11 | test8 | test13 | testerror有代码错误 , test11有死循环
                                                    //暂时约定一下每一个方法都要有return，不然得对void类型没有return做特殊处理，即为func_end添加处理事务
    ASTNODE* result = program();

    if(error_num == 0)
    {
        build_table(result);

        typecheck(result);

        printf("\nSemantic Analyze Complete...\n\n");

        if(opticlock)
        {
            deadcode(result);
        }

        gen_code(result);

        printtable();

        printf("\nmidcode :\n");

        if(opticlock)
        {
            optimizer();
        }

        output_midcode();

        fp_result = fopen("./result.asm","w");

        gen_mips();

        printf("\nmips code already generated...\n");
    }else
    {
        printf("\nTotal error:%d \n",error_num);
    }


    return 0;
}
