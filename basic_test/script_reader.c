#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "script_reader.h"
#include "dictionary.h"

script_reader reader;
char tmp_key[VARIABLE_NAME_LENGTH];
char tmp_value[VARIABLE_NAME_LENGTH];
char tmp_eval_left[VARIABLE_NAME_LENGTH];
char tmp_eval_right[VARIABLE_NAME_LENGTH];
script_token tmp_eval_op;

#define STR_CMD_LIST_LENGTH (11)
char* STR_CMD_LIST[STR_CMD_LIST_LENGTH] =
{
	"IF",
	"THEN",
	"ELSE",
	"ELSEIF"

	"FOR",
	"TO",
	"STEP",
	"NEXT",

	"GOTO",
	"GOSUB",
	"RETURN",
	"END",
};

//�\�񖽗ߌ���
static CMD_IDX search_cmd_idx(char* str)
{
	int idx;
	for (idx = 0; idx < STR_CMD_LIST_LENGTH; idx++)
	{
		if (0 == strcmp(STR_CMD_LIST[idx], str)) return idx + 1;
	}
	return CMD_NONE;
}

//�Ǎ��݃|�C���^�ړ��i���Βl�j
static inline void seek(int offset)
{
	if ('\0' != *reader.rp) reader.rp += offset;
}

//1�����Ǎ���
static inline char get_char(void) 
{
  char ch = *reader.rp;
  if ('\0' == ch)
  {
	  reader.err = ERR_EOF;
	  return '\0';
  }
  reader.rp++;
  return ch;
}

//�󔒍s�X�L�b�v
static inline void skip_space(void)
{
	while ((' ' == *reader.rp) || ('\t' == *reader.rp)) reader.rp++;
}

//�i
static inline void parse_gt(void)
{
	reader.token = PRI_GT;
	reader.context[0] = '\0';
}

//�j
static inline void parse_ge(void)
{
	reader.token = PRI_GE;
	reader.context[0] = '\0';
}

//���s[\r, \,\n, \r\n]����
static inline void parse_newline(void)
{
	if ('\n' == reader.rp[1]) reader.rp++;
	reader.token = CUR_NEWLINE;
	reader.context[0] = '\0';
}

//�����擾(0x00, 012345)
static inline void parse_num(char ch)
{
	int idx = 0;
	reader.token = VAR_NUM;

	if ('X' == toupper(*reader.rp))
	{
		//16�i��
		reader.context[idx++] = '0';
		reader.context[idx++] = 'x';
		while (true)
		{
			reader.rp++;
			ch = toupper(*reader.rp);
			if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F'))
			{
				reader.context[idx++] = ch;
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
		reader.context[idx++] = ch;
		while (true)
		{
			ch = *reader.rp;
			if ('0' <= ch && ch <= '9')
			{
				reader.context[idx++] = ch;
				*reader.rp++;
			}
			else
			{
				break;
			}
		}
	}
	reader.context[idx] = '\0';
}

//�����񂩂�ϐ��܂��̓R�}���h���𒲂ׂ�
static inline void parse_cmd_or_val(char ch)
{
	int idx = 0;
	reader.context[idx++] = ch;

	while (true) 
	{
		ch = get_char();
		if ('A' <= ch && ch <= 'Z' || '_' == ch)
		{
			reader.context[idx++] = ch;
		}
		else if ('%' == ch) 
		{
			reader.context[idx++] = '%';
			if ('(' == *reader.rp)
			{
				int idx;
				if (!rpn_num(&idx))
				{
					reader.err = ERR_SYNTAX_ERROR;
					return;
				}
				reader.context[idx++] = '(';
				itoa(idx, tmp_value, 10);

				char* pval = tmp_value;
				int len = strlen(tmp_value);
				while (0 < len--)
				{
					reader.context[idx++] = *(pval++);
				}
				reader.context[idx++] = ')';
			}
			reader.token = SYN_NUM;
			reader.context[idx] = '\0';
			break;
		}
		else if ('$' == ch) 
		{
			reader.context[idx++] = '$';
			reader.token = SYN_STR;
			reader.context[idx] = '\0';
			break;
		}
		else 
		{
			seek(-1);
			reader.context[idx] = '\0';
			reader.cmd = search_cmd_idx(reader.context);
			if (CMD_NONE == reader.cmd)
			{
				reader.context[idx++] = '%';
				reader.token = SYN_NUM;
			}
			else
			{
				reader.token = SYN_CMD;
			}
			break;
		}
	}
}

static inline void parse_str(void)
{
	int idx = 0;

	reader.token = VAR_STR;
	while (true)
	{
		char ch = get_char();
		if ('\\' == ch)
		{
			//���ꕶ��
			ch = get_char();
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
		reader.context[idx++] = ch;
	}
	reader.context[idx] = '\0';
}

static inline void parse_plus(void)
{
	reader.token = CALC_PLUS;
	reader.context[0] = '\0';
}

static inline void parse_minus(void)
{
  reader.token = CALC_MINUS;
  reader.context[0] = '\0';
}

static inline void parse_mul(void)
{
  reader.token = CALC_MUL;
  reader.context[0] = '\0';
}

static inline void parse_div(void) 
{
  reader.token = CALC_DIV;
  reader.context[0] = '\0';
}

static inline void parse_equ(void) 
{
  char ch = get_char();
  if ('=' == ch)
  {
    reader.token = OPE_EQ;
  } 
  else 
  {
    seek(-1);
    reader.token = CALC_EQUAL;
  }
  reader.context[0] = '\0';
}

static inline void parse_ge_ne_gt(void)
{
	char ch = get_char();
	if ('=' == ch)
	{
		reader.token = OPE_GE;
	}
	else if ('>' == ch)
	{
		reader.token = OPE_NE;
	}
	else
	{
		seek(-1);
		reader.token = OPE_GT;
	}
}

//�������
static inline void parse_le_lt(void)
{
	char ch = get_char();
	if ('=' == ch)
	{
		reader.token = OPE_LE;
	}
	else {
		seek(-1);
		reader.token = OPE_LT;
	}
}

//���x���ړ�
static inline void parse_label(void)
{
	int idx = 0;
	reader.token = CUR_LABEL;
	reader.context[idx++] = '@';

	while (true)
	{
		char ch = get_char();
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || '_' == ch)
		{
			reader.context[idx++] = ch;
		}
		else
		{
			seek(-1);
			reader.context[idx] = '\0';
			break;
		}
	}
}

//���s�܂ňړ�
bool reader_seek_to_newline(void)
{
	while (true)
	{
		if (CUR_NEWLINE == reader.token) return true;
		if (!reader_next()) return false;
	}
}

//������
void reader_init(char* text)
{
	reader.text = reader.rp = text;
	reader.no = 0;
	reader.err = ERR_NONE;
}

//���̃g�[�N�����擾����
bool reader_next(void)
{
	skip_space();
	char ch = toupper(get_char());
	if ('(' == ch) parse_gt();
	else if (')' == ch) parse_ge();
	else if ('\r' == ch || '\n' == ch) parse_newline();
	else if ('0' <= ch && ch <= '9') parse_num(ch);
	else if ('A' <= ch && ch <= 'Z' || '_' == ch) parse_cmd_or_val(ch);
	else if ('"' == ch) parse_str();
	else if ('+' == ch) parse_plus();
	else if ('-' == ch) parse_minus();
	else if ('*' == ch) parse_mul();
	else if ('/' == ch) parse_div();
	else if ('=' == ch) parse_equ();
	else if ('<' == ch) parse_ge_ne_gt();
	else if ('>' == ch) parse_le_lt();
	else if ('@' == ch) parse_label();
	else return false;
	return true;
}

//�����ɕϊ�
static inline int convert_value(script_token token, char* value)
{
	int result = 0;
	if (VAR_NUM == token) result = strtol(value, NULL, 0);
	else if (SYN_NUM == token) result = strtol(dic_get(value), NULL, 0);
	return result;
}

//�t�|�[�����h�L�@�f�R�[�h
static void rpn_decode(rpn_info* self)
{
	script_token token = reader.token;

	switch (self->state)
	{
	case 0:
		self->left = convert_value(token, reader.context);
		self->state = 1;
		break;

	case 1:
		self->old_op = token;
		self->state = 2;
		break;

	case 2:
		if (CALC_MUL == self->old_op)
		{
			self->left = self->left * convert_value(token, reader.context);
			self->state = 1;
		}
		else if (CALC_DIV == self->old_op)
		{
			self->left = self->left / convert_value(token, reader.context);
			self->state = 1;
		}
		else
		{
			self->right = convert_value(token, reader.context);
			self->state = 3;
		}
		break;

	case 3:
		self->cur_op = token;
		self->state = 4;
		break;

	case 4:
		if (CALC_MUL == self->cur_op)
		{
			self->right = self->right * convert_value(token, reader.context);
		}
		else if (CALC_DIV == self->old_op)
		{
			self->right = self->right / convert_value(token, reader.context);
		}
		else if (CALC_PLUS == self->old_op)
		{
			self->left = self->left + self->right;
			self->right = convert_value(token, reader.context);
			self->old_op = self->cur_op;
		}
		else if (CALC_MINUS == self->old_op)
		{
			self->left = self->left - self->right;
			self->right = convert_value(token, reader.context);
			self->old_op = self->cur_op;
		}
		self->state = 3;
		break;
	}
}

//�v�Z���ʂ�Ԃ�
static int rpn_result(rpn_info* self)
{
	if (3 == self->state)
	{
		if (CALC_MUL == self->cur_op) return self->left * self->right;
		else if (CALC_DIV == self->old_op) return self->left / self->right;
		else if (CALC_PLUS == self->old_op) return self->left + self->right;
		else if (CALC_MINUS == self->old_op) return self->left - self->right;
	}
	return self->left;
}


//�����v�Z����
bool rpn_num(int* result)
{
	rpn_info rpn_contexts[4];
	rpn_info* pt_rpn = rpn_contexts;
	strcpy(tmp_key, reader.context);
	pt_rpn->state = 0;

	if (!reader_next()) return false;
	if (reader.token != CALC_EQUAL) return false;

	while (true)
	{
		reader_next();
		switch (reader.token)
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
			rpn_decode(pt_rpn);
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
			itoa(rpn_result(pt_rpn--), reader.context, 10);
			reader.token = VAR_NUM;
			rpn_decode(pt_rpn);
			break;
		}
	}
}

//������̌���
bool rpn_str(void)
{
	strcpy(tmp_key, reader.context);
	tmp_value[0] = '\0';

	reader_next();
	if (reader.token != CALC_EQUAL) return false;

	while (true)
	{
		reader_next();
		switch (reader.token)
		{
		case VAR_STR:
		case VAR_NUM:
			strcat(tmp_value, reader.context);
			break;

		case SYN_STR:
		case SYN_NUM:
			strcat(tmp_value, dic_get(reader.context));
			break;

		case CALC_PLUS:
			break;

		case CUR_NEWLINE:
			dic_set(tmp_key, tmp_value);
			return true;

		default:
			return false;
		}
	}
}

//������
bool rpn_judge(void)
{
	//���ӎ�荞��
	if (!reader_next())  return false;
	switch (reader.token)
	{
	case VAR_NUM:
	case VAR_STR:
		strcpy(tmp_eval_left, reader.context);
		break;

	case SYN_NUM:
	case SYN_STR:
		strcpy(tmp_eval_left, dic_get(reader.context));
		break;

	default:
		return false;
	}

	//������荞��
	if (!reader_next())  return false;
	tmp_eval_op = reader.token;

	//�E�ӎ�荞��
	if (!reader_next())  return false;
	switch (reader.token)
	{
	case VAR_NUM:
	case VAR_STR:
		strcpy(tmp_eval_right, reader.context);
		break;
	case SYN_NUM:
	case SYN_STR:
		strcpy(tmp_eval_right, dic_get(reader.context));
		break;
	}

	//�]��
	if (OPE_EQ == tmp_eval_op) return 0 == strcmp(tmp_eval_left, tmp_eval_right);
	if (OPE_NE == tmp_eval_op) return 0 != strcmp(tmp_eval_left, tmp_eval_right);
	int valL = strtol(tmp_eval_left, NULL, 0);
	int valR = strtol(tmp_eval_right, NULL, 0);

	switch (tmp_eval_op)
	{
	case OPE_GT: return (valL < valR);
	case OPE_GE: return (valL <= valR);
	case OPE_LT: return (valL > valR);
	case OPE_LE: return (valL >= valR);
	default: return false;
	}
}

//�擪�̍s�ԍ���T��
static inline bool decode_seek_no()
{
	while (true)
	{
		if (!reader_next()) return false;
		if (CUR_NEWLINE == reader.token) continue;
		if (VAR_NUM != reader.token) return false;
		reader.no = strtol(reader.context, NULL, 0);
		return true;
	}
}

//�X�N���v�g��́i��ʁj
bool decoder_execute(void)
{
	if (!decode_seek_no())	return false;
	if (!reader_next())		return false;
	switch (reader.token)
	{
	case SYN_STR:			return rpn_str();
	case SYN_NUM:			return cmd_calc();
	case SYN_CMD:			return cmd_exe(reader.cmd);
	case CUR_LABEL:			return reader_seek_to_newline();
	case CUR_NEWLINE:		return true;
	}
	return false;
}
