#include "config.h"
#include "dic.h"
#include "rpn.h"

static bool rpn_num(rpn_instance* self, int* result);


//�Ǎ��݃|�C���^�ړ��i���Βl�j
static inline void seek(rpn_instance* self, int offset)
{
	if ('\0' != *self->rp) self->rp += offset;
}

//1�����Ǎ���
static inline char get_char(rpn_instance* self)
{
	char ch = *self->rp;
	if ('\0' == ch)
	{
		return '\0';
	}
	self->rp++;
	return ch;
}

//�󔒍s�X�L�b�v
static inline void skip_space(rpn_instance* self)
{
	while ((' ' == *self->rp) || ('\t' == *self->rp)) self->rp++;
}

//�i
static inline void parse_gt(rpn_instance* self)
{
	self->token = PRI_GT;
	self->context[0] = '\0';
}

//�j
static inline void parse_ge(rpn_instance* self)
{
	self->token = PRI_GE;
	self->context[0] = '\0';
}

//���s[\r, \,\n, \r\n]����
static inline void parse_newline(rpn_instance* self)
{
	if ('\n' == self->rp[1]) self->rp++;
	self->token = CUR_NEWLINE;
	self->context[0] = '\0';
}

//�����擾(0x00, 012345)
static inline void parse_num(rpn_instance* self, char ch)
{
	int idx = 0;
	self->token = VAR_NUM;

	if ('X' == toupper(*self->rp))
	{
		//16�i��
		self->context[idx++] = '0';
		self->context[idx++] = 'x';
		while (true)
		{
			self->rp++;
			ch = toupper(*self->rp);
			if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F'))
			{
				self->context[idx++] = ch;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		//10�i��
		self->context[idx++] = ch;
		while (true)
		{
			ch = *self->rp;
			if ('0' <= ch && ch <= '9')
			{
				self->context[idx++] = ch;
				*self->rp++;
			}
			else
			{
				break;
			}
		}
	}
	self->context[idx] = '\0';
}

//�����^�ϐ��E�����^�ϐ��ǂ��炩�𒲂ׂ�
static inline void parse_val(rpn_instance* self, char ch)
{
	int idx = 0;
	self->context[idx++] = ch;

	while (true)
	{
		ch = get_char(self);
		if ('A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9' || '_' == ch || '.' == ch)
		{
			//�ϐ���
			self->context[idx++] = ch;
		}
		else if ('%' == ch)
		{
			//�����F�I�[
			self->token = SYN_NUM;
			self->context[idx] = '\0';
			break;
		}
		else if ('$' == ch)
		{
			//������F�I�[
			self->token = SYN_STR;
			self->context[idx] = '\0';
			break;
		}
		else
		{
			//�s���ȕ����F�I�[
			seek(self, -1);
			self->token = SYN_NUM;
			self->context[idx] = '\0';
			break;
		}
	}
}

//��������
static inline void parse_str(rpn_instance* self)
{
	int idx = 0;

	self->token = VAR_STR;
	while (true)
	{
		char ch = get_char(self);
		if ('\\' == ch)
		{
			//���ꕶ��
			ch = get_char(self);
			if ('\t' == ch) ch = '\t';
			else if ('\r' == ch) ch = '\r';
			else if ('\n' == ch) ch = '\n';
			else if ('\"' == ch) ch = '\"';
		}
		else if ('\"' == ch)
		{
			//�I�[
			break;
		}
		self->context[idx++] = ch;
	}
	self->context[idx] = '\0';
}

//�L�����
static inline void parse_plus(rpn_instance* self)
{
	self->token = CALC_PLUS;
	self->context[0] = '\0';
}

//�L�����
static inline void parse_minus(rpn_instance* self)
{
	self->token = CALC_MINUS;
	self->context[0] = '\0';
}

//�L�����
static inline void parse_mul(rpn_instance* self)
{
	self->token = CALC_MUL;
	self->context[0] = '\0';
}

//�L�����
static inline void parse_div(rpn_instance* self)
{
	self->token = CALC_DIV;
	self->context[0] = '\0';
}

//�L�����
static inline void parse_equ(rpn_instance* self)
{
	char ch = get_char(self);
	if ('=' == ch)
	{
		self->token = OPE_EQ;
	}
	else
	{
		*self->rp--;
		self->token = CALC_EQUAL;
	}
	self->context[0] = '\0';
}

//�L�����
static inline void parse_ge_ne_gt(rpn_instance* self)
{
	char ch = get_char(self);
	if ('=' == ch)
	{
		self->token = OPE_GE;
	}
	else if ('>' == ch)
	{
		self->token = OPE_NE;
	}
	else
	{
		seek(self, -1);
		self->token = OPE_GT;
	}
}

//�������
static inline void parse_le_ne_lt(rpn_instance* self)
{
	char ch = get_char(self);
	if ('=' == ch)
	{
		self->token = OPE_LE;
	}
	else if ('<' == ch)
	{
		self->token = OPE_NE;
	}
	else
	{
		seek(self, -1);
		self->token = OPE_LT;
	}
}

//���̃g�[�N�����擾����
static bool reader_next(rpn_instance* self)
{
	skip_space(self);
	char ch = toupper(get_char(self));
	if ('(' == ch) parse_gt(self);
	else if (')' == ch) parse_ge(self);
	else if ('\r' == ch || '\n' == ch) parse_newline(self);
	else if ('0' <= ch && ch <= '9') parse_num(self, ch);
	else if ('A' <= ch && ch <= 'Z' || '_' == ch || '.' == ch) parse_val(self, ch);
	else if ('"' == ch) parse_str(self);
	else if ('+' == ch) parse_plus(self);
	else if ('-' == ch) parse_minus(self);
	else if ('*' == ch) parse_mul(self);
	else if ('/' == ch) parse_div(self);
	else if ('=' == ch) parse_equ(self);
	else if ('<' == ch) parse_ge_ne_gt(self);
	else if ('>' == ch) parse_le_ne_lt(self);
	else return false;
	return true;
}

//�����ɕϊ�
static inline int convert_value(rpn_token token, char* value)
{
	int result = 0;
	if (VAR_NUM == token) result = strtol(value, NULL, 0);
	else if (SYN_NUM == token) result = strtol(dic_get(value), NULL, 0);
	return result;
}

//�t�|�[�����h�L�@�f�R�[�h
static void rpn_decode(rpn_instance* self, rpn_info* info)
{
	rpn_token token = self->token;

	switch (info->state)
	{
	case 0:
		if ((CALC_PLUS == token) || (CALC_MINUS == token))
		{
			info->left = 0;
			info->old_op = token;
			info->state = 2;
		}
		else
		{
			info->left = convert_value(token, self->context);
			info->state = 1;
		}
		break;

	case 1:
		info->old_op = token;
		info->state = 2;
		break;

	case 2:
		if (CALC_MUL == info->old_op)
		{
			info->left = info->left * convert_value(token, self->context);
			info->state = 1;
		}
		else if (CALC_DIV == info->old_op)
		{
			info->left = info->left / convert_value(token, self->context);
			info->state = 1;
		}
		else
		{
			info->right = convert_value(token, self->context);
			info->state = 3;
		}
		break;

	case 3:
		info->cur_op = token;
		info->state = 4;
		break;

	case 4:
		if (CALC_MUL == info->cur_op)
		{
			info->right = info->right * convert_value(token, self->context);
		}
		else if (CALC_DIV == info->old_op)
		{
			info->right = info->right / convert_value(token, self->context);
		}
		else if (CALC_PLUS == info->old_op)
		{
			info->left = info->left + info->right;
			info->right = convert_value(token, self->context);
			info->old_op = info->cur_op;
		}
		else if (CALC_MINUS == info->old_op)
		{
			info->left = info->left - info->right;
			info->right = convert_value(token, self->context);
			info->old_op = info->cur_op;
		}
		info->state = 3;
		break;
	}
}

//�v�Z���ʂ�Ԃ�
static int rpn_result(rpn_info* self)
{
	if (3 == self->state)
	{
		if (CALC_MUL == self->old_op) return self->left * self->right;
		else if (CALC_DIV == self->old_op) return self->left / self->right;
		else if (CALC_PLUS == self->old_op) return self->left + self->right;
		else if (CALC_MINUS == self->old_op) return self->left - self->right;
	}
	return self->left;
}

//�������v�Z����
static bool rpn_calc(rpn_instance* self)
{
	if (*self->rp == '=')
	{
		int result;
		if (!rpn_num(self, &result)) return false;
		itoa(result, self->tmp_value, 10);
		bas_parser.result_buff = dic_set(self->tmp_key, self->tmp_value);
	}
	else
	{
		bas_parser.result_buff = dic_get(self->context);
	}
	return true;
}

//�����v�Z����
static bool rpn_num(rpn_instance* self, int* result)
{
	rpn_info rpn_contexts[4];
	rpn_info* pt_rpn = rpn_contexts;
	strcpy(self->tmp_key, self->context);
	pt_rpn->state = 0;

	if (!reader_next(self)) return false;
	if (self->token != CALC_EQUAL) return false;

	while (true)
	{
		if (!reader_next(self))
		{
			*result = rpn_result(pt_rpn);
			return true;
		}
		switch (self->token)
		{
		case VAR_STR:
		case SYN_STR:
		case SYN_CMD:
		default:
			return false;

		case VAR_NUM:
		case SYN_NUM:
		case CALC_PLUS:
		case CALC_MINUS:
		case CALC_MUL:
		case CALC_DIV:
			rpn_decode(self, pt_rpn);
			break;

		case CUR_NEWLINE:
			*result = rpn_result(pt_rpn);
			return true;

		case PRI_GT:
			//�l�X�g�ő�R�܂�
			pt_rpn++;
			pt_rpn->state = 0;
			break;

		case PRI_GE:
			itoa(rpn_result(pt_rpn--), self->context, 10);
			self->token = VAR_NUM;
			rpn_decode(self, pt_rpn);
			break;
		}
	}
}

//������̌���
static bool rpn_str(rpn_instance* self)
{
	if (*self->rp == '=')
	{
		strcpy(self->tmp_key, self->context);
		self->tmp_value[0] = '\0';

		if (!reader_next(self)) return false;
		if (self->token != CALC_EQUAL) return false;

		while (true)
		{
			if (!reader_next(self))
			{
				bas_parser.result_buff = dic_set(self->tmp_key, self->tmp_value);
				return true;
			}
			switch (self->token)
			{
			case VAR_STR:
			case VAR_NUM:
				strcat(self->tmp_value, self->context);
				break;

			case SYN_STR:
			case SYN_NUM:
				strcat(self->tmp_value, dic_get(self->context));
				break;

			case CALC_PLUS:
				break;

			case CUR_NEWLINE:
				bas_parser.result_buff = dic_set(self->tmp_key, self->tmp_value);
				return true;

			default:
				return false;
			}
		}
	}
	else
	{
		bas_parser.result_buff = dic_get(self->context);
		return true;
	}
}

//������
bool rpn_judge(BAS_PACKET* packet)
{
	rpn_instance self;
	self.rp = packet->prm2;

	//���ӎ�荞��
	if (!reader_next(&self))  return false;
	switch (self.token)
	{
	case VAR_NUM:
	case VAR_STR:
		strcpy(self.tmp_eval_left, self.context);
		break;

	case SYN_NUM:
	case SYN_STR:
		strcpy(self.tmp_eval_left, dic_get(self.context));
		break;

	default:
		return false;
	}

	//������荞��
	if (!reader_next(&self))  return false;
	self.tmp_eval_op = self.token;

	//�E�ӎ�荞��
	if (!reader_next(&self))  return false;
	switch (self.token)
	{
	case VAR_NUM:
	case VAR_STR:
		strcpy(self.tmp_eval_right, self.context);
		break;
	case SYN_NUM:
	case SYN_STR:
		strcpy(self.tmp_eval_right, dic_get(self.context));
		break;
	}

	//�]��
	if (OPE_EQ == self.tmp_eval_op) return 0 == strcmp(self.tmp_eval_left, self.tmp_eval_right);
	if (OPE_NE == self.tmp_eval_op) return 0 != strcmp(self.tmp_eval_left, self.tmp_eval_right);
	int valL = strtol(self.tmp_eval_left, NULL, 0);
	int valR = strtol(self.tmp_eval_right, NULL, 0);

	switch (self.tmp_eval_op)
	{
	case OPE_GT: return (valL < valR);
	case OPE_GE: return (valL <= valR);
	case OPE_LT: return (valL > valR);
	case OPE_LE: return (valL >= valR);
	default: return false;
	}
}

//�X�N���v�g��́i��ʁj
bool rpn_execute(BAS_PACKET* packet)
{
	rpn_instance self;
	self.rp = packet->prm1;

	if (reader_next(&self))
	{
		rpn_token token = self.token;
		if (token == SYN_STR) return rpn_str(&self);
		if (token == SYN_NUM) return rpn_calc(&self);
	}
	return false;
}

//�ϐ��̒��g�擾
char* rpn_get_value(char* key)
{
	static rpn_instance self;
	self.rp = key;
	if (reader_next(&self))
	{
		rpn_token token = self.token;
		if (token == SYN_STR) return dic_get(self.context);
		if (token == SYN_NUM) return dic_get(self.context);
	}
	return "\0";
}
