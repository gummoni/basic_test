#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "csv_basic.h"
#include "bas_script.h"
#include "bas_packet.h"
#include "script_reader.h"

//GOSUB系のヒープ領域
#define MAX_GOSUB_HEAP_SIZE		4
static unsigned char heap_memory[MAX_GOSUB_HEAP_SIZE];
static unsigned char heap_idx = 0;


//=============================================================================
//script decoder
//=============================================================================

//IF文
static void bas_script_if(BAS_PACKET* packet)
{
}

//GOTO文
static void bas_script_goto(BAS_PACKET* packet)
{
	state.run_no = strtol(packet->prm1, NULL, 0);
}

//GOSUB文
static void bas_script_gosub(BAS_PACKET* packet)
{
	if (heap_idx < MAX_GOSUB_HEAP_SIZE)
	{
		heap_memory[heap_idx++] = state.run_no;
		state.run_no = strtol(packet->prm1, NULL, 0);
	}
	else
	{
		state.err_code = err_out_of_return;
	}
}

//RETURN文
static void bas_script_return(BAS_PACKET* packet)
{
	if (0 < heap_idx)
	{
		state.run_no = heap_memory[--heap_idx];
	}
	else
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
	bas_send_message(program_areas[0], packet->opcode, NOTIFY, packet->prm1);
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
	//エラーでなければスクリプト実行
	if ((0 != state.err_code) || (0 == state.run_no)) return;
	//プログラム番地のコードをCSV分解して実行
	BAS_PACKET packet;
	packet.reciever = packet.sender = program_areas[0];
	packet.response = NULL;
	strcpy(recv_buf, program_areas[state.run_no++]);
	if (!bas_parse_parameter(&packet, recv_buf)) return;
	bas_script_execute(&packet);
}
