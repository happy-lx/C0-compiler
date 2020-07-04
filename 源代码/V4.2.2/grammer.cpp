#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "lex.h"
#include "global.h"
#include "grammer.h"


tokentype token;//当前读到的token的类型
tokentype pretoken;//需要向前看的时候，把之前的token类型保存在pretoken中
char pretokenvalue[MaxTokenLength+1];//之前的token的值
int prelinenumber;//之前的linenumber的值
tokentype prepretoken;
char prepretokenvalue[MaxTokenLength+1];
int preprelinenumber;
char output[MaxTokenLength];//存放前序遍历的结果

int linenumber = 0;


ASTNODE* nodeconstructor(NodeType);//生成一个抽象语法树节点

ASTNODE* const_dec();//构造program的常量声明部分
//ASTNODE* dec_head();
ASTNODE* vari_dec();//构造program的变量声明部分
//ASTNODE* vari_dec_stmt();//构造compond_stmt的变量声明部分
ASTNODE* no_void_fun_dec();
ASTNODE* void_fun_dec();
ASTNODE* compond_stmt();
ASTNODE* arg_lists();
ASTNODE* arg_list();
ASTNODE* main_dec();
ASTNODE* exp();
ASTNODE* item();
ASTNODE* factor();
ASTNODE* stmt();
ASTNODE* assgin_stmt();
ASTNODE* condition_stmt();
ASTNODE* condition();
ASTNODE* for_stmt();
ASTNODE* while_stmt();
ASTNODE* fun_call();
ASTNODE* arg_values();
ASTNODE* stmt_seq();
ASTNODE* scanf_stmt();
ASTNODE* printf_stmt();
ASTNODE* return_stmt();
void match(tokentype);
void copypre();
void copyprepre();
void eval(char* ,char* );
void getASTNODE_STR(int);//得到节点类型的字符串如"PROGRAM"
void tranverse(ASTNODE*);//前序遍历

void copypre()
{
    pretoken = token;
    strcpy(pretokenvalue,tokenvalue);
    prelinenumber = linenumber;
}
void copyprepre()
{
    prepretoken = pretoken;
    strcpy(prepretokenvalue,pretokenvalue);
    preprelinenumber = prelinenumber;
}

void match(tokentype input)
{
    if(token == input)
    {
        token = getToken();
    }else
    {
        //错误处理，待完善
        printf("ERRO at line %d",linenumber);
        exit(1);
    }
}

ASTNODE* nodeconstructor(NodeType input)
{
    ASTNODE* result = (ASTNODE*)malloc(sizeof(ASTNODE));
    int i;

    result->type = input;
    for(i=0;i<MAXCHILDREN;i++)
    {
        result->childen[i] = NULL;
    }
    result->sibling = NULL;

    if(input == IDTYPE)
    {
        result->attr.name = (char*)malloc(sizeof(char)*(MaxTokenLength+1));
    }

    if(input == CONSTSTRING)
    {
        result->attr.str = (char*)malloc(sizeof(char)*(MaxTokenLength+1));
    }
    result->useful = TRUE;//对于exp生成的节点设置为false

    return result;
}

ASTNODE* program()
{
    token = getToken();
    ASTNODE* result = nodeconstructor(PROGRAM);

    if(token == CONST)
    {
        //常量定义部分
        match(token);
        result->childen[0] = const_dec();
    }

    while(token == INT || token == CHAR)
    {
        //变量声明部分
        //int和char有可能是带返回值的函数声明所以得区分一下
        //int a;
        //int a(){};
        copypre();
        match(token);
        if(token == ID)
        {
            copyprepre();
            copypre();
            match(token);
            if(token == LB)
            {
                //是函数声明
                break;//类型在prepretoken中 id在pretokenvalue中，token现在是LB
            }else if(token == DOT || token == SEMI || token == LSB)
            {
                //是变量声明
                result->childen[1] = vari_dec();//类型在prepretoken中 id在pretokenvalue中，token现在是DOT|SEMI|LSB
                break;//接着去处理函数声明
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }

    while(token == INT || token == CHAR || token == VOID || (token == LB && pretoken == ID && prepretoken != VOID))
    {
        //函数声明分四种情况进入，1.在变量声明中分析了一下发现应该是函数声明
        //2.完成变量声明的分析（需要修改）
        //3.没有变量声明和常数声明，直接来函数声明
        //4.完成了const声明，没有variable声明
        //为了让函数声明的处理一致，调用function_declare时都让prevalue和preprevalue置好值
        if(token == INT || token == CHAR)
        {
            //肯定是函数声明
            copypre();
            match(token);
            copyprepre();
            copypre();
            match(ID);
            result->childen[2] = no_void_fun_dec();//preprevalue为函数的返回值类型，pretokenvalue是id，token现在是LB
            break;//去识别main
        }else if(token == LB && pretoken == ID && prepretoken != VOID)
        {
                result->childen[2] = no_void_fun_dec();//preprevalue为函数的返回值类型，pretokenvalue是id，token现在是LB
                break;//去识别main

        }else if(token == VOID)
        {
            //需要区分是不是main
            copypre();
            match(token);
            if(token == MAIN)
            {
                break;//去识别main,pretoken为VOID，token为main
            }else if(token == ID)
            {
                copyprepre();
                copypre();
                match(token);
                result->childen[2] = void_fun_dec();//preprevalue为VOID，pretokenvalue是id，token现在是LB
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }
        }
    }

    if(token == VOID || (token == MAIN && pretoken == VOID))
    {
        if(token == VOID)
        {
            copypre();
            match(token);
        }
        if(token == MAIN)
        {
            result->childen[3] = main_dec();//去识别main,pretoken为VOID，token为main
        }
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    return result;
}

ASTNODE* const_dec()//需要重构
{
    //const已经被识别过了
    ASTNODE * result = nodeconstructor(CONSTDECLARE);
    ASTNODE * t = result;
    ASTNODE * sub = nodeconstructor(CONSTSUBDECLARE);
    ASTNODE * temp1 = NULL;
    ASTNODE * temp2 = NULL;
    ASTNODE * p = NULL;
    Expkind constkind;

    result->childen[0] = sub;
    p = sub;
    if(token == INT)
    {
        constkind = Integer;
        result->kind = Integer;
    }else if(token == CHAR)
    {
        constkind = Char;
        result->kind = Char;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
    match(token);
    if(token == ID)
    {
        temp1 = nodeconstructor(IDTYPE);
        strcpy(temp1->attr.name,tokenvalue);
        temp1->lineno = linenumber;
        temp1->kind = constkind;
        p->childen[0] = temp1;

        match(ID);
        match(ASSIGN);

        if(token == INTGE)
        {
            temp2 = nodeconstructor(CONSTNUM);
            temp2->attr.val = atoi(tokenvalue);
        }else if(token == CHR)
        {
            temp2 = nodeconstructor(CONSTCHAR);
            temp2->attr.ch = tokenvalue[1];
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
        p->childen[1] = temp2;
        match(token);

        while(token == DOT)
        {
            p->sibling = nodeconstructor(CONSTSUBDECLARE);
            p = p->sibling;
            match(DOT);
            if(token == ID)
            {
                temp1 = nodeconstructor(IDTYPE);
                strcpy(temp1->attr.name,tokenvalue);
                temp1->lineno = linenumber;
                temp1->kind = constkind;
                p->childen[0] = temp1;

                match(ID);
                match(ASSIGN);

                if(token == INTGE)
                {
                    temp2 = nodeconstructor(CONSTNUM);
                    temp2->attr.val = atoi(tokenvalue);
                }else if(token == CHR)
                {
                    temp2 = nodeconstructor(CONSTCHAR);
                    temp2->attr.ch = tokenvalue[1];
                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }
                p->childen[1] = temp2;
                match(token);
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }
        }

        match(SEMI);

        while(token == CONST)
        {
            t->sibling = nodeconstructor(CONSTDECLARE);
            t = t->sibling;
            sub = nodeconstructor(CONSTSUBDECLARE);
            t->childen[0] = sub;
            p = sub;
            match(CONST);
            if(token == INT)
            {
                constkind = Integer;
                t->kind = Integer;
            }else if(token == CHAR)
            {
                constkind = Char;
                t->kind = Char;
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }
            match(token);
            if(token == ID)
            {
                temp1 = nodeconstructor(IDTYPE);
                strcpy(temp1->attr.name,tokenvalue);
                temp1->lineno = linenumber;
                temp1->kind = constkind;
                p->childen[0] = temp1;

                match(ID);
                match(ASSIGN);

                if(token == INTGE)
                {
                    temp2 = nodeconstructor(CONSTNUM);
                    temp2->attr.val = atoi(tokenvalue);
                }else if(token == CHR)
                {
                    temp2 = nodeconstructor(CONSTCHAR);
                    temp2->attr.ch = tokenvalue[1];
                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }

                p->childen[1] = temp2;
                match(token);

                while(token == DOT)
                {
                    p->sibling = nodeconstructor(CONSTSUBDECLARE);
                    p = p->sibling;
                    match(DOT);
                    if(token == ID)
                    {
                        temp1 = nodeconstructor(IDTYPE);
                        strcpy(temp1->attr.name,tokenvalue);
                        temp1->lineno = linenumber;
                        temp1->kind = constkind;
                        p->childen[0] = temp1;

                        match(ID);
                        match(ASSIGN);

                        if(token == INTGE)
                        {
                            temp2 = nodeconstructor(CONSTNUM);
                            temp2->attr.val = atoi(tokenvalue);
                        }else if(token == CHR)
                        {
                            temp2 = nodeconstructor(CONSTCHAR);
                            temp2->attr.ch = tokenvalue[1];
                        }else
                        {
                            //错误处理，待完善
                            printf("%d line err",linenumber);
                            exit(1);
                        }
                        p->childen[1] = temp2;
                        match(token);
                    }else
                    {
                        //错误处理，待完善
                        printf("%d line err",linenumber);
                        exit(1);
                    }
                }

                match(SEMI);

            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }

        }

    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    return result;
}//const识别玩了之后token为分号后面的那个符号 const a = 2;(.)

ASTNODE* vari_dec()//需要重构
{
    //类型在prepretoken中 id在pretokenvalue中，token现在是DOT|SEMI|LSB
    ASTNODE* result = nodeconstructor(VARIDECLARE);
    ASTNODE* t = result;
    ASTNODE* temp = NULL;
    ASTNODE* temp1 = NULL;
    ASTNODE* temp2 = NULL;
    ASTNODE* sub = nodeconstructor(SUBVARIDECLARE);
    ASTNODE* p = sub;
    Expkind constkind;
    //为VARIDECLARE节点指明一下所声明的变量的类型 , 需要修改一下
    if(prepretoken == CHAR)
    {
        constkind = Char;
        result->kind = Char;
    }else
    {
        constkind = Integer;
        result->kind = Integer;
    }

    t->childen[0] = p;

    if(token == SEMI)
    {
        temp = nodeconstructor(IDTYPE);
        strcpy(temp->attr.name,pretokenvalue);
        temp->lineno = prelinenumber;
        temp->kind = constkind;
        p->childen[0] = temp;

        match(SEMI);

        if(token == VOID || token == IF || token == DO || token == FOR || token == LBB || token == ID || token == PRINTF || token == SCANF || token == RETURN)
        {
            //如果是VOID已经是函数定义部分了可以返回了，如果是其他的则是从复合语句的声明过来的
            return result;
        }else if(token == INT || token == CHAR)
        {
            copypre();
            match(token);
            copyprepre();
            copypre();
            match(ID);

            if(token == DOT || token == SEMI || token == LSB)
            {
                    result->sibling = vari_dec();
                    return result;
            }else if(token == LB)
            {
                    return result;
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }

        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }

    if(token == DOT || token == LSB)//int a , c[2] , d , e , f;
    {
        if(token == DOT)
        {
            temp = nodeconstructor(IDTYPE);
            strcpy(temp->attr.name,pretokenvalue);
            temp->lineno = prelinenumber;
            temp->kind = constkind;
            p->childen[0] = temp;

            match(DOT);
            copypre();
            match(ID);
        }
        else
        {
            temp = nodeconstructor(ARRAYTYPE);
            temp->kind = constkind;

            p->childen[0] = temp;

            temp1 = nodeconstructor(IDTYPE);
            strcpy(temp1->attr.name,pretokenvalue);
            temp1->lineno = prelinenumber;
            temp1->kind = constkind;

            match(LSB);

            if(token == INTGE)
            {
                temp2 = nodeconstructor(CONSTNUM);
                temp2->attr.val = atoi(tokenvalue);

                temp->childen[0] = temp1;
                temp->childen[1] = temp2;
            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }

            match(token);
            match(RSB);

            if(token == DOT)
            {
                match(DOT);
                copypre();
                match(ID);
            }else if(token == SEMI)
            {
                match(SEMI);

                if(token == VOID || token == IF || token == DO || token == FOR || token == LBB || token == ID || token == PRINTF || token == SCANF || token == RETURN)//可改造
                {
                    //如果是VOID已经是函数定义部分了可以返回了，如果是其他的则是从复合语句的声明过来的
                    return result;
                }else if(token == INT || token == CHAR)
                {
                    copypre();
                    match(token);
                    copyprepre();
                    copypre();
                    match(ID);

                    if(token == DOT || token == SEMI || token == LSB)
                    {
                            result->sibling = vari_dec();
                            return result;
                    }else if(token == LB)
                    {
                            return result;
                    }else
                    {
                        //错误处理，待完善
                        printf("%d line err",linenumber);
                        exit(1);
                    }

                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }

            }else
            {
                //错误处理，待完善
                printf("%d line err",linenumber);
                exit(1);
            }
        }

        while(token == LSB || token == DOT || token == SEMI)
        {
            if(token == LSB)
            {
                //说明是一个数组
                p->sibling = nodeconstructor(SUBVARIDECLARE);
                p = p->sibling;

                temp = nodeconstructor(ARRAYTYPE);
                temp->kind = constkind;

                p->childen[0] = temp;

                temp1 = nodeconstructor(IDTYPE);
                strcpy(temp1->attr.name,pretokenvalue);
                temp1->lineno = prelinenumber;
                temp1->kind = constkind;

                match(LSB);

                if(token == INTGE)
                {
                    temp2 = nodeconstructor(CONSTNUM);
                    temp2->attr.val = atoi(tokenvalue);

                    temp->childen[0] = temp1;
                    temp->childen[1] = temp2;
                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }
                match(token);
                match(RSB);

                if(token == DOT)
                {
                    match(DOT);
                    copypre();
                    match(ID);
                    continue;
                }else if(token == SEMI)
                {
                     match(SEMI);

                    if(token == VOID || token == IF || token == DO || token == FOR || token == LBB || token == ID || token == PRINTF || token == SCANF || token == RETURN)//可改造
                    {
                        //如果是VOID已经是函数定义部分了可以返回了，如果是其他的则是从复合语句的声明过来的
                        return result;
                    }else if(token == INT || token == CHAR)
                    {
                        copypre();
                        match(token);
                        copyprepre();
                        copypre();
                        match(ID);

                        if(token == DOT || token == SEMI || token == LSB)
                        {
                                result->sibling = vari_dec();
                                return result;
                        }else if(token == LB)
                        {
                                return result;
                        }else
                        {
                            //错误处理，待完善
                            printf("%d line err",linenumber);
                            exit(1);
                        }

                    }else
                    {
                        //错误处理，待完善
                        printf("%d line err",linenumber);
                        exit(1);
                    }

                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }


            }else if(token == DOT)
            {
                //说明是一个变量
                p->sibling = nodeconstructor(SUBVARIDECLARE);
                p = p->sibling;

                temp = nodeconstructor(IDTYPE);
                strcpy(temp->attr.name,pretokenvalue);
                temp->lineno = prelinenumber;
                temp->kind = constkind;

                p->childen[0] = temp;

                match(DOT);

                if(token == ID)
                {
                    copypre();
                    match(ID);
                    continue;
                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }

            }else if(token == SEMI)
            {
                //说明结束了

                p->sibling = nodeconstructor(SUBVARIDECLARE);
                p = p->sibling;

                temp = nodeconstructor(IDTYPE);
                strcpy(temp->attr.name,pretokenvalue);
                temp->lineno = prelinenumber;
                temp->kind = constkind;

                p->childen[0] = temp;

                match(SEMI);

                if(token == VOID || token == IF || token == DO || token == FOR || token == LBB || token == ID || token == PRINTF || token == SCANF || token == RETURN)//可改造
                {
                    //如果是VOID已经是函数定义部分了可以返回了，如果是其他的则是从复合语句的声明过来的
                    return result;
                }else if(token == INT || token == CHAR)
                {
                    copypre();
                    match(token);
                    copyprepre();
                    copypre();
                    match(ID);

                    if(token == DOT || token == SEMI || token == LSB)
                    {
                            result->sibling = vari_dec();
                            return result;
                    }else if(token == LB)
                    {
                            return result;
                    }else
                    {
                        //错误处理，待完善
                        printf("%d line err",linenumber);
                        exit(1);
                    }

                }else
                {
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
                }
            }

        }

    }
    else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }


}//可能在token为VOID的时候返回，可能在token为LB，pretoken为ID，prepretoken为类型返回||在复合语句部分是在do for等终结符返回
ASTNODE* no_void_fun_dec()
{
    //prepretoken为符号的类型，pretoken为标识符，token为LB
    //接下来每次都还要识别到token为main或者标识符才可以确定接下来是什么
    //所以对于main来说token为MAIN，pretoken为VOID
    ASTNODE* result = nodeconstructor(FUNCDECLARE);
    ASTNODE* temp1 = nodeconstructor(IDTYPE);

    result->childen[0] = temp1;

    if(prepretoken == INT)
    {
        result->kind = Integer;
    }else
    {
        result->kind = Char;
    }

    temp1->lineno = prelinenumber;
    strcpy(temp1->attr.name , pretokenvalue);

    match(LB);
    result->childen[1] = arg_lists();//token为int|char|)

    match(LBB);

    result->childen[2] = compond_stmt();//token为{后面的

    match(RBB);

    if(token == VOID)
    {
        copypre();
        match(token);
        if(token == MAIN)
        {
            return result;
        }else if(token == ID)
        {
            copyprepre();
            copypre();
            match(ID);
            result->sibling = void_fun_dec();
            return result;
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }else if(token == INT || token == CHAR)
    {
        copypre();
        match(token);
        copyprepre();
        copypre();
        match(ID);
        result->sibling = no_void_fun_dec();
        return result;

    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
}
ASTNODE* void_fun_dec()
{
    ASTNODE* result = nodeconstructor(FUNCDECLARE);
    ASTNODE* temp1 = nodeconstructor(IDTYPE);

    result->childen[0] = temp1;
    result->kind = Void;

    temp1->lineno = prelinenumber;
    strcpy(temp1->attr.name , pretokenvalue);

    match(LB);
    result->childen[1] = arg_lists();//token为int|char|)

    match(LBB);

    result->childen[2] = compond_stmt();//token为{后面的

    match(RBB);

    if(token == VOID)
    {
        copypre();
        match(token);
        if(token == MAIN)
        {
            return result;
        }else if(token == ID)
        {
            copyprepre();
            copypre();
            match(ID);
            result->sibling = void_fun_dec();
            return result;
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }else if(token == INT || token == CHAR)
    {
        copypre();
        match(token);
        copyprepre();
        copypre();
        match(ID);
        result->sibling = no_void_fun_dec();
        return result;

    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
}
ASTNODE* arg_lists()
{
    //可能为char|int|)
    if(token == RB)
    {
        match(RB);
        return NULL;
    }
    else if(token == CHAR||token == INT )
    {
        return arg_list();//符号为char|int

    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
}
ASTNODE* arg_list()
{
    ASTNODE* result = nodeconstructor(ARG);
    ASTNODE* temp = nodeconstructor(IDTYPE);

    result->childen[0] = temp;
    if(token == CHAR)
    {
        result->kind = Char;
    }else if(token == INT)
    {
        result->kind = Integer;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
    match(token);
    if(token == ID)
    {
        temp->lineno = linenumber;
        strcpy(temp->attr.name,tokenvalue);
        match(ID);
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    if(token == DOT)
    {
        match(DOT);
        result->sibling = arg_list();
        return result;
    }else if(token == RB)
    {
        match(RB);
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
        return result;
}
ASTNODE* main_dec()
{
    //识别main,pretoken为VOID，token为main
    ASTNODE* result = nodeconstructor(MAINTYPE);
    result->kind = Void;
    match(MAIN);
    match(LB);
    match(RB);
    match(LBB);
    result->childen[0] = compond_stmt();
    match(RBB);

    //2020/5/02修改,识别完main之后应该是整个文件结束
    match(ENDFILE);

    return result;
}
ASTNODE* compond_stmt()
{
    ASTNODE* result = nodeconstructor(COM_STMT);

    switch (token)
    {
    case CONST :
            match(CONST);
            result->childen[0] = const_dec();
    default:
        switch(token)
        {
            case INT :
            case CHAR:
                copypre();
                match(token);
                copyprepre();
                copypre();
                match(token);
                result->childen[1] = vari_dec();//完了之后便是下一个符号 if do等等
            case IF:
            case DO:
            case FOR:
            case LBB:
            case ID:
            case PRINTF:
            case SCANF:
            case RETURN:
            case SEMI://{;}空语句
                    result->childen[2] = stmt_seq();
                    break;
            case RBB://{}里面没有任何东西
                    return NULL;
                    break;
            default :
                    //错误处理，待完善
                    printf("%d line err",linenumber);
                    exit(1);
        }
    }
    return result;
}
ASTNODE* stmt_seq()
{
    ASTNODE* result = nodeconstructor(STMT);
    result->childen[0] = stmt();

    if(token == IF || token == DO || token == FOR || token == LBB || token == ID || token == PRINTF || token == SCANF || token == RETURN || token == SEMI )
    {
        result->sibling = stmt_seq();
    }
    return result;
}
ASTNODE* stmt()
{
    ASTNODE* result = NULL;

    switch(token)
    {
    case IF:
        result = condition_stmt();
        break;
    case DO:
        result = while_stmt();
        break;
    case FOR:
        result = for_stmt();
        break;
    case LBB:
        match(LBB);
        result = stmt_seq();
        match(RBB);
        break;
    case PRINTF:
        result = printf_stmt();
        match(SEMI);
        break;
    case SCANF:
        result = scanf_stmt();
        match(SEMI);
        break;
    case RETURN:
        result = return_stmt();
        match(SEMI);
        break;
    case SEMI:
        match(SEMI);
        return NULL;
    case RBB:
        return NULL;
    case ID:
        //这里需要区分是赋值语句还是函数调用语句
        copypre();
        match(ID);
        if(token == ASSIGN || token == LSB)
        {
            result = assgin_stmt();//a = || a[
            match(SEMI);
        }else if(token == LB)
        {
            result = fun_call();//a(
            match(SEMI);
        }
        break;
    default:
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    return result;
}
ASTNODE* condition_stmt()
{
    //条件if语句
    ASTNODE* result = nodeconstructor(IF_STME);

    match(IF);
    match(LB);
    result->childen[0] = condition();
    match(RB);
    result->childen[1] = stmt();
    if(token == ELSE)
    {
        match(ELSE);
        result->childen[2] = stmt();
    }
    return result;
}
ASTNODE* while_stmt()
{
    ASTNODE * result = nodeconstructor(WHILE_STMT);

    match(DO);
    result->childen[0] = stmt();
    match(WHILE);
    match(LB);
    result->childen[1] = condition();
    match(RB);

    return result;
}
ASTNODE* for_stmt()
{
    ASTNODE* result = nodeconstructor(FOR_STMT);
    ASTNODE* temp = nodeconstructor(IDTYPE);

    match(FOR);
    match(LB);
    if(token == ID)
    {
        temp->lineno = linenumber;
        strcpy(temp->attr.name,tokenvalue);
        result->childen[0] = temp;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(ID);
    match(ASSIGN);
    result->childen[1] = exp();
    match(SEMI);
    result->childen[2] = condition();
    match(SEMI);

    if(token == ID)
    {
        temp = nodeconstructor(IDTYPE);
        temp->lineno = linenumber;
        strcpy(temp->attr.name,tokenvalue);
        result->childen[3] = temp;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(ID);
    match(ASSIGN);

    if(token == ID)
    {
        temp = nodeconstructor(IDTYPE);
        temp->lineno = linenumber;
        strcpy(temp->attr.name,tokenvalue);
        result->childen[4] = temp;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(ID);
    if(token == PLUS || token == MINUS)
    {
        result->ttype = token;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(token);

    temp = nodeconstructor(CONSTNUM);

    if(token == INTGE)
    {
        temp->lineno = linenumber;
        temp->attr.val = atoi(tokenvalue);
        result->childen[5] = temp;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(INTGE);
    match(RB);
    result->childen[6] = stmt();

    return result;
}

ASTNODE* printf_stmt()
{
    ASTNODE* result = nodeconstructor(PRINTF_STMT);
    ASTNODE* temp = NULL;

    match(PRINTF);
    match(LB);

    if(token == STRING)
    {
        temp = nodeconstructor(CONSTSTRING);
        temp->lineno = linenumber;
        eval(temp->attr.str,tokenvalue);
        result->childen[0] = temp;
        match(token);

        if(token == RB)
        {
            match(RB);
            return result;
        }else if(token == DOT)
        {
            match(DOT);
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }

    if(token == PLUS || token == MINUS || token == ID || token == INTGE || token == CHR || token == RB)
    {
        result->childen[1] = exp();
        match(RB);
        return result;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

}

void eval(char* dest,char* source)//用来去掉字符串的“ ”符号
{
    char temp[MaxTokenLength+1];
    int length = strlen(source);
    int i;
    //提取从下标1到length-2
    for(i=1;i<length-1;i++)
    {
        *(temp+i-1) = *(source+i);
    }
    *(temp+i-1) = '\0';

    strcpy(dest,temp);
}
ASTNODE* scanf_stmt()
{
    ASTNODE* result = nodeconstructor(SCANF_STMT);
    result->lineno = linenumber;
    ASTNODE* temp = nodeconstructor(IDTYPE);
    ASTNODE* p = temp;

    match(SCANF);
    match(LB);

    if(token == ID)
    {
        temp->lineno = linenumber;
        strcpy(temp->attr.name,tokenvalue);
        result->childen[0] = temp;
        match(ID);
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    while(token == DOT)
    {
        match(DOT);
        if(token == ID)
        {
            p->sibling = nodeconstructor(IDTYPE);
            p = p->sibling;
            p->lineno = linenumber;
            strcpy(p->attr.name,tokenvalue);
            match(ID);
        }else
        {
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);
        }
    }

    if(token == RB)
    {
        match(RB);
        return result;

    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }
}
ASTNODE* return_stmt()
{
    ASTNODE* result = nodeconstructor(RETURN_STMT);

    match(RETURN);

    if(token == SEMI)
    {
        return result;
    }else if(token == LB)
    {
        match(LB);
        result->childen[0] = exp();
        match(RB);
        return result;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

}
ASTNODE* assgin_stmt()
{
    //a[ || a =
    ASTNODE* result = nodeconstructor(ASSIGN_STMT);
    result->lineno = linenumber;
    ASTNODE* temp = NULL;
    ASTNODE* temp1 = NULL;

    if(token == LSB)
    {
        temp = nodeconstructor(ARRAYTYPE);
        temp->lineno = linenumber;
        temp1 = nodeconstructor(IDTYPE);
        temp1->lineno = prelinenumber;
        strcpy(temp1->attr.name,pretokenvalue);
        temp->childen[0] = temp1;

        match(LSB);
        temp->childen[1] = exp();
        match(RSB);
        result->childen[0] = temp;
    }else if(token == ASSIGN)
    {
        temp = nodeconstructor(IDTYPE);
        temp->lineno = prelinenumber;
        strcpy(temp->attr.name,pretokenvalue);

        result->childen[0] = temp;
    }else
    {
        //错误处理，待完善
        printf("%d line err",linenumber);
        exit(1);
    }

    match(ASSIGN);

    result->childen[1] = exp();

    return result;
}

ASTNODE* fun_call()
{
    //a(
    ASTNODE* result = nodeconstructor(CALL);
    result->lineno = linenumber;
    ASTNODE* temp = nodeconstructor(IDTYPE);

    temp->lineno = prelinenumber;
    strcpy(temp->attr.name,pretokenvalue);
    result->childen[0] = temp;

    match(LB);
    result->childen[1] = arg_values();
    match(RB);

    return result;
}

ASTNODE* exp()
{
    //如果开头是+|- 就构造一个0+..|0-.. 如果不是就构造一个0+...
    ASTNODE* result = NULL;
    ASTNODE* p = NULL;
    ASTNODE* temp = NULL;

    if(token == PLUS || token == MINUS)
    {
        p = nodeconstructor(OPTYPE);
        p->lineno = linenumber;
        p->ttype = token;
        p->useful = FALSE;
        match(token);
    }else
    {
        p = nodeconstructor(OPTYPE);
        p->lineno = linenumber;
        p->ttype = PLUS;
        p->useful = FALSE;
    }

    temp = nodeconstructor(CONSTNUM);
    temp->attr.val = 0;
    p->childen[0] = temp;
    p->childen[1] = item();

    result = p;

    while(token == PLUS || token == MINUS)
    {
        p = nodeconstructor(OPTYPE);
        p->lineno = linenumber;
        p->ttype = token;
        match(token);
        p->childen[0] = result;
        p->childen[1] = item();

        result = p;
    }
    return result;

}

ASTNODE* item()
{
    ASTNODE* result = NULL;
    ASTNODE* p = nodeconstructor(OPTYPE);
    p->lineno = linenumber;

    p->childen[0] = factor();
    if(token == MULTIP || token == DIVD)
    {
        p->ttype = token;
        match(token);
        p->childen[1] = factor();
        result = p;
    }else
    {
        return p->childen[0];
    }

    while(token == MULTIP || token == DIVD)
    {
        p = nodeconstructor(OPTYPE);
        p->lineno = linenumber;
        p->ttype = token;
        match(token);
        p->childen[0] = result;
        p->childen[1] = factor();
        result = p;
    }

    return result;
}

ASTNODE* factor()//在构建符号表时，可能对常量还需要处理一下
{
    ASTNODE* result = NULL;
    ASTNODE* temp = NULL;

    switch(token)
    {
        case INTGE:
            result = nodeconstructor(CONSTNUM);
            result->attr.val = atoi(tokenvalue);
            match(INTGE);
            return result;
        case CHR:
            result = nodeconstructor(CONSTCHAR);
            result->attr.ch = tokenvalue[1];
            match(CHR);
            return result;
        case ID:
            //可以为a,a[...],a(...)
            copypre();
            match(ID);
            if(token == LSB)
            {
                //a[]
                result = nodeconstructor(ARRAYTYPE);
                result->lineno = linenumber;
                temp = nodeconstructor(IDTYPE);
                temp->lineno = prelinenumber;
                strcpy(temp->attr.name,pretokenvalue);
                result->childen[0] = temp;
                match(LSB);
                result->childen[1] = exp();
                match(RSB);
                return result;

            }else if(token == LB)
            {
                //a()
                result = nodeconstructor(CALL);
                result->lineno = linenumber;
                temp = nodeconstructor(IDTYPE);
                temp->lineno = prelinenumber;
                strcpy(temp->attr.name,pretokenvalue);
                result->childen[0] = temp;
                match(LB);
                result->childen[1] = arg_values();
                match(RB);
                return result;
            }else
            {
                //a
                result = nodeconstructor(IDTYPE);
                result->lineno = prelinenumber;
                strcpy(result->attr.name,pretokenvalue);
                return result;
            }
        case LB:
            match(LB);
            result = exp();
            match(RB);
            return result;
        default :
            //错误处理，待完善
            printf("%d line err",linenumber);
            exit(1);

    }

}

ASTNODE* condition()
{
    ASTNODE* result = NULL;
    ASTNODE* temp = exp();

    if(token == LT || token == LTAE || token == BT || token == BTAE || token == EQ || token == NEQ)
    {
        result = nodeconstructor(OPTYPE);//不知道妥不妥
        result->lineno = linenumber;
        result->ttype = token;
        result->childen[0] = temp;
        match(token);
        result->childen[1] = exp();
        return result;
    }else
    {
        return temp;
    }

}

ASTNODE* arg_values()
{
    ASTNODE* result = nodeconstructor(VALUE_ARG);
    result->lineno = linenumber;
    ASTNODE* p = result;

    if(token == PLUS || token == MINUS || token == ID || token == INTGE || token == CHR || token == LB)
    {
        p->childen[0] = exp();
        while(token == DOT)
        {
            match(DOT);
            p->sibling = arg_values();
        }
        return p;
    }else
    {
        return NULL;

    }
}
void getASTNODE_STR(int input)
{
    switch(input)
    {
    case PROGRAM:
        strcpy(output,"PROGRAM");
        break;
    case CONSTDECLARE:
        strcpy(output,"CONSTDECLARE");
        break;
    case CONSTSUBDECLARE:
        strcpy(output,"CONSTSUBDECLARE");
        break;
    case IDTYPE:
        strcpy(output,"IDTYPE");
        break;
    case CONSTNUM:
        strcpy(output,"CONSTNUM");
        break;
    case CONSTCHAR:
        strcpy(output,"CONSTCHAR");
        break;
    case CONSTSTRING:
        strcpy(output,"CONSTSTRING");
        break;
    case VARIDECLARE:
        strcpy(output,"VARIDECLARE");
        break;
    case SUBVARIDECLARE:
        strcpy(output,"SUBVARIDECLARE");
        break;
    case ARRAYTYPE:
        strcpy(output,"ARRAYTYPE");
        break;
    case FUNCDECLARE:
        strcpy(output,"FUNCDECLARE");
        break;
    case ARGS:
        strcpy(output,"ARGS");
        break;
    case ARG:
        strcpy(output,"ARG");
        break;
    case MAINTYPE:
        strcpy(output,"MAINTYPE");
        break;
    case COM_STMT:
        strcpy(output,"COM_STMT");
        break;
    case STMT_SEQ:
        strcpy(output,"STMT_SEQ");
        break;
    case STMT:
        strcpy(output,"STMT");
        break;
    case OPTYPE:
        strcpy(output,"OPTYPE");
        break;
    case CALL:
        strcpy(output,"CALL");
        break;
    case ASSIGN_STMT:
        strcpy(output,"ASSIGN_STMT");
        break;
    case IF_STME:
        strcpy(output,"IF_STME");
        break;
    case FOR_STMT:
        strcpy(output,"FOR_STMT");
        break;
    case RELATION:
        strcpy(output,"RELATION");
        break;
    case VALUE_ARG:
        strcpy(output,"VALUE_ARG");
        break;
    case WHILE_STMT:
        strcpy(output,"WHILE_STMT");
        break;
    case SCANF_STMT:
        strcpy(output,"SCANF_STMT");
        break;
    case PRINTF_STMT:
        strcpy(output,"PRINTF_STMT");
        break;
    case RETURN_STMT:
        strcpy(output,"RETURN_STMT");
        break;
    default :
        strcpy(output,"");
    }
}
void tranverse(ASTNODE* tree)
{
    int i;

    if(tree != NULL)
    {
        getASTNODE_STR(tree->type);
        printf("%s\n",output);

        for(i=0;i<MAXCHILDREN;i++)
        {
            tranverse(tree->childen[i]);
        }
        tranverse(tree->sibling);
    }
}

//int main()
//{
//    fp_source = fopen("./测试代码V3/error2.txt","r");
//
//    ASTNODE* result = program();
//
//    printf("grammer scan complete!\n");
//
//    printf("start the pre order tranverse ... \n");
//
//    tranverse(result);
//
//    return 0;
//}
