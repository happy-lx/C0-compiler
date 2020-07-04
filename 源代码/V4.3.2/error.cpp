#include "global.h"
#include "error.h"
#include "grammer.h"
#include "lex.h"
#include<stdio.h>

int error_num = 0;

void error_msg(char* str,int choice)
{
    error_num++;

    printf("%s\n",str);

    switch(choice)
    {
    case skip_to_void_int_char:
        while(token != ENDFILE && token != VOID && token != INT && token != CHAR)
        {
            token = getToken();
        }
        break;
    case skip_to_void:
        while(token != ENDFILE && token != VOID)
        {
            token = getToken();
        }
        break;
    case skip_to_endfile:
        while(token != ENDFILE)
        {
            token = getToken();
        }
        break;
    case skip_to_RBB:
        while(token != ENDFILE && token != RBB)
        {
            token = getToken();
        }
        break;
    case skip_to_next:
        if(token != ENDFILE)
            token = getToken();
        break;
    case skip_to_RB_next:
        while(token != ENDFILE && token != RB)
        {
            token = getToken();
        }
        if(token != ENDFILE)
            token = getToken();
        break;
    case skip_to_SEMI_next:
        while(token != ENDFILE && token != SEMI)
        {
            token = getToken();
        }
        if(token != ENDFILE)
            token = getToken();
        break;
    case skip_to_SEMI:
        while(token != ENDFILE && token != SEMI)
        {
            token = getToken();
        }
        break;
    case skip_to_RBB_next:
        while(token != ENDFILE && token != RBB)
        {
            token = getToken();
        }
        if(token != ENDFILE)
            token = getToken();
        break;
    case skip_to_RB:
        while(token != ENDFILE && token != RB)
        {
            token = getToken();
        }
        break;

    }

}
