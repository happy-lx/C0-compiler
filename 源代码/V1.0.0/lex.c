#include"global.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"lex.h"

#define MaxTokenLength 255//一个单词最大长度
#define MaxBufferSize 1000//一行句子中最大的字符数量
#define MaxReservedLength 15

typedef enum
{
    START,END,EXCLAM,LESS,GREATER,EQUAL,CHARIN,CHARON,STR,POSANDNEG,IDM,INTGER
}statetype;

struct
{
    char* name;
    tokentype type;
}reservedwords[MaxReservedLength]={
{"void",VOID},{"int",INT},{"char",CHAR},{"if",IF},{"else",ELSE},{"const",CONST},{"while",WHILE},{"for",FOR},
{"do",DO},{"return",RETURN},{"break",BREAK},{"continue",CONTINUE},{"printf",PRINTF},{"scanf",SCANF},{"main",MAIN}
};

char tokenvalue[MaxTokenLength+1];
char linebuffer[MaxBufferSize];
int lineposition = 0;
int linebuffersize = 0;
int is_EOF = FALSE;
int print_log = FALSE;

int isDigit(int);//0-9
int isLetter(int);//a-z|A-Z
int isNonZeroDigit(int);//1-9
int isSymbol(int);//+|-|*|/
int getnextchar();
void ungetnextchar();
tokentype lookup(tokentype);
void errorhander(int ,int);

void errorhander(int number ,int input)
{
    printf("err at line %5d : token \'%c\' is not expected\n",number,(char)input);
}

tokentype lookup(tokentype input)
{
    int i;

    for(i=0;i<MaxReservedLength;i++)
    {
        if(strcmp(tokenvalue,reservedwords[i].name) == 0)
        {
            return reservedwords[i].type;
        }
    }

    return ID;
}

void ungetnextchar()
{
    lineposition--;
}

int getnextchar()
{
    if(lineposition<linebuffersize)
    {
        return linebuffer[lineposition++];
    }else
    {
        linenumber++;
        lineposition = 0;
        if(fgets(linebuffer,MaxBufferSize-1,fp_source))
        {
            linebuffersize = strlen(linebuffer);
            return linebuffer[lineposition++];
        }
        else
        {
            is_EOF = TRUE;
            return EOF;
        }
    }
}

int isDigit(int input)
{
    if(input >= '0' && input <= '9')
        return TRUE;
    return FALSE;
}
int isLetter(int input)
{
    if((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z'))
        return TRUE;
    return FALSE;
}
int isNonZeroDigit(int input)
{
    if(input >= '1' && input <= '9')
        return TRUE;
    return FALSE;
}
int isSymbol(int input)
{
    if(input == '+' || input == '-' || input == '*' || input == '/')
        return TRUE;
    return FALSE;
}
tokentype getToken()
{
    int tokenvalueindex = 0;
    statetype currentstate = START;
    tokentype currenttype;
    int is_save ;

    while(currentstate != END)
    {
        is_save = TRUE;
        int temp = getnextchar();
        switch (currentstate)
        {
            case START:
                if(temp == '!')
                {
                    currentstate = EXCLAM;
                    break;
                }else if(temp == '<')
                {
                    currentstate = LESS;
                    break;
                }else if(temp == '>')
                {
                    currentstate = GREATER;
                    break;
                }else if(temp == '=')
                {
                    currentstate = EQUAL;
                    break;
                }else if(temp == '\"')
                {
                    currentstate = STR;
                    break;
                }else if(temp == '\'')
                {
                    currentstate = CHARIN;
                    break;
                }else if(isLetter(temp) || temp == '_')
                {
                    currentstate = IDM;
                    break;
                }else if(isNonZeroDigit(temp))
                {
                    currentstate = INTGER;
                    break;
                }else if(temp == '-' || temp == '+')
                {
                    currentstate = POSANDNEG;
                    break;
                }else if(temp == '0')
                {
                    currentstate = END;
                    currenttype = INTGE;
                    break;
                }else if(temp == '\t' || temp == ' ' || temp == '\n')
                {
                    is_save = FALSE;
                    break;
                }else if(temp == EOF)
                {
                    is_save = FALSE;
                    currenttype = ENDFILE;
                    currentstate = END;
                    break;
                }
                else if(temp == ',' || temp == '*' || temp == '{' || temp == '}' || temp == '/' || temp == '(' || temp == ')' || temp == ';' || temp == '[' || temp == ']')
                {
                    currentstate = END;
                    if(temp == ',')
                        currenttype = DOT;
                    else if(temp == '*')
                        currenttype = MULTIP;
                    else if(temp == '{')
                        currenttype = LBB;
                    else if(temp == '}')
                        currenttype = RBB;
                    else if(temp == '(')
                        currenttype = LB;
                    else if(temp == ')')
                        currenttype = RB;
                    else if(temp == '/')
                        currenttype = DIVD;
                    else if(temp == ';')
                        currenttype = SEMI;
                    else if(temp == '[')
                        currenttype = LSB;
                    else if(temp == ']')
                        currenttype = RSB;
                    break;
                }else
                {
                    currentstate = END;
                    currenttype = ERR;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }
            case EXCLAM :
                if(temp == '=')
                {
                    currenttype = NEQ;
                    currentstate = END;
                    break;
                }else if(temp == EOF)
                {
                    is_save = FALSE;
                    currenttype = ENDFILE;
                    currentstate = END;
                    break;
                }
                else
                {
                    currenttype = ERR;
                    currentstate = END;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }
            case LESS :
                if(temp == '=')
                {
                    currenttype = LTAE;
                    currentstate = END;
                    break;
                }else
                {
                    currenttype = LT;
                    currentstate = END;
                    is_save = FALSE;
                    ungetnextchar();
                    break;
                }
            case GREATER :
                if(temp == '=')
                {
                    currenttype = BTAE;
                    currentstate = END;
                    break;
                }else
                {
                    currenttype = BT;
                    currentstate = END;
                    is_save = FALSE;
                    ungetnextchar();
                    break;
                }
            case EQUAL:
                if(temp == '=')
                {
                    currenttype = EQ;
                    currentstate = END;
                    break;
                }else
                {
                    currenttype = ASSIGN;
                    currentstate = END;
                    is_save = FALSE;
                    ungetnextchar();
                    break;
                }
            case CHARIN :
                if(isSymbol(temp) || isDigit(temp) || isLetter(temp))
                {
                    currentstate = CHARON;
                    break;
                }else if(temp == EOF)
                {
                    is_save = FALSE;
                    currenttype = ENDFILE;
                    currentstate = END;
                    break;
                }
                else
                {
                    currenttype = ERR;
                    currentstate = END;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }
            case CHARON :
                if(temp == '\'')
                {
                    currenttype = CHR;
                    currentstate = END;
                    break;
                }else if(temp == EOF)
                {
                    is_save = FALSE;
                    currenttype = ENDFILE;
                    currentstate = END;
                    break;
                }
                else
                {
                    currenttype = ERR;
                    currentstate = END;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }
            case STR :
                if(temp == 32 || temp == 33 || (temp >= 35 && temp <= 126))
                {
                    break;
                }else if(temp == '\"')
                {
                    currenttype = STRING;
                    currentstate = END;
                    break;
                }else if(temp == EOF)
                {
                    is_save = FALSE;
                    currenttype = ENDFILE;
                    currentstate = END;
                    break;
                }else
                {
                    currenttype = ERR;
                    currentstate = END;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }
            case POSANDNEG :
                if(temp == '0')
                {
                    currenttype = INTGE;
                    currentstate = END;
                    break;
                }else if(isNonZeroDigit(temp))
                {
                    currentstate = INTGER;
                    break;
                }else
                {
                    is_save = FALSE;
                    currentstate = END;
                    if(tokenvalue[tokenvalueindex-1] == '+')
                        currenttype = PLUS;
                    else if(tokenvalue[tokenvalueindex-1] == '-')
                        currenttype = MINUS;
                    ungetnextchar();
                    break;
                }
            case IDM :
                if(temp == '_' || isLetter(temp) || isDigit(temp))
                {
                    break;
                }else
                {
                    is_save = FALSE;
                    currenttype = ID;
                    currentstate = END;
                    ungetnextchar();
                    break;
                }
            case INTGER :
                if(isDigit(temp))
                {
                    break;
                }else
                {
                    is_save = FALSE;
                    currenttype = INTGE;
                    currentstate = END;
                    ungetnextchar();
                    break;
                }
            default :
                {
                    currenttype = ERR;
                    currentstate = END;
                    is_save = FALSE;
                    errorhander(linenumber,temp);
                    break;
                }

        }
        if(is_save && tokenvalueindex<MaxTokenLength)
        {
            tokenvalue[tokenvalueindex++] = (char)temp;
        }
        if(currentstate == END)
        {
            tokenvalue[tokenvalueindex] = '\0';
        }

    }
    if(currenttype == ID)
    {
        currenttype = lookup(currenttype);
    }
    if(print_log)
    {
        if(currenttype == ENDFILE)
            fprintf(fp_log,"%d\t%s\n",currenttype,"ENDFILE");
        else if(currenttype == ERR)
            fprintf(fp_log,"%d\t%s\n",currenttype,"ERR");
        else
            fprintf(fp_log,"%d\t%s\n",currenttype,tokenvalue);
    }

    return currenttype;
}

//int main()
//{
//    fp_source = fopen("test.txt","r");
//    fp_log = fopen("result.txt","w");
//    printf("Start lex scan...\n");
//    while(getToken() != ENDFILE);
//    printf("Lex scan complete!\n");
//
//    return 0;
//}
