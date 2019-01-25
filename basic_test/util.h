#ifndef __UTIL_H__
#define __UTIL_H__


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
	INVOKE = 'I',
	INVOKE_ACK = 'i',

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

//エラーコード(＋ステータスコード)
typedef enum
{
	err_none = 0,			//エラー無し
	err_busy = 1,			//動作中
	err_out_of_return,		//ヒープ領域オーバー
	err_jump,				//ジャンプ失敗
} error_code;

//動作状態
typedef struct
{
	int run_no;				//実行行数
	error_code err_no;		//エラー番号
	int stp_no;				//内部ステップ番号
	int timer_count;		//タイマーカウント
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
	char* prm5;		//リードポインタ5
	char* response;	//返信用メッセージポインタ
} BAS_PACKET;

typedef struct
{
	char* opcode;	//リードポインタ(コマンド)
	char* prm1;		//リードポインタ1
	char* prm2;		//リードポインタ2
	char* prm3;		//リードポインタ3
	char* prm4;		//リードポインタ4
	char* prm5;		//リードポインタ5
	char* response;	//返信用メッセージポインタ
} BAS_PACKET_BODY;


//プログラム領域
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]

//BAS翻訳機の状態
extern BASIC_STATE state;

extern void send_message(char* from, char* to, char cmd, char* message);
extern bool parse_parameter(BAS_PACKET* packet, char* msg, char separator);
extern int label_search(char* label, bool* is_label);
extern bool get_topic_id(char* topic, int* result);

#endif//__UTIL_H__
