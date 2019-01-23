#ifndef __BAS_COMM_H__
#define __BAS_COMM_H__

//�R�}���h�e�[�u��
typedef struct
{
	char name;
	bool(*execute)(BAS_PACKET_BODY* context);
} BAS_COMM_TABLE;


extern void bas_comm_job(char* recv_message);
extern bool bas_comm_parse(BAS_PACKET* packet, char* msg);

#endif//__BAS_COMM_H__
