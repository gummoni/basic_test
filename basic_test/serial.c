#include "config.h"
#include "serial.h"

static BAS_PACKET serial_packet;


//転送データ
static char* ser_msgs[] = {
	//IF、GOSUB（引数）、Invoke、RETURN、GOTOテスト
	"SENDER,AXIS_Z1,W:10,I=0\n",
	"SENDER,AXIS_Z1,W:11,IF I==11 *FIN1 21\n",
	"SENDER,AXIS_Z1,W:12,I=1\n",
	"SENDER,AXIS_Z1,W:13,GOSUB *TEST1 11\n",
	"SENDER,AXIS_Z1,W:14,GOTO 11\n",
	"SENDER,AXIS_Z1,W:15,*TEST1 J\n",
	"SENDER,AXIS_Z1,W:16,INVOKE USER I\n",
	"SENDER,AXIS_Z1,W:17,I=J\n",
	"SENDER,AXIS_Z1,W:18,RETURN\n",
	"SENDER,AXIS_Z1,W:19,*FIN1\n",
	"SENDER,AXIS_Z1,W:20,RETURN\n",
	"SENDER,AXIS_Z1,W:21,GOTO 12\n",
	"SENDER,AXIS_Z1,S:10\n",



	//"SENDER,AXIS_Z1,W:10,I=0\n",
	//"SENDER,AXIS_Z1,W:11,IF I<>11 *TEST\n",
	//"SENDER,AXIS_Z1,W:12,I=1\n",
	//"SENDER,AXIS_Z1,W:13,INVOKE USER I\n",
	//"SENDER,AXIS_Z1,W:14,GOSUB 22\n",
	//"SENDER,AXIS_Z1,W:15,*TEST\n",
	//"SENDER,AXIS_Z1,W:16,GOSUB *TEST2 11\n",
	//"SENDER,AXIS_Z1,W:17,\n",
	//"SENDER,AXIS_Z1,W:18,RETURN\n",
	//"SENDER,AXIS_Z1,W:19,*TEST2 J\n",
	//"SENDER,AXIS_Z1,W:20,I=J\n",
	//"SENDER,AXIS_Z1,W:21,RETURN\n",
	//"SENDER,AXIS_Z1,W:22,A=1\n",
	//"SENDER,AXIS_Z1,W:23,RETURN\n",
	//"SENDER,AXIS_Z1,W:24,C$=\"A==1 AND B==1\"",
	//"SENDER,AXIS_Z1,W:25,IF C$ 26 27",
	//"SENDER,AXIS_Z1,W:26,RETURN\n",
	//"SENDER,AXIS_Z1,W:27,RETURN\n",
	//"SENDER,AXIS_Z1,S:10\n",


		////変数（数字）計算テスト
		//"SENDER,AXIS_Z1,I:I=5+(3*(1+2)+1)*2+4\n",
		//"SENDER,AXIS_Z1,I:I=I*2+1\n",
		//"SENDER,AXIS_Z1,I:I\n",

		////変数（文字列）計算テスト
		//"SENDER,AXIS_Z1,I:J$=\"12+34\"\n",
		//"SENDER,AXIS_Z1,I:J$=\"GOOD=\" + J$\n",
		//"SENDER,AXIS_Z1,I:J$\n",

		////アドレスRWテスト
		//"SENDER,AXIS_Z1,R:10\n",
		//"SENDER,AXIS_Z1,W:10,I=0\n",
		//"SENDER,AXIS_Z1,R:10\n",
		//"SENDER,AXIS_Z1,W:11,I=I+1\n",
		//"SENDER,AXIS_Z1,W:12,IF I<10 11 *JMP\n",
		//"SENDER,AXIS_Z1,W:13,*JMP\n",
		//"SENDER,AXIS_Z1,W:14,INVOKE USER I\n",
		//"SENDER,AXIS_Z1,W:15,GOSUB 20\n",
		//"SENDER,AXIS_Z1,W:16,GOTO 30\n",
		//"SENDER,AXIS_Z1,W:20,INVOKE USER I\n",
		//"SENDER,AXIS_Z1,W:21,RETURN\n",
		//"SENDER,AXIS_Z1,W:30,RETURN\n",

		////プログラム実行テスト
		//"SENDER,AXIS_Z1,S:10\n",
		//-----------------------------
		NULL,
};
static int ser_i = 0;

//1行読込み
bool serial_read(char** msg, int* length)
{
	*msg = ser_msgs[ser_i];
	if (NULL != *msg)
	{
		ser_i++;
		*length = strlen(*msg);
		return true;
	}
	return false;
}

//データ送信
void serial_write(char* msg, int length)
{
	printf("SER:%s", msg);
}
