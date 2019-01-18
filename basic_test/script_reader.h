#ifndef __SCRIPT_READER_H__
#define __SCRIPT_READER_H__
#include "config.h"

typedef enum {
	// 0  : enter
	CUR_NEWLINE = 0x00000001,
	// 1  : numeric[0-9]
	VAR_NUM = 0x00000002,
	// 2  : "****"
	VAR_STR = 0x00000004,
	// 3  : command
	SYN_CMD = 0x00000008,
	// 4  : variable:string
	SYN_STR = 0x00000010,
	// 5  : variable:numeric
	SYN_NUM = 0x00000020,
	// 6  : =
	CALC_EQUAL = 0x00000040,
	// 7  : +
	CALC_PLUS = 0x00000080,
	// 8  : -
	CALC_MINUS = 0x00000100,
	// 9  : *
	CALC_MUL = 0x00000200,
	// 10 : /
	CALC_DIV = 0x00000400,
	// 11 : \ 
	CALC_MOD = 0x00000800,
	// 12 : &
	CALC_AND = 0x00001000,
	// 13 : |
	CALC_OR = 0x00002000,
	// 14 : ==
	OPE_EQ = 0x00004000,
	// 15 : <>
	OPE_NE = 0x00008000,
	// 16 : <
	OPE_GT = 0x00010000,
	// 17 : <=
	OPE_GE = 0x00020000,
	// 18 : >
	OPE_LT = 0x00040000,
	// 19 : >=
	OPE_LE = 0x00080000,
	// 20 : @
	CUR_LABEL = 0x00100000,
	// 21 : (
	PRI_GT = 0x00200000,
	// 22 : )
	PRI_GE = 0x00400000,
} script_token;

typedef struct {
	char* rp;								// read pointer
	script_token token;						// token
	char context[VARIABLE_NAME_LENGTH];		// command / variable:number / variable:string
	char** result;
} script_reader;

typedef struct
{
	int left;
	int right;
	script_token old_op;
	script_token cur_op;
	int state;
} rpn_info;


extern bool rpn_execute(BAS_PACKET_BODY* context);
extern bool rpn_judge(void);


#endif//__SCRIPT_READER_H__
