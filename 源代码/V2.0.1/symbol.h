#ifndef _SYMBAL_H
#define _SYMBAL_H

#include "global.h"

#define MaxBukSize 211

typedef enum{
TConst , TVariable , TArray , TFunc , TArg
}entrytype;//符号表条目的类型

typedef struct entry//符号表的条目
{
    entrytype type;//条目的类型
    char* name;//identifier的名字，a()的a | int a 的a | const int a 的a | arg
    int addr;//条目的相对偏移地址
    Expkind kind;//identifier的类型，int a 的int | const int a 的int | char a()的char | arg的类型
    union
    {
        char ch_val;
        int int_val;
    }val;//对于常量来说，保存常量的值
    int paranum ;//对于function来说保存参数的个数，以便类型检测
    int array_size;//对于数组来说，保存数组的大小，防止越界
    int used;//用来表示定义过后是否使用了
    struct entry* next;//对于哈希表来说，指向下一个节点
}entry;

typedef struct symboltable
{
    entry* bucket[MaxBukSize];//hash table
    char* name;//对于全局的符号表为global ，函数的局部符号表为function的名字，
    int totaladdr;//总的地址

    struct symboltable* funchild;//对于global来说指向第一个function
    struct symboltable* mainchild;//对于global来说指向main
    struct symboltable* sibling;//对于funtion来说指向第二个function
    struct symboltable* parent;//指向父节点
}symboltable;

int hash(char*);
entry* find_table(char* in_name);
void build_table(ASTNODE* tree);
void printtable();
void insert_vari(char* in_name,Expkind in_kind);

extern symboltable* top;
extern symboltable* cur;

#endif // _SYMBAL_H
