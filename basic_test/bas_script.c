//=============================================================================
//スクリプト解析
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"

//受信バッファ
static char recv_buf[PROGRAM_LINE_COUNT];


void bas_script_init(void)
{
	dic_clear();
	state.run_no = state.err_code = 0;
}

//IF文
static void bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(&packet->prm1))
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
}

//GOTO文
static void bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm1, &is_label);

	if (is_label)
	{
		//引数代入
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		parse.response = NULL;
		strcpy(recv_buf, program_areas[state.run_no++]);
		if (parse_parameter(&parse, recv_buf, ' '))
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
	send_message(SELF_NAME, to, NOTIFY, resp);
}

//速度設定
static void bas_script_speed(BAS_PACKET* packet)
{
	//TODO
}

//絶対値移動
static void bas_script_abs(BAS_PACKET* packet)
{
	//TODO
}

//相対値移動
static void bas_script_inc(BAS_PACKET* packet)
{
	//TODO
}

//原点復帰
static void bas_script_org(BAS_PACKET* packet)
{
	//TODO
}

//モータ停止
static void bas_script_stop(BAS_PACKET* packet)
{
	//TODO
}

//エラーチェック処理
static void bas_script_check(BAS_PACKET* packet)
{
	//TODO
}

//待ち処理
static void bas_script_loop(BAS_PACKET* packet)
{
	//TODO
}

//遅延処理
static void bas_script_delay(BAS_PACKET* packet)
{
	//TODO
}

#define SCRIPT_COMMAND_TABLE_LENGTH	14
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	//---基本命令---
	{ "IF"		, bas_script_if			},	//0
	{ "GOTO"	, bas_script_goto		},	//1
	{ "GOSUB"	, bas_script_gosub		},	//2
	{ "RETURN"	, bas_script_return		},	//3
	{ "END"		, bas_script_end		},	//4
	{ "NOTIFY"	, bas_script_notify		},	//5
	//----拡張命令（PB210）---
	{ "SPEED"	, bas_script_speed		},	//6
	{ "ABS"		, bas_script_abs		},	//7
	{ "INC"		, bas_script_inc		},	//8
	{ "ORG"		, bas_script_org		},	//9
	{ "STOP"	, bas_script_stop		},	//10
	{ "CHECK"	, bas_script_check		},	//11
	{ "LOOP"	, bas_script_loop		},	//12
	{ "DELAY"	, bas_script_delay		},	//13
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
		if (!parse_parameter(&packet, recv_buf, ' ')) return;
		if ('*' == packet.opcode[0]) continue;
		bas_script_execute(&packet);
		return;
	}
}
