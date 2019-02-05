#include "config.h"
#include "serial.h"

static BAS_PACKET serial_packet;


//転送データ
static char* ser_msgs[] = {
	//変数（数字）計算テスト
	"SENDER,AXIS_Z1,I:I=5+(3*(1+2)+1)*2+4\n",
	"SENDER,AXIS_Z1,I:I=I*2+1\n",
	"SENDER,AXIS_Z1,I:I\n",

	//変数（文字列）計算テスト
	"SENDER,AXIS_Z1,I:J$=\"12+34\"\n",
	"SENDER,AXIS_Z1,I:J$=\"GOOD=\" + J$\n",
	"SENDER,AXIS_Z1,I:J$\n",

	//IF、GOSUB（引数）、Invoke、RETURN、GOTOテスト
	"SENDER,AXIS_Z1,W:20,INVOKE USER I\n",
	"SENDER,AXIS_Z1,W:21,INVOKE USER J$\n",
	"SENDER,AXIS_Z1,W:30,I=0\n",
	"SENDER,AXIS_Z1,W:31,IF I==11 *FIN1 42\n",
	"SENDER,AXIS_Z1,W:32,I=1\n",
	"SENDER,AXIS_Z1,W:33,GOSUB *TEST1 41\n",
	"SENDER,AXIS_Z1,W:34,GOTO 41\n",
	"SENDER,AXIS_Z1,W:35,*TEST1 J\n",
	"SENDER,AXIS_Z1,W:36,INVOKE USER I\n",
	"SENDER,AXIS_Z1,W:37,I=J\n",
	"SENDER,AXIS_Z1,W:38,INVOKE USER I\n",
	"SENDER,AXIS_Z1,W:39,RETURN\n",
	"SENDER,AXIS_Z1,W:40,GOTO 42\n",
	"SENDER,AXIS_Z1,W:41,*FIN1\n",
	"SENDER,AXIS_Z1,W:42,C$=\"A==1 & B==1\"\n",
	"SENDER,AXIS_Z1,W:43,IF C$ 44 45\n",
	"SENDER,AXIS_Z1,W:44,END\n",
	"SENDER,AXIS_Z1,W:45,A=1\n",
	"SENDER,AXIS_Z1,W:46,B=1\n",
	"SENDER,AXIS_Z1,W:47,GOTO 42\n",
	"SENDER,AXIS_Z1,S:10\n",

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
