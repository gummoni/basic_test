//=============================================================================
//電文解析
//=============================================================================
#include "config.h"
#include "bas.h"
#include "dic.h"
#include "usb.h"
#include "serial.h"
#include "bas_script.h"
#include "bas_property.h"
#include "bas_comm.h"

//=============================================================================
//解析バッファ
//=============================================================================
static BAS_PACKET packet_up;	//上位側
static BAS_PACKET packet_dn;	//下位側

//BAS翻訳機の状態
BASIC_STATE state;

//24文字x100行のプログラムエリア
char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT] =
{
	//リッスン
	"AXIS_Z1",			//  0: UNIQUE_NAME（自分宛、返事を返す）
	"AXIS_Z",			//  1: LISTEN1(NOTIFY)
	"",				    //  2: LISTEN2(NOTIFY)
	"",				    //  3: LISTEN3(NOTIFY)
	"",					//  4: LISTEN4(NOTIFY)
	"",					//  5: LISTEN5(NOTIFY)
	"",					//  6: LISTEN6(NOTIFY)
	"",					//  7: LISTEN7(NOTIFY)
	"",					//  8: LISTEN8(NOTIFY)
	"",					//  9: LISTEN9(NOTIFY)
						// 10:プログラム領域以降すべて
};


//=============================================================================
//受信データ取り込み(TRUE=パケット取り込み完了）
//=============================================================================
static bool bas_parse(BAS_PACKET* packet, char ch)
{
	//===改行検知===
	if ('\r' == ch || '\n' == ch)
	{
		if (RECV_STATE_CORON < packet->state)
		{
			//正常に受信
			packet->recv_buff[packet->recv_length++] = '\0';
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
		if (RECV_STATE_CORON == packet->state)
		{
			packet->state = RECV_STATE_PRM1;
		}
		else
		{
			//エラー（リセット待ち）
			packet->state = RECV_STATE_ERROR;
		}
		return false;
	}

	//===ボディ区切り検知===
	if ((',' == ch) && !packet->is_quot)
	{
		switch (packet->state)
		{
		case RECV_STATE_FROM:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->reciever = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_TO;
			break;

		case RECV_STATE_TO:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm1 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_COMMAND;
			break;

		case RECV_STATE_COMMAND:
		case RECV_STATE_CORON:
		case RECV_STATE_PRM6:
			packet->state = RECV_STATE_ERROR;
			break;

		case RECV_STATE_PRM1:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm2 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_PRM2;
			break;

		case RECV_STATE_PRM2:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm3 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_PRM3;
			break;

		case RECV_STATE_PRM3:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm4 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_PRM4;
			break;

		case RECV_STATE_PRM4:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm5 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_PRM5;
			break;

		case RECV_STATE_PRM5:
			packet->recv_buff[packet->recv_length++] = '\0';
			packet->prm6 = &packet->recv_buff[packet->recv_length];
			packet->state = RECV_STATE_PRM6;
			break;
		}
		return false;
	}

	//===データ解析===
	switch (packet->state)
	{
	case RECV_STATE_FROM:
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//文字格納
			packet->recv_buff[packet->recv_length++] = ch;
		}
		else
		{
			//エラー（リセット待ち）
			packet->state = RECV_STATE_ERROR;
		}
		break;

	case RECV_STATE_TO:
		if ('a' <= ch && ch <= 'z') ch &= 0xbf;
		if (('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('_' == ch))
		{
			//文字格納
			packet->recv_buff[packet->recv_length++] = ch;
		}
		else
		{
			//エラー（リセット待ち）
			packet->state = RECV_STATE_ERROR;
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
		case STATUS:
		case LOAD:
		case SAVE:
			packet->command = ch;
			packet->state = RECV_STATE_CORON;
			break;
		default:
			packet->state = RECV_STATE_ERROR;
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
			packet->is_quot ^= true;
			packet->recv_buff[packet->recv_length++] = '"';
		}
		else
		{
			if (!packet->is_quot)
			{
				if ('a' <= ch && ch <= 'z') ch &= 0xbf;
			}
			if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ('+' == ch) || ('-' == ch) || ('*' == ch) || ('/' == ch) || ('.' == ch) || ('<' == ch) || ('>' == ch) || ('=' == ch) || ('%' == ch) || ('$' == ch) || (' ' == ch) || (',' == ch) || ('_' == ch) || ('(' == ch) || (')' == ch))
			{
				packet->recv_buff[packet->recv_length++] = ch;
			}
			else
			{
				packet->state = RECV_STATE_ERROR;
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
		if (STATUS == packet->command)
		{
			//ステータス返送
			sprintf(result, "OK,%d,%d", state.run_no, state.err_no);
		}
		else if (NULL == packet->resp_buff)
		{
			//レスポンス無し
			strcpy(result, "OK");
		}
		else
		{
			//レスポンス有り
			sprintf(result, "OK,%s", packet->resp_buff);
		}
	}
	else
	{
		//エラー応答
		sprintf(result, "ER,%d,%d", state.run_no, state.err_no);
	}
	//返信
	char* response_message = make_message(SELF_NAME, packet->sender, packet->command | 0x20, result);
	packet->response(response_message, strlen(response_message));
}

//=============================================================================
//受信処理
//=============================================================================
static void bas_dispatch(BAS_PACKET* packet)
{
	int length;
	char* msg;

	if (packet->recieve(&msg, &length))
	{
		//受信解析
		while (0 < length--)
		{
			char ch = *(msg++);

			if (bas_parse(packet, ch))
			{
				if (get_listen_id(packet->reciever, &packet->listen_id))
				{
					bool ack = bas_comm_execute(packet);
					if (0 == packet->listen_id)
					{
						bas_response(packet, ack);
					}
				}
				//次のパケット受信へ
				clear_packet(packet);
			}
		}
	}
}

//=============================================================================
//初期化
//=============================================================================
void bas_init(void)
{
	dic_clear();

	state.run_no = state.err_no = state.stp_no = state.timer_count = 0;

	clear_packet(&packet_up);
	packet_up.recieve = serial_read;
	packet_up.response = serial_write;
	packet_up.broadcast = usb_write;
	
	clear_packet(&packet_dn);
	packet_dn.recieve = usb_read;
	packet_dn.response = usb_write;
	packet_dn.broadcast = serial_write;
}

//=============================================================================
//メッセージ処理
//=============================================================================
//TODO:データ取得は別ドライバで行う
void bas_job(void)
{
	bas_property_job(&packet_up);
	bas_script_job();
	bas_dispatch(&packet_up);
	bas_dispatch(&packet_dn);
}



