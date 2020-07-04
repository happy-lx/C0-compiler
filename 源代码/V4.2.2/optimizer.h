#ifndef _OPTIMIZER_H_
#define _OPTIMIZER_H_

#include "global.h"
#include "lex.h"
#include "grammer.h"
#include "symbol.h"
#include "semantic.h"
#include "midcode.h"
#include "mips.h"
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void optimizer();
void deadcode(ASTNODE*);

#endif // _OPTIMIZER_H_
