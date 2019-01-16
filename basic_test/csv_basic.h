#ifndef __CSV_BASIC_H__
#define __CSV_BASIC_H__

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
	
	//コマンド送信	C$="100" というパラメータを送れば基板の中で実行する。
	EVAL = 'E',
	EVAL_ACK = 'e',
	
	//パラメータ値取得
	GET_PARAM = 'P',
	GET_PARAM_ACK = 'p',
	
	//変化通知(自発的に送信)
	NOTIFY = 'N',

} COMMAND;

//エラーコード
typedef enum
{
	err_invalid_return,		//リターン先が存在しない
	err_out_of_return,		//ヒープ領域オーバー
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
} BAS_PACKET;

//コマンドテーブル
typedef struct
{
	char* name;
	bool(*execute)(BAS_PACKET*);
} BAS_PACKET_TABLE;

typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;

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


extern void bas_main(char* data);


#endif//__CSV_BASIC_H__
