#ifndef __SOCKET_BOOSTASIO_H__
#define __SOCKET_BOOSTASIO_H__

#pragma once

//禁止BOOST Exception使用dynamic_cast和动态的typeid.
//作用：模板函数get_error_info只能用于boost::exception类型
#define BOOST_NO_RTTI
#define BOOST_NO_STDC_NAMESPACE
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <vector>
#include <map>
#include <list>

#include "TCPNetwork.h"

namespace NS_CSocketBoostAsio
{
	//tcp连接
	class TCP_Connection;
	typedef boost::shared_ptr<TCP_Connection> TCP_Connection_ptr;

	//实现CSocketIOBase
	class CSocketBoostAsio :
		public CSocketIOBase
	{
	public:
		CSocketBoostAsio(void);
		virtual ~CSocketBoostAsio(void);

	public:			
		BOOL IsActive();
		WORD		Start(DWORD ip,WORD wPort,BOOL bForce = TRUE);
		void	Stop();
		virtual UINT SocketConnect(const char * szRemoteAddr,UINT nContextID);
		virtual UINT SocketConnect(DWORD ip,WORD port);									   		
		virtual BOOL SocketSend(UINT nID,BYTE * pData,DWORD dwLen);
		virtual void SocketClose(UINT nID);

		void handle_connect(TCP_Connection_ptr pConn,const boost::system::error_code& err);
        bool CanSend();
        bool IsSendOver();

	private:
		int		StartAsClient();
		void handle_accept(const boost::system::error_code& e);
		boost::recursive_mutex lock_;
		typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
		typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
		typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;
		work_ptr work_ptr_;
		io_service_ptr io_service_ptr_; 
		acceptor_ptr acceptor_ptr_;

		TCP_Connection_ptr pConn_;
		WORD wListenPort_;
		bool bStart_;
	private:
		int thread_num_;
		std::vector< boost::shared_ptr<boost::thread> > vecThreads_;
	private:
		void CloseAllTcpConnection();
		TCP_Connection_ptr GetTcpConnection(UINT nID);
		bool AddTcpConnection(UINT nID,TCP_Connection_ptr pConn);
		TCP_Connection_ptr RemoveTcpConnection(UINT nID);
		boost::recursive_mutex lockForMap_;
		std::map<UINT,TCP_Connection_ptr> m_mapConn;
	private:
		void boost_asio_ioservice_run_wrapper();
	};
};

#endif //__ProtocolSession_H__
