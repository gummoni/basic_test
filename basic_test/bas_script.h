#ifndef __BAS_SCRIPT_H__
#define __BAS_SCRIPT_H__

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

	//ファーム読込み
	LOAD = 'X',
	LOAD_ACK = 'x',

	//ファーム保存
	SAVE = 'O',
	SAVE_ACK = 'o',

} COMMAND;


//コマンドテーブル
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET* context);
} BAS_COMM_TABLE;

//コマンドテーブル
typedef struct
{
	char* name;
	bool(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;


//BAS翻訳機の状態
extern BAS_PACKET script_packet;

//関数一覧
extern char* make_message(char* from, char* to, char cmd, char* message);
extern void bas_script_init(void);
extern bool bas_comm_execute(BAS_PACKET* packet);
extern void bas_script_job(void);

#endif//__BAS_SCRIPT_H__

