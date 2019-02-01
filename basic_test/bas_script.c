//=============================================================================
//スクリプト解析
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"


//受信バッファ
static BAS_PACKET script_packet;

//送信バッファ
static char send_buf[PROGRAM_LINE_COUNT];


//ラベル照合
static bool label_compare(char* label, char* msg)
{
	while (true)
	{
		char a = *(label++);
		char b = *(msg++);
		if (a == '\0') return true;
		if (a != b) return false;
	}
}

//ラベル位置取得（行番号も含む）
static int label_search(char* label, bool* is_label)
{
	int idx;
	char ch = label[0];

	if ('0' <= ch && ch <= '9')
	{
		//数字
		*is_label = false;
		return strtol(label, NULL, 0);
	}
	if ('*' == ch)
	{
		//ジャンプ先
		for (idx = 0; idx < PROGRAM_LINE_MAX; idx++)
		{
			if (label_compare(label, &program_areas[idx][0]))
			{
				*is_label = true;
				return idx;
			}
		}
	}
	//該当なし
	state.err_no = err_jump;
	return 0;
}

//スクリプト構文解析
static bool parse_script(BAS_PACKET* packet, char* msg)
{
	packet->prm1 = bas_parser.parse_buff;
	packet->prm2 = packet->prm3 = packet->prm4 = packet->prm5 = packet->prm6 = NULL;
	char** prms = &packet->prm2;
	char* dst = bas_parser.parse_buff;

	bool qout = false;
	for (;; msg++)
	{
		char ch = *msg;
		if ('"' == ch)
		{	//文字列範囲(区切り文字で区切らないようにする)
			qout ^= true;
		}
		else if ((' ' == ch) && !qout)
		{	//区切り記号検知
			*(dst++) = '\0';
			*(prms++) = dst;
		}
		else if (('\n' == ch) || ('\0' == ch))
		{	//終端
			*dst = '\0';
			return true;
		}
		else
		{	//上記条件出ない場合はデータコピー
			*(dst++) = ch;
		}
	}
}

//=============================================================================
//返信電文作成
//=============================================================================
char* make_message(char* from, char* to, char cmd, char* message)
{
	//電文例：SENDER,RECIEVER,1:PARAMETER...\n	
	char* msg = send_buf;
	while ('\0' != *from) *(msg++) = *(from++);
	*(msg++) = ',';
	while ('\0' != *to) *(msg++) = *(to++);
	*(msg++) = ',';
	*(msg++) = cmd;
	*(msg++) = ':';
	while ('\0' != *message) *(msg++) = *(message++);
	*(msg++) = '\n';
	*(msg++) = '\0';

	return send_buf;
}

//=============================================================================
//基本命令セット
//=============================================================================

//IF文
static bool bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(packet))
	{
		//TRUE
		state.run_no = label_search(packet->prm2, &is_label);
		if (is_label) state.run_no++;
	}
	else
	{
		//FALSE
		if (NULL != packet->prm3)
		{
			state.run_no = label_search(packet->prm3, &is_label);
			if (is_label) state.run_no++;
		}
	}
	return true;
}

//GOTO文
static bool bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm1, &is_label);

	if (is_label)
	{
		//引数代入
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		strcpy(bas_parser.parse_buff, program_areas[state.run_no++]);
		if (parse_script(&parse, bas_parser.parse_buff))
		{
			if (NULL == parse.prm1) return false;
			if (NULL == packet->prm2) return false;
			dic_set(parse.prm1, packet->prm2);

			if (NULL == parse.prm2) return false;
			if (NULL == packet->prm3) return false;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm3) return false;
			if (NULL == packet->prm4) return false;
			dic_set(parse.prm3, packet->prm4);

			if (NULL == parse.prm4) return false;
			if (NULL == packet->prm5) return false;
			dic_set(parse.prm4, packet->prm5);
		}
	}
	return true;
}

//GOSUB文
static bool bas_script_gosub(BAS_PACKET* packet)
{
	if (heap_enqueue(state.run_no))
	{
		bas_script_goto(packet);
	}
	else
	{
		state.err_no = err_out_of_return;
	}
	return true;
}

//RETURN文
static bool bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		//戻り先がないのでEND扱い
		state.run_no = state.stp_no = 0;
	}
	return true;
}

//メッセージ通知
static bool bas_script_invoke(BAS_PACKET* packet)
{
	char resp[32];
	char* to = packet->prm1;
	char* key = packet->prm2;
	char* val = rpn_get_value(key);
	sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	char* response_message = make_message(SELF_NAME, to, INVOKE, resp);
	int length = strlen(response_message);
	packet->response(response_message, length);
	packet->broadcast(response_message, length);
	return true;
}

//遅延処理
static bool bas_script_delay(BAS_PACKET* packet)
{
	return true;														//TODO
}

//------------------------------拡張ここから------------------------------

//原点復帰
static bool bas_script_org(BAS_PACKET* packet)
{
	return true;														//TODO
}

//絶対値移動
static bool bas_script_abs(BAS_PACKET* packet)
{
	return true;														//TODO
}

//相対値移動
static bool bas_script_inc(BAS_PACKET* packet)
{
	return true;														//TODO
}

//モータ停止
static bool bas_script_stop(BAS_PACKET* packet)
{
	return true;														//TODO
}

//エラーチェック処理
static bool bas_script_check(BAS_PACKET* packet)
{
	return true;														//TODO
}

//エラー通知処理
static bool bas_script_err(BAS_PACKET* packet)
{
	return true;														//TODO
}

//------------------------------拡張ここまで------------------------------

#define SCRIPT_COMMAND_TABLE_LENGTH	12
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	//---基本命令---
	{ "IF"		, bas_script_if			},	//0
	{ "GOTO"	, bas_script_goto		},	//1
	{ "GOSUB"	, bas_script_gosub		},	//2
	{ "RETURN"	, bas_script_return		},	//3
	{ "INVOKE"	, bas_script_invoke		},	//4
	{ "DELAY"	, bas_script_delay		},	//5
	{ "CHK"		, bas_script_check		},	//6
	{ "ERR"		, bas_script_err		},	//7
	//----拡張命令（PB210）---
	{ "ORG"		, bas_script_org		},	//8
	{ "ABS"		, bas_script_abs		},	//9
	{ "INC"		, bas_script_inc		},	//10
	{ "STOP"	, bas_script_stop		},	//11
};

//コマンド実行
static bool bas_script_execute(BAS_PACKET* packet)
{
	//予約命令実行
	int i = 0;
	for (i = 0; i < SCRIPT_COMMAND_TABLE_LENGTH; i++)
	{
		BAS_SCRIPT_TABLE command = script_command_table[i];
		if (0 == strcmp(packet->prm1, command.name))
		{
			return command.execute(packet);
		}
	}

	//RPN実行
	return rpn_execute(packet);
}

//スクリプト実行処理
void bas_script_job(void)
{
	while (true)
	{
		if ((0 != state.err_no) || (10 > state.run_no)) return;			//プログラム実行中かどうか判別
		char* msg = program_areas[state.run_no++];						//プログラムコード取得
		if ('*' == msg[0]) continue;									//ラベルならスキップ
		if (!parse_script(&script_packet, msg)) return;					//プログラム番地のコードを解析
		if (!bas_script_execute(&script_packet)) return;				//実行
	}
}
