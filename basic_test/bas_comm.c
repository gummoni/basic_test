//=============================================================================
//通信プロトコル
//=============================================================================
#include "config.h"
#include "bas_comm.h"
#include "rpn.h"

//スクリプト読込み
static bool bas_comm_read(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int address = strtol(addr, NULL, 0);
	packet->response = program_areas[address];

	return true;
}

//スクリプト書込み
static bool bas_comm_write(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	char* value = packet->prm2;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//スクリプト実行
static bool bas_comm_start(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_no) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//中断
static bool bas_comm_abort(BAS_PACKET* packet)
{
	if (0 != state.err_no) return false;
	state.run_no = 0;
	return true;
}

//エラークリア
static bool bas_comm_errclear(BAS_PACKET* packet)
{
	state.err_no = state.run_no = state.stp_no = 0;
	return true;
}

//ステータス取得(返信処理の中で返信電文を作成している)
static bool bas_comm_status(BAS_PACKET* packet)
{
	return true;
}

//ファーム読込み
static bool bas_comm_load(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//ファーム書込み
static bool bas_comm_save(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//通信パケット受信解析
#define PACKET_BAS_COMMAND_TABLE 9
static BAS_COMM_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ READ		, bas_comm_read			},		// パラメータ読込み（戻り値：値）
	{ WRITE		, bas_comm_write		},		// パラメータ書込み（戻り値：なし）
	{ START		, bas_comm_start		},		// 開始コマンド（戻り値：なし）
	{ ABORT		, bas_comm_abort		},		// 中断コマンド（戻り値：なし）
	{ ERR_CLEAR	, bas_comm_errclear		},		// エラー解除（戻り値：なし）
	{ INVOKE	, rpn_execute			},		// 計算コマンド（戻り値：値）
	{ STATUS	, bas_comm_status       },		// ステータス取得
	{ LOAD		, bas_comm_load			},		// ファーム読込み
	{ SAVE		, bas_comm_save			},		// ファーム保存
};

//パケット実行
bool bas_comm_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_COMM_TABLE cmd = packet_command_table[i];
		if (packet->command == (char)cmd.name)
		{
 			return cmd.execute(packet);
		}
	}
	return false;
}
