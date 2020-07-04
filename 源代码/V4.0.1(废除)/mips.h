#ifndef _MIPS_H_
#include<vector>

#define maxregnum 8
#define maxrefernum 100000


typedef struct Reg
{
    int id;
    bool busy;//表示这个寄存器内是否有值 为true表示已经被占用 false表示没有被占用
    std::vector<char*> vari_info;//表示在这个寄存器里面的变量内容a | 常量内容 'c' | 1
}Reg;

#endif // _MIPS_H_
