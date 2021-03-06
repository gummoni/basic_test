#ifndef __BAS_COMM_H__
#define __BAS_COMM_H__

//パケット解析ステート
#define RECV_STATE_FROM		0
#define RECV_STATE_TO		1
#define RECV_STATE_COMMAND	2
#define RECV_STATE_CORON	3
#define RECV_STATE_PRM1		4
#define RECV_STATE_PRM2		5
#define RECV_STATE_PRM3		6
#define RECV_STATE_PRM4		7
#define RECV_STATE_PRM5		8
#define RECV_STATE_PRM6		9
#define RECV_STATE_PRM7		10
#define RECV_STATE_ERROR	254

extern void bas_comm_job(BAS_PACKET* packet);
extern void clear_packet(BAS_PACKET* packet);

#endif//__BAS_COMM_H__
