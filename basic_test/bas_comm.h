#ifndef __BAS_COMM_H__
#define __BAS_COMM_H__

//�R�}���h�e�[�u��
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET* context);
} BAS_COMM_TABLE;


extern bool bas_comm_execute(BAS_PACKET* packet);


#endif//__BAS_COMM_H__
