#ifndef __RPN_H__
#define __RPN_H__

typedef enum {
	TOKEN_NONE = 0x00000000,
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
	OPE_AND = 0x00001000,
	// 13 : |
	OPE_OR = 0x00002000,
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
} rpn_token;

typedef struct {
	char* rp;								// read pointer
	rpn_token token;						// token
	char context[VARIABLE_NAME_LENGTH];		// command / variable:number / variable:string

	char tmp_key[VARIABLE_NAME_LENGTH];
	char tmp_value[VARIABLE_NAME_LENGTH];
	rpn_token tmp_eval_op;
} rpn_instance;

typedef struct
{
	int left;
	int right;
	rpn_token old_op;
	rpn_token cur_op;
	char* tmp_eval_left;
	char* tmp_eval_right;
	int state;
} rpn_info;


extern bool rpn_execute(BAS_PACKET* packet);
extern bool rpn_judge(BAS_PACKET* packet);
extern char* rpn_get_value(char* key);

#endif//__RPN_H__
