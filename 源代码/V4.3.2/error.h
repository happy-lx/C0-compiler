#ifndef _ERROR_H_
#define _ERROR_H_

#define skip_to_void_int_char 0
#define skip_to_void 1
#define skip_to_endfile 2
#define skip_to_RBB 3
#define no_skip 4
#define skip_to_next 5
#define skip_to_RB_next 6
#define skip_to_SEMI_next 7
#define skip_to_SEMI 8
#define skip_to_RBB_next 9
#define skip_to_RB 10


void error_msg(char* , int );

#endif // _ERROR_H_
