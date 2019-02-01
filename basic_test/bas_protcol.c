#include "config.h"
#include "bas_protcol.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"


//=============================================================================
//スクリプト読込み
//=============================================================================
static bool bas_comm_read(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int address = strtol(addr, NULL, 0);
	
	char* msg = make_message(SELF_NAME, packet->sender, packet->command, program_areas[address]);
	int len = strlen(msg);
	packet->response(msg, len);

	return true;
}

//=============================================================================
//スクリプト書込み
//=============================================================================
static bool bas_comm_write(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	char* value = packet->prm2;
	int address = strtol(addr, NULL, 0);
	strcpy(program_areas[address], value);
	return true;
}

//=============================================================================
//スクリプト実行
//=============================================================================
static bool bas_comm_start(BAS_PACKET* packet)
{
	char* addr = packet->prm1;
	int run_no = strtol(addr, NULL, 0);
	if (0 != state.err_no) return false;
	if (0 != state.run_no) return false;
	state.run_no = run_no;
	return true;
}

//=============================================================================
//中断
//=============================================================================
static bool bas_comm_abort(BAS_PACKET* packet)
{
	if (0 != state.err_no) return false;
	state.run_no = 0;
	return true;
}

//=============================================================================
//エラークリア
//=============================================================================
static bool bas_comm_errclear(BAS_PACKET* packet)
{
	state.err_no = state.run_no = state.stp_no = 0;
	return true;
}

//=============================================================================
//ステータス取得(返信処理の中で返信電文を作成している)
//=============================================================================
static bool bas_comm_status(BAS_PACKET* packet)
{
	return true;
}

//=============================================================================
//ファーム読込み
//=============================================================================
static bool bas_comm_load(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//=============================================================================
//ファーム書込み
//=============================================================================
static bool bas_comm_save(BAS_PACKET* packet)
{
	//TODO
	return true;
}

//=============================================================================
//通信パケット受信解析
//=============================================================================
#define PACKET_BAS_COMMAND_TABLE 9
static BAS_COMM_TABLE packet_command_table[PACKET_BAS_COMMAND_TABLE] =
{
	{ READ		, bas_comm_read			},		// パラメータ読込み（戻り値：値）
	{ WRITE		, bas_comm_write		},		// パラメータ書込み（戻り値：なし）
	{ START		, bas_comm_start		},		// 開始コマンド（戻り値：なし）
	{ ABORT		, bas_comm_abort		},		// 中断コマンド（戻り値：なし）
	{ ERR_CLEAR	, bas_comm_errclear		},		// エラー解除（戻り値：なし）
	{ INVOKE	, rpn_execute			},		// 計算コマンド（戻り値：値）
	{ LOAD		, bas_comm_load			},		// ファーム読込み
	{ SAVE		, bas_comm_save			},		// ファーム保存
};

//=============================================================================
//パケット実行
//=============================================================================
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
//=============================================================================
//受信データ取り込み(TRUE=パケット取り込み完了）
//=============================================================================
static bool bas_parse(BAS_PACKET* packet, char ch)
{
	//データ長エラー
	if (bas_parser.parse_length >= BUFF_COUNT)
	{
		clear_packet(packet);
		return false;
	}

	//===改行検知===
	if ('\r' == ch || '\n' == ch || '\0' == ch)
	{
		if (RECV_STATE_CORON < bas_parser.state)
		{
			//正常に受信
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			return true;
		}
		else
		{
			//異常受信リセット
			clear_packet(packet);
			return false;
		}
	}

	//===ヘッダ区切り検知===
	if (':' == ch)
	{
		if (RECV_STATE_CORON == bas_parser.state)
		{
			bas_parser.state = RECV_STATE_PRM1;
		}
		else
		{
			//エラー（リセット待ち）
			bas_parser.state = RECV_STATE_ERROR;
		}
		return false;
	}

	//===ボディ区切り検知===
	if ((',' == ch) && !bas_parser.is_quot)
	{
		switch (bas_parser.state)
		{
		case RECV_STATE_FROM:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->reciever = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_TO;
			break;

		case RECV_STATE_TO:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm1 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_COMMAND;
			break;

		case RECV_STATE_COMMAND:
		case RECV_STATE_CORON:
		case RECV_STATE_PRM6:
			bas_parser.state = RECV_STATE_ERROR;
			break;

		case RECV_STATE_PRM1:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm2 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM2;
			break;

		case RECV_STATE_PRM2:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm3 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM3;
			break;

		case RECV_STATE_PRM3:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm4 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM4;
			break;

		case RECV_STATE_PRM4:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm5 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM5;
			break;

		case RECV_STATE_PRM5:
			bas_parser.parse_buff[bas_parser.parse_length++] = '\0';
			packet->prm6 = &bas_parser.parse_buff[bas_parser.parse_length];
			bas_parser.state = RECV_STATE_PRM6;
			break;
		}
		return false;
	}

	//===データ解析===
	switch (bas_parser.state)
	{
	case RECV_STATE_FROM:
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//文字格納
			bas_parser.parse_buff[bas_parser.parse_length++] = ch;
		}
		else
		{
			//エラー（リセット待ち）
			bas_parser.state = RECV_STATE_ERROR;
		}
		break;

	case RECV_STATE_TO:
		if ('a' <= ch && ch <= 'z') ch &= 0xbf;
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//文字格納
			bas_parser.parse_buff[bas_parser.parse_length++] = ch;
		}
		else
		{
			//エラー（リセット待ち）
			bas_parser.state = RECV_STATE_ERROR;
		}
		break;

	case RECV_STATE_COMMAND:
		switch (ch)
		{
		case READ:
		case WRITE:
		case START:
		case ABORT:
		case ERR_CLEAR:
		case INVOKE:
		case LOAD:
		case SAVE:
			packet->command = ch;
			bas_parser.state = RECV_STATE_CORON;
			break;
		default:
			bas_parser.state = RECV_STATE_ERROR;
			break;
		}
		break;

	case RECV_STATE_CORON:
		//先頭行でチェック済み
		break;

	case RECV_STATE_PRM1:
	case RECV_STATE_PRM2:
	case RECV_STATE_PRM3:
	case RECV_STATE_PRM4:
	case RECV_STATE_PRM5:
	case RECV_STATE_PRM6:
		if ('"' == ch)
		{
			//文字列記号検知
			bas_parser.is_quot ^= true;
			bas_parser.parse_buff[bas_parser.parse_length++] = '"';
		}
		else
		{
			if (!bas_parser.is_quot)
			{
				if ('a' <= ch && ch <= 'z') ch &= 0xbf;
			}
			if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('+' == ch) || ('-' == ch) || ('*' == ch) || ('/' == ch) || ('.' == ch) || ('<' == ch) || ('>' == ch) || ('=' == ch) || ('%' == ch) || ('$' == ch) || (' ' == ch) || (',' == ch) || ('_' == ch) || ('(' == ch) || (')' == ch))
			{
				bas_parser.parse_buff[bas_parser.parse_length++] = ch;
			}
			else
			{
				bas_parser.state = RECV_STATE_ERROR;
			}
		}
		break;
	default:
		break;
	}
	return false;
}

//=============================================================================
//パケット実行
//=============================================================================
static void bas_response(BAS_PACKET* packet, bool ack)
{
	//参照対象だったのでコマンド実行
	byte result[16];

	if (ack)
	{
		if ('\0' == bas_parser.resp_buff[0])
		{
			//レスポンス無し
			strcpy(result, "OK");
		}
		else
		{
			//レスポンス有り
			sprintf(result, "OK,%s", bas_parser.resp_buff);
		}
	}
	else
	{
		//エラー応答
		sprintf(result, "ER,%d,%d", state.run_no, state.err_no);
	}
	//返信
	char* response_message = make_message(SELF_NAME, packet->sender, packet->command | 0x20, result);
	int length = strlen(response_message);
	packet->response(response_message, length);
}

//=============================================================================
//宛先確認（トピックID取得）
//=============================================================================
static bool get_listen_id(char* topic, int* result)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		int length = strlen(program_areas[i]);
		if ('\0' == topic[length])
		{	//長さチェックしてから鑑定
			if (0 == strcmp(topic, program_areas[i]))
			{
				*result = i;
				return true;
			}
		}
	}

	*result = -1;
	return false;
}

//=============================================================================
//受信処理
//=============================================================================
void bas_comm_job(BAS_PACKET* packet)
{
	int length;
	char* msg;

	if (packet->recieve(&msg, &length))
	{
		printf(">>>%s", msg);
		//受信解析
		while (0 < length--)
		{
			char ch = *(msg++);

			if (bas_parse(packet, ch))
			{
				//1パケット受信完了
				if (get_listen_id(packet->reciever, &bas_parser.listen_id))
				{
					bool ack = bas_comm_execute(packet);
					if (0 == bas_parser.listen_id)
					{
						bas_response(packet, ack);
					}
				}
				//次のパケット受信へ
				clear_packet(packet);
				return;
			}
		}
	}
}

//=============================================================================
//パケットクリア
//=============================================================================
void clear_packet(BAS_PACKET* packet)
{
	bas_parser.parse_length = 0;
	bas_parser.state = 0;
	bas_parser.is_quot = false;
	packet->sender = bas_parser.parse_buff;
	packet->reciever = NULL;
	packet->command = '\0';
	packet->prm1 = NULL;
	packet->prm2 = NULL;
	packet->prm3 = NULL;
	packet->prm4 = NULL;
	packet->prm5 = NULL;
	packet->prm6 = NULL;
}
