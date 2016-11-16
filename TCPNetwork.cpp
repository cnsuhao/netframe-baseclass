#include "stdafx.h"
#include "TCPNetwork.h"

namespace CLivenet5
{

int	CNetProtocol::GetSessionCount(void)
{
	CCriticalSection lock(&m_NPCSMgr);
	return static_cast<int>(m_mapSession.size());
}

void CNetProtocol::AddSession(UINT nID,CProtocolSession_Ptr pSession)
{
	if(pSession)
	{
		CCriticalSection cs(&m_NPCSMgr);
		pSession->BindNetProtocol(nID,this);
		map<UINT,CProtocolSession_Ptr>::iterator it;
		it = m_mapSession.find(nID);
		if(it != m_mapSession.end())
		{
			//Session被覆盖
			it->second = pSession;
		}
		else
			m_mapSession.insert(pair<UINT,CProtocolSession_Ptr>(nID,pSession));
	}
}
void CNetProtocol::CopySession(map<UINT,CProtocolSession_Ptr> & mapSession)
{
	CCriticalSection cs(&m_NPCSMgr);	
	mapSession = m_mapSession;
}
void CNetProtocol::RemoveSession(UINT nID)
{
	CCriticalSection cs(&m_NPCSMgr);
	map<UINT,CProtocolSession_Ptr>::iterator it;
	it = m_mapSession.find(nID);
	if(it != m_mapSession.end())
	{
		it->second->Disable();
		m_mapSession.erase(it);		
	}
}
void CNetProtocol::RemoveAllSession()
{
	CCriticalSection cs(&m_NPCSMgr);
	map<UINT,CProtocolSession_Ptr>::iterator it;
	for(it = m_mapSession.begin();it!=m_mapSession.end();++it)
	{
		it->second->Disable();
	}
	m_mapSession.clear();
}
CProtocolSession_Ptr CNetProtocol::GetSession(UINT nID)
{
	CProtocolSession_Ptr sessPtr;
	CCriticalSection cs(&m_NPCSMgr);
	map<UINT,CProtocolSession_Ptr>::iterator it;
	it = m_mapSession.find(nID);
	if(it != m_mapSession.end())
	{
		sessPtr = it->second;		
	}
	return sessPtr;
}

void CNetProtocol::OnAccept(UINT nID)
{
}

void CNetProtocol::OnConnect(UINT nID)
{
}

void CNetProtocol::OnClose(UINT nID)
{
}

BOOL CNetProtocol::OnReceived(UINT nID,BYTE * pData,DWORD dwLen)
{
	return TRUE;
}

void CNetProtocol::OnSendOver(UINT nID)
{
}
    
}

