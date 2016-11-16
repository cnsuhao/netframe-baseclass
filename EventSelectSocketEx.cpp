// EventSelectSocket.cpp: implementation of the CSocketEventSelectEx class.
//
//////////////////////////////////////////////////////////////////////

#include "EventSelectSocketEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define PRINTINFO(x)
// char szBufpi[100];sprintf(szBufpi,"\r\n%d %s",::GetCurrentThreadId(),x);OutputDebugString(szBufpi)
CSocketEventSelectEx::CSocketEventSelectEx()
{
	PRINTINFO("CSocketEventSelectEx");
	m_bStart = FALSE;
	m_bStop = FALSE;
	
}

CSocketEventSelectEx::~CSocketEventSelectEx()
{
	PRINTINFO("~CSocketEventSelectEx");
}

THREAD_RETURN_TYPE CSocketEventSelectThread::DriverThread(void* param)
{
    CLivenet5::SetLivenetThreadName("L5::EventSelect::Drive");
	CSocketEventSelectThread*	pThis	= reinterpret_cast<CSocketEventSelectThread*>(param);
	if(pThis)
	{
		pThis->DriverProc();
	}
	return THREAD_RETURN_VALUE;
}

void CSocketEventSelectThread::DriverProc(void)
{
	  ThreadWorkFunc();
}

bool  CSocketEventSelectThread::Start(const char * szName)
{
	running_	= true;
	if(driver_thread_ == THREAD_HANDLE_INIT_VALUE)
	{
#ifdef _WINDOWS
		unsigned threadId=0;
		driver_thread_	= (HANDLE)_beginthreadex(NULL, 0, DriverThread, this, 0, &threadId);
#else
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);

		int iCreate = pthread_create(&driver_thread_,&attr,CSocketEventSelectThread::DriverThread,this);
		if (iCreate!=0)
		{
			if (errno == EAGAIN)
			{

			}
		}
		pthread_attr_destroy(&attr);
#endif
	}

	return true;
}

void CSocketEventSelectThread::Stop(HANDLE hEvent,int iTimeout)
{
#ifdef _WINDOWS
	if(THREAD_HANDLE_INIT_VALUE!=driver_thread_ && driver_thread_ != INVALID_HANDLE_VALUE)
	{
		if(hEvent)
			SetEvent(hEvent);
		running_	= false;
		
		for (int i = 0; i < 5; ++i)
        {
            if(WaitForSingleObject(driver_thread_, 300) == WAIT_OBJECT_0)
            {
                break;
            }
            else
            {
                if(hEvent)
                    SetEvent(hEvent);
                running_	= false;
            }
        }
        WaitForSingleObject(driver_thread_, INFINITE);
		CloseHandle(driver_thread_);
		driver_thread_	= THREAD_HANDLE_INIT_VALUE;
	}
#else
	if(THREAD_HANDLE_INIT_VALUE!=driver_thread_ )
	{
		if(hEvent)
			SetEvent(hEvent);
		running_	= false;
		pthread_join(driver_thread_,NULL);
		driver_thread_	= THREAD_HANDLE_INIT_VALUE;
	}
#endif
}

//侦听线程
DWORD CSocketEventSelectThread::ThreadWorkFunc()
{
	//目前的网络线程,在关闭SOCKET的时候,会将未接受数据接受完毕,这样导致关闭不了SOCKET,需要纠正此错误
	PRINTINFO("ThreadWorkFunc");
	WSANETWORKEVENTS we;
	int iCurConnectCount = 0;
	DWORD dwFlag;
	int iPort = 0;
	long nCookie = 0;
	CClientContext * pcc;
	int iTimes = 0;
	//在这里开始线程工作循环
//	char szInfo[200];
	DWORD dwLastWorkTick = GetTickCount();
	DWORD dwCurWorkTick = dwLastWorkTick;
	DWORD dwStartWorkTick = dwLastWorkTick;
	m_dwCurSecondSendBytes = 0;	
	m_dwCurSecondRecvBytes = 0;
	while(running_)
	{
		const WSAEVENT * hEvent = NULL;
		try
		{
			hEvent = GetEventTable();
		}
		catch(...)
		{
			m_bActive = FALSE;
			return NULL;
		}
		int iEventCount = GetEventCount();
		dwFlag=WSAWaitForMultipleEvents(iEventCount,hEvent,FALSE,100,FALSE);
		if(dwFlag == WSA_WAIT_FAILED)
		{
			if(GetLastError()==6)
			{
				m_bEventTableDirty = TRUE;
				if(m_bStop)
					break;
				continue;
			}
			break;
		}
		dwCurWorkTick = GetTickCount();
		if((dwCurWorkTick-dwLastWorkTick)>=1000)
		{
			DWORD dwTmpErrorCode = WSAGetLastError();
			dwLastWorkTick = dwCurWorkTick;
			///////////////////////////////////////////////////////////
			if(m_dwCurSecondSendBytes || m_dwCurSecondRecvBytes)
			{
				//sprintf(szInfo,"\nTime:%d send:%d,recv:%d",GetTickCount()-dwStartWorkTick,m_dwCurSecondSendBytes,m_dwCurSecondRecvBytes);
				//OutputDebugString("time");				
			}
			m_dwCurSecondSendBytes = 0;	
			m_dwCurSecondRecvBytes = 0;
			///////////////////////////////////////////////////////////
			set<WSAEVENT> setRecv;
			set<WSAEVENT>::iterator itRecv;
			m_RecvCSMgr.lock();	
			setRecv = m_setRecv;
			m_setRecv.clear();			
			m_RecvCSMgr.unlock();
			for(itRecv = setRecv.begin();itRecv!=setRecv.end();++itRecv)
			{
				pcc = GetClientContext((*itRecv));
				if(pcc)
				{
					DWORD dwRecvBytes = 0;
					DWORD dwFlags = 0;
					WSABUF buf;
					buf.buf = (char*)pcc->pRecvBuffer;
					buf.len = pcc->dwRecvBytes;					
					WSASetLastError(0);
					BOOL bNeedRecv = TRUE;
					while(bNeedRecv)
					{
						bNeedRecv = m_pSocketEventSelectEx->OnSocketReceived(pcc->Context,(BYTE*)buf.buf,dwRecvBytes);
						if(bNeedRecv)
						{
							if(dwRecvBytes && dwRecvBytes<buf.len)
							{
								//bNeedRecv = FALSE;
								break;
							}
							if(WSARecv(pcc->Socket, &buf, 1,&dwRecvBytes,&dwFlags, NULL, NULL)== SOCKET_ERROR || dwRecvBytes==0)
								break;
							buf.buf[dwRecvBytes]=0;
							m_dwCurSecondRecvBytes +=dwRecvBytes;
						}
					}
					int iError = WSAGetLastError();
					if(iError>=WSABASEERR && iError!=WSAEWOULDBLOCK)
					{
						m_pSocketEventSelectEx->SocketClose(pcc->Context);
						continue;
					}
					if(!bNeedRecv)//由于速度控制或者缓冲区控制而终止接收.这时候,缓存区是存在数据的.应该在某一时刻再次投递接收请求
					{
						//所以,在这里控制记录该socket的ID,以便于在定时器到达的时候检查再次接收数据
						m_RecvCSMgr.lock();
						m_setRecv.insert(pcc->Event);
						m_RecvCSMgr.unlock();
					}
				}
			}
			setRecv.clear();
			WSASetLastError(dwTmpErrorCode);
		}
		if(dwFlag != WSA_WAIT_TIMEOUT)
		{
			int iID = dwFlag - WSA_WAIT_EVENT_0;
			if(iID<iEventCount)
			{
				if(iID==0)
				{
					WSAResetEvent(m_hEvent[0]);					
					set<WSAEVENT>::iterator itClosed;
					set<WSAEVENT> setClosed;
					m_CloseCSMgr.lock();
					setClosed = m_setClosed;
					m_setClosed.clear();
					m_CloseCSMgr.unlock();
					BOOL bStop = m_bStop;
					for(itClosed = setClosed.begin();itClosed!=setClosed.end();++itClosed)
						RemoveSocket((*itClosed));
					setClosed.clear();

					if(bStop||m_pSocketEventSelectEx->m_bStop)
					{
						BOOL bDelThread = FALSE;
						m_SocketCSMgr.lock();
						if(m_mapClientSocket.size()==0)
							bDelThread = TRUE;
						m_SocketCSMgr.unlock();		
						if(bDelThread)
							break;				
					}
						/*

					if(bDelThread)
					{
						Release();						
						break;
					}
					//*/
					continue;
				}
				memset(&we,0,sizeof(we));
				pcc = GetClientContext(hEvent[iID]);
				if(!pcc)
					break;
				WSAEnumNetworkEvents(pcc->Socket,pcc->Event,&we);

				//下面是处理网络事件.
				if(we.lNetworkEvents&FD_ACCEPT)
				{
					if(we.iErrorCode[FD_ACCEPT_BIT])
						break;
					SOCKET s = accept(pcc->Socket,NULL,NULL);
					if(s == INVALID_SOCKET)
						break;
					if(!m_bStop&&m_pSocketEventSelectEx->InsertSocket(s,(UINT)s))//处于停止状态
						m_pSocketEventSelectEx->OnSocketAccept((UINT)s);
					else
						closesocket(s);
					continue;
				}
				if(we.lNetworkEvents&FD_CONNECT)
				{	
					//如果连接失败了就直接调用OnSocketClose事件，而不需要调用OnSocketConnect事件
					if(pcc->Context && we.iErrorCode[FD_CONNECT_BIT] == 0)
						m_pSocketEventSelectEx->OnSocketConnect(pcc->Context);
					else
					{						
						//m_pSocketEventSelectEx->OnSocketConnectFailed(pcc->Context);
						m_pSocketEventSelectEx->SocketClose(pcc->Context);						
						continue;
					}					
				}
				if(we.lNetworkEvents & FD_READ || we.lNetworkEvents & FD_WRITE)
				{
					if(we.lNetworkEvents & FD_READ)
					{
						if(we.iErrorCode[FD_READ_BIT])
							break;
						if(!pcc->pRecvBuffer)
						{
							pcc->pRecvBuffer = (BYTE *)LocalAlloc( LMEM_FIXED, CLivenet5::RECVBUFSIZE+1 );
							pcc->dwRecvBytes = CLivenet5::RECVBUFSIZE;
						}
						if(!pcc->pRecvBuffer)
						{							
							m_pSocketEventSelectEx->SocketClose(pcc->Context);
							continue;
						}
						DWORD dwRecvBytes = 0;
						DWORD dwFlags = 0;
						WSABUF buf;
						buf.buf = (char*)pcc->pRecvBuffer;
						buf.len = pcc->dwRecvBytes;						
						//在这里应该增加一个虚函数,以询问是否读数据,如果处于速度方面的考虑暂时不读数据,可以直接跳过
						//...
						WSASetLastError(0);
						BOOL bNeedRecv = TRUE;
						while(bNeedRecv)
						{
							if(WSARecv(pcc->Socket, &buf, 1,&dwRecvBytes,&dwFlags, NULL, NULL)== SOCKET_ERROR || dwRecvBytes==0)
								break;
							buf.buf[dwRecvBytes]=0;
							m_dwCurSecondRecvBytes +=dwRecvBytes;
							bNeedRecv = m_pSocketEventSelectEx->OnSocketReceived(pcc->Context,(BYTE*)buf.buf,dwRecvBytes);
						}
						int iError = WSAGetLastError();
						if(iError>=WSABASEERR && iError!=WSAEWOULDBLOCK)
						{
							m_pSocketEventSelectEx->SocketClose(pcc->Context);
							continue;
						}
						if(!bNeedRecv)//由于速度控制或者缓冲区控制而终止接收.这时候,缓存区是存在数据的.应该在某一时刻再次投递接收请求
						{
							//所以,在这里控制记录该socket的ID,以便于在定时器到达的时候检查再次接收数据
							m_RecvCSMgr.lock();
							m_setRecv.insert(pcc->Event);
							m_RecvCSMgr.unlock();
						}
					}
					if(we.lNetworkEvents & FD_WRITE)
					{
						if(we.iErrorCode[FD_WRITE_BIT])
							break;
						m_SocketCSMgr.lock();
						pcc->bCanSend = TRUE;
						UINT nErrorID = 0;
						while(pcc->listSendBuf.size())
						{
							WSABUF buf = pcc->listSendBuf.front();
							DWORD dwSendBytes = 0;
							if (WSASend(pcc->Socket, &buf, 1, &dwSendBytes, 0,NULL, NULL) == SOCKET_ERROR)
							{
								pcc->bCanSend = FALSE;
								int iError = WSAGetLastError();
								if ( iError != WSAEWOULDBLOCK && iError != WSAENOBUFS)
									nErrorID = pcc->Context;
								else
								{
									m_dwCurSecondSendBytes +=buf.len;
								}
								break;
							}
							else
							{
								m_dwCurSecondSendBytes +=dwSendBytes;
								pcc->listSendBuf.pop_front();
								LocalFree( buf.buf);
							}
						}
						m_SocketCSMgr.unlock();
						//socket可以在任何时候发送数据.
						//但是发送后应该检查其错误值.
						//如果错误值指示在阻塞状态(如果在阻塞状态再次发送就会进入NoBufs状态)则尽量不要再试图发送大量数据.
						//框架保存了发送的数据.在可以再次发送时(也就是现在这个位置)会调用OnSocketSendOver事件.
						//OnSocketSendOver事件被激发的条件是socket因发送数据进入过阻塞状态.
						//故:如果在此处发送了缓存的数据.应该判断是否再次进入了阻塞状态,如果没有,就应该激发OnSocketSendOver,以便于客户再次发送数据.
						//如果进入了阻塞状态,则socket会再数据发送完毕后再次自动进入改处.
						if(pcc->bCanSend == TRUE)
							m_pSocketEventSelectEx->OnSocketSendOver(pcc->Context);
						if(nErrorID)
							m_pSocketEventSelectEx->SocketClose(nErrorID);
					}
				}
				if(we.lNetworkEvents&FD_CLOSE)
				{					
					DWORD dwRecvBytes = 0;
					DWORD dwFlags = 0;
					WSABUF buf;
					buf.buf = (char*)pcc->pRecvBuffer;
					buf.len = pcc->dwRecvBytes;
					//在这里应该增加一个虚函数,以询问是否读数据,如果处于速度方面的考虑暂时不读数据,可以直接跳过
					//...
					while(TRUE)
					{
						if(WSARecv(pcc->Socket, &buf, 1,&dwRecvBytes,&dwFlags, NULL, NULL)== SOCKET_ERROR || dwRecvBytes==0)
							break;
					}
					//设置远端主动断开的错误代码
					WSASetLastError(-1);
					m_pSocketEventSelectEx->SocketClose(pcc->Context);										
					continue;
				}
			}
		}
		else
		{
			if(m_bStop||m_pSocketEventSelectEx->m_bStop)
			{
				BOOL bDelThread = FALSE;
				m_SocketCSMgr.lock();
				if(m_mapClientSocket.size()==0)
					bDelThread = TRUE;
				m_SocketCSMgr.unlock();
				if(bDelThread)
					break;		
			}

		}
	}
	m_bActive = FALSE;
	return NULL;
}

//发送一个数据块
//nID是发送的连接标识
//pData数据块
//dwLen数据块长度
BOOL CSocketEventSelectEx::SocketSend(UINT nID,BYTE * pData,DWORD dwLen)
{
	CSocketEventSelectThread * pThread  = GetIDThread(nID);	
	if(pThread)
	{
		BOOL bRes = pThread->SocketSend(nID,pData,dwLen);
		pThread->Release();
		return bRes;
	}
	return FALSE;
}
BOOL CSocketEventSelectThread::SocketSend(UINT nID,BYTE * pData,DWORD dwLen)
{
	WSASetLastError(0);
	PRINTINFO("SocketSend");
	if(dwLen == 0 || !pData)
		return FALSE;
	CCriticalSection cs(&m_SocketCSMgr);	
	CMapIDContext::iterator it;
	it = m_mapClientSocket.find(GetEvent(nID));
	if(it == m_mapClientSocket.end())
		return FALSE;
	CClientContext * pcc = it->second;
	WSABUF buf;
	buf.buf = (char*)LocalAlloc( LMEM_FIXED,  dwLen+1);
	if(buf.buf == NULL)
		return FALSE;
	buf.len = dwLen;
	memcpy(buf.buf,pData,dwLen);	
	pcc->listSendBuf.push_back(buf);	
	if(pcc->bCanSend && !pcc->pTransmittedBuffer)
	{
		buf = pcc->listSendBuf.front();
		DWORD dwSendBytes = 0;
		//TRACE2("WSASend %08X,Bytes:%d",pcc->Event,pcc->dwTransmittedLen);
		if (WSASend(pcc->Socket, &buf, 1, &dwSendBytes, 0,NULL, NULL) == SOCKET_ERROR)
		{
			int iError = WSAGetLastError();
			if (iError  != WSAEWOULDBLOCK && iError != WSAENOBUFS)
			{
				//发送出错，结束改连接
				m_pSocketEventSelectEx->SocketClose(pcc->Context);
				return FALSE;
			}
			m_dwCurSecondSendBytes +=buf.len;
			pcc->bCanSend = FALSE;
		}
		else
		{
			m_dwCurSecondSendBytes +=dwSendBytes;
			pcc->listSendBuf.pop_front();
			LocalFree( buf.buf);
		}
	}
	return TRUE;
}

void CSocketEventSelectEx::SocketClose(UINT nID)
{
	CSocketEventSelectThread * pThread  = GetIDThread(nID);	
	if(pThread)
	{
		pThread->SocketClose(nID);
		pThread->Release();
	}
	m_csmap.lock();
	m_mapIDThread.erase(nID);
	m_csmap.unlock();
}
void CSocketEventSelectThread::SocketClose(UINT nID)
{	
	PRINTINFO("SocketClose");
	m_SocketCSMgr.lock();
	CMapIDContext::iterator it,itend;
	WSAEVENT hEvent = GetEvent(nID);
	it = m_mapClientSocket.find(hEvent);
	if(it==m_mapClientSocket.end())
		hEvent = NULL;
	if(hEvent)
	{
		m_CloseCSMgr.lock();		
		m_setClosed.insert(hEvent);
		m_CloseCSMgr.unlock();		
		WSASetEvent(m_hEvent[0]);
	}
	m_SocketCSMgr.unlock();
}

//关闭所有的socket
void CSocketEventSelectThread::CloseAllClient()
{
	m_SocketCSMgr.lock();
	CMapIDContext mapClientSocket = m_mapClientSocket;
	m_SocketCSMgr.unlock();
	m_CloseCSMgr.lock();
	CMapIDContext::iterator it;	
	for(it = mapClientSocket.begin(); it != mapClientSocket.end();++it)
		m_setClosed.insert(it->first);
	m_CloseCSMgr.unlock();
	WSASetEvent(m_hEvent[0]);
}
//成功返回侦听的端口.失败返回-1
//bForce为TRUE标识如果指定端口侦听失败就由系统分配一个端口侦听
WORD	CSocketEventSelectEx::Start(DWORD ip,WORD wPort,BOOL bForce )
{	
	WORD ret = 0xFFFF;
	if (wPort == 0)
	{
		m_bStart = TRUE;
		return 0;
	}
	if( ! m_bStart  )
	{
		m_bStart = TRUE;
		ret = Listen(ip,wPort,bForce);
		if (ret == 0)
		{
			ret = 0xFFFF;
			m_bStart = FALSE;
		}	 
	}
	return ret;
}

void CSocketEventSelectEx::Stop()
{
	if(m_bStart)
	{
		m_bStop = TRUE;
		//等待结束侦听线程
		m_csset.lock();		
		set<CSocketEventSelectThread*> setTmpThread = m_setThread;
		m_setThread.clear(); // 复制后，就清除
		m_csset.unlock();
		set<CSocketEventSelectThread*>::iterator it;
		for(it = setTmpThread.begin();it!=setTmpThread.end();++it)
		{
			CSocketEventSelectThread* pThread = (*it);
			//表示现在处于结束状态
			pThread->m_bStop = TRUE;		
			//关闭侦听socket以结束侦听线程		
			pThread->CloseAllClient();
			//(*it)->MsgStop((*it)->m_hEvent[0]);
			pThread->Stop((*it)->m_hEvent[0],0);
			pThread->m_bStop = FALSE;
//			if(pThread->m_iRef == 0)			
			delete pThread;
		}
	//	m_setThread.clear(); 如果在这里clear，如果stop被3个线程一起调用，就会出现异常，因为 m_setThread 可能被复制几分
		setTmpThread.clear();
		m_csmap.lock();
		m_mapIDThread.clear();
		m_csmap.unlock();
		m_bStop = FALSE;
		m_bStart = FALSE;
	}
}
//bForce标识在指定端口侦听失败后就自动分配一个端口侦听
//成功返回侦听的端口,失败返回0
int	CSocketEventSelectEx::Listen(DWORD ip,WORD wPort,BOOL bForce)
{
	PRINTINFO("Listen");
	SOCKADDR_IN sin;
    // Create a listening socket that we'll use to accept incoming
    // conections.	
	WORD wNewPort = wPort;
    SOCKET ListenSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if ( ListenSocket == INVALID_SOCKET ) 
		return 0;
    // Bind the socket to the POP3 well-known port.
    sin.sin_family = AF_INET;
    sin.sin_port = htons(wPort);
    sin.sin_addr.s_addr = ip;//INADDR_ANY;
	int bDontLinger = 0;
	//本地端口重用模式
	//int nREUSEADDR = 1;
	//::setsockopt(ListenSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&nREUSEADDR,sizeof(int));
	//强制关闭

	if (wPort)
	{
		::setsockopt(ListenSocket,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(int));

		if (bForce)
		{
			int nREUSEADDR = 1;
			::setsockopt(ListenSocket,SOL_SOCKET,SO_REUSEADDR,(const char*)&nREUSEADDR,sizeof(int));
		}
		if (::bind(ListenSocket, (PSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR )
		{
			if (bForce)
			{
				return 0;
			}

			sin.sin_port = htons(0);
			//如果绑定缺省的端口失败就由系统分配一个端口用于侦听
			if (::bind(ListenSocket , (PSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR)
			{			
				closesocket(ListenSocket);
				return 0;
			}
		}
		int inamelen = sizeof(sin);
		if(getsockname(ListenSocket,(PSOCKADDR) &sin,&inamelen ) == SOCKET_ERROR)
		{
			closesocket(ListenSocket);
			return 0;
		}
		wNewPort = ntohs(sin.sin_port);    
		// Listen for incoming connections on the socket.    
		if ( listen( ListenSocket, 5 ) == SOCKET_ERROR )
		{
			closesocket( ListenSocket);
			return 0;
		}	
		//
		if(InsertSocket(ListenSocket,(UINT)ListenSocket)==NULL)
		{
			closesocket( ListenSocket);
			return 0;
		}
		//通知连接线程,更新事件等待表.
		NotifyThread((UINT)ListenSocket);
	}				   
	return wNewPort;
}


//连接一个目标主机参数是形如IP:Port的字符串
//返回这个连接的cookie，失败为0
//连接函数如果未找到线程也要创建一个线程出来
UINT CSocketEventSelectEx::SocketConnect(const char * szRemoteAddr,UINT nContextID)
{
	if(m_bStop)
		return 0;
	CSocketEventSelectThread * pThread  = NULL;
		
	m_csset.lock();
	set<CSocketEventSelectThread*>::iterator it = m_setThread.begin();
	if(it!=m_setThread.end())
	{
		pThread = (*it);
		pThread->AddRef();
	}
	m_csset.unlock();	
	if(!pThread)
	{
		pThread = new CSocketEventSelectThread;
		if(pThread)
		{
			pThread->AddRef();
			pThread->AddRef();
			pThread->m_pSocketEventSelectEx = this;			
			m_csset.lock();
			m_setThread.insert(pThread);
			m_csset.unlock();
			if(m_bStart)
			{
				if(!pThread->Start("SC-SThread"))
				{
					delete pThread;
					pThread = NULL;
				}
				else
				{
//					TRACE("\nCSocketEventSelectEx--->>>>>>>>>>>创建线程%08X %d秒 %d个线程,%d个句柄",pThread,GetTickCount()/1000,m_setThread.size(),m_mapIDThread.size());
				}
			}
		}
	}
	if(pThread)
	{		
		UINT nRes = pThread->SocketConnect(szRemoteAddr,nContextID);
		pThread->Release();
		return nRes;
	}
	return 0;
}
UINT CSocketEventSelectThread::SocketConnect(const char * szRemoteAddr,UINT nContextID)
{
	PRINTINFO("SocketConnect");
	CHostInfo hi;
	hi.SetNodeString(szRemoteAddr);
	if(hi.IsValid())
	{
		//创建socket,加入到socket管理器中.
		SOCKADDR_IN InternetAddr;
		SOCKET s = (SOCKET)nContextID;
		if(s ==0)
			s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if(s == INVALID_SOCKET) 
			return 0;
		InternetAddr.sin_family = AF_INET;
		InternetAddr.sin_addr.s_addr = hi.IP;
		InternetAddr.sin_port = htons(hi.Port);	
		WSAEVENT hEvent = m_pSocketEventSelectEx->InsertSocket(s,(UINT)s);
		if(hEvent)
		{
			if(connect(s,(const sockaddr*)&InternetAddr,sizeof(InternetAddr))==SOCKET_ERROR)
			{
				int iErr = WSAGetLastError();
				if(iErr != WSAEWOULDBLOCK)
				{		
					m_pSocketEventSelectEx->SocketClose((UINT)s);					
					return 0;
				}
				else
				{
				}
			}
			m_pSocketEventSelectEx->NotifyThread((UINT)s);			
			return (UINT)s;
		}
		else
			closesocket(s);
	}
	return 0;
}



const WSAEVENT * CSocketEventSelectThread::GetEventTable()
{	
	PRINTINFO("GetEventTable");
	CCriticalSection cs(&m_SocketCSMgr);	
	if(m_bEventTableDirty)
	{
		CMapIDContext::iterator it;	
		WSAEVENT hEvent = m_hEvent[0];		
		memset(m_hEvent,0,sizeof(m_hEvent));
		m_hEvent[0] = hEvent;
		int i = 0;
		for(it = m_mapClientSocket.begin(); it != m_mapClientSocket.end()&&i<63;++it)
			m_hEvent[++i] = it->first;
		m_bEventTableDirty = FALSE;
	}
	return m_hEvent;
}
int CSocketEventSelectThread::GetEventCount()
{
	PRINTINFO("GetEventCount");
	CCriticalSection cs(&m_SocketCSMgr);
	return static_cast<int>(m_mapClientSocket.size()+1);
}

//根据事件句柄找到其上下文结构指针
CSocketEventSelectThread::CClientContext * CSocketEventSelectThread::GetClientContext(WSAEVENT hEvent)
{
	PRINTINFO("GetClientContext");
	CCriticalSection cs(&m_SocketCSMgr);
	CMapIDContext::iterator it;	
	it = m_mapClientSocket.find(hEvent);
	if(it != m_mapClientSocket.end())
		return it->second;		
	return NULL;
}

//插入一个socket到socket事件关联表中.如果成功返回TRUE
//如果失败返回FALSE,通常失败是因为表空间不够.
WSAEVENT CSocketEventSelectThread::InsertSocket(SOCKET s,UINT nCookie)
{
	PRINTINFO("InsertSocket");
	CCriticalSection cs(&m_SocketCSMgr);
	int iSize = static_cast<int>(m_mapClientSocket.size());
	if(iSize<m_iSocketLimit && iSize<62 && m_bActive)
	{
		CClientContext * pcc = new CClientContext;
		pcc->bCanSend = FALSE;
		pcc->pTransmittedBuffer = NULL;
		pcc->dwTransmittedLen = 0;
		pcc->pRecvBuffer = NULL;
		pcc->dwRecvBytes = 0;

		pcc->Context = nCookie;
		pcc->Socket = s;
		pcc->Event = WSACreateEvent();		
		pcc->pThread = this;
		if(WSAEventSelect(s,pcc->Event,FD_ACCEPT|FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE)==SOCKET_ERROR)
		{
			WSACloseEvent(pcc->Event);
			delete pcc;
			return NULL;
		}
//		TRACE("\n(");
		m_mapClientSocket[pcc->Event] = pcc;
		m_mapIDEvent[nCookie] = pcc->Event;
		m_bEventTableDirty = TRUE;
//		TRACE("插入IDEvent:%d at %d,%d",pcc->Event,GetTickCount(),::GetCurrentThreadId());
//		TRACE(")");
		return pcc->Event;
	}
	return NULL;	
}
//从socket事件关联表中移出一个socket,关闭socket
//参数bForceClose,强制关闭连接

void CSocketEventSelectThread::RemoveSocket(WSAEVENT hEvent,BOOL bForceClose)
{
	BOOL bRemote = FALSE;
	PRINTINFO("RemoveSocket");
	SOCKET sock = NULL;
	UINT nContextID = 0;
				BOOL bCatch = FALSE;
	if(TRUE)
	{
		CCriticalSection cs(&m_SocketCSMgr);	
		CMapIDContext::iterator it;
		it = m_mapClientSocket.find(hEvent);
		if(it != m_mapClientSocket.end())
		{
			bRemote = TRUE;
			CClientContext* pCC = it->second;
			nContextID = pCC->Context;
//			TRACE("\n<");
			CMapIDEvent::iterator ideit = m_mapIDEvent.find(pCC->Context);
			if(ideit !=m_mapIDEvent.end())
			{
//				TRACE("删除IDEvent:%d at %d,%d",ideit->second,GetTickCount(),::GetCurrentThreadId());
				try
				{
				m_mapIDEvent.erase(ideit);
				}
				catch(...)
				{
					bCatch = TRUE;
				}
				
			}
//			TRACE(">");
			//去掉关联的事件
			WSAEventSelect(pCC->Socket,pCC->Event,0);
			WSACloseEvent(pCC->Event);
			sock = pCC->Socket;		
			if(pCC->pTransmittedBuffer)
				LocalFree(pCC->pTransmittedBuffer);
			if(pCC->pRecvBuffer)
				LocalFree(pCC->pRecvBuffer);
			list<WSABUF>::iterator itb;
			
			for(itb = pCC->listSendBuf.begin();itb!=pCC->listSendBuf.end();++itb)
				LocalFree(itb->buf);
			it->second->listSendBuf.clear();
			delete pCC;
			//try
			{
				try
				{
				m_bEventTableDirty = TRUE;
				m_mapClientSocket.erase(it);
				}
				catch(...)
				{
					bCatch = TRUE;
				}

			}
			//catch(...)
			{
				//	AfxMessageBox("CSocketEventSelectThread::RemoveSocket::m_mapClientSocket.erase error",MB_OK);
			}
		}
	}
	//在OnSocketClose事件中,所有socket相关操作均不能进行		
	if(nContextID)
		m_pSocketEventSelectEx->OnSocketClose(nContextID);

	if(bForceClose && sock)
	{	
		LINGER lingerStruct;    
		// If we're supposed to abort the connection, set the linger value
		// on the socket to 0.    
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;
		setsockopt( sock, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct) );
		shutdown( sock,SD_BOTH);
		closesocket(sock);
	}
	if(bRemote)
		Release();
}
//根据ID找到其时间句柄
WSAEVENT CSocketEventSelectThread::GetEvent(UINT nCookie)
{
	CMapIDEvent::iterator it;	
	it = m_mapIDEvent.find(nCookie);
	if(it != m_mapIDEvent.end())
		return it->second;
	return NULL;
}

UINT CSocketEventSelectEx::CreateContextID()
{	
	PRINTINFO("CreateContextID");
	return (UINT)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);	
}

void CSocketEventSelectEx::ReleaseContextID(UINT nID)
{
	PRINTINFO("ReleaseContextID");
	closesocket((SOCKET)nID);
}
CSocketEventSelectThread::CSocketEventSelectThread()
{	
	m_iSocketLimit = iMaxSocketCount;
	memset(m_hEvent,0,sizeof(m_hEvent));
	//这个事件是用来通知WSAWaitForMultipleEvents新事件加入表中,重新进入等待状态
	m_hEvent[0] = WSACreateEvent();
	m_bStop = FALSE;
	m_bEventTableDirty = TRUE;
	m_pSocketEventSelectEx = NULL;
	m_iRef = 0;
	m_bActive = TRUE;
	running_ = false;
	driver_thread_ = THREAD_HANDLE_INIT_VALUE;
}
CSocketEventSelectThread::~CSocketEventSelectThread()
{
	WSACloseEvent(m_hEvent[0]);
}

//从线程列表中找到一个有空闲空间的线程,插入socket,如果没有找到,则创建一个线程
WSAEVENT CSocketEventSelectEx::InsertSocket(SOCKET s,UINT nCookie)
{	
	WSAEVENT hRes = NULL;
	if(m_bStop)
		return hRes;
	CCriticalSection cs(&m_csset);
	set<CSocketEventSelectThread*>::iterator it;
	for(it = m_setThread.begin();it!=m_setThread.end();++it)
	{
		int iRef = (*it)->AddRef();
		hRes = (*it)->InsertSocket(s,nCookie);
		if(!hRes)
			iRef = (*it)->Release();
//		TRACE("\n线程0x%08x引用计数:%d",(*it),iRef);
		if(hRes)
		{			
			m_csmap.lock();
			m_mapIDThread[nCookie] = (*it);
			m_csmap.unlock();
			/*
			TRACE("\n插入线程a 0x%08x",(*it));
			DWORD dwV = (DWORD)(*it);
			if(dwV>0x10000000||dwV<0x100000)
				AfxMessageBox("err");
			//*/
			break;
		}		
	}
	if(hRes==NULL)
	{
		CSocketEventSelectThread * pNewThread = new CSocketEventSelectThread;
		if(pNewThread)
		{
			pNewThread->m_pSocketEventSelectEx = this;			
			pNewThread->AddRef();
			pNewThread->AddRef();

			hRes = pNewThread->InsertSocket(s,nCookie);
			if(hRes)
			{
				m_setThread.insert(pNewThread);
				m_csmap.lock();				
				m_mapIDThread[nCookie] = pNewThread;
				m_csmap.unlock();
				/*
				TRACE("\n插入线程b 0x%08x",pNewThread);
			DWORD dwV = (DWORD)pNewThread;
			if(dwV>0x10000000||dwV<0x0000000)
				AfxMessageBox("err");
			//*/
				if(m_bStart)
					pNewThread->Start("IS-SThread");
//				TRACE("\nCSocketEventSelectEx--->>>>>>>>>>>创建线程%08X %d秒 %d个线程,%d个句柄",pNewThread,GetTickCount()/1000,m_setThread.size(),m_mapIDThread.size());
			}
			else
			{
				delete pNewThread;
			}
		}
	}
	return hRes;		
}

//通知ID对应的处理线程
void CSocketEventSelectEx::NotifyThread(UINT nID)
{
	CCriticalSection cs(&m_csmap);
	map<UINT,CSocketEventSelectThread*>::iterator it;
	it = m_mapIDThread.find(nID);
	if(it!=m_mapIDThread.end())
	{
		CSocketEventSelectThread * pThread = it->second;
		WSASetEvent(pThread->m_hEvent[0]);		
	}
}

CSocketEventSelectThread* CSocketEventSelectEx::GetIDThread(UINT nID)
{
	CSocketEventSelectThread* pRes = NULL;
	CCriticalSection cs(&m_csmap);
	map<UINT,CSocketEventSelectThread*>::iterator it = m_mapIDThread.find(nID);	
	if(it!=m_mapIDThread.end())
	{
		pRes = it->second;		
		InterlockedIncrement(&(long)pRes->m_iRef);				
	}
	return pRes;
}

BOOL CSocketEventSelectEx::IsActive()
{
	return m_bStart;
}
//在线程的网络函数中关闭socket后发现没有socket了,就主动删除线程
//此函数不等待线程结束,因此必须保证调用此函数后不再访问线程的任何成员变量
void CSocketEventSelectEx::RemoveThread(CSocketEventSelectThread *pThread)
{
	m_csset.lock();
	set<CSocketEventSelectThread*>::iterator it = m_setThread.find(pThread);
	if(it!=m_setThread.end())
	{
		m_setThread.erase(it);
//		TRACE("\nCSocketEventSelectEx--->>>>>>>>>>>删除线程%08X %d秒 引用计数:%d",pThread,GetTickCount()/1000,pThread->m_iRef);
		delete pThread;
	}
	m_csset.unlock();
}

int CSocketEventSelectThread::AddRef()
{	
	InterlockedIncrement(&(long)m_iRef);
	return m_iRef;	
}

int CSocketEventSelectThread::Release()
{
	InterlockedDecrement(&(long)m_iRef);	
	if(m_iRef<=0)
	{
		int iRef = m_iRef;
		m_pSocketEventSelectEx->RemoveThread(this);	
		return iRef;
	}
	return m_iRef;
}
