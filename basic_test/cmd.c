#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "cmd.h"
#include "script_reader.h"
#include "dictionary.h"
#include "stack.h"

static inline bool do_end(void)
{
	int len = strlen(reader.text);
	reader.rp = &reader.text[len];
	return false;
}

//行番号までジャンプ
static bool jump_line_no(char* line_no)
{
	//念のためローカル変数に格納
	strcpy(tmp_key, line_no);
	reader.rp = reader.text;

	while (true)
	{
		char* old_rp = reader.rp;

		if (!reader_next()) return false;
		if (VAR_NUM != reader.token) return false;

		//行番号チェック
		if (0 == strcmp(tmp_key, reader.context))
		{
			reader.rp = old_rp;
			return true;
		}

		//次の行へ
		if (!reader_seek_to_newline()) return false;
	}
}

//ラベルまでジャンプ
static bool jump_label(char* label)
{
	//念のためローカル変数に格納
	strcpy(tmp_key, label);
	reader.rp = reader.text;

	while (true)
	{
		char* old_rp = reader.rp;

		if (!reader_next()) return false;
		if (VAR_NUM != reader.token) return false;

		//ラベルチェック
		if (!reader_next()) return false;
		if (CUR_LABEL == reader.token)
		{
			if (0 == strcmp(tmp_key, reader.context))
			{
				reader.rp = old_rp;
				return true;
			}
		}

		//次の行へ
		if (!reader_seek_to_newline()) return false;
	}
}

static inline bool do_jump(script_token token, char* label)
{
	switch (token)
	{
	case VAR_NUM:     return jump_line_no(label);
	case SYN_NUM:     return jump_line_no(dic_get(label));
	case CUR_LABEL:   return jump_label(label);
	case CUR_NEWLINE: return true;
	default:          return false;
	}
}

static inline bool do_goto()
{
	if (!reader_next())  return false;
	return do_jump(reader.token, reader.context);
}

static inline bool do_gosub(void)
{
	if (!reader_next()) return false;
	script_token token = reader.token;
	strcpy(tmp_value, reader.context);

	if (!reader_seek_to_newline()) return false;
	stack_push(reader.rp);

	return do_jump(token, tmp_value);
}

static inline bool do_return(void)
{
	reader.rp = stack_pop();
	if (0 == reader.rp)
	{
		do_end();
		return false;
	}
	return true;
}

static inline bool do_if(void)
{
	//IF ABC$ == DEF$ THEN 10 ELSEIF ABC == "1" THEN @NNN

	while (true)
	{
		//eval
		bool ret = rpn_judge();

		//then
		if (!reader_next()) return false;
		if (CMD_THEN != reader.cmd) return false;

		//number
		if (ret) return do_goto();
		if (!reader_next()) return false;

		//else, elseif
		if (!reader_next()) return false;
		switch (reader.cmd)
		{
		case CMD_ELSE: return do_goto();
		case CMD_ELSEIF: continue;
		default: return false;
		}
	}
}

static inline bool do_for(void)
{
	//FOR I=0 TO 10 STEP 1 : PPPP : NEXT
	  /*
	  char tmp_key[16];
	  char tmp_max[16];
	  int tmp_step = 1;
	  int tmp_val;
	  char* loop_st;
	  char* loop_ed;
	  if (!reader_next())  return false;
	  if (SYN_NUM != reader.token) return false;
	  strcpy(tmp_key, reader.context);
	  if (!decode_num()) return false;

	  if (0 != strcmp("TO", reader.context)) return false;

	  if (!reader_next())  return false;
	  if (VAR_NUM != reader.token) return false;
	  //tmp_max = strtol(reader.context, NULL, 0);
	  strcpy(tmp_max, reader.context);

	  if (!reader_next())  return false;
	  if (0 == strcmp("STEP", reader.context))
	  {
		  if (!reader_next())  return false;
		  if (VAR_NUM != reader.token) return false;
		  strcpy(tmp_key, reader.context);
	  }

	  if (!reader_next())  return false;
	  loop_st = reader.rp;
	  if (!reader_next())  return false;
	  if (CUR_NEWLINE != reader.token) return false;

	  while (true)
	  {
		  if (CUR_NEWLINE == reader.token) return true;
		  if (VAR_NUM != reader.token) return false;

		  if (0 == reader_next()) return false;
		  switch (reader.token) {
		  case SYN_CMD:
			  if (0 == strcmp("NEXT", reader.context))
			  {
				  tmp_val = dic_get(tmp_key);
			  }
			  else
			  {
				  if (!decode_cmd()) return false;
			  }

		  case SYN_STR: return decode_str();
		  case SYN_NUM: return decode_num();
		  case CUR_LABEL: return reader_seek_to_newline();
		  case CUR_NEWLINE: return true;
		  }
	  }
	  */

	return true;
}

static inline bool do_request(void)
{
	return true;
}

static inline bool do_notify(void)
{
	return true;
}

//コマンド解析
bool cmd_exe(CMD_IDX index)
{
	switch (index)
	{
	case CMD_NONE:		return rpn_str();
	case CMD_IF:		return do_if();
	case CMD_FOR:		return do_for();
	case CMD_GOTO:		return do_goto();
	case CMD_GOSUB:		return do_gosub();
	case CMD_RETURN:	return do_return();
	case CMD_END:		return do_end();
	default:			return false;
	}
}

//計算して変数に格納
bool cmd_calc(void)
{
	int result;
	strcpy(tmp_key, reader.context);
	if (!rpn_num(&result)) return false;
	itoa(result, tmp_value, 10);
	dic_set(tmp_key, tmp_value);
	return true;
}
