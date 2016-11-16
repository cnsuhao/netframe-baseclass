// EventSelectSocket.h: interface for the CSocketEventSelectEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTSELECTSOCKET_H__D30FED9A_ADDC_4C0E_9317_C2CD5CC9CD07__INCLUDED_)
#define AFX_EVENTSELECTSOCKET_H__D30FED9A_ADDC_4C0E_9317_C2CD5CC9CD07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TCPNetwork.h"
#include "../HostInfo.h"

#include <set>
#include <list>
using namespace std;

const int iMaxSocketCount = 40;
//const int iMaxSocketCount = 62;
class CSocketEventSelectEx;

class CSocketEventSelectThread /*: public CThreadObject*/
{
public:	
	int Release();
	int AddRef();
	class CClientContext
	{
	public:
		SOCKET Socket;
		UINT Context;
		WSAEVENT Event;		
		BYTE * pTransmittedBuffer;
		DWORD dwTransmittedLen;
		BYTE * pRecvBuffer;
		DWORD dwRecvBytes;
		BOOL bCanSend;
		list<WSABUF> listSendBuf;
		CSocketEventSelectThread * pThread;
	};
	typedef map<WSAEVENT,CClientContext*> CMapIDContext;
	typedef map<UINT,WSAEVENT> CMapIDEvent;
	typedef list<WSAEVENT> CListContext;

	friend class CSocketEventSelectEx;
	CSocketEventSelectThread();
	virtual ~CSocketEventSelectThread();
	virtual	DWORD ThreadWorkFunc();	
private:
	static THREAD_RETURN_TYPE DriverThread(void* param);
	void DriverProc(void);
private:
	THREAD_HANDLE_TYPE driver_thread_;
	volatile bool							running_;	 
public:
	bool  Start(const char * szName);
	void Stop(HANDLE hEvent,int iTimeout);

protected:
	UINT SocketConnect(const char * szRemoteAddr,UINT nContextID = 0);
	BOOL SocketSend(UINT nID,BYTE * pData,DWORD dwLen);
	void SocketClose(UINT nID);
private:	
	CSocketEventSelectThread::CClientContext * GetClientContext(WSAEVENT hEvent);
	const WSAEVENT * GetEventTable();
	int GetEventCount();
	WSAEVENT GetEvent(UINT nCookie);
	WSAEVENT InsertSocket(SOCKET s,UINT nCookie);
	void RemoveSocket(WSAEVENT hEvent,BOOL bForceClose = TRUE);
	void CloseAllClient();
private:
	CCriticalSectionMgr m_SocketCSMgr;	//保护m_hEvent,m_mapClientSocket,m_mapIDEvent以及CClientContext的listSendBuf
	volatile BOOL	m_bStop;
	volatile BOOL	m_bEventTableDirty;
	volatile BOOL	m_bActive;

	WSAEVENT m_hEvent[64];//事件数组
//	CListContext m_listContext;//上下文链表
	CMapIDContext m_mapClientSocket;//通过事件找到上下文关联
	CMapIDEvent m_mapIDEvent;//通过id找到事件
	DWORD m_dwCurSecondSendBytes;
	DWORD m_dwCurSecondRecvBytes;
	
	CCriticalSectionMgr m_CloseCSMgr;	//保护m_setClosed
	set<WSAEVENT> m_setClosed;//已经关闭的socket关联事件列表
	CCriticalSectionMgr m_RecvCSMgr;	//保护m_setRecv
	set<WSAEVENT> m_setRecv;//需要再次投递接收请求的socket列表		
	volatile int		m_iSocketLimit;//允许的连接个数		
	volatile long m_iRef;
	CSocketEventSelectEx * m_pSocketEventSelectEx;
};
class CSocketEventSelectEx : public CLivenet5::CSocketIOBase
{
public:			
	friend class CSocketEventSelectThread;
	void RemoveThread(CSocketEventSelectThread * pThread);
	BOOL IsActive();
	CSocketEventSelectEx();
	virtual ~CSocketEventSelectEx();
	
	WORD		Start(DWORD ip,WORD wPort,BOOL bForce = TRUE);
	void	Stop();
	virtual UINT SocketConnect(const char * szRemoteAddr,UINT nContextID = 0);
	virtual BOOL SocketSend(UINT nID,BYTE * pData,DWORD dwLen);
	virtual void SocketClose(UINT nID);
	static UINT CreateContextID();
	static void ReleaseContextID(UINT nID);	
private:
	CSocketEventSelectThread* GetIDThread(UINT nID);
	int		Listen(DWORD ip,WORD wPort,BOOL bForce);	
	void NotifyThread(UINT nID);
	WSAEVENT InsertSocket(SOCKET s,UINT nCookie);
private:
	CCriticalSectionMgr m_csset;
	set<CSocketEventSelectThread*> m_setThread;//处理网络事件的线程集合
	CCriticalSectionMgr m_csmap;
	map<UINT,CSocketEventSelectThread*> m_mapIDThread;//通过ID找到其处理线程
	volatile BOOL m_bStart;
	volatile BOOL m_bStop;	
};

#endif // !defined(AFX_EVENTSELECTSOCKET_H__D30FED9A_ADDC_4C0E_9317_C2CD5CC9CD07__INCLUDED_)

