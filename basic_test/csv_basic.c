#include <stdbool.h>
#include "csv_basic.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"


//メッセージ

//2400Byte program area
static char program_areas[100][24] =
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

//リッスンしている
static char notify_memory[32][16];

//IF,I=0,20,30
//GOTO
//GOSUB
//RETURN
//END
//

typedef struct
{
	char* cmd;
	char* judge;
	char* jump_true;
	char* jump_false;
} SYNTAX_IF;

typedef struct
{
	char* cmd;
} SYNTAX_QUIT;

typedef struct
{
	char* eval;
} SYNTAX_EVAL;




static bool bas_script_if(BAS_PACKET* packet);
static bool bas_script_if(BAS_PACKET* packet)
{
	return true;
}

static bool bas_script_goto(BAS_PACKET* packet);
static bool bas_script_goto(BAS_PACKET* packet)
{
	return true;
}

static bool bas_script_gosub(BAS_PACKET* packet);
static bool bas_script_gosub(BAS_PACKET* packet)
{
	return true;
}

static bool bas_script_return(BAS_PACKET* packet);
static bool bas_script_return(BAS_PACKET* packet)
{
	return true;
}

static bool bas_script_end(BAS_PACKET* packet);
static bool bas_script_end(BAS_PACKET* packet)
{
	return true;
}

static bool bas_script_notify(BAS_PACKET* packet);
static bool bas_script_notify(BAS_PACKET* packet)
{
	return true;
}

#define SCRIPT_COMMAND_TABLE_LENGTH	6
static BAS_COMANND_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	{ "IF"		, bas_script_if			},
	{ "GOTO"	, bas_script_goto		},
	{ "GOSUB"	, bas_script_gosub		},
	{ "RETURN"	, bas_script_return		},
	{ "END"		, bas_script_end		},
	{ "NOTIFY"	, bas_script_notify		},
};

static bool bas_pcket_read(void* packet);
static bool bas_pcket_read(void* packet)
{
	SYNTAX_READ* context = packet;
	return true;
}

static bool bas_pcket_write(void* packet);
static bool bas_pcket_write(void* packet)
{
	return true;
}

static bool bas_pcket_start(BAS_PACKET* packet);
static bool bas_pcket_start(BAS_PACKET* packet)
{
	return true;
}

static bool bas_pcket_abort(BAS_PACKET* packet);
static bool bas_pcket_abort(BAS_PACKET* packet)
{
	return true;
}

static bool bas_pcket_errclear(BAS_PACKET* packet);
static bool bas_pcket_errclear(BAS_PACKET* packet)
{
	return true;
}

static bool bas_pcket_get_param(BAS_PACKET* packet);
static bool bas_pcket_get_param(BAS_PACKET* packet)
{
	return true;
}

static bool bas_pcket_notify(BAS_PACKET* packet);
static bool bas_pcket_notify(BAS_PACKET* packet)
{
	return true;
}

#define PACKET_BAS_COMMAND_TABLE 8
static BAS_COMANND_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ 'R'		, bas_pcket_read		},
	{ 'W'		, bas_pcket_write		},
	{ 'S'		, bas_pcket_start		},
	{ 'A'		, bas_pcket_abort		},
	{ 'C'		, bas_pcket_errclear	},
	{ 'P'		, bas_pcket_get_param	},
	{ 'N'		, bas_pcket_notify		},
};

typedef struct
{
	char* addr;
} SYNTAX_READ;

typedef struct
{
	char* addr;
	char* value;
} SYNTAX_WRITE;

typedef struct
{
	char* addr;
} SYNTAX_START;


typedef struct
{
	char* addr;
} SYNTAX_GET_PARAM;

typedef struct
{
	char* addr;
} SYNTAX_READ;


//パケット実行
static void bas_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_COMANND_TABLE cmd = packet_command_table[i];
		if (packet->command == cmd.name)
		{
			//実行
			cmd.execute(packet->prm1);
			return;
		}
	}
	
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

//--------------------------------------------
//受信メッセージを解析する
//--------------------------------------------
//電文例：SENDER,RECIEVER,1:PARAMETER...\n
static bool bas_parse(BAS_PACKET* self, char* msg)
{
	//----------------------------------------------------------------------------
	//パケット解析
	//----------------------------------------------------------------------------
	//[SENDER]
	self->sender = msg;
	for (msg++; ',' != *msg; msg++) if ('\0' == *msg) return false;
	*(msg++) = '\0';
	//----------------------------------------------------------------------------
	//[RECIEVER]
	self->reciever = msg;
	for (msg++; ',' != *msg; msg++) if ('\0' == *msg) return false;
	*(msg++) = '\0';
	self->listen_id = bas_get_topic_id(self->reciever);
	if (0 > self->listen_id) return false;
	//----------------------------------------------------------------------------
	//[COMMAND]
	self->command = *(msg++);
	if (':' != *(msg++)) return false;
	//----------------------------------------------------------------------------
	//[PARAMETER]
	self->prm1 = msg;
	self->prm2 = self->prm3 = self->prm4 = self->prm5 = NULL;
	char** prms = &self->prm2;
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
	//----------------------------------------------------------------------------
}

//コマンド解析
void bas_main(char* recv_message)
{
	BAS_PACKET packet;
	if (bas_parse(&packet, recv_message))
	{
		bas_execute(&packet);
	}
}
