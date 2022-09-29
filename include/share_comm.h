/*****************************************************************************
*
* Copyright Next-Generation System Software Research Group, All rights reserved.
* Future Computing Research Division, Artificial Intelligence Reserch Laboratory
* Electronics and Telecommunications Research Institute (ETRI)
*
* THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
* WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
* TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
* REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
* SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
* IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
* COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
*
* LICENSE file : README_LICENSE_ETRI located in the top directory
*
*****************************************************************************/
#ifndef SHARE_COMM_H
#define SHARE_COMM_H

#define MAX_NOS_NUM	32
#define MAX_NNID_NUM	256

#define NTASK_WSMEM_SIZE   64*1024*1024 // 64 MBytes
#define NOS_WSMEM_SIZE 	 64*1024*1024 // 64 MBytes

#define SERVER_MSGQ_KEY         5678

#define SERVER_PORT		8080
#define DATA_SERVER_PORT	9090
#define DATA_SERVER_PORT_MAX	50000

#define DEFAULT_SERVER_IP	"127.0.0.1" // local
//#define SERVER_IP	"129.254.74.241" // cocoa
//#define SERVER_IP	"192.168.0.4" // muse(129.254.74.87)

#define SOCKADDR_LEN	20

// all message types must be positive
#define CL_PREDICT_S1	1
#define CL_PREDICT_S2	2
#define CL_PREDICT	CL_PREDICT_S2
#define CL_PORT		3
#define CL_INFO		4
#define CL_NOS_INFO	5
#define CL_GET_NOS_MASK	6	

#define CL_LOAD		11
#define CL_UNLOAD	12

#define CL_DMKILL  	100

#define OS_CONN		 21
#define OS_DISCONN	 22
#define OS_LOAD_ECHO	 23
#define OS_UNLOAD_ECHO	 24
#define OS_PREDICT_ECHO	 25
#define OS_PREDICT_FINI	 26
#define OS_OUTDATA_ECHO	 27
#define OS_NOS_INFO_ECHO 28

#define ECHO_OK	 (255)
#define ECHO_ERR (999)

////////////// NTASK INPUT DATA TYPE ///////
#define INPUT_DATA_BIT  (0x01)
#define INPUT_FILE_BIT  (0x02)
#define INPUT_DATA_AND_FILE_BIT (0x03)

////////////// INFO NOS COMMAND ID ////////////
#define NOS_VERSION	1
#define NOS_BASEOS	2

////////////// NOS BASE OS /////////////////
#define NON_OS  0
#define LINUX   1
#define RTOS    2

// Client Message Structure 15*4(int) + 1*8(long) = 60+8 = 68 bytes
typedef struct msg_client_predict
{
        long mtype;
        int mqid; // message queue id of client
	int nnid; 	// neural_net type: YOLOV2(1 | CPU_ONLY)
	int input_type;
	int infile_size;
	int indata_start_offset_remote;
	int indata_size;
	int indata_push;
	int outdata_pull;
	int part_num_start;
	int part_num_delta;
	int priority; 	// client/loader task pointer
	unsigned int affinity_mask; // affinity in scheduing
	unsigned int nos_mask; // resulting mask after scheduling
	int port; // communication port
	int osid; // NPU OS id determined by L1 scheduler
} MsgClientPredict;

// For IPC
typedef struct msg_client_echo
{
        long mtype;
	int osid;
	unsigned int mask;
} MsgClientEcho;

// For Sock 
typedef struct sock_msg_client_echo
{
        int mtype;
	int osid;
} SockMsgClientEcho;

typedef struct msg_client_load
{
        long mtype;
	int mqid; 	// loader msgq id
	int nnid; 	// neural_net type: YOLOV2(1 | CPU_ONLY)
	int osid; 	// loader : osid
	int filesize; // filesize if a file exists for load
	int port;
	int part_num_start;
	int part_num_delta;
} MsgClientLoad;

typedef struct msg_client_unload
{
        long mtype;
	int mqid; 	// loader msgq id
	int nnid; 	// neural_net type: YOLOV2(1 | CPU_ONLY)
	int osid; 	// loader : osid
	int filesize;
	int part_num_start;
	int part_num_delta;
} MsgClientUnload;

typedef struct msg_client_dmkill
{
        long mtype;
	unsigned int nos_mask; // nos_mask to be killed
} MsgClientDmkill;

typedef struct msg_client_port
{
	long mtype;
	int mqid; // client mqid
} MsgClientPort;

typedef struct msg_client_port_echo
{
	long mtype;
	int port;
} MsgClientPortEcho;

typedef struct msg_client_nest_info
{
	long mtype;
	int mqid; // client mqid
} MsgClientNestInfo;

typedef struct msg_client_nest_info_echo
{
	long mtype;
	int num_npuos;
} MsgClientNestInfoEcho;

typedef struct msg_client_nest_config_echo
{
	long mtype;
	int osid;
	int wsmem_size;
	char ip[20];
} MsgClientNestConfigEcho;

typedef struct msg_client_nos_info
{
	long mtype;
	int mqid; // client mqid
	int osid;
	int queryid;
} MsgClientNosInfo;

typedef struct msg_client_nos_info_echo
{
	long mtype;
	int query_reply;
} MsgClientNosInfoEcho;

typedef struct msg_server_dmkill
{
	long mtype;
	void *npuos;
} MsgServerDmkill;

// Server Message Structure : Sock
typedef struct sock_msg_server_predict
{
	int mtype;
        int mqid; // message queue id of client
        int nnid; // neural net type, e.g. YOLOV2
	int input_type;
	int infile_size;
	int indata_start_offset_remote;
	int indata_size;
	int indata_push;
	int outdata_pull;
	int part_num_start;
	int part_num_delta;
	int priority;
	int port;
	double prev_wcet;
	void *npuos;
} SockMsgServerPredict;

typedef struct sock_msg_server_load
{
	int mtype;
	int mqid;
	int nnid;
	int filesize;
	int port;
	int part_num_start;
	int part_num_delta;
	void *npuos;
} SockMsgServerLoad;

typedef struct sock_msg_server_unload
{
	int mtype;
	int mqid;
	int nnid;
	int filesize;
	int part_num_start;
	int part_num_delta;
	void *npuos;
} SockMsgServerUnload;

typedef struct sock_msg_server_nos_info
{
	int mtype;
	int mqid;
	int queryid;
	void *npuos;
} SockMsgServerNosInfo;

typedef struct sock_msg_server_dmkill
{
	int mtype;
	void *npuos;
} SockMsgServerDmkill;

///////////////////// OS MESSAGE ///////////////////
// For IPC
#if 0
typedef struct msg_os_connect
{
        long mtype; // 4
	int id;
	int inf_type;
	int msgq;
	int state;
} MsgOSConnect;

typedef struct msg_os_disconnect
{
        long mtype; // 4
	int id;
	int inf_type;
	int msgq;
	int state;
} MsgOSDisconnect;

typedef struct msg_os_echo
{
        long mtype;
	int mqid; // client mqid
	int nnid;
	int osid;
	long rtype;
} MsgOSEcho;

typedef struct msg_os_predict_echo
{
        long mtype;
	int mqid; // client mqid
	int nnid;
	int osid;
	double time;
	long rtype;
} MsgOSPredictEcho;
#endif

// For Sock
typedef struct sock_msg_os_connect
{
        int mtype; // 4
	int id;
	//int inf_type;
	//int msgq;
	int wsmem_size;
	int state;
} SockMsgOSConnect;

typedef struct sock_msg_os_disconnect
{
        int mtype; // 4
	int id;
	//int inf_type;
	int msgq;
	int state;
} SockMsgOSDisconnect;

typedef struct sock_msg_os_echo
{
        int mtype;
	int mqid; // client mqid
	int nnid;
	int osid;
	int part_num_start;
	int part_num_delta;
	int rtype;
} SockMsgOSEcho;

typedef struct sock_msg_os_predict_echo
{
        int mtype;
	int mqid; // client mqid
	int nnid;
	int part_num_start;
	int part_num_delta;
	int osid;
	double prev_wcet;
	double curr_wcet;
	int rtype;
} SockMsgOSPredictEcho;

typedef struct sock_msg_os_outdata_echo
{
	int mtype;
	int nnid;
	int osid;
	int data_offset;
	int data_size;
} SockMsgOSOutdataEcho;

typedef struct sock_msg_os_nos_info_echo
{
	int mtype;
	int mqid;
	int osid;
	int query_reply;
	int rtype;
} SockMsgOSNosInfoEcho;

///////////////////// COMMON MESSAGE TYPE for RECEIVER //////////////
#define MESSAGE_SIZE	124 	// 124 bytes reserved for message

typedef struct message
{
        long mtype;
	char pbyte[MESSAGE_SIZE]; 
} Message;

typedef struct sock_message
{
        int mtype;
	char pbyte[MESSAGE_SIZE]; 
} SockMessage;

#endif
