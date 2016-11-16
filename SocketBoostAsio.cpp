#include "SocketBoostAsio.h"
#include <boost/format.hpp>

#ifdef _WINDOWS
#else
#include "baseclass/GetTickCount.h"
#include "baseclass/VDAtom.h"
#endif

#include "baseclass/HostInfo.h"
#include <boost/format.hpp>
#include "livebase/PSLog.h"


using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;

namespace NS_CSocketBoostAsio
{
#define recv_max_len (1024)
#define NID_FROM_SOCKET(socket_) ((UINT)socket_.native())
#define START_FAILD (0XFFFF)
	class TCP_Connection
		: public boost::enable_shared_from_this<TCP_Connection>
	{
	public:	
		TCP_Connection(boost::asio::io_service& io_service,CSocketIOBase * pIOBase);
		boost::asio::ip::tcp::socket& socket(){return socket_;}
		void StartRead();

		~TCP_Connection();
		BOOL SocketSend(BYTE * pData,DWORD dwLen);
		void SocketClose();
		void SocketError(const boost::system::error_code& e);
		//UINT SocketConnect(const char * szRemoteAddr);
		UINT SocketConnect(DWORD ip,WORD port);
        bool CanSend();
        bool IsSendOver();
	private:
		void handle_write(const boost::system::error_code& e,size_t bytes_transferred);
		void handle_read(const boost::system::error_code& e,size_t bytes_transferred) ;

		boost::recursive_mutex lock_;

		BYTE data_[recv_max_len];


		struct send_buffer : private boost::noncopyable
		{
			send_buffer(const PBYTE buf, DWORD length)
			{
				bytes_transferred = 0;
				buffer_length	= 0;
				buffer = NULL;

				if( length > 0xFFF0 || length==0 )
				{
					return;
				}

				buffer_length = length ;
				buffer	= new BYTE[buffer_length];

				if(buffer)
				{
					memcpy(buffer, buf, length);
				}
				else
				{
					buffer_length	= 0;
				}
			}

			~send_buffer(void)
			{
				if(buffer)
				{
					delete[] buffer;
					buffer	= NULL;
				}
				buffer_length	= 0;
			}

			DWORD	buffer_length;
			DWORD	bytes_transferred;
			PBYTE	buffer;
		};
		std::list< boost::shared_ptr<send_buffer> >	send_buffers_;
		long volatile sending_;

		boost::asio::ip::tcp::socket socket_;
		CSocketIOBase * pIOBase_;
	};
};

namespace NS_CSocketBoostAsio
{
	CSocketBoostAsio::CSocketBoostAsio(void):bStart_(false),wListenPort_(0),thread_num_(1)
	{
		thread_num_ = 1;
		vecThreads_.resize(thread_num_);
	}

	CSocketBoostAsio::~CSocketBoostAsio(void)
	{
		Stop();
	}

	BOOL CSocketBoostAsio::IsActive()
	{
		//CCriticalSection lock(&cs_);
		return bStart_?TRUE:FALSE;
	}

	void CSocketBoostAsio::handle_accept(const boost::system::error_code& e)
	{
		if (!e)
		{
			boost::lock_guard<boost::recursive_mutex> lock(lock_);
			if( bStart_ )
			{
				UINT nID = NID_FROM_SOCKET(pConn_->socket());
				sockaddr_in name;
				socklen_t len = sizeof(name);
				memset(&name, 0, sizeof(name));
				if (getpeername(nID, (sockaddr*)&name, &len)==SOCKET_ERROR)
				{

				}
				else
				{
					//PTRACE("core:accept user %u\r\n",nID);
					AddTcpConnection(nID,pConn_);
					//CHostInfo hi(name.sin_addr.S_un.S_addr,ntohs(name.sin_port));
					OnSocketAccept(nID/*,hi*/);
					pConn_->StartRead();
				}	

				pConn_ = TCP_Connection_ptr(new TCP_Connection(*io_service_ptr_,this));
				acceptor_ptr_->async_accept(pConn_->socket(),
					boost::bind(&CSocketBoostAsio::handle_accept, this,
					boost::asio::placeholders::error));
			}
		}
		else
		{
			std::string sDesc = e.message();
			//OnSocketError(0,sDesc);

// 			if( bStart_ )
// 			{
// 				Stop();
// 			}
			//throw e;
		}
	}

    bool CSocketBoostAsio::CanSend()
    {
        return pConn_->CanSend();
    }

    bool CSocketBoostAsio::IsSendOver()
    {
        return pConn_->IsSendOver();
    }

	void CSocketBoostAsio::handle_connect(TCP_Connection_ptr pConn,const boost::system::error_code& e)
	{
		boost::lock_guard<boost::recursive_mutex> lock(lock_);

		if( !e )
		{
			if( bStart_ )
			{
				UINT nID = NID_FROM_SOCKET(pConn->socket());
				AddTcpConnection(nID,pConn);
				OnSocketConnect(nID);
				pConn->StartRead();
			}
			else
			{
				pConn->SocketClose();
			}
		}
		else
		{
			pConn->SocketError(e);
			pConn->SocketClose();
		}
	}

	int		CSocketBoostAsio::StartAsClient()
	{

		if( bStart_ )return wListenPort_;

		for( int i = 0; i < thread_num_; ++i)
		{
			if( vecThreads_[i] && vecThreads_[i]->joinable() )
			{
				//debug
				CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"thread error in CSocketBoostAsio::StartAsClient() 222");
				return START_FAILD;
			}			
		}

		boost::lock_guard<boost::recursive_mutex> lock(lock_);
		if( bStart_ )
		{
			return wListenPort_;
		}
		pConn_.reset();
		acceptor_ptr_.reset();

		if( io_service_ptr_ )
		{
			io_service_ptr_->reset();
		}
		io_service_ptr_.reset(new io_service(thread_num_));


		wListenPort_ = 0;

		for( int i = 0; i< thread_num_; ++i)
		{
			vecThreads_[i].reset(
				new boost::thread( 
				//boost::bind(&boost::asio::io_service::run, io_service_ptr_.get()))
				boost::bind(&CSocketBoostAsio::boost_asio_ioservice_run_wrapper,this)
				)
				);
			break;
		}

		work_ptr_.reset(new boost::asio::io_service::work(*io_service_ptr_));

		bStart_ = true;

		return wListenPort_;
	}


	WORD		CSocketBoostAsio::Start(DWORD ip,WORD wPort,BOOL bForce /*= TRUE*/)
	{
		if( wPort == 0 )
		{
			return StartAsClient();
		}
		if( bStart_ )return wListenPort_;

		for( int i = 0; i < thread_num_; ++i)
		{
			if( vecThreads_[i] && vecThreads_[i]->joinable() )
			{
				CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"thread error in CSocketBoostAsio::start() 273");
				return START_FAILD;
			}			
		}

		boost::lock_guard<boost::recursive_mutex> lock(lock_);
		if( bStart_ )
		{
			return wListenPort_;
		}
		pConn_.reset();
		acceptor_ptr_.reset();

		if( io_service_ptr_ )
		{
			io_service_ptr_->reset();
		}
		io_service_ptr_.reset(new io_service(thread_num_));
		acceptor_ptr_.reset(new boost::asio::ip::tcp::acceptor(*io_service_ptr_));
		boost::asio::ip::tcp::endpoint endpoint(tcp::v4(),wPort);
		//boost::asio::ip::tcp::endpoint endpoint(
		//	boost::asio::ip::address::from_string("127.0.0.1"),wPort);
		acceptor_ptr_->open(endpoint.protocol());

		boost::system::error_code bind_err;

		if(bForce)
		{
		    asio::socket_base::reuse_address option(true);
		    acceptor_ptr_->set_option(option);
		}
		
		acceptor_ptr_->bind(endpoint,bind_err);

		if( bind_err )
		{
			{
				std::string sDesc = bind_err.message();
				//OnSocketError(0,sDesc);
				CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"bind error1:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
			}			

			if( bForce )
			{
				{
					boost::system::error_code e;
					acceptor_ptr_->cancel(e);
					if( e )
					{
						std::string sDesc = e.message();
						//OnSocketError(0,sDesc);
						CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"cancel1 error:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
					}
				}

				{
					boost::system::error_code e;
					acceptor_ptr_->close(e);
					if( e )
					{
						std::string sDesc = e.message();
						//OnSocketError(0,sDesc);
						CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"close1 error:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
					}
				}
				CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"bind error1 && bforce ==true in CSocketBoostAsio::Start() 273");
				return START_FAILD;
			}
			else
			{
				endpoint.port(0);
				boost::system::error_code bind_err2;
				acceptor_ptr_->bind(endpoint,bind_err2);
				if( bind_err2 )
				{
					{
						std::string sDesc = bind_err2.message();
						//OnSocketError(0,sDesc);
						CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"bind error2:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
					}	

					{
						boost::system::error_code e;
						acceptor_ptr_->cancel(e);
						if( e )
						{
							std::string sDesc = e.message();
							//OnSocketError(0,sDesc);
							CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"cancel error2:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
						}
					}

					{
						boost::system::error_code e;
						acceptor_ptr_->close(e);
						if( e )
						{
							std::string sDesc = e.message();
							//OnSocketError(0,sDesc);
								CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"close1 error2:%s in CSocketBoostAsio::Start() 273",sDesc.c_str());
						}
					}
					CPSLog::GetPSLog()->Log_Debug(PSLOG_LEVEL_WARNING,"bind error2 && bforce ==false in CSocketBoostAsio::Start() 273");
					return START_FAILD;
				}
			}
		}

		wListenPort_ = acceptor_ptr_->local_endpoint().port();
		acceptor_ptr_->listen();

		pConn_.reset( new TCP_Connection(*io_service_ptr_,this)  );

		acceptor_ptr_->async_accept(pConn_->socket(),
			boost::bind(&CSocketBoostAsio::handle_accept, this,
			boost::asio::placeholders::error));


		for( int i = 0; i< thread_num_; ++i)
		{
			vecThreads_[i].reset(
				new boost::thread( 
					//boost::bind(&boost::asio::io_service::run, io_service_ptr_.get()))
					boost::bind(&CSocketBoostAsio::boost_asio_ioservice_run_wrapper,this)
					)
				);
		}

		work_ptr_.reset(new boost::asio::io_service::work(*io_service_ptr_));

		bStart_ = true;

		return wListenPort_;
	}

	void CSocketBoostAsio::boost_asio_ioservice_run_wrapper()
	{
		int index = 0;
		char szTmp [260];
		sprintf_s(szTmp,260,"boost asio tcp thread:%d",index);

		//SetThreadName(-1,szTmp);
		io_service_ptr_->run();
	}

	void	CSocketBoostAsio::Stop()
	{
		if( ! bStart_ )return;
		do 
		{
			boost::lock_guard<boost::recursive_mutex> lock(lock_);
			if( !bStart_ )
			{
				return;
			}
			bStart_ = false;

			if( acceptor_ptr_ )
			{
				boost::system::error_code e;
				acceptor_ptr_->cancel(e);
				if( e )
				{
					std::string sDesc = e.message();
					//OnSocketError(0,sDesc);
				}
			}

			if( acceptor_ptr_ )
			{
				boost::system::error_code e;
				acceptor_ptr_->close(e);
				if( e )
				{
					std::string sDesc = e.message();
					//OnSocketError(0,sDesc);
				}
			}

			if( pConn_ )
			{
				pConn_->SocketClose();
			}

			//停止所有的会话
			CloseAllTcpConnection();

			work_ptr_.reset();
		} while (false);

		for( int i = 0; i< thread_num_; ++i)
		{
			if( vecThreads_[i] )
			{
				vecThreads_[i]->join();
			}			
		}
	}

	UINT CSocketBoostAsio::SocketConnect(const char * szRemoteAddr,UINT nContextID)
	{
		CHostInfo hi;
		hi.SetNodeString(szRemoteAddr);
		if(hi.IsValid())
		{
				return SocketConnect(hi.GetIP(),hi.GetPort());
		}
		return 0;
	}

	UINT CSocketBoostAsio::SocketConnect(DWORD ip,WORD port )
	{
		TCP_Connection_ptr pConn = TCP_Connection_ptr(new TCP_Connection(*io_service_ptr_,this) );

		if( pConn )
		{
			return pConn->SocketConnect(ip,port);
		}

		return 0;
	}

	BOOL CSocketBoostAsio::SocketSend(UINT nID,BYTE * pData,DWORD dwLen)
	{
		TCP_Connection_ptr pConn = GetTcpConnection(nID);
		if( pConn )
		{
			return pConn->SocketSend(pData,dwLen);
		}

		return FALSE;
	}

	void CSocketBoostAsio::SocketClose(UINT nID)
	{
		TCP_Connection_ptr pConn = RemoveTcpConnection(nID);
		if( pConn )
		{
			pConn->SocketClose();
		}		
	}

	TCP_Connection_ptr CSocketBoostAsio::GetTcpConnection(UINT nID)
	{
		TCP_Connection_ptr pConn;
		if( nID == 0  || nID == INVALID_SOCKET )
		{
			return pConn;
		}
		boost::lock_guard<boost::recursive_mutex> lock(lockForMap_);
		map<UINT,TCP_Connection_ptr>::iterator it = m_mapConn.find(nID);
		if( it != m_mapConn.end() )
		{
			pConn = it->second;
		}
		return pConn;
	}

	bool CSocketBoostAsio::AddTcpConnection(UINT nID,TCP_Connection_ptr pConn)
	{
		if( nID == 0  || nID == INVALID_SOCKET )
		{
			return false;
		}
		boost::lock_guard<boost::recursive_mutex> lock(lockForMap_);
		map<UINT,TCP_Connection_ptr>::iterator it = m_mapConn.find(nID);
		if( it != m_mapConn.end() )
		{
			return false;
		}

		m_mapConn[nID] = pConn;
		return true;
	}

	TCP_Connection_ptr CSocketBoostAsio::RemoveTcpConnection(UINT nID)
    {
		TCP_Connection_ptr ret;
		if( nID == 0  || nID == INVALID_SOCKET )
		{
			return ret;
		}
		boost::lock_guard<boost::recursive_mutex> lock(lockForMap_);
		map<UINT,TCP_Connection_ptr>::iterator it = m_mapConn.find(nID);
		if( it == m_mapConn.end() )
		{
			return ret;
		}

		ret = it->second;
        m_mapConn.erase(it);
		return ret;
	}

	void CSocketBoostAsio::CloseAllTcpConnection()
    {
		map<UINT,TCP_Connection_ptr> mapConnTemp;
		{
			boost::lock_guard<boost::recursive_mutex> lock(lockForMap_);
			mapConnTemp = m_mapConn;
		}

		map<UINT,TCP_Connection_ptr>::iterator it = mapConnTemp.begin();
		for( ; it != mapConnTemp.end() ; ++it)
		{
			it->second->SocketClose();
		}

		{
			boost::lock_guard<boost::recursive_mutex> lock(lockForMap_);
			m_mapConn.clear();
		}
	}

	TCP_Connection::TCP_Connection(asio::io_service& io_service,CSocketIOBase * pIOBase) : 
	socket_(io_service),
		pIOBase_(pIOBase),
		sending_(0)
	{
	}

	TCP_Connection::~TCP_Connection()
	{
	}

	void TCP_Connection::StartRead()
	{
		if (socket_.is_open())
		{
			socket_.async_read_some(boost::asio::buffer(data_, recv_max_len),
				boost::bind(
				&TCP_Connection::handle_read, 
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
				)
				);
		}
	}

    bool TCP_Connection::CanSend()
    {
        return send_buffers_.size() < 2;
    }

    bool TCP_Connection::IsSendOver()
    {
        return send_buffers_.empty();
    }

	BOOL TCP_Connection::SocketSend(BYTE * pData,DWORD dwLen)
	{
		boost::shared_ptr<send_buffer>	bufData(new send_buffer(pData, dwLen));
		if( bufData->buffer_length == 0 )
		{
			return FALSE;
		}

		boost::lock_guard<boost::recursive_mutex> lock(lock_);
		send_buffers_.push_back(bufData);
		if(0==InterlockedCompareExchange(&sending_, 1, 0))
		{			
			boost::shared_ptr<send_buffer>&	send_buf	= send_buffers_.front();
			if (socket_.is_open())
			{
				socket_.async_send(
					boost::asio::buffer(send_buf->buffer+send_buf->bytes_transferred,send_buf->buffer_length - send_buf->bytes_transferred),
					boost::bind(
					&TCP_Connection::handle_write,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred			
					)
					);
			}
		}

		return TRUE;
	}

	void TCP_Connection::SocketClose()
	{
		if (socket_.is_open())
		{
			UINT nID = NID_FROM_SOCKET(socket_);
			if( nID && nID != INVALID_SOCKET )
			{
				pIOBase_->OnSocketClose(nID);
				boost::system::error_code e;
				socket_.cancel(e);
				socket_.shutdown(tcp::socket::shutdown_both,e);
				socket_.close(e);	
			}
		}
	}

	void TCP_Connection::SocketError(const boost::system::error_code& e)
	{
		UINT nID = NID_FROM_SOCKET(socket_);
		if( nID && nID != INVALID_SOCKET )
		{
			std::string sDesc = e.message();
			//pIOBase_->OnSocketError(nID,sDesc);
		}
	}

	/*
	UINT TCP_Connection::SocketConnect(const char * szRemoteAddr)
	{
		DWORD ip = 0;
		WORD port  = 0;

		if( !PaserAddr(szRemoteAddr,ip,port) )
		{
			return NULL;
		}

		tcp::endpoint ep(tcp::v4(),port);
		ep.address(asio::ip::address_v4(ntohl(ip)));

		socket_.async_connect(ep,
			boost::bind(
			&CSocketBoostAsio::handle_connect, 
			dynamic_cast<CSocketBoostAsio*>(pIOBase_),
			shared_from_this(),
			boost::asio::placeholders::error
			)
			);

		return NID_FROM_SOCKET(socket_);
	}
	*/
	
	UINT TCP_Connection::SocketConnect(DWORD ip,WORD port)
	{
		if( ip==0 || port==0 )
		{
			return 0;
		}

		tcp::endpoint ep(tcp::v4(),port);
		ep.address(asio::ip::address_v4(ntohl(ip)));

		socket_.async_connect(ep,
			boost::bind(
			&CSocketBoostAsio::handle_connect, 
			dynamic_cast<CSocketBoostAsio*>(pIOBase_),
			shared_from_this(),
			boost::asio::placeholders::error
			)
			);

		return NID_FROM_SOCKET(socket_);
	}

	void TCP_Connection::handle_read(const boost::system::error_code& e,size_t bytes_transferred) 
	{
		if (!e) 
        {
			if( pIOBase_->OnSocketReceived(NID_FROM_SOCKET(socket_),data_, static_cast<int>(bytes_transferred)) )
			{
				if (socket_.is_open())
				{
					socket_.async_read_some(boost::asio::buffer(data_, recv_max_len),
						boost::bind(&TCP_Connection::handle_read, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));	
				}
			}

		} 
		else 
		{
			SocketError(e);
			pIOBase_->SocketClose(NID_FROM_SOCKET(socket_));
		}	
	}

	void TCP_Connection::handle_write(const boost::system::error_code& e, size_t bytes_transferred)
	{
		if (!e) 
		{
			boost::lock_guard<boost::recursive_mutex> lock(lock_);
			{
				bool send_complete = false;
				{
					boost::shared_ptr<send_buffer>&	send_buf	= send_buffers_.front();
					send_buf->bytes_transferred += bytes_transferred;
					if( send_buf->bytes_transferred == send_buf->buffer_length )
					{
						send_complete = true;					
					}
				}
				if( send_complete )
				{
				    send_buffers_.pop_front();
				}
			}

			InterlockedExchange(&sending_, 0);

			if(send_buffers_.size())
			{
				InterlockedExchange(&sending_, 1);
				boost::shared_ptr<send_buffer>&	send_buf	= send_buffers_.front();
				if (socket_.is_open())
				{
					socket_.async_send(boost::asio::buffer(send_buf->buffer+send_buf->bytes_transferred,send_buf->buffer_length - send_buf->bytes_transferred), 
						boost::bind(
						&TCP_Connection::handle_write,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred			
						)
						);
				}
            }
            else
            {
                pIOBase_->OnSocketSendOver(NID_FROM_SOCKET(socket_));
            }
		} 
		else 
		{			
			SocketError(e);
			pIOBase_->SocketClose(NID_FROM_SOCKET(socket_));
		}
	}
}
