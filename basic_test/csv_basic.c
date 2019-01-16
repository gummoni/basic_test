#include <stdbool.h>
#include "csv_basic.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

//24文字x100行のプログラムエリア
#define PROGRAM_LINE_COUNT	24
#define PROGRAM_LINE_MAX	100
static char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT] =
{
	//リッスン
	"AXIS_Z1",			// 0: UNIQUE_NAME（自分宛、返事を返す）
	"AXIS_Z",			// 1: LISTEN1(NOTIFY)
	"",				    // 2: LISTEN2(NOTIFY)
	"",				    // 3: LISTEN3(NOTIFY)
	"",					// 4: LISTEN4(NOTIFY)
	"",					// 5: LISTEN5(NOTIFY)
	"",					// 6: LISTEN6(NOTIFY)
	"",					// 7: LISTEN7(NOTIFY)
	"",					// 8: LISTEN8(NOTIFY)
	"",					// 9: LISTEN9(NOTIFY)
	//プログラム領域
	"",					//10: 
	"",					//11: 
	"",					//12: 
	"",					//13: 
	"",					//14: 
	"",					//15: 
	"",					//16: 
	"",					//17: 
	"",					//18: 
	"",					//19: 
	"",					//20: 
	"",					//21: 
	"REPORT,$STATUS",   //22: 
	"QUIT",				//23: 
	"*K1",				//24: 
	"IF,I=1,10,*K1",	//25: 
	"",					//26: 
	"",					//27: 
	"",					//28: 
	"",					//29: 
	"",					//30: 
	"",					//31: 
	"",					//32: 
	"",					//33: 
	"",					//34: 
	"",					//35: 
	"",					//36: 
	"",					//37: 
	"",					//38: 
	"",					//39: 
	"",					//40: 
	"",					//41: 
	"",					//42: 
	"",					//43: 
	"",					//44: 
};

//BAS翻訳機の状態
static BASIC_STATE state;
//送信バッファ
static char comm_buf[128];
//コマンド解析用バッファ
static char decode_message[PROGRAM_LINE_COUNT];
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

//返信電文作成
static void bas_send_message(char* from, char* to, char cmd, char* message)
{
	//電文例：SENDER,RECIEVER,1:PARAMETER...\n	
	char* msg = comm_buf;
	while ('\0' != *from) *(msg++) = *(from++);
	*(msg++) = ',';
	while ('\0' != *to) *(msg++) = *(to++);
	*(msg++) = ',';
	*(msg++) = cmd;
	*(msg++) = ':';
	while ('\0' != *message) *(msg++) = *(message++);
	*(msg++) = '\n';
	*(msg++) = '\0';
	printf(comm_buf);
}

//メッセージ通知
static void bas_script_notify(BAS_PACKET* packet)
{
	bas_send_message(program_areas[0], packet->prm1, NOTIFY, packet->prm2);
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

static void bas_do_script(BAS_PACKET* packet)
{
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
}

//コマンド実行
static void bas_script_dispatch(BAS_PACKET* packet)
{
	if (EVAL != packet->command)
	{
		//自身で動いているときは、プログラム領域からコードを取得して分解
		if (state.err_code != 0) return;
		if (state.run_no == 0) return;
		packet->reciever = packet->sender = program_areas[0];
		strcpy(decode_message, program_areas[state.run_no++]);
		if (!bas_parse_parameter(&packet, decode_message)) return;
	}
	bas_do_script(&packet);
}


//=============================================================================
//packet decoder
//=============================================================================
//スクリプト読込み
static bool bas_pcket_read(void* packet)
{
	SYNTAX_READ *context = (SYNTAX_READ*)packet;
	int address = strtol(context->addr, NULL, 0);
	printf("%s\n", program_areas[address]);
	return true;
}

//スクリプト書込み
static bool bas_pcket_write(void* packet)
{
	SYNTAX_WRITE *context = (SYNTAX_WRITE*)packet;
	int address = strtol(context->addr, NULL, 0);
	strcpy(program_areas[address], context->value);
	return true;
}

//スクリプト実行
static bool bas_pcket_start(BAS_PACKET* packet)
{
	SYNTAX_START *context = (SYNTAX_START*)packet;
	int run_no = strtol(context->addr, NULL, 0);
	if (0 != state.err_code) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//中断
static bool bas_pcket_abort(BAS_PACKET* packet)
{
	if (0 != state.err_code) return false;
	state.run_no = 0;
	return true;
}

//エラークリア
static bool bas_pcket_errclear(BAS_PACKET* packet)
{
	state.err_code = state.run_no = 0;
	return true;
}

//実行
static bool bas_packet_eval(BAS_PACKET* packet)
{
	bas_script_dispatch(packet);
	return true;
}

//通知
static bool bas_pcket_notify(BAS_PACKET* packet)
{
	return true;
}

#define PACKET_BAS_COMMAND_TABLE 8
static BAS_PACKET_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ 'R'		, bas_pcket_read		},
	{ 'W'		, bas_pcket_write		},
	{ 'S'		, bas_pcket_start		},
	{ 'A'		, bas_pcket_abort		},
	{ 'C'		, bas_pcket_errclear	},
	{ 'E'		, bas_packet_eval		},
	{ 'N'		, bas_pcket_notify		},
};


//パケット実行
static bool bas_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_PACKET_TABLE cmd = packet_command_table[i];
		if (packet->command == (char)cmd.name)
		{
			return cmd.execute(packet);
		}
	}
	return false;
}

//トピックID取得(0-9=該当あり,-1=該当なし
static int bas_get_topic_id(char* topic)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (0 == strcmp(topic, program_areas[i])) return i;
	}
	return -1;
}

//パラメータ解析
static bool bas_parse_parameter(BAS_PACKET* packet, char* msg)
{
	packet->opcode = msg;
	packet->prm1 = packet->prm2 = packet->prm3 = packet->prm4 = NULL;
	char** prms = &packet->prm1;
	for (;; msg++)
	{
		switch (*msg)
		{
		case ',':
			*(msg++) = '\0';
			*(prms++) = msg;
			break;
		case '\n':
			*msg = '\0';
			return true;
		case '\0':
			return false;
		}
	}
}

//--------------------------------------------
//受信メッセージを解析する
//--------------------------------------------
//電文例：SENDER,RECIEVER,1:PARAMETER...\n
static bool bas_parse(BAS_PACKET* packet, char* msg)
{
	//----------------------------------------------------------------------------
	//パケット解析
	//----------------------------------------------------------------------------
	//[SENDER]
	packet->sender = msg;
	for (msg++; ',' != *msg; msg++) if ('\0' == *msg) return false;
	*(msg++) = '\0';
	//----------------------------------------------------------------------------
	//[RECIEVER]
	packet->reciever = msg;
	for (msg++; ',' != *msg; msg++) if ('\0' == *msg) return false;
	*(msg++) = '\0';
	packet->listen_id = bas_get_topic_id(packet->reciever);
	if (0 > packet->listen_id) return false;
	//----------------------------------------------------------------------------
	//[COMMAND]
	packet->command = *(msg++);
	if (':' != *(msg++)) return false;
	//----------------------------------------------------------------------------
	//[PARAMETER]
	return bas_parse_parameter(packet, msg);
	//----------------------------------------------------------------------------
}

//宛先確認
static bool bas_check_from(BAS_PACKET* self)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (0 == strcmp(program_areas[i], self->reciever)) return true;
	}
	return false;
}

//コマンド解析
void bas_main(char* recv_message)
{
	BAS_PACKET packet;
	if (bas_parse(&packet, recv_message))
	{
		if (bas_check_from(&packet))
		{
			bool is_success = bas_execute(&packet);
			if (0 == strcmp(program_areas[0], packet.reciever))
			{
				char result[2] = "\0\0";
				result[0] = (is_success) ? 'Y' : 'N';
				bas_send_message(program_areas[0], packet.prm1, packet.command + 0x20, result);
			}
		}
	}

	//プログラム実行
	if (0 == state.err_code)
	{
		packet.sender = program_areas[0];
		
		int cur = state.run_no++;
		bas_script_dispatch(&packet);
	}
}
