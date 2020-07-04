#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include<stdio.h>
#include<stdlib.h>
#define TRUE 1
#define FALSE 0
#define MAXCHILDREN 7

extern FILE * fp_source;
extern FILE * fp_log;

extern int print_log;

extern int linenumber;

typedef enum
{
    PROGRAM , CONSTDECLARE , CONSTSUBDECLARE , IDTYPE , CONSTNUM , CONSTCHAR , CONSTSTRING,
    VARIDECLARE , SUBVARIDECLARE , ARRAYTYPE , FUNCDECLARE , ARGS , ARG , MAINTYPE , COM_STMT ,
    STMT_SEQ , STMT , OPTYPE ,  CALL , ASSIGN_STMT , IF_STME ,
    FOR_STMT , RELATION , VALUE_ARG, WHILE_STMT , SCANF_STMT , PRINTF_STMT , RETURN_STMT
}NodeType;

typedef enum
{
    Void , Integer , Char
}Expkind;


typedef enum
{
    ID,PLUS,MINUS,MULTIP,DIVD,LT,LTAE,BT,BTAE,EQ,NEQ,CHR,STRING,LBB,RBB,LB,RB,DOT,SEMI,ASSIGN,INTGE,
    VOID,INT,CHAR,IF,ELSE,CONST,WHILE,FOR,DO,RETURN,BREAK,CONTINUE,PRINTF,SCANF,MAIN,ERR,ENDFILE,LSB,RSB
}tokentype;

typedef struct ASTNODE
{
    NodeType type;//AST节点的类型标记

    union{
        char* name ;//对IDTYPE类型节点，标识符的值进行存储，用于填符号表||对函数定义节点，保存函数的名字，用于填符号表
        int val;//对于CONSTNUM类型节点，存储数字的值 || 让value_arg节点记录函数调用时有多少个参数
        char ch;//对于CONSTCHAR类型节点，存储字面量
        char* str;//对于CONSTSTRING类型节点，存储字符串的值
    } attr;
    tokentype ttype;//记录是+,-,<,>,<=,>=,==,!= || 在for中记录步长是加还是减
    struct ASTNODE* childen[MAXCHILDREN];//子节点
    struct ASTNODE* sibling;//兄弟节点
    int lineno;//如果是id需要记录所在行数，用于填符号表
    Expkind kind;//用来记录id是void,int,char类型||对于CONSTDECLARE VARIDECLARE这种节点也使用这个变量来保存声明的类型
    int useful;//这个属性是用来记录对于exp构造的0+和0-，如果为false则表示是构造出来的辅助节点，不需要生成代码和类型检查
}ASTNODE;


#endif
