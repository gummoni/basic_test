#include "config.h"
#include "csv_basic.h"
#include "bas_packet.h"
#include "rpn.h"

//転送データ
static char* msgs[] = {
	//IFテスト
	"SENDER,AXIS_Z1,W:10,I=0",
	"SENDER,AXIS_Z1,W:11,IF I<>0 *TEST",
	"SENDER,AXIS_Z1,W:12,I=1",
	"SENDER,AXIS_Z1,W:13,I=2",
	"SENDER,AXIS_Z1,W:14,END",
	"SENDER,AXIS_Z1,W:15,*TEST",
	"SENDER,AXIS_Z1,W:16,GOSUB *TEST2 11",
	"SENDER,AXIS_Z1,W:17,GOTO 13",
	"SENDER,AXIS_Z1,W:18,RETURN",
	"SENDER,AXIS_Z1,W:19,*TEST2 J",
	"SENDER,AXIS_Z1,W:20,I=J",
	"SENDER,AXIS_Z1,W:21,RETURN",
	"SENDER,AXIS_Z1,S:10",
	
	//変数（数字）計算テスト
	"SENDER,AXIS_Z1,N:I=5+(3*(1+2)+1)*2+4\n",
	"SENDER,AXIS_Z1,N:I=I*2+1\n",
	"SENDER,AXIS_Z1,N:I\n",
	
	//変数（文字列）計算テスト
	"SENDER,AXIS_Z1,N:J$=\"12+34\"\n",
	"SENDER,AXIS_Z1,N:J$=\"GOOD=\" + J$\n",
	"SENDER,AXIS_Z1,N:J$\n",
	
	//アドレスRWテスト
	"SENDER,AXIS_Z1,R:10\n",
	"SENDER,AXIS_Z1,W:10,I=0\n",
	"SENDER,AXIS_Z1,R:10\n",
	"SENDER,AXIS_Z1,W:11,I=I+1",
	"SENDER,AXIS_Z1,W:12,IF I<10 11 *JMP\n",
	"SENDER,AXIS_Z1,W:13,*JMP\n",
	"SENDER,AXIS_Z1,W:14,NOTIFY USER I\n",
	"SENDER,AXIS_Z1,W:15,GOSUB 20\n",
	"SENDER,AXIS_Z1,W:16,GOTO 30\n",
	"SENDER,AXIS_Z1,W:20,NOTIFY USER I\n",
	"SENDER,AXIS_Z1,W:21,RETURN\n",
	"SENDER,AXIS_Z1,W:30,END\n",
	
	//プログラム実行テスト
	"SENDER,AXIS_Z1,S:10",
	//-----------------------------
	NULL,
};

void main() {
	char msg[64];

	bas_init();

	//プログラム書込み&実行
	for (int i = 0; msgs[i] != NULL; i++)
	{
		printf("%d:", state.run_no);
		strcpy(msg, msgs[i]);
		bas_main(msg);
		printf("\n");
	}

	//スクリプト実行中
	static char* status_read = "S,AXIS_Z1,Z:\n";
	while (0 < state.run_no)
	{
		printf("%d:", state.run_no);
		strcpy(msg, status_read);
		bas_main(msg);
		printf("\n");
	}
	printf("\n\n\n\n\n\n\n\n");
 }
