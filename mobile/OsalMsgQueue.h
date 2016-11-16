#ifndef __OSAL_MSG_QUEUE_H__
#define __OSAL_MSG_QUEUE_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
//Android 1.5 has IPC msg
//        1.6+ had removed it.


#if !defined(ANDROID) && !defined(OS_IOS)
#include <sys/msg.h>
#else
#include <pthread.h>

typedef struct _msgq_moitor_tag {
	int msg_cnt;
	pthread_mutex_t msg_mutex;
	pthread_cond_t msg_cond;
} msgq_moitor_t;

typedef struct _msgq_dispatch_tag {
	int pdm_cnt;
	pthread_mutex_t pdm_mutex;
	pthread_cond_t pdm_cond;
} msgq_dispatch_t;

typedef struct _msgq_post_tag {
	int spm_cnt;
	pthread_mutex_t spm_mutex;
	pthread_cond_t spm_cond;
} msgq_post_t;
#endif

#define OSAL_MESSAGE_LEN 256

#define OSAL_MESSAGE_MAIN          0x0001
#define OSAL_MESSAGE_MSGMONITOR    0x0002
#define OSAL_MESSAGE_DFIACT        0x0004
#define OSAL_MESSAGE_HOOKFILE      0x0008
#define OSAL_MESSAGE_DISPATCHER    0x0010
#define OSAL_MESSAGE_UDPRECEIVER   0x0020
#define OSAL_MESSAGE_UPDATER       0x0040
#define	OSAL_MESSAGE_VERIFY        0x0080
#define OSAL_MESSAGE_POST          0x0100
#define OSAL_MESSAGE_BIP           0x0200

#define PpsPostMessage(msgkey,to,from,msgtype,wparam,lparam)         \
    do {                                                             \
		OsalMsg msg;                                         \
		int msgID;                                           \
                                                                     \
		msgID = OsalMsgGetID(msgkey);                        \
		memset(&msg, 0, sizeof(OsalMsg));                    \
		msg.receiver = to;                                   \
		msg.sender = from;                                   \
		msg.msgType = msgtype;                               \
		msg.wParam = (unsigned long)wparam;                   \
		msg.lParam = (unsigned long)lparam;                   \
		OsalMsgSend(msgID, &msg);                            \
	}while(0)

#define PpsPostMessage2(msgkey,to,from,msgtype,wparam,lparam,message,msgsize) \
    do {                                                             \
		OsalMsg msg;                                         \
		int msgID;                                           \
                                                                     \
		msgID = OsalMsgGetID(msgkey);                        \
		memset(&msg, 0, sizeof(OsalMsg));                    \
		msg.receiver = to;                                   \
		msg.sender = from;                                   \
		msg.msgType = msgtype;                               \
		msg.wParam = (unsigned long)wparam;                   \
		msg.lParam = (unsigned long)lparam;                   \
		memcpy(msg.extra, message, msgsize);                 \
		OsalMsgSend(msgID, &msg);                            \
	}while(0)
extern key_t gMsgKey;
extern key_t gpdmk;
extern key_t gspmk;

typedef struct _OsalMsg{
	long receiver;                     // Destination id
	int  sender;
	int  msgType;
#ifdef __64BITS__
	unsigned long wParam;
	unsigned long lParam;
#else
	unsigned int wParam;
	unsigned int lParam;
#endif
	char extra[OSAL_MESSAGE_LEN];
}OsalMsg;

void ResetMsgPool();
int OsalMsgDestroy(key_t key);

int OsalMsgSend(int msgid, void *msgp);
int OsalMsgRecv(int msgid, int msgtype, void *msgp);

inline int OsalMsgCreate(key_t* key)
{
#if (!defined(ANDROID) && !(defined(OS_IOS)))
	int ret;

	if(key == NULL)
	{
		//printf("OsalMsgCreate's key param. is NULL!\n");
		return -1;
	}	

	ret = msgget(*key, IPC_CREAT | IPC_EXCL | 0660);
	//perror("Osal message queue creation FIRST time failed.");
	if (ret < 0)
	{
		ret = OsalMsgDestroy(*key);
		if(ret != 0)
		{
			*key ++;
		}

		if ((ret = msgget(*key, IPC_CREAT | IPC_EXCL | 0660)) < 0)
		{
			//perror("Osal message queue creation failed.");
			return -1;
		}
		return ret;
	}
	return ret;
#else
	return *key;
#endif
}

inline int OsalMsgGetID(key_t key)
{
//#ifndef OS_ANDROID
#if (!defined(ANDROID) && !(defined(OS_IOS)))
	return msgget(key, 0);
#else
	return key;
#endif
}

inline int OsalMsgDestroy(key_t key)
{
	int ret;

//#ifndef OS_ANDROID
#if (!defined(ANDROID) && !(defined(OS_IOS)))
	int mqid = OsalMsgGetID(key);
	
	ret = msgctl(mqid, IPC_RMID, NULL);
	if(ret != 0 )
	{
		perror("Osal message queue destroy failed.");
	}
#else  //Always succeed
	ResetMsgPool();	
	ret = 0;
#endif
	return ret;
}


#endif /* __OSAL_MSG_QUEUE_H__ */
