#include "config.h"
#include "iot_basic.h"
#include "dictionary.h"
#include "bas_comm.h"
#include "bas_script.h"


void bas_init(void)
{
	dic_clear();
	state.run_no = state.err_code = 0;
}

//ƒRƒ}ƒ“ƒh‰ğÍ
void bas_main(char* recv_message)
{
	bas_comm_job(recv_message);
 	bas_script_job();
}
