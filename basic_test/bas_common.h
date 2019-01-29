#ifndef __UTIL_H__
#define __UTIL_H__

//	コマンドフォーマット
//	[送元],[宛先],[CMD]:[ADDR],([PRM1],[PRM2])[\n]

//プログラム領域
#define PROGRAM_LINE_COUNT	32
#define PROGRAM_LINE_MAX	200
extern char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT];
#define SELF_NAME	program_areas[0]



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

#endif//__UTIL_H__
