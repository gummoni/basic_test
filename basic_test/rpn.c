#include "config.h"
#include "dic.h"
#include "rpn.h"

static rpn_instance self;
static bool rpn_num(int* result);


//読込みポインタ移動（相対値）
static inline void seek(int offset)
{
	if ('\0' != *self.rp) self.rp += offset;
}

//1文字読込み
static inline char get_char(void) 
{
  char ch = *self.rp;
  if ('\0' == ch)
  {
	  return '\0';
  }
  self.rp++;
  return ch;
}

//空白行スキップ
static inline void skip_space(void)
{
	while ((' ' == *self.rp) || ('\t' == *self.rp)) self.rp++;
}

//（
static inline void parse_gt(void)
{
	self.token = PRI_GT;
	self.context[0] = '\0';
}

//）
static inline void parse_ge(void)
{
	self.token = PRI_GE;
	self.context[0] = '\0';
}

//改行[\r, \,\n, \r\n]処理
static inline void parse_newline(void)
{
	if ('\n' == self.rp[1]) self.rp++;
	self.token = CUR_NEWLINE;
	self.context[0] = '\0';
}

//数字取得(0x00, 012345)
static inline void parse_num(char ch)
{
	int idx = 0;
	self.token = VAR_NUM;

	if ('X' == toupper(*self.rp))
	{
		//16進数
		self.context[idx++] = '0';
		self.context[idx++] = 'x';
		while (true)
		{
			self.rp++;
			ch = toupper(*self.rp);
			if (('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F'))
			{
				self.context[idx++] = ch;
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
		self.context[idx++] = ch;
		while (true)
		{
			ch = *self.rp;
			if ('0' <= ch && ch <= '9')
			{
				self.context[idx++] = ch;
				*self.rp++;
			}
			else
			{
				break;
			}
		}
	}
	self.context[idx] = '\0';
}

//文字列から変数またはコマンドかを調べる
static inline void parse_val(char ch)
{
	int idx = 0;
	self.context[idx++] = ch;

	while (true) 
	{
		ch = get_char();
		if ('A' <= ch && ch <= 'Z' || '_' == ch)
		{
			//文字列
			self.context[idx++] = ch;
		}
		else if ('%' == ch) 
		{
			//数字：終端
			if ('(' == *self.rp)
			{
				int idx;
				if (!rpn_num(&idx))
				{
					return;
				}
				self.context[idx++] = '(';
				itoa(idx, self.tmp_value, 10);

				char* pval = self.tmp_value;
				int len = strlen(pval);
				while (0 < len--)
				{
					self.context[idx++] = *(pval++);
				}
				self.context[idx++] = ')';
			}
			self.token = SYN_NUM;
			self.context[idx] = '\0';
			break;
		}
		else if ('$' == ch) 
		{
			//文字列：終端
			self.token = SYN_STR;
			self.context[idx] = '\0';
			break;
		}
		else
		{
			//不明な文字：終端
			seek(-1);
			self.context[idx] = '\0';
			self.token = SYN_NUM;
			break;
		}
	}
}

static inline void parse_str(void)
{
	int idx = 0;

	self.token = VAR_STR;
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
		self.context[idx++] = ch;
	}
	self.context[idx] = '\0';
}

static inline void parse_plus(void)
{
	self.token = CALC_PLUS;
	self.context[0] = '\0';
}

static inline void parse_minus(void)
{
	self.token = CALC_MINUS;
	self.context[0] = '\0';
}

static inline void parse_mul(void)
{
	self.token = CALC_MUL;
	self.context[0] = '\0';
}

static inline void parse_div(void) 
{
	self.token = CALC_DIV;
	self.context[0] = '\0';
}

static inline void parse_equ(void) 
{
  char ch = get_char();
  if ('=' == ch)
  {
	  self.token = OPE_EQ;
  } 
  else 
  {
	*self.rp--;
	self.token = CALC_EQUAL;
  }
  self.context[0] = '\0';
}

static inline void parse_ge_ne_gt(void)
{
	char ch = get_char();
	if ('=' == ch)
	{
		self.token = OPE_GE;
	}
	else if ('>' == ch)
	{
		self.token = OPE_NE;
	}
	else
	{
		seek(-1);
		self.token = OPE_GT;
	}
}

//符号解析
static inline void parse_le_lt(void)
{
	char ch = get_char();
	if ('=' == ch)
	{
		self.token = OPE_LE;
	}
	else {
		seek(-1);
		self.token = OPE_LT;
	}
}

//次のトークンを取得する
static bool reader_next(void)
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
	else return false;
	return true;
}

//数字に変換
static inline int convert_value(rpn_token token, char* value)
{
	int result = 0;
	if (VAR_NUM == token) result = strtol(value, NULL, 0);
	else if (SYN_NUM == token) result = strtol(dic_get(value), NULL, 0);
	return result;
}

//逆ポーランド記法デコード
static void rpn_decode(rpn_info* info)
{
	rpn_token token = self.token;

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
			info->left = convert_value(token, self.context);
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
			info->left = info->left * convert_value(token, self.context);
			info->state = 1;
		}
		else if (CALC_DIV == info->old_op)
		{
			info->left = info->left / convert_value(token, self.context);
			info->state = 1;
		}
		else
		{
			info->right = convert_value(token, self.context);
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
			info->right = info->right * convert_value(token, self.context);
		}
		else if (CALC_DIV == info->old_op)
		{
			info->right = info->right / convert_value(token, self.context);
		}
		else if (CALC_PLUS == info->old_op)
		{
			info->left = info->left + info->right;
			info->right = convert_value(token, self.context);
			info->old_op = info->cur_op;
		}
		else if (CALC_MINUS == info->old_op)
		{
			info->left = info->left - info->right;
			info->right = convert_value(token, self.context);
			info->old_op = info->cur_op;
		}
		info->state = 3;
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
	if (*self.rp == '=')
	{
		int result;
		if (!rpn_num(&result)) return false;
		itoa(result, self.tmp_value, 10);
		*self.result = dic_set(self.tmp_key, self.tmp_value);
	}
	else
	{
		*self.result = dic_get(self.context);
	}
	return true;
}

//式を計算する
static bool rpn_num(int* result)
{
	rpn_info rpn_contexts[4];
	rpn_info* pt_rpn = rpn_contexts;
	strcpy(self.tmp_key, self.context);
	pt_rpn->state = 0;

	if (!reader_next()) return false;
	if (self.token != CALC_EQUAL) return false;

	while (true)
	{
		if (!reader_next())
		{
			*result = rpn_result(pt_rpn);
			return true;
		}
		switch (self.token)
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
			itoa(rpn_result(pt_rpn--), self.context, 10);
			self.token = VAR_NUM;
			rpn_decode(pt_rpn);
			break;
		}
	}
}

//文字列の結合
static bool rpn_str(void)
{
	if (*self.rp == '=')
	{
		strcpy(self.tmp_key, self.context);
		self.tmp_value[0] = '\0';

		if (!reader_next()) return false;
		if (self.token != CALC_EQUAL) return false;

		while (true)
		{
			if (!reader_next())
			{
				*self.result = dic_set(self.tmp_key, self.tmp_value);
				return true;
			}
			switch (self.token)
			{
			case VAR_STR:
			case VAR_NUM:
				strcat(self.tmp_value, self.context);
				break;

			case SYN_STR:
			case SYN_NUM:
				strcat(self.tmp_value, dic_get(self.context));
				break;

			case CALC_PLUS:
				break;

			case CUR_NEWLINE:
				*self.result = dic_set(self.tmp_key, self.tmp_value);
				return true;

			default:
				return false;
			}
		}
	}
	else
	{
		*self.result = dic_get(self.context);
		return true;
	}
}

//条件式
bool rpn_judge(BAS_PACKET_BODY* body)
{
	self.rp = body->opcode;

	//左辺取り込み
	if (!reader_next())  return false;
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

	//符号取り込み
	if (!reader_next())  return false;
	self.tmp_eval_op = self.token;

	//右辺取り込み
	if (!reader_next())  return false;
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

	//評価
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

//スクリプト解析（上位）
bool rpn_execute(BAS_PACKET_BODY* body)
{
	self.rp = body->opcode;
	self.result = &body->response;

	if (reader_next())
	{
		rpn_token token = self.token;
		if (token == SYN_STR) return rpn_str();
		if (token == SYN_NUM) return rpn_calc();
	}
	return false;
}

//変数の中身取得
char* rpn_get_value(char* key)
{
	self.rp = key;
	if (reader_next())
	{
		rpn_token token = self.token;
		if (token == SYN_STR) return dic_get(self.context);
		if (token == SYN_NUM) return dic_get(self.context);
	}
	return "\0";
}
