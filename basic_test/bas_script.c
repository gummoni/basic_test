#include "config.h"
#include "csv_basic.h"
#include "bas_script.h"
#include "bas_packet.h"
#include "dictionary.h"
#include "rpn.h"
#include "gosub_heap.h"

//受信バッファ
static char recv_buf[PROGRAM_LINE_COUNT];


//=============================================================================
//script decoder
//=============================================================================

//ラベル照合
static bool bas_check_label(char* label, char* msg)
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
static int bas_search_label(char* label, bool* is_label)
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
			if (bas_check_label(label, &program_areas[idx]))
			{
				*is_label = true;
				return idx;
			}
		}
	}
	//該当なし
	state.err_code = err_jump;
	return 0;
}

//IF文
static void bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(&packet->prm1))
	{
		//TRUE
		state.run_no = bas_search_label(packet->prm2, &is_label);
		if (is_label) state.run_no++;
	}
	else
	{
		//FALSE
		if (NULL != packet->prm3)
		{
			state.run_no = bas_search_label(packet->prm3, &is_label);
			if (is_label) state.run_no++;
		}
	}
}

//GOTO文
static void bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = bas_search_label(packet->prm1, &is_label);

	if (is_label)
	{
		//引数代入
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		parse.response = NULL;
		strcpy(recv_buf, program_areas[state.run_no++]);
		if (bas_parse_parameter(&parse, recv_buf, ' '))
		{
			if (NULL == parse.prm1) return;
			if (NULL == packet->prm2) return;
			dic_set(parse.prm1, packet->prm2);

			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);
		}
	}
}

//GOSUB文
static void bas_script_gosub(BAS_PACKET* packet)
{
	if (heap_enqueue(state.run_no))
	{
		bas_script_goto(packet);
	}
	else
	{
		state.err_code = err_out_of_return;
	}
}

//RETURN文
static void bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		state.err_code = err_invalid_return;
	}
}

//END文
static void bas_script_end(BAS_PACKET* packet)
{
	state.run_no = 0;
}

//メッセージ通知
static void bas_script_notify(BAS_PACKET* packet)
{
	char resp[32];
	char* to = packet->prm1;
	char* key = packet->prm2;
	char* val = rpn_get_value(key);
	sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	bas_send_message(SELF_NAME, to, NOTIFY, resp);
}

#define SCRIPT_COMMAND_TABLE_LENGTH	6
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	{ "IF"		, bas_script_if			},
	{ "GOTO"	, bas_script_goto		},
	{ "GOSUB"	, bas_script_gosub		},
	{ "RETURN"	, bas_script_return		},
	{ "END"		, bas_script_end		},
	{ "NOTIFY"	, bas_script_notify		},
};

//コマンド実行
static void bas_script_execute(BAS_PACKET* packet)
{
	//予約命令実行
	int i = 0;
	for (i = 0; i < SCRIPT_COMMAND_TABLE_LENGTH; i++)
	{
		BAS_SCRIPT_TABLE command = script_command_table[i];
		if (0 == strcmp(packet->opcode, command.name))
		{
			command.execute(packet);
			return;
		}
	}

	//RPN実行
	rpn_execute((BAS_PACKET_BODY*)&packet->opcode);
}

//スクリプト実行処理
void bas_script_job(void)
{
	BAS_PACKET packet;

	while (true)
	{
		if ((0 != state.err_code) || (0 == state.run_no)) return;
		//プログラム番地のコードをCSV分解して実行
		packet.reciever = packet.sender = SELF_NAME;
		packet.response = NULL;
		strcpy(recv_buf, program_areas[state.run_no++]);
		if (!bas_parse_parameter(&packet, recv_buf, ' ')) return;
		if ('*' == packet.opcode[0]) continue;
		bas_script_execute(&packet);
		return;
	}
}
