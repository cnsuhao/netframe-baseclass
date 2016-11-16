#ifndef __TCP_NETWORK_H__
#define __TCP_NETWORK_H__

#ifdef _WINDOWS
#include <Winsock2.h>
#include <Mswsock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#else
#include <string.h>
#endif //_WINDOWS

#include "../OsalTypedef.h"
#include <boost/shared_ptr.hpp>
#include <map>
using namespace std;

#include "../CriticalSectionMgr.h"

namespace CLivenet5
{

const int RECVBUFSIZE = 4096;

class CSocketIOBase
{
public:
	virtual void OnSocketSendOver(UINT nID)=0;
	virtual BOOL OnSocketReceived(UINT nID,BYTE * pData,DWORD dwLen)=0;
	virtual void OnSocketAccept(UINT nID)=0;
	virtual void OnSocketConnect(UINT nID)=0;
	virtual void OnSocketClose(UINT nID)=0;	 
	virtual BOOL SocketSend(UINT nID,BYTE * pData,DWORD dwLen) = 0;
	virtual void SocketClose(UINT nID) = 0;
	virtual UINT SocketConnect(const char * szRemoteAddr,UINT nContextID) = 0;//nContextID指定一个ID(socket句柄)作为连接的标识.返回值是连接的标识.失败返回NULL
};

class CProtocolSession;
typedef boost::shared_ptr<CProtocolSession> CProtocolSession_Ptr;
class CNetProtocol
{
public:
	CProtocolSession_Ptr GetSession(UINT nID);
	void RemoveAllSession();
	void RemoveSession(UINT nID);
	CNetProtocol(){}
	virtual ~CNetProtocol()
	{
		RemoveAllSession();
	}
	int	GetSessionCount(void);
	void	AddSession(UINT nID,CProtocolSession_Ptr pSession);
	void	CopySession(map<UINT,CProtocolSession_Ptr> & mapSession);
	
	virtual void OnAccept(UINT nID);
	virtual void OnConnect(UINT nID);
	virtual void OnClose(UINT nID);
	virtual BOOL OnReceived(UINT nID,BYTE * pData,DWORD dwLen);
	virtual void OnSendOver(UINT nID);

	virtual	BOOL Send(UINT nID,BYTE * pData,DWORD dwLen)=0;
	virtual void Close(UINT nID) = 0;
	virtual UINT Connect(const char * szRemoteAddr,UINT nContextID) = 0;

protected:
	CCriticalSectionMgr m_NPCSMgr;
	map<UINT,CProtocolSession_Ptr> m_mapSession;
};

class CProtocolSession
{
public:
	void BindNetProtocol(UINT nID,CNetProtocol * pNP){m_nID = nID;m_pNP = pNP;}
	BOOL IsValid()
	{
		try
		{
			if(m_nFlag == 0x32412512)			
				return TRUE;
		}
		catch(...){}
		return FALSE;
	}
	void Disable()
	{
		try
		{
			m_nFlag =0;		
		}
		catch(...){}
	}
	CProtocolSession(){m_nFlag = 0x32412512;}
	virtual ~CProtocolSession()
	{
		m_nFlag = 0;
	}	
	virtual BOOL Send(BYTE * pData,DWORD dwLen){return m_pNP->Send(m_nID,pData,dwLen);}
	BOOL Send(const char *szData)
	{
		if(szData)
#ifdef _WINDOWS
			return Send((BYTE*)szData,lstrlen(szData));
#else
			return Send((BYTE*)szData,strlen(szData));
#endif
		return FALSE;
	}

	virtual void Close(){m_pNP->Close(m_nID);}
	virtual UINT Connect(const char * szRemoteAddr,UINT nContextID = 0){return m_pNP->Connect(szRemoteAddr,nContextID);}
private:
	UINT m_nFlag;
protected:
	CNetProtocol * m_pNP;	
	UINT m_nID;
};


template <class SocketIOT,class ProtocolT,class SessionT>
class CTCPNetwork : public SocketIOT,
					public ProtocolT		
{
	typedef boost::shared_ptr<SessionT> SessionT_Ptr;
public:
	CTCPNetwork(){}
	virtual ~CTCPNetwork()
	{
	}
	virtual void OnSocketSendOver(UINT nID)
	{
		ProtocolT::OnSendOver(nID);
	}
	virtual BOOL OnSocketReceived(UINT nID,BYTE * pData,DWORD dwLen)
	{
		return ProtocolT::OnReceived(nID,pData,dwLen);
	}	
	virtual void OnSocketAccept(UINT nID)
	{	
		CProtocolSession_Ptr pSession(new SessionT);
		ProtocolT::AddSession(nID,pSession);
		ProtocolT::OnAccept(nID);		
	}
	virtual void OnSocketConnect(UINT nID)
	{
		CProtocolSession_Ptr pSession(new SessionT);
		ProtocolT::AddSession(nID,pSession);
		ProtocolT::OnConnect(nID);		
	}

	virtual void OnSocketClose(UINT nID)
	{
		ProtocolT::OnClose(nID);		
		ProtocolT::RemoveSession(nID);
	}
	virtual	BOOL Send(UINT nID,BYTE * pData,DWORD dwLen)
	{
		return SocketIOT::SocketSend(nID,pData,dwLen);
	}
	virtual void Close(UINT nID)
	{
		SocketIOT::SocketClose(nID);
	}
	virtual UINT Connect(const char * szRemoteAddr,UINT nContextID = 0)
	{
		return SocketIOT::SocketConnect(szRemoteAddr,nContextID);
	}
};
    
} //CLivenet5

#endif //__TCP_NETWORK_H__
