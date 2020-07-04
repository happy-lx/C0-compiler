#include "global.h"
#include "symbol.h"
#include "grammer.h"
#include "lex.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

symboltable* top ;//指向global符号表的指针
symboltable* cur;//当前处在的符号表


int in ;//这个变量来标识找表目的时候，找到的是当前符号表，还是外部global符号表，为true的话是当前符号表

char* typestr[5] = {"TConst","TVariable","TArray","TFunc","TArg"};

entry* entryconstructor(entrytype );
symboltable* symboltableconstructor(char* );
void insert_const_char(char* in_name , char in_val);
void insert_const_int(char* in_name ,int in_val);
void insert_func(char* in_name,Expkind in_kind,int in_paranum);
void insert_array(char* in_name,Expkind in_kind,int in_size);
void insert_arg(char* in_name,Expkind in_kind);

int hash(char* name)//hash 操作
{
    const int shift = 1;
    int i;
    int length = strlen(name);
    int temp = 0;

    for(i=0;i<length;i++)
    {
        temp = ((temp+name[i])<<shift) % MaxBukSize;
    }

    return temp;
}

entry* entryconstructor(entrytype input)
{
    entry* result = (entry*)malloc(sizeof(entry));

    result->type = input;
    result->name = (char*)malloc(sizeof(char)*MaxTokenLength);
    result->next = NULL;
    result->used = FALSE;
    return result;
}

symboltable* symboltableconstructor(char* input)
{
    int i;

    symboltable* result = (symboltable*)malloc(sizeof(symboltable));
    result->name = (char*)malloc(sizeof(char)*MaxTokenLength);

    strcpy(result->name,input);

    for(i=0;i<MaxBukSize;i++)
    {
        result->bucket[i] = NULL;
    }
    result->funchild = NULL;
    result->mainchild = NULL;
    result->sibling = NULL;
    result->parent = NULL;
    result->totaladdr= 0;

    return result;
}

void insert_const_char(char* in_name , char in_val)
{
    //往符号表中插入一个表目，const char类型
    entry* temp = entryconstructor(TConst);
    int hs;

    strcpy(temp->name,in_name);
    temp->val.ch_val = in_val;
    temp->kind = Char;
    temp->addr = cur->totaladdr;
    cur->totaladdr += 1;

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }

}
void insert_const_int(char* in_name , int in_val)
{
    entry* temp = entryconstructor(TConst);
    int hs;

    strcpy(temp->name,in_name);
    temp->val.int_val = in_val;
    temp->kind = Integer;
    temp->addr = cur->totaladdr;
    cur->totaladdr += 1;//这里注意：虽然是int型但是统一只加1，因为编译器前端与机器无关，即这只是一个逻辑偏移，内存分配时再仔细考虑

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }
}

void insert_vari(char* in_name,Expkind in_kind)
{
    entry* temp = entryconstructor(TVariable);
    int hs;

    strcpy(temp->name,in_name);
    temp->kind = in_kind;
    temp->addr = cur->totaladdr;
    cur->totaladdr += 1;

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))//在符号表中没找到 | 找到了但是是在外层，还是可以在内层插入
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }
}
void insert_func(char* in_name,Expkind in_kind,int in_paranum)
{
    entry* temp = entryconstructor(TFunc);
    int hs;

    strcpy(temp->name,in_name);
    temp->kind = in_kind;
    temp->addr = cur->totaladdr;
    cur->totaladdr += 1;
    temp->paranum = in_paranum;

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }
}
void insert_array(char* in_name,Expkind in_kind,int in_size)
{
    entry* temp = entryconstructor(TArray);
    int hs;

    strcpy(temp->name,in_name);
    temp->kind = in_kind;
    temp->addr = cur->totaladdr;
    cur->totaladdr += in_size;//每一个数组元素都要占一个大小的空间
    temp->array_size = in_size;

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }
}
void insert_arg(char* in_name,Expkind in_kind)
{
    entry* temp = entryconstructor(TArg);
    int hs;

    strcpy(temp->name,in_name);
    temp->kind = in_kind;
    temp->addr = cur->totaladdr;
    cur->totaladdr += 1;

    if(!find_table(in_name) || (find_table(in_name) && in == FALSE))
    {
        hs = hash(in_name);
        temp->next = cur->bucket[hs];
        cur->bucket[hs] = temp;

    }else
    {
        printf("符号表中已存在相同的项目\n");
    }
}
entry* find_table(char* in_name)
{
    //通过名字查找符号表，找到了返回entry，没有返回NULL
    //当前指向的符号表为cur，没找到就一直往上找

    int hs = hash(in_name);
    entry* temp = cur->bucket[hs];

    while(temp)
    {
        if(strcmp(temp->name,in_name) == 0)
        {
            //找到了
            in = TRUE;
            return temp;
        }
        temp = temp->next;
    }
    if(cur == top)
    {
        return NULL;
    }else
    {
        temp = cur->parent->bucket[hs];

        while(temp)
        {
            if(strcmp(temp->name,in_name) == 0)
            {
                //找到了
                in = FALSE;
                return temp;
            }
            temp = temp->next;
        }

        return temp;
    }

}

void build_table(ASTNODE* tree)
{
    //通过遍历语法树建立符号表
    //需要操作的节点为1.program节点，观察其有无常量变量定义，函数定义，有的话创建top符号表，并加入到表中。
    //2.function节点，观察有无常量变量定义，创建其符号表，并设置为cur的兄弟节点，或者top的孩子节点
    //3.function节点，需要加入到top符号表中，function节点，还要数出arg的个数
    //4.arg节点，需要加入到function的符号表中
    //5.变量和常量声明节点
    //6.main节点，创建main符号表，加入到top符号表中
    int i, accumulater;
    ASTNODE* p1  ;
    symboltable* temp;

    if(tree != NULL)
    {
        if(tree->type == PROGRAM)
        {
            top = symboltableconstructor("Global");
            cur = top;
        }else if(tree->type == CONSTDECLARE)
        {
            p1 = tree->childen[0];
            if(tree->kind == Integer)
            {
                while(p1)
                {
                    insert_const_int(p1->childen[0]->attr.name,p1->childen[1]->attr.val);
                    p1 = p1->sibling;
                }
            }else
            {
                //Char
                while(p1)
                {
                    insert_const_char(p1->childen[0]->attr.name,p1->childen[1]->attr.ch);
                    p1 = p1->sibling;
                }
            }
            build_table(tree->sibling);
            return ;
        }else if(tree->type == VARIDECLARE)
        {
            p1 = tree->childen[0];
            while(p1)
            {
                if(p1->childen[0]->type == IDTYPE)
                {
                    //说明是插入一个int a
                    insert_vari(p1->childen[0]->attr.name,tree->kind);
                }else
                {
                    //说明是插入一个int a[1]
                    insert_array(p1->childen[0]->childen[0]->attr.name,tree->kind,p1->childen[0]->childen[1]->attr.val);
                }
                p1 = p1->sibling;
            }
            build_table(tree->sibling);
            return ;
        }else if(tree->type == FUNCDECLARE)
        {
            //先保存cur，再将cur指向top，再将function插入到top中，然后在还原cur，再创建function的符号表，在遍历子节点
            temp = cur;
            cur = top;
            accumulater = 0;//记录arg的数量

            //先求出arg的数量
            p1 = tree->childen[1];
            while(p1)
            {
                accumulater++;
                p1 = p1->sibling;
            }

            //function如top表
            insert_func(tree->childen[0]->attr.name,tree->kind,accumulater);

            //还原cur
            cur = temp;
            //创建属于function的符号表
            temp = symboltableconstructor(tree->childen[0]->attr.name);

            //连接符号表
            if(cur == top)
            {
                cur->funchild = temp;
                cur = temp;
                cur->parent = top;
            }else
            {
                cur->sibling = temp;
                cur = temp;
                cur->parent = top;
            }
            //接下来遍历子节点，加入符号表了

        }else if(tree->type == ARG)
        {
            //遍历加入符号表，return
            insert_arg(tree->childen[0]->attr.name,tree->kind);

        }else if(tree->type == MAINTYPE)
        {
            //先保存cur，再将cur指向top，再将main插入到top中，然后在还原cur，再创建main的符号表，在遍历子节点
            temp = cur;
            cur = top;
            insert_func("main",Void,0);
            cur = temp;

            temp = symboltableconstructor("main");

            if(cur == top)
            {
                cur->funchild = temp;
                cur->mainchild = temp;
                cur = temp;
                cur->parent = top;
            }else
            {
                cur->sibling = temp;
                cur = temp;
                cur->parent = top;
                top->mainchild = cur;
            }
        }


        for(i=0;i<MAXCHILDREN;i++)
        {
            build_table(tree->childen[i]);
        }

        build_table(tree->sibling);
    }
}

void printtable()
{
    symboltable* temp;
    entry* p;
    int i;

    if(top)
    {
        printf("Global table:\n");
        printf("EntryType \t %-10s \t addr \t kind \t value \t paranum \t arrarysize \t \n","name");
        for(i=0;i<MaxBukSize;i++)
        {
            if(top->bucket[i] != NULL)
            {
                p = top->bucket[i];
                while(p)
                {   if(p->type == TConst && p->kind == Char)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t %-5c \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->val.ch_val);
                    else if(p->type == TConst && p->kind == Integer)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t %-5d \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->val.int_val);
                    else if(p->type == TVariable)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind);
                    else if(p->type == TArray)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t %-9d \n",typestr[p->type],p->name,p->addr,p->kind,p->array_size);
                    else if(p->type == TFunc)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t %-7d \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->paranum);
                    else if(p->type == TArg)
                    printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind);
                    p = p->next;
                }
            }
        }

        temp = top->funchild;
        while(temp)
        {
            printf("\n%s:\n",temp->name);
            printf("EntryType \t %-10s \t addr \t kind \t value \t paranum \t arrarysize \t \n","name");
            for(i=0;i<MaxBukSize;i++)
            {
                if(temp->bucket[i] != NULL)
                {
                    p = temp->bucket[i];
                    while(p)
                    {   if(p->type == TConst && p->kind == Char)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t %-5c \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->val.ch_val);
                        else if(p->type == TConst && p->kind == Integer)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t %-5d \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->val.int_val);
                        else if(p->type == TVariable)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind);
                        else if(p->type == TArray)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t %-9d \n",typestr[p->type],p->name,p->addr,p->kind,p->array_size);
                        else if(p->type == TFunc)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t %-7d \t ------- \n",typestr[p->type],p->name,p->addr,p->kind,p->paranum);
                        else if(p->type == TArg)
                        printf("%9s \t %-10s \t %-3d \t %-3d \t ----- \t ------- \t ------- \n",typestr[p->type],p->name,p->addr,p->kind);
                        p = p->next;
                    }
                }
            }
            temp = temp->sibling;
        }
    }
}

//int main()
//{
//    fp_source = fopen("./测试程序/1.c","r");
//
//    ASTNODE* result = program();
//
//    build_table(result);
//
//    printtable();
//
//    printf("\nComplete...");
//
//    return 0;
//}
