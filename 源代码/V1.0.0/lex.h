#ifndef _LEX_H_
#define _LEX_H_

#include"global.h"

#define MaxTokenLength 255

tokentype getToken();

extern char tokenvalue[MaxTokenLength+1];

#endif
