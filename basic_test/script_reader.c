#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "dictionary.h"
#include "bas_packet.h"
#include "script_reader.h"

static bool rpn_num(int* result);

static script_reader reader;
static char tmp_key[VARIABLE_NAME_LENGTH];
static char tmp_value[VARIABLE_NAME_LENGTH];
static char tmp_eval_left[VARIABLE_NAME_LENGTH];
static char tmp_eval_right[VARIABLE_NAME_LENGTH];
static script_token tmp_eval_op;


//読込みポインタ移動（相対値）
static inline void seek(int offset)
{
	if ('\0' != *reader.rp) reader.rp += offset;
}

//1文字読込み
static inline char get_char(void) 
{
  char ch = *reader.rp;
  if ('\0' == ch)
  {
	  return '\0';
  }
  reader.rp++;
  return ch;
}

//空白行スキップ
static inline void skip_space(void)
{
	while ((' ' == *reader.rp) || ('\t' == *reader.rp)) reader.rp++;
}

//（
static inline void parse_gt(void)
{
	reader.token = PRI_GT;
	reader.context[0] = '\0';
}

//）
static inline void parse_ge(void)
{
	reader.token = PRI_GE;
	reader.context[0] = '\0';
}

//改行[\r, \,\n, \r\n]処理
static inline void parse_newline(void)
{
	if ('\n' == reader.rp[1]) reader.rp++;
	reader.token = CUR_NEWLINE;
	reader.context[0] = '\0';
}

//数字取得(0x00, 012345)
static inline void parse_num(char ch)
{
	int idx = 0;
	reader.token = VAR_NUM;

	if ('X' == toupper(*reader.rp))
	{
		//16進数
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
		//10進数
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

//文字列から変数またはコマンドかを調べる
static inline void parse_val(char ch)
{
	int idx = 0;
	reader.context[idx++] = ch;

	while (true) 
	{
		ch = get_char();
		if ('A' <= ch && ch <= 'Z' || '_' == ch)
		{
			//文字列
			reader.context[idx++] = ch;
		}
		else if ('%' == ch) 
		{
			//数字：終端
			reader.context[idx++] = '%';
			if ('(' == *reader.rp)
			{
				int idx;
				if (!rpn_num(&idx))
				{
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
			//文字列：終端
			reader.context[idx++] = '$';
			reader.token = SYN_STR;
			reader.context[idx] = '\0';
			break;
		}
		else
		{
			//不明な文字：終端
			seek(-1);
			reader.context[idx++] = '%';
			reader.context[idx] = '\0';
			reader.token = SYN_NUM;
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
			//特殊文字
			ch = get_char();
			if ('\t' == ch) ch = '\t';
			else if ('\r' == ch) ch = '\r';
			else if ('\n' == ch) ch = '\n';
			else if ('\"' == ch) ch = '\"';
		}
		else if ('\"' == ch)
		{
			//終端
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
	*reader.rp--;
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

//符号解析
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

//ラベル移動
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


//次のトークンを取得する
bool reader_next(void)
{
	skip_space();
	char ch = toupper(get_char());
	if ('(' == ch) parse_gt();
	else if (')' == ch) parse_ge();
	else if ('\r' == ch || '\n' == ch) parse_newline();
	else if ('0' <= ch && ch <= '9') parse_num(ch);
	else if ('A' <= ch && ch <= 'Z' || '_' == ch) parse_val(ch);
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

//数字に変換
static inline int convert_value(script_token token, char* value)
{
	int result = 0;
	if (VAR_NUM == token) result = strtol(value, NULL, 0);
	else if (SYN_NUM == token) result = strtol(dic_get(value), NULL, 0);
	return result;
}

//逆ポーランド記法デコード
static void rpn_decode(rpn_info* self)
{
	script_token token = reader.token;

	switch (self->state)
	{
	case 0:
		if ((CALC_PLUS == token) || (CALC_MINUS == token))
		{
			self->left = 0;
			self->old_op = token;
			self->state = 2;
		}
		else
		{
			self->left = convert_value(token, reader.context);
			self->state = 1;
		}
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

//計算結果を返す
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

//数式を計算する
static bool rpn_calc(void)
{
	if (*reader.rp == '=')
	{
		int result;
		if (!rpn_num(&result)) return false;
		itoa(result, tmp_value, 10);
		*reader.result = dic_set(tmp_key, tmp_value);
	}
	else
	{
		*reader.result = dic_get(reader.context);
	}
	return true;
}

//式を計算する
static bool rpn_num(int* result)
{
	rpn_info rpn_contexts[4];
	rpn_info* pt_rpn = rpn_contexts;
	strcpy(tmp_key, reader.context);
	pt_rpn->state = 0;

	if (!reader_next()) return false;
	if (reader.token != CALC_EQUAL) return false;

	while (true)
	{
		if (!reader_next())
		{
			*result = rpn_result(pt_rpn);
			return true;
		}
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
			//ネスト最大３まで
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

//文字列の結合
static bool rpn_str(void)
{
	if (*reader.rp == '=')
	{
		strcpy(tmp_key, reader.context);
		tmp_value[0] = '\0';

		if (!reader_next()) return false;
		if (reader.token != CALC_EQUAL) return false;

		while (true)
		{
			if (!reader_next())
			{
				*reader.result = dic_set(tmp_key, tmp_value);
				return true;
			}
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
				*reader.result = dic_set(tmp_key, tmp_value);
				return true;

			default:
				return false;
			}
		}
	}
	else
	{
		*reader.result = dic_get(reader.context);
		return true;
	}
}

//条件式
bool rpn_judge(BAS_PACKET_BODY* body)
{
	reader.rp = body->opcode;

	//左辺取り込み
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

	//符号取り込み
	if (!reader_next())  return false;
	tmp_eval_op = reader.token;

	//右辺取り込み
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

	//評価
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

//スクリプト解析（上位）
bool rpn_execute(BAS_PACKET_BODY* body)
{
	reader.rp = body->opcode;
	reader.result = &body->response;

	if (reader_next())
	{
		script_token token = reader.token;
		if (token == SYN_STR) return rpn_str();
		if (token == SYN_NUM) return rpn_calc();
	}
	return false;
}

//変数の中身取得
char* rpn_get_value(char* key)
{
	reader.rp = key;
	if (reader_next())
	{
		script_token token = reader.token;
		if (token == SYN_STR) return dic_get(reader.context);
		if (token == SYN_NUM) return dic_get(reader.context);
	}
	return "\0";
}