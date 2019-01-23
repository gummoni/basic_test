#ifndef __BAS_PACKET_H__
#define __BAS_PACKET_H__

//	コマンドフォーマット
//	[送元],[宛先],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

//コマンド一覧
typedef enum
{
	//A=0x41, a=0x61
	//データ読込み（アドレスから読み取る）
	//-->SEND,RECV,R:10
	READ = 'R',
	READ_ACK = 'r',

	//データ書込み（アドレスに書き込む）
	WRITE = 'W',
	WRITE_ACK = 'w',

	//動作開始(番号を指定して実行)
	START = 'S',
	START_ACK = 's',

	//動作中断
	ABORT = 'A',
	ABORT_ACK = 'a',

	//エラー解除
	ERR_CLEAR = 'C',
	ERR_CLEAR_ACK = 'c',

	//コマンド送信	C$="100" というパラメータを送れば基板の中で実行する。、変化通知もこのコマンド
	NOTIFY = 'N',
	NOTIFY_ACK = 'n',

	//ステータス取得
	STATUS = 'Z',
	STATUS_ACK = 'z',

	//ファーム読込み
	LOAD = 'X',
	LOAD_ACK = 'x',

	//ファーム保存
	SAVE = 'O',
	SAVE_ACK = 'o',

} COMMAND;

//エラーコード
typedef enum
{
	err_none,				//エラー無し
	err_invalid_return,		//リターン先が存在しない
	err_out_of_return,		//ヒープ領域オーバー
	err_jump,				//ジャンプ失敗
} error_code;

//動作状態
typedef struct
{
	int run_no;
	error_code err_code;
} BASIC_STATE;

//パケット構造体
typedef struct
{
	int listen_id;
	char* sender;
	char* reciever;
	char command;
	char* opcode;	//リードポインタ(コマンド)
	char* prm1;		//リードポインタ1
	char* prm2;		//リードポインタ2
	char* prm3;		//リードポインタ3
	char* prm4;		//リードポインタ4
	char* response;	//返信用メッセージポインタ
} BAS_PACKET;

typedef struct
{
	char* opcode;	//リードポインタ(コマンド)
	char* prm1;		//リードポインタ1
	char* prm2;		//リードポインタ2
	char* prm3;		//リードポインタ3
	char* prm4;		//リードポインタ4
	char* response;	//返信用メッセージポインタ
} BAS_PACKET_BODY;

//コマンドテーブル
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET_BODY* context);
} BAS_PACKET_TABLE;

typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;

//プログラム領域
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]

//BAS翻訳機の状態
extern BASIC_STATE state;

extern void bas_send_message(char* from, char* to, char cmd, char* message);
extern bool bas_parse_parameter(BAS_PACKET* packet, char* msg, char separator);
extern bool bas_comm_parse(BAS_PACKET* packet, char* msg);

#endif//__BAS_PACKET_H__

