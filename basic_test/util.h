#ifndef __UTIL_H__
#define __UTIL_H__

//	コマンドフォーマット
//	[送元],[宛先],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

//プログラム領域
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]

//BAS翻訳機の状態
extern BASIC_STATE state;


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
	uint8_t run_no;			//実行行数
	error_code err_no;		//エラー番号
	int stp_no;				//内部ステップ番号
	int timer_count;		//タイマーカウント
} BASIC_STATE;

//通信パケット構造体
typedef struct
{
	//-----内部パラメータ-----
	char resp_buff[PROGRAM_LINE_COUNT];		//送信バッファ
	char recv_buff[PROGRAM_LINE_COUNT];		//受信バッファ
	char recv_length;		//受信データ長
	bool is_quot;			//文字列解析中(ダブルクウォーテーション)
	int state;				//パケット解析ステート	0=送信者, 1=受信者, 2=コマンド, 3=コマンド区切り, 4=opcode, 5=prm1, 6=prm2, 7=prm3, 8=prm4, 9=prm5
	int listen_id;			//リッスンID（受信者名から調べ、該当するIDを登録）
	//-----公開プロパティ-----
	char* sender;			//送信者名
	char* reciever;			//受信者名
	char command;			//コマンド（１文字）
	char* prm1;				//リードポインタ1
	char* prm2;				//リードポインタ2
	char* prm3;				//リードポインタ3
	char* prm4;				//リードポインタ4
	char* prm5;				//リードポインタ5
	char* prm6;				//リードポインタ6
	//-----メソッド-----
	bool(*recieve)(char** msg, int* length);
	void(*response)(char* msg, int length);
	void(*broadcast)(char* msg, int length);
} BAS_PACKET;

extern char* make_message(char* from, char* to, char cmd, char* message);
extern bool parse_parameter(BAS_PACKET* packet, char* msg, char separator);
extern int label_search(char* label, bool* is_label);
extern bool get_listen_id(char* topic, int* result);
extern void clear_packet(BAS_PACKET* packet);

#endif//__UTIL_H__
