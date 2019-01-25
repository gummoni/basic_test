//=============================================================================
//通信プロトコル
//=============================================================================
#include "config.h"
#include "bas_comm.h"
#include "rpn.h"

//スクリプト読込み
static bool bas_comm_read(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	int address = strtol(addr, NULL, 0);
	context->response = program_areas[address];

	return true;
}

//スクリプト書込み
static bool bas_comm_write(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	char* value = context->prm1;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//スクリプト実行
static bool bas_comm_start(BAS_PACKET_BODY* context)
{
	char* addr = context->opcode;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_no) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//中断
static bool bas_comm_abort(BAS_PACKET_BODY* context)
{
	if (0 != state.err_no) return false;
	state.run_no = 0;
	return true;
}

//エラークリア
static bool bas_comm_errclear(BAS_PACKET_BODY* context)
{
	state.err_no = state.run_no = state.stp_no = 0;
	return true;
}

//ステータス取得(返信処理の中で返信電文を作成している)
static bool bas_comm_status(BAS_PACKET_BODY* context)
{
	return true;
}

//ファーム読込み
static bool bas_comm_load(BAS_PACKET_BODY* context)
{
	//TODO
	return true;
}

//ファーム書込み
static bool bas_comm_save(BAS_PACKET_BODY* context)
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
static bool bas_comm_execute(BAS_PACKET* packet)
{
	for (int i = 0; i < PACKET_BAS_COMMAND_TABLE; i++)
	{
		BAS_COMM_TABLE cmd = packet_command_table[i];
		if (packet->command == (char)cmd.name)
		{
 			return cmd.execute((BAS_PACKET_BODY*)&packet->opcode);
		}
	}
	return false;
}

//１文字取得
static bool bas_check_message(char* msg)
{
	int i;
	bool is_qout = false;
	for (i = 0; i < 64; i++)
	{
		char ch = msg[i];
		if ('a' <= ch && ch <= 'z')
		{
			if (!is_qout)
			{
				//大文字に変換する
				msg[i] = ch & 0xbf;
			}
			continue;
		}
		if ('A' <= ch && ch <= 'Z') continue;
		if ('0' <= ch && ch <= '9') continue;
		if ('(' == ch) continue;
		if (')' == ch) continue;
		if ('+' == ch) continue;
		if ('-' == ch) continue;
		if ('*' == ch) continue;
		if ('/' == ch) continue;
		if ('.' == ch) continue;
		if (',' == ch) continue;
		if (' ' == ch) continue;
		if ('=' == ch) continue;
		if ('<' == ch) continue;
		if ('>' == ch) continue;
		if ('$' == ch) continue;
		if ('%' == ch) continue;
		if ('_' == ch) continue;
		if (':' == ch) continue;
		if ('\"' == ch)
		{
			is_qout ^= true;
			continue;
		}
		if ('\t' == ch)
		{
			if (!is_qout)
			{
				//タブをスペースに変換
				msg[i] = ' ';
			}
			continue;
		}
		if ('\n' == ch) continue;
		if ('\0' == ch) return true;
		return false;
	}
	return false;
}

//--------------------------------------------
//受信メッセージを解析する
//--------------------------------------------
//電文例：SENDER,RECIEVER,1:PARAMETER...\n
bool bas_comm_parse(BAS_PACKET* packet, char* msg)
{
	packet->response = NULL;
	if (!bas_check_message(msg)) return false;

	//----------------------------------------------------------------------------
	//パケット解析
	//----------------------------------------------------------------------------
	//[SENDER]
	packet->sender = msg;	
	for (msg++; ',' != *msg; msg++) if (('\0' == *msg) || ('\n' == *msg)) return false;
	*(msg++) = '\0';

	//[RECIEVER]
	packet->reciever = msg;
	for (msg++; ',' != *msg; msg++) if (('\0' == *msg) || ('\n' == *msg)) return false;
	*(msg++) = '\0';
	if (!get_topic_id(packet->reciever, &packet->listen_id)) return false;		//宛先出なかった

	//[COMMAND]
	packet->command = *(msg++);
	if (':' != *(msg++)) return false;

	//[PARAMETER]
	return parse_parameter(packet, msg, ',');
}

//通信電文解析処理
void bas_comm_job(char* recv_message)
{
	BAS_PACKET packet;

	//通信パケット解析
	if (!bas_comm_parse(&packet, recv_message)) return;

	//TODO:右から左、左から右へブロードキャスト

	//リッスン対象かチェック
	if (0 <= packet.listen_id)
	{
		//パケット実行
		bool is_success = bas_comm_execute(&packet);
		//自分あてなら返信
		if (0 == strcmp(SELF_NAME, packet.reciever))
		{
			//自分宛であった
			char result[16];
			if (is_success)
			{
				if (STATUS == packet.command)
				{
					//ステータス返送
					sprintf(result, "OK,%d,%d", state.run_no, state.err_no);
				}
				else if (NULL == packet.response)
				{
					//レスポンス無し
					strcpy(result, "OK");
				}
				else
				{
					//レスポンス有り
					sprintf(result, "OK,%s", packet.response);
				}
			}
			else
			{
				//エラー応答
				sprintf(result, "NG,%d,%d", state.run_no, state.err_no);
			}
			//返信
			send_message(SELF_NAME, packet.sender, packet.command | 0x20, result);
		}
	}
}
