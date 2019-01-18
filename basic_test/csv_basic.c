#include <stdbool.h>
#include "csv_basic.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "bas_comm.h"
#include "bas_script.h"
#include "bas_packet.h"

//24文字x100行のプログラムエリア
char program_areas[PROGRAM_LINE_MAX][PROGRAM_LINE_COUNT] =
{
	//リッスン
	"AXIS_Z1",			// 0: UNIQUE_NAME（自分宛、返事を返す）
	"AXIS_Z",			// 1: LISTEN1(NOTIFY)
	"",				    // 2: LISTEN2(NOTIFY)
	"",				    // 3: LISTEN3(NOTIFY)
	"",					// 4: LISTEN4(NOTIFY)
	"",					// 5: LISTEN5(NOTIFY)
	"",					// 6: LISTEN6(NOTIFY)
	"",					// 7: LISTEN7(NOTIFY)
	"",					// 8: LISTEN8(NOTIFY)
	"",					// 9: LISTEN9(NOTIFY)
	//プログラム領域
	"",					//10: 
	"",					//11: 
	"",					//12: 
	"",					//13: 
	"",					//14: 
	"",					//15: 
	"",					//16: 
	"",					//17: 
	"",					//18: 
	"",					//19: 
	"",					//20: 
	"",					//21: 
	"REPORT,$STATUS",   //22: 
	"QUIT",				//23: 
	"*K1",				//24: 
	"IF,I=1,10,*K1",	//25: 
	"",					//26: 
	"",					//27: 
	"",					//28: 
	"",					//29: 
	"",					//30: 
	"",					//31: 
	"",					//32: 
	"",					//33: 
	"",					//34: 
	"",					//35: 
	"",					//36: 
	"",					//37: 
	"",					//38: 
	"",					//39: 
	"",					//40: 
	"",					//41: 
	"",					//42: 
	"",					//43: 
	"",					//44: 
};


//コマンド解析
void bas_main(char* recv_message)
{
	printf(">");
	printf(recv_message);
	printf("<");
	bas_comm_job(recv_message);
 	bas_script_job();
}
