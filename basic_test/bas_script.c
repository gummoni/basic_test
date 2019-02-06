//=============================================================================
//スクリプト解析
//=============================================================================
#include "config.h"
#include "bas_script.h"
#include "dic.h"
#include "rpn.h"
#include "heap.h"
#include "bas_property.h"

//受信バッファ
BAS_PACKET script_packet;

//送信バッファ
static char send_buf[PROGRAM_LINE_COUNT];


//ラベル照合
static bool label_compare(char* label, char* msg)
{
	int i;
	for (i = 0; i < PROGRAM_LINE_MAX; i++)
	{
		char a = *(label++);
		char b = *(msg++);
		if (a == '\0') return true;
		if (a != b) return false;
	}

	state.err_no = err_too_long;
	return false;
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

//スクリプト構文解析(CSV区切りを分解)
bool csv_split(BAS_PACKET* packet, char* dst, char* msg)
{
	packet->prm1 = dst;
	packet->prm2 = packet->prm3 = packet->prm4 = packet->prm5 = packet->prm6 = packet->prm7 = NULL;
	char** prms = &packet->prm2;

	bool qout = false;
	int i;
	for (i = 0; i < BUFF_COUNT; i++, msg++)
	{
		char ch = *msg;
		if ('"' == ch)
		{	//文字列範囲(区切り文字で区切らないようにする)
			qout ^= true;
			*(dst++) = '"';
		}
		else if ((' ' == ch) && !qout)
		{	//区切り記号検知
			*(dst++) = '\0';
			*(prms++) = dst;
		}
		else if ('\r' == ch)
		{
			//スキップ
		}
		else if (('\n' == ch) || ('\0' == ch))
		{	//終端
			*dst = '\0';
			return true;
		}
		else
		{	//上記条件以上はデータコピー
			*(dst++) = ch;
		}
	}
	state.err_no = err_parse;
	return false;
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

//初期化
void bas_script_init(void)
{
	heap_init();
	state.run_no = state.err_no = state.stp_no = state.timer_count = 0;
}

//IF文
static void bas_script_if(BAS_PACKET* packet)
{
	bool is_label;
	if (rpn_judge(packet))
	{
		//TRUE
		state.run_no = label_search(packet->prm3, &is_label);
		if (is_label) state.run_no++;
	}
	else
	{
		//FALSE
		if (NULL != packet->prm4)
		{
			state.run_no = label_search(packet->prm4, &is_label);
			if (is_label) state.run_no++;
		}
	}
}

//GOTO文
static void bas_script_goto(BAS_PACKET* packet)
{
	bool is_label;
	state.run_no = label_search(packet->prm2, &is_label);

	if (is_label)
	{
		//引数代入
		BAS_PACKET parse;
		parse.reciever = parse.sender = SELF_NAME;
		if (csv_split(&parse, bas_parser.resp_buff, program_areas[state.run_no++]))
		{
			if (NULL == parse.prm2) return;
			if (NULL == packet->prm3) return;
			dic_set(parse.prm2, packet->prm3);

			if (NULL == parse.prm3) return;
			if (NULL == packet->prm4) return;
			dic_set(parse.prm3, packet->prm4);

			if (NULL == parse.prm4) return;
			if (NULL == packet->prm5) return;
			dic_set(parse.prm4, packet->prm5);

			if (NULL == parse.prm5) return;
			if (NULL == packet->prm6) return;
			dic_set(parse.prm5, packet->prm6);

			if (NULL == parse.prm6) return;
			if (NULL == packet->prm7) return;
			dic_set(parse.prm6, packet->prm7);
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
		state.err_no = err_out_of_return;
	}
}

//RETURN文
static void bas_script_return(BAS_PACKET* packet)
{
	if (!heap_dequeue(&state.run_no))
	{
		//戻り先がないのでEND扱い
		bas_script_init();
	}
}

static void bas_script_end(BAS_PACKET* packet)
{
	bas_script_init();
}

//ブロードキャスト
void bas_script_broadcast(BAS_PACKET* packet, char* to, char* resp)
{
	char* response_message = make_message(SELF_NAME, to, INVOKE, resp);
	int length = strlen(response_message);
	packet->response(response_message, length);
	packet->broadcast(response_message, length);
}

//メッセージ通知
static void bas_script_invoke(BAS_PACKET* packet)
{
	char resp[PROGRAM_LINE_COUNT];
	char* to = packet->prm1;
	char* key = packet->prm3;
	if (key == NULL)
	{
		state.err_no = err_null;
		return;
	}
	char* val = rpn_get_value(key);
	int len = strlen(key);
	if ('$' == key[len - 1])
	{
		sprintf(resp, "%s.%s=\"%s\"", SELF_NAME, key, val);
	}
	else
	{
		sprintf(resp, "%s.%s=%s", SELF_NAME, key, val);
	}
	bas_script_broadcast(packet, to, resp);
}

static void bas_set_error(BAS_PACKET* packet, error_code err_no)
{
	char resp[PROGRAM_LINE_COUNT];
	state.err_no = err_no;
	//エラーをブロードキャスト
	sprintf(resp, "%s.ERR_NO=%d", SELF_NAME, state.err_no);
	bas_script_broadcast(packet, SELF_NAME, resp);
}

//遅延処理
static void bas_script_delay(BAS_PACKET* packet)
{
	state.stp_no = (0 < state.timer_count) ? 1 : 0;
}

//------------------------------拡張ここから------------------------------

//原点復帰
static void bas_script_org(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//原点復帰コマンド実行
		state.stp_no = 2;
		bas_do_abs(rpn_get_value("SPD_ORGH"), HAZUSI);
		break;

	case 2:
		//停止待ち
		if (1 == REFR)
		{
			//ぶつかり停止
			bas_do_stop();
			state.err_no = err_butukari;
		}
		else if (0 == MOVING)
		{
			//停止したら原点復帰コマンド実行
			state.stp_no = 3;
		}
		break;

	case 3:
		//停止待ち
		if (1 == REFL)
		{
			//ぶつかり停止
			bas_do_stop();
			state.stp_no = 0;
		}
		else if (0 == MOVING)
		{
			//停止したら原点復帰コマンド実行
			state.stp_no = 3;
		}
		break;
	}
}

//絶対値移動
static void bas_script_abs(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//高速移動開始
		state.stp_no = 1;
		bas_do_abs(packet->prm2, rpn_get_value(packet->prm3));
		break;

	case 1:
		//停止待ち
		if (0 == MOVING)
		{
			//停止したら終了
			state.stp_no = 0;
		}
		break;
	}
}

//相対値移動
static void bas_script_inc(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//高速移動開始
		state.stp_no = 1;
		rpn_get_value(packet->prm2, dic_get(packet->prm3));
		break;

	case 1:
		//停止待ち
		if (0 == MOVING)
		{
			//停止したら終了
			state.stp_no = 0;
		}
		break;
	}
}

//モータ停止
static void bas_script_stop(BAS_PACKET* packet)
{
	bas_do_stop();
}

//エラー通知処理
static void bas_script_err(BAS_PACKET* packet)
{
	if (rpn_judge(packet))
	{
		//条件が一致したらエラー確定
		bas_set_error(packet, strtol(packet->prm3, NULL, 0));
	}
}

//パラメータ:Z1, Z2, 圧入PLS, WAIT
static void bas_script_tipon(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//初期化
		state.timer_count = -1;
		//Z1移動コマンド実行
		bas_do_abs(dic_get("SPD_HIGH"), strtol(packet->prm2, NULL, 0));	//Z1移動
		if ("1" == rpn_get_value("NOZLE"))
		{
			//ノズル検知
			bas_do_stop();
			bas_set_error(packet, err_butukari);
		}
		else if (MOVING == 0)
		{
			//動作完了次へ
			state.run_no = 1;
			bas_do_inc(dic_get("SPD_HIGH"), strtol(packet->prm3, NULL, 0));	//Z2移動
		}

		bas_do_inc(dic_get("SPD_SLOW"), strtol(packet->prm4, NULL, 0));	//圧入
		//WAIT
		bas_do_org(dic_get("SPD_ORGL"), dic_get("LIMIT"));
		state.stp_no = 2;
		break;

	case 2:	
		//Z1移動完了待ち
		if (dic_get("MOVING") == "0")
		{
			//Z1移動完了
			if (0 > state.timer_count)
			{
				//タイマー設定
				state.timer_count = 100;
			}
			if (0 == state.timer_count)
			{
				//タイマーカウントアップ
				state.stp_no = 3;
				//Z2移動コマンド実行
			}
		}
		break;

	case 3:	
		//Z2移動
		if (dic_get("MOVING") == "0")
		{
			//Z2移動完了
			if (0 > state.timer_count)
			{
				//タイマー設定
				state.timer_count = 100;
			}
			if (0 == state.timer_count)
			{
				//タイマーカウントアップ
				state.stp_no = 3;
				//Z2移動コマンド実行
			}
		}
		break;

	case 4:	//Z圧入
	case 5:	//原点復帰


		//動作完了
		if (dic_get("REFR") == "1")
		{
			//ぶつかり停止
			state.err_no = err_butukari;
		}
		else if (dic_get("MOVING") == "0")
		{
			//停止したら原点復帰コマンド実行
			state.stp_no = 3;
		}
		break;
	}
}

static void bas_script_kin1(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//動作開始コマンド送信
		state.stp_no = 1;
		break;
	case 1:
		//動作完了
		state.stp_no = 0;
		break;
	}
}

static void bas_script_kin2(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//動作開始コマンド送信
		state.stp_no = 1;
		break;
	case 1:
		//動作完了
		state.stp_no = 0;
		break;
	}
}

static void bas_script_kout(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//動作開始コマンド送信
		state.stp_no = 1;
		break;
	case 1:
		//動作完了
		state.stp_no = 0;
		break;
	}
}

static void bas_script_tipoff(BAS_PACKET* packet)
{
	switch (state.stp_no)
	{
	case 0:
		//動作開始コマンド送信
		state.stp_no = 1;
		break;
	case 1:
		//動作完了
		state.stp_no = 0;
		break;
	}
}

//------------------------------拡張ここまで------------------------------

#define SCRIPT_COMMAND_TABLE_LENGTH	13
static BAS_SCRIPT_TABLE script_command_table[SCRIPT_COMMAND_TABLE_LENGTH] =
{
	//---基本命令---
	{ "IF"		, bas_script_if			},	//0
	{ "GOTO"	, bas_script_goto		},	//1
	{ "GOSUB"	, bas_script_gosub		},	//2
	{ "RETURN"	, bas_script_return		},	//3
	{ "END"		, bas_script_end		},	//4
	{ "INVOKE"	, bas_script_invoke		},	//5
	{ "DELAY"	, bas_script_delay		},	//6
	{ "ERR"		, bas_script_err		},	//8
	//----拡張命令（PB210）---
	{ "ORG"		, bas_script_org		},	//9
	{ "ABS"		, bas_script_abs		},	//10
	{ "INC"		, bas_script_inc		},	//11
	{ "STOP"	, bas_script_stop		},	//12
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
			command.execute(packet);
			if (0 == state.stp_no)
			{
				//動作完了
				state.run_no++;
				return true;
			}
			else
			{
				//実行中
				return false;
			}
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
		printf("%d\n", state.run_no);
		if ((0 != state.err_no) || (10 > state.run_no)) return;					//プログラム実行中かどうか判別
		char* msg = program_areas[state.run_no];								//プログラムコード取得
		if ('*' == msg[0]) continue;											//ラベルならスキップ
		if (!csv_split(&script_packet, bas_parser.parse_buff, msg)) return;		//プログラム番地のコードを解析
		if (!bas_script_execute(&script_packet)) return;						//実行
	}
}
