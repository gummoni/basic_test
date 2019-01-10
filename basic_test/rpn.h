#ifndef __RPN_H__
#define __RPN_H__

typedef struct
{
	int left;
	int right;
	char old_op;
	char cur_op;
	int state;
} rpn_info;


extern void rpn_decode(rpn_info* self, int value);
extern int rpn_result(rpn_info* self);

#endif//__RPN_H__
