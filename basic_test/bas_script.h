#ifndef __BAS_SCRIPT_H__
#define __BAS_SCRIPT_H__

//コマンドテーブル
typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;


extern void bas_script_job(void);

#endif//__BAS_SCRIPT_H__

