#ifndef __CSV_BASIC_H__
#define __CSV_BASIC_H__

//	コマンドフォーマット
//	[送元],[宛先],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

//コマンド一覧
typedef enum
{
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
	
	//コマンド送信	C$="100" というパラメータを送れば基板の中で実行する。
	EVAL = 'E',
	EVAL_ACK = 'e',
	
	//パラメータ値取得
	GET_PARAM = 'P',
	GET_PARAM_ACK = 'p',
	
	//変化通知(自発的に送信)
	NOTIFY = 'N',

} COMMAND;

//パケット構造体
typedef struct
{
	int listen_id;
	char* sender;
	char* reciever;
	char command;
	char* prm1;		//リードポインタ1
	char* prm2;		//リードポインタ2
	char* prm3;		//リードポインタ3
	char* prm4;		//リードポインタ4
	char* prm5;		//リードポインタ5
} BAS_PACKET;

//コマンドテーブル
typedef struct
{
	char* name;
	bool(*execute)(void*);
} BAS_COMANND_TABLE;


extern void bas_main(char* data);


#endif//__CSV_BASIC_H__
