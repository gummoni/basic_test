#ifndef __BAS_SCRIPT_H__
#define __BAS_SCRIPT_H__

//�R�}���h�e�[�u��
typedef struct
{
	char* name;
	void(*execute)(BAS_PACKET*);
} BAS_SCRIPT_TABLE;


extern void bas_script_job(void);

#endif//__BAS_SCRIPT_H__

