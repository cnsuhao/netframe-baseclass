/* //////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "EventSelectSocketEx.h"
#include "baseclass/HostInfo.h"
#include "stdafx.h"

//#ifdef __APPLE__
//#include "baseclass/LivenetIPV6.h"
//#endif

/* ////////////////////////////////////////////////////////////////////////////// 
 * macros
 */

// trace
#ifdef ems_trace
# 	undef ems_trace
#endif

#ifdef ANDROID
    #if 0
    //#	define ems_trace(fmt, arg ...)			do { EMSDBG("[wrq]: [socket]: " fmt "\n" , ## arg); } while (0)
    #include <android/log.h>
    #	define ems_trace(...)       __android_log_print(ANDROID_LOG_DEBUG, "Livenet5", __VA_ARGS__)
    #else
    #	define ems_trace(...)
    #endif
#else
#ifdef __APPLE__
    #if 0
    #include "../../../livebase/PSLog.h"
    #   define ems_trace(...)      CLivenet5::AppleMicOutPut(__VA_ARGS__)
    #else
    #	define ems_trace(...)
    #endif
#else
#	define ems_trace(...)
#endif
#endif

// fd to handle
#define ems_fd2handle(fd) 					((fd) >= 0? (handle_type)((long_type)(fd) + 1) : ems_null)

// handle to fd
#define ems_handle2fd(handle) 				(int_type)((handle)? (((long_type)(handle)) - 1) : -1)

// align
#define ems_align(x, b) 					(((x) + ((b) - 1)) & ~((b) - 1))


//_NAMESPACE_BEGIN
/* ////////////////////////////////////////////////////////////////////////////// 
 * interface implementation
 */
CSocketEventSelectEx::CSocketEventSelectEx()
	: m_stop(1)
	, m_sock(ems_null)
	, m_aiop(ems_null)
    , m_loop( 0 )
{
    pthread_mutex_init(&m_lock, ems_null);
}

CSocketEventSelectEx::~CSocketEventSelectEx()
{
	// stop it
    InnerStop();
    // exit lock
    pthread_mutex_destroy(&m_lock);
}
CSocketEventSelectEx::uint_type CSocketEventSelectEx::CreateContextID()
{
	// init
	handle_type handle = socket_init();
	//ems_assert_return_val(handle, 0);

	// ok?
	return ems_handle2fd(handle);
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::ReleaseContextID(uint_type fd)
{
	if (fd) socket_exit(ems_fd2handle(fd));
}
CSocketEventSelectEx::int_type CSocketEventSelectEx::Start(uint32_type ip,uint16_type port, bool_type bforce)
{
	// trace
	ems_trace("Start: port: %u, bforce: %d: ..", port, bforce);

	// stoped?
	//ems_assert_return_val(loop_bstoped(), 0);

	// init sock
	bool_type 	ok = ems_false;
	handle_type sock = ems_null;
	do
	{
		// init loop
		if (!loop_init()) break;
	
		// listen it
		if (port || bforce)
		{
            if (m_sock != ems_null)
            {
                // already exists
                ok = ems_true;
                ems_trace("listen socket already exists, do nothing");
                break;
            }
            
			// init sock
			sock = socket_init();
			//ems_assert_break(sock);

			// listen it
			port = socket_listen(sock, port, bforce);
			//ems_assert_break(port);

			// save sock
			m_sock = sock;

			// addo sock
			if (!aiop_addo(m_aiop, sock, AIOE_CODE_ACPT)) break;
		}

		// ok
		ok = ems_true;

 	} while (0);

	// failed?
	if (!ok)
	{
		// exit sock
		if (sock) socket_exit(sock);
		m_sock = sock = ems_null;

		// exit port
		// port = 0;
        port = 0xFFFF;
	}

	// trace
	ems_trace("Start: port: %u, bforce: %d: %s", port, bforce, ok? "ok" : "no");

	// ok?
	return port;
}

CSocketEventSelectEx::void_type CSocketEventSelectEx::InnerStop()
{
    if (!loop_bstoped()) {
        // trace
        Stop();
    }
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::Stop()
{
	// stoped?
	// ems_check_return(!loop_bstoped());

	// trace
	ems_trace("Stop: ..");

	// exit sock
	if (m_sock) 
	{
		if (m_aiop) aiop_delo(m_aiop, m_sock);
		socket_exit(m_sock);
	}
	m_sock = ems_null;

	// exit loop
	loop_exit();
	
	// trace
	ems_trace("Stop: ok");
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::IsActive()
{
	return loop_bstoped()? ems_false : ems_true;
}
CSocketEventSelectEx::uint_type CSocketEventSelectEx::SocketConnect(char_type const* addr, uint_type fd)
{
	// stoped?
	ems_check_return_val(!loop_bstoped(), 0);

	// check
	//ems_assert_return_val(addr && m_aiop, 0);

	// done
	long_type 	ok = -1;
	handle_type sock = ems_null;
	conn_type 	conn;
	do
	{
		// init sock
		sock = fd? (handle_type)(fd + 1) : socket_init();
		//ems_assert_break(sock);

		// connect it
		ok = socket_connect(sock, addr);

		// init conn
		if (!conn_init(conn, addr)) break;

		// wait?
		if (!ok)
		{
			// addo sock
			if (!aiop_addo(m_aiop, sock, AIOE_CODE_CONN)) ok = -1;

			// trace
			ems_trace("connect[%p]: %s: ..", sock, addr);

			// check
			//ems_assert(!ok);
		}
		// ok
		else if (ok > 0)
		{
			// trace
			ems_trace("connect[%p]: %s: ok", sock, addr);
	
			// addo sock
			if (!aiop_addo(m_aiop, sock, AIOE_CODE_RECV)) ok = -1;

			// on connect
			if (ok > 0) OnSocketConnect(ems_handle2fd(sock));
			
			// check
			//ems_assert(ok > 0);
		}
		else
		{
			// trace
			ems_trace("connect[%p]: %s: no", sock, addr);
		}

	} while (0);

	// failed?
	if (ok < 0)
	{
		// exit conn
		if (sock) conn_exit(conn);

		// exit sock
		if (!fd && sock) socket_exit(sock);
		sock = ems_null;
	}
	else
	{
		// add conn to pool
		if (sock) 
		{
			pthread_mutex_lock(&m_lock);
			m_pool[sock] = conn;
			pthread_mutex_unlock(&m_lock);
		}
	}

	// ok?
	return sock? (((uint_type)sock) - 1) : 0;
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::SocketSend(uint_type fd, byte_type* data, dword_type size)
{
	// check
	//ems_assert_return_val(fd && data, ems_false);

	// send it
	if (conn_send(ems_fd2handle(fd), data, size) < 0)
	{
		// exit it
		SocketClose(fd);

		// failed
		return ems_false;
	}

	// ok
	return ems_true;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::SocketClose(uint_type fd)
{
	// check
	//ems_assert_return(fd);

	// exit sock
	handle_type sock = ems_fd2handle(fd);

	// del conn from pool
	bool_type ok = ems_false;
	if (sock) 
	{
		// find conn
		pthread_mutex_lock(&m_lock);
		pool_type::iterator itor = m_pool.find(sock);
		if (itor != m_pool.end())
		{
			// trace
			ems_trace("close[%p]", sock);

			// exit sock
			socket_exit(sock);

			// delo sock
			if (m_aiop) aiop_delo(m_aiop, sock);

			// exit conn
			conn_exit(itor->second);

			// remove conn
			m_pool.erase(itor);

			// ok
			ok = ems_true;
		}
		else ems_trace("close[%p]: double closed", sock);
		pthread_mutex_unlock(&m_lock);
	}
	// on close
	if (ok) OnSocketClose(fd);
}


/* ////////////////////////////////////////////////////////////////////////////// 
 * loop implementation
 */
CSocketEventSelectEx::bool_type CSocketEventSelectEx::loop_init()
{
	// done
	bool_type 		ok = ems_false;
#ifdef OSAL_SET_PTHREAD_ATTRIBUTE
	pthread_attr_t 	attr;
#endif
	do
	{
		// check
		//ems_assert_break(!m_aiop);

		// init attr
#ifdef OSAL_SET_PTHREAD_ATTRIBUTE
		if (pthread_attr_init(&attr)) break;
# 	ifdef OSAL_PTHREAD_STACKSIZE
		pthread_attr_setstacksize(&attr, OSAL_PTHREAD_STACKSIZE);
# 	endif
#endif

		// init lock
		//if (pthread_mutex_init(&m_lock, ems_null)) break;

        pthread_mutex_lock(&m_lock);
        if (m_aiop == ems_null)
        {
            // init aiop
            m_aiop = aiop_init();
            //ems_assert_break(m_aiop);
        }

        if (m_aiop != ems_null)
        {
            if (m_loop == 0)
            {
                // init loop
                ems_atomic_set0(&m_stop);
                errno = 0;
#ifdef OSAL_SET_PTHREAD_ATTRIBUTE
                if (pthread_create(&m_loop, &attr, CSocketEventSelectEx::loop_done, (pointer_type)this))
#else
                if (pthread_create(&m_loop, ems_null, CSocketEventSelectEx::loop_done, (pointer_type)this))
#endif
                {
                    int nerr = errno;
                    //loop_stop();
                    ems_trace("pthread_create: failed: errno: %d", nerr);
                }
                else
                {
                    // init loop
                    // ems_atomic_set0(&m_stop);
                    // ok
                    ok = ems_true;
                }
            }
            else
            {
                // already exists
                ok = ems_true;
                ems_trace("aiop and pthread already exists, do nothing");
            }
        }
        pthread_mutex_unlock(&m_lock);
	} while (0);

	// failed?
	if (!ok) loop_exit();

	// exit attr
#ifdef OSAL_SET_PTHREAD_ATTRIBUTE
	pthread_attr_destroy(&attr);
#endif

	// ok?
	return ok;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::loop_exit()
{
    // stoped?
    ems_check_return(!loop_bstoped());
    
	// stop it
	loop_stop();

	// kill aiop
	if (m_aiop) aiop_kill(m_aiop);

	// wait loop
    if( m_loop != 0 )
    {
        pthread_join(m_loop, ems_null);
        m_loop = 0;
    }

	// exit aiop
	if (m_aiop != ems_null) aiop_exit(m_aiop);
	m_aiop = ems_null;

	// exit pool
	pthread_mutex_lock(&m_lock);
	{
		// walk
		pool_type::iterator itor = m_pool.begin();
		pool_type::iterator tail = m_pool.end();
		for (; itor != tail; ++itor)
		{
            // close socket
            socket_exit(itor->first);
			// exit conn
			conn_exit(itor->second);
		}
	}
	m_pool.clear();
	pthread_mutex_unlock(&m_lock);

	// exit lock
	// pthread_mutex_destroy(&m_lock);
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::loop_post(handle_type handle, size_type code, pointer_type data)
{
	// check
	//ems_assert_return_val(m_aiop && handle, ems_false);

	// post
	return aiop_post(m_aiop, handle, code, data);
}
CSocketEventSelectEx::long_type CSocketEventSelectEx::loop_wait(aioe_type* list, size_type maxn, long_type timeout)
{
	// check
	//ems_assert_return_val(m_aiop && list && maxn, -1);

	// wait
	return aiop_wait(m_aiop, list, maxn, timeout);
}

CSocketEventSelectEx::pointer_type CSocketEventSelectEx::loop_impl()
{
    L5CHK;
    // trace
	ems_trace("loop: init");
    
	// init list
	aioe_type 	list[64] = {0};
    
	// init data
	size_type 	recv_maxn = 8192 << 1;
	byte_type* 	recv_data = ems_null;
    bool_type   bexit = ems_false;
    bool_type   bstoped = ems_false;
    long_type   wait = 1;
    
	// done
	do
	{
		// check
		//ems_assert_break(recv_data);
        
		// loop
		while (!(bstoped = loop_bstoped()))
		{
			// trace
			ems_trace("loop: wait: ..");
            
			// wait
			//long_type wait = loop_wait(list, 64, -1);
            wait = loop_wait(list, 64, 80*1000/*1500*/);
            
			// trace
			ems_trace("loop: wait: %ld", wait);
            
			// error?
			ems_check_break(wait >= 0);
            
			// timeout?
			ems_check_continue(wait > 0);
            
			// walk list
			size_type i = 0;
			for (i = 0; i < wait; i++)
			{
                bstoped = loop_bstoped();
                ems_check_break(!bstoped);
                //ems_check_break(!loop_bstoped());
                
				// the aioe
				aioe_type const* aioe = list + i;
				//ems_assert_break(aioe->handle && aioe->code);
                
				// the sock
				handle_type sock = aioe->handle;
				//ems_assert_break(sock);
                
				// the code
				size_type code = aioe->code;
				//ems_assert_break(code);
                
				// recv?
				if (code & AIOE_CODE_RECV)
				{
                    // malloc it
                    if (ems_null == recv_data)
                    {
                        int len = ems_align(recv_maxn+2, 4);
                        recv_data = (byte_type*)malloc(len);
                    }
                    
                    // recv it
                    long_type real = -1;
                    while( (real = socket_recv(sock, recv_data, recv_maxn)) > 0 )
                    {
                        recv_data[real] = '\0';
                        // trace
                        ems_trace("loop: recv[%p]: %ld", sock, real);
                        // on recv
                        bool_type ok = OnSocketReceived(ems_handle2fd(sock), recv_data, real);
                        
                        // trace
                        ems_trace("loop: onrecv[%p]: %d", sock, ok);
                        
                        if (ok == ems_false)
                        {
                            // application force close
                            socket_recv(sock, recv_data, recv_maxn);
                            real = -1;
                            break;
                        }
                    }
                    
                    if( real < 0)
                    {
                        // exit it
                        SocketClose(ems_handle2fd(sock));
                    }
                    
                    /*
					// recv it
					long_type real = socket_recv(sock, recv_data, recv_maxn);
                    
					// trace
					ems_trace("loop: recv[%p]: %ld", sock, real);
                    
					// ok?
					if (real > 0)
					{
						// on recv
						bool_type ok = OnSocketReceived(ems_handle2fd(sock), recv_data, real);
                        
						// trace
						ems_trace("loop: onrecv[%p]: %d", sock, ok);
                        
						if (ok == ems_false)
                        SocketClose(ems_handle2fd(sock));
					}
					else
					{
						// exit it
						SocketClose(ems_handle2fd(sock));
					}
                     */
				}
				// send?
				else if (code & AIOE_CODE_SEND)
				{
					// send it
					if (conn_send(sock, ems_null, 0) <= 0)
					{
						// exit it
						SocketClose(ems_handle2fd(sock));
					}
				}
				// conn?
				else if (code & AIOE_CODE_CONN)
				{
					// the addr
					char_type addr[32] = {0};
					if (conn_addr(sock, addr, 32))
					{
						// connect it
						long_type ok = socket_connect(sock, addr);
                        
						// trace
						ems_trace("loop: connect[%p]: %s: %s", sock, addr, ok > 0? "ok" : "no");
                        
						// ok
						if (ok > 0)
						{
							// done connect
							if (loop_post(sock, AIOE_CODE_RECV))
                            {
								OnSocketConnect(ems_handle2fd(sock));
                            }
							else
                            {
                                SocketClose(ems_handle2fd(sock));
                            }
						}
						// closed
						else
                        {
                            SocketClose(ems_handle2fd(sock));
                        }
					}
					else
                    {
                        SocketClose(ems_handle2fd(sock));
                    }
				}
				// acpt?
				else if (code & AIOE_CODE_ACPT)
				{
					// accept it
					handle_type acpt = conn_acpt(sock);
                    
					// trace
					ems_trace("loop: acpt[%p]: %p", sock, acpt);
                    
					// has accept?
					if (acpt)
					{
						// on accept
						OnSocketAccept(ems_handle2fd(acpt));
					}
					else
					{
						// stop it
						//loop_stop();
                        bexit = ems_true;
						break;
					}
				}
				else 
				{
					// trace
					ems_trace("loop: code[%p]: unknown: %lu", sock, code);
				}
			}
            
            if(ems_true == bexit)
            {
                break;
            }
		}
        
	} while (0);
    
	// trace
	ems_trace("loop: exit");
    
    // log
    ems_trace("value: stop:%d wait:%d exit:%d", bstoped, wait, bexit);
    
	// stop it
    // loop_stop();
    
	// exit data
	if (recv_data) free(recv_data);
	recv_data = ems_null;
    
	// exit it
	pthread_exit(ems_null);
	return ems_null;
}
CSocketEventSelectEx::pointer_type CSocketEventSelectEx::loop_done(pointer_type data)
{
    CLivenet5::SetLivenetThreadName("L5::SocEvSlectEx::loop");
	// this
	CSocketEventSelectEx* pthis = (CSocketEventSelectEx*)data;
	//ems_assert_return_val(pthis, ems_null);
    if (pthis) {
        return pthis->loop_impl();
    }
    
    return ems_null;
}
/* ////////////////////////////////////////////////////////////////////////////// 
 * conn implementation
 */
CSocketEventSelectEx::bool_type CSocketEventSelectEx::conn_init(conn_type& conn, char_type const* addr)
{
	// done
	bool_type ok = ems_false;
	do
	{
		// init addr
		memset(conn.addr, 0, sizeof(conn.addr));
		if (addr) strncpy(conn.addr, addr, sizeof(conn.addr) - 1);

		// init data
		conn.maxn = 8192;
		conn.size = 0;
		conn.data = (byte_type*)malloc(conn.maxn);
		//ems_assert_break(conn.data);

		// ok
		ok = ems_true;

	} while (0);

	// failed? exit conn
	if (!ok) conn_exit(conn);

	// ok?
	return ok;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::conn_exit(conn_type& conn)
{
	// exit data
	if (conn.data) free(conn.data);
	conn.data = ems_null;
	conn.size = 0;
	conn.maxn = 0;

	// exit addr
	memset(conn.addr, 0, sizeof(conn.addr));
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::conn_addr(handle_type sock, char_type* addr, size_type maxn)
{
	// check
	//ems_assert_return_val(sock && maxn, ems_false);

	// enter
	pthread_mutex_lock(&m_lock);

	// done
	bool_type 			ok = ems_false;
	pool_type::iterator itor = m_pool.find(sock);
	if (itor != m_pool.end())
	{
		// copy addr
		strncpy(addr, itor->second.addr, maxn - 1);

		// ok
		ok = ems_true;
	}

	// leave
	pthread_mutex_unlock(&m_lock);

	// ok?
	return ok;
}
CSocketEventSelectEx::handle_type CSocketEventSelectEx::conn_acpt(handle_type sock)
{
	// check
	//ems_assert_return_val(sock, ems_null);

	// done
	bool_type 	ok = ems_false;
	handle_type acpt = ems_null;
	conn_type 	conn;
	do
	{
		// accept it
		acpt = socket_accept(sock);
		//ems_assert_break(acpt);

		// init conn
		if (!conn_init(conn)) break;

		// addo apct
		if (!aiop_addo(m_aiop, acpt, AIOE_CODE_RECV)) break;

		// ok
		ok = ems_true;

	} while (0);

	// failed?
	if (!ok)
	{
		// exit conn
		if (acpt) conn_exit(conn);

		// exit acpt
		if (acpt) socket_exit(acpt);
		acpt = ems_null;
	}
	else
	{
		// add conn to pool
		if (sock) 
		{
			pthread_mutex_lock(&m_lock);
			m_pool[acpt] = conn;
			pthread_mutex_unlock(&m_lock);
		}
	}

	// ok?
	return acpt;
}
CSocketEventSelectEx::long_type CSocketEventSelectEx::conn_send(handle_type sock, byte_type const* data, size_type size)
{
	// check
	//ems_assert_return_val(sock, -1);

	// enter
	pthread_mutex_lock(&m_lock);

	// done
	long_type ok = -1;
	do
	{
		// find conn
		pool_type::iterator itor = m_pool.find(sock);
		ems_check_break(itor != m_pool.end());

		// the conn
		conn_type& conn = itor->second;

		// append data
		if (data && size) 
		{
            if(!conn.data)
            {
                conn.maxn = 8192;
                conn.size = 0;
                conn.data = (byte_type*)malloc(conn.maxn);
                if (!conn.data) break;
            }
            
			// not enough? grow it
			if (conn.size + size > conn.maxn)
			{
				conn.maxn = ems_align(conn.size + size, 8192);
				conn.data = (byte_type*)realloc(conn.data, conn.maxn);
				if (!conn.data) break;
			}

			// append it
			memcpy(conn.data + conn.size, data, size);
			conn.size += size;
		}

		// send it
		if (conn.size)
		{
			size_type writ = 0;
			long_type real = -1;
			while (writ < conn.size)
			{
				// send
				real = socket_send(sock, conn.data + writ, conn.size - writ);
				if (real > 0) writ += real;
				else break;
			}

			// move the left data to head
			if (writ && writ <= conn.size) 
			{
				memmove(conn.data, conn.data + writ, conn.size - writ);
				conn.size -= writ;
                if (conn.size <= 0 && conn.data)
                {
                    memset(conn.data, 0, conn.maxn);
                }
			}
		
			// ok?
			ok = writ? writ : (real < 0? -1 : writ);

			// continue to send?
			if (conn.size)
			{
				// wait it
				if (!loop_post(sock, AIOE_CODE_RECV | AIOE_CODE_SEND)) ok = -1;
			}
			// finished? recv it
			else 
			{
				// wait it
				if (!loop_post(sock, AIOE_CODE_RECV)) ok = -1;
			}

		}
		// ok
		else ok = 0;
	
		// trace
		ems_trace("send[%p]: %ld, left: %u\n", sock, ok, conn.size);

	} while (0);

	// leave
	pthread_mutex_unlock(&m_lock);

	// ok?
	return ok;
}

/* ////////////////////////////////////////////////////////////////////////////// 
 * socket implementation
 */
CSocketEventSelectEx::handle_type CSocketEventSelectEx::socket_init()
{
	// socket
//#ifdef __APPLE__
//    if (LivenetIPV6::is_ipv6)
//        int_type fd = LivenetIPV6::Socket(AF_INET6 , SOCK_STREAM, IPPROTO_TCP);
//    else
//        int_type fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//#else
//    int_type fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//#endif
    
	int_type fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//ems_assert_return_val(fd >= 0, ems_null);

	// non-block
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	// ok
	return ems_fd2handle(fd);
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::socket_pair(handle_type pair[2])
{
	// check
	//ems_assert_return_val(pair, ems_false);

	// make pair
	int_type fd[2] = {0};
    errno = 0;
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd) == -1)
    {
        char msg[100] = { 0 };
        int nerr = errno;
        sprintf(msg, "socket pair: failed: errno: %d", nerr);
        ems_trace("%s", msg);
        pair[0] = ems_null;
        pair[1] = ems_null;
        return ems_false;
    }

	// non-block
	fcntl(fd[0], F_SETFL, fcntl(fd[0], F_GETFL) | O_NONBLOCK);
	fcntl(fd[1], F_SETFL, fcntl(fd[1], F_GETFL) | O_NONBLOCK);

	// save pair
	pair[0] = ems_fd2handle(fd[0]);
	pair[1] = ems_fd2handle(fd[1]);

	// ok
	return ems_true;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::socket_exit(handle_type handle)
{
	// check
	//ems_assert_return(handle);

	// shutdown
	shutdown(ems_handle2fd(handle), SHUT_RDWR);

	// close it
	close(ems_handle2fd(handle));
}
CSocketEventSelectEx::handle_type CSocketEventSelectEx::socket_accept(handle_type handle)
{
	// check
	//ems_assert_return_val(handle, ems_null);

	// accept  
	struct sockaddr_in d;
	socklen_t	n = sizeof(struct sockaddr_in);
	long_type 	fd = (long_type)accept(ems_handle2fd(handle), (struct sockaddr *)&d, &n);

	// no client?
	ems_check_return_val(fd > 0, ems_null);

	// non-block
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

	// ok
	return ems_fd2handle(fd);
}
CSocketEventSelectEx::size_type CSocketEventSelectEx::socket_listen(handle_type handle, size_type port, bool_type bforce)
{
	// check
	//ems_assert_return_val(handle, 0);

	// init
	struct sockaddr_in d = {0};
	d.sin_family = AF_INET;
	d.sin_port = htons(port);
	d.sin_addr.s_addr = INADDR_ANY;

	// reuse addr
#ifdef SO_REUSEADDR
	{
		int_type reuseaddr = 1;
		if (setsockopt(ems_handle2fd(handle), SOL_SOCKET, SO_REUSEADDR, (int_type *)&reuseaddr, sizeof(reuseaddr)) < 0) 
			ems_trace("reuseaddr: %lu failed", port);
	}
#endif

	// reuse port
#ifdef SO_REUSEPORT
	if (port)
	{
		int_type reuseport = 1;
		if (setsockopt(ems_handle2fd(handle), SOL_SOCKET, SO_REUSEPORT, (int_type *)&reuseport, sizeof(reuseport)) < 0) 
			ems_trace("reuseport: %lu failed", port);
	}
#endif

	// bind 
	if (::bind(ems_handle2fd(handle), (struct sockaddr *)&d, sizeof(d)) < 0)
	{
		// force to bind it
		if (bforce)
		{
			// rebind it
			d.sin_port = htons(0);
			if (::bind(ems_handle2fd(handle), (struct sockaddr *)&d, sizeof(d)) < 0) return 0;
		}
		else return 0;
	}
	
	// no port?
	if (!port)
	{
		// get the new port
		int_type n = sizeof(d);
		if (getsockname(ems_handle2fd(handle), (struct sockaddr *)&d, (socklen_t *)&n) == -1) return 0;
		port = ntohs(d.sin_port);
	}

	// listen it
	if (listen(ems_handle2fd(handle), 5) < 0) return 0;

	// ok?
	return port;
}
CSocketEventSelectEx::long_type CSocketEventSelectEx::socket_connect(handle_type handle, char_type const* addr)
{
	// check
	//ems_assert_return_val(handle && addr, -1);

	// init host info
	CHostInfo hi;
	hi.SetNodeString(addr);
	//ems_assert_return_val(hi.IsValid(), -1);

	// init
	struct sockaddr_in d = {0};
	d.sin_family = AF_INET;
	d.sin_port = htons(hi.GetPort());
	d.sin_addr.s_addr = hi.GetIP();

    errno = 0;
	// connect
	long_type r = connect(ems_handle2fd(handle), (struct sockaddr *)&d, sizeof(d));
    int nerr = errno;
    
    ems_trace("connect[%p]: %s: ret:%d errno:%d", handle, addr, r, nerr);

	// ok?
	if (!r || nerr == EISCONN) return 1;

	// continue?
	if (nerr == EINTR || nerr == EINPROGRESS || nerr == EAGAIN) return 0;

    ems_trace("pthread_create: failed: errno: %d", nerr);
	// error
	return -1;
}
CSocketEventSelectEx::long_type CSocketEventSelectEx::socket_recv(handle_type handle, byte_type* data, size_type size)
{
	// check
	//ems_assert_return_val(handle && data, -1);

	// no data?
	ems_check_return_val(size, 0);

    errno = 0;
	// recv
	long_type real = recv(ems_handle2fd(handle), data, (int_type)size, 0);
    int nerr = errno;
    ems_trace("recieve[%p]: ret:%d errno:%d", handle, real, nerr);

	// ok?
	if (real >= 0) return real;

	// continue?
	if (nerr == EINTR || nerr == EAGAIN) return 0;

    ems_trace("pthread_create: failed: errno: %d", nerr);
	// error
	return -1;
}
CSocketEventSelectEx::long_type CSocketEventSelectEx::socket_send(handle_type handle, byte_type const* data, size_type size)
{
	// check
	//ems_assert_return_val(handle && data, -1);

	// no data?
	ems_check_return_val(size, 0);

    errno = 0;
	// send
	long_type real = send(ems_handle2fd(handle), data, (int_type)size, 0);
    int nerr = errno;

	// ok?
	if (real >= 0) return real;

	// continue?
	if (nerr == EINTR || nerr == EAGAIN) return 0;
    
    ems_trace("pthread_create: failed: errno: %d", nerr);
	// error
	return -1;
}

/* ////////////////////////////////////////////////////////////////////////////// 
 * aiop implementation
 */
CSocketEventSelectEx::aiop_type* CSocketEventSelectEx::aiop_init()
{
	// done
	bool_type 	ok = ems_false;
	aiop_type* 	aiop = ems_null;
	do
	{
		// make aiop
		aiop = (aiop_type*)calloc(1, sizeof(aiop_type));
		//ems_assert_break(aiop);

		// init fds
		FD_ZERO(&aiop->rfdi);
		FD_ZERO(&aiop->wfdi);
		FD_ZERO(&aiop->efdi);
		FD_ZERO(&aiop->rfdo);
		FD_ZERO(&aiop->wfdo);
		FD_ZERO(&aiop->efdo);

		// init lock
		if (pthread_mutex_init(&aiop->pfds_lock, ems_null)) break;
		if (pthread_mutex_init(&aiop->hash_lock, ems_null)) break;

		// init hash
		aiop->hash = new std::map<handle_type, aioo_type>();
		//ems_assert_break(aiop->hash);

		// init spak
		if (!socket_pair(aiop->spak)) break;

		// addo spak
        if (aiop->spak[0] != ems_null && aiop->spak[1] != ems_null)
        {
            if (!aiop_addo(aiop, aiop->spak[1], AIOE_CODE_RECV, ems_null)) break;
        }

		// ok
		ok = ems_true;

	} while (0);

	// failed?
	if (!ok) 
	{
		aiop_exit(aiop);
		aiop = ems_null;
	}

	// ok?
	return aiop;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::aiop_exit(aiop_type* aiop)
{
	// check
	//ems_assert_return(aiop);

    if (aiop) {
        // exit fds
        pthread_mutex_lock(&aiop->pfds_lock);
        FD_ZERO(&aiop->rfdi);
        FD_ZERO(&aiop->wfdi);
        FD_ZERO(&aiop->efdi);
        FD_ZERO(&aiop->rfdo);
        FD_ZERO(&aiop->wfdo);
        FD_ZERO(&aiop->efdo);
        pthread_mutex_unlock(&aiop->pfds_lock);
        
        // exit hash
        pthread_mutex_lock(&aiop->hash_lock);
        if (aiop->hash) delete aiop->hash;
        aiop->hash = ems_null;
        pthread_mutex_unlock(&aiop->hash_lock);
        
        // exit mutx
        pthread_mutex_destroy(&aiop->pfds_lock);
        pthread_mutex_destroy(&aiop->hash_lock);
        
        // exit spak
        if (ems_null != aiop->spak[0]) socket_exit(aiop->spak[0]);
        if (ems_null != aiop->spak[1]) socket_exit(aiop->spak[1]);
        aiop->spak[0] = ems_null;
        aiop->spak[1] = ems_null;
        
        // exit aiop
        free(aiop);
    }
	
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::aiop_kill(aiop_type* aiop)
{
	// check
	//ems_assert_return(aiop);

	// kill it
    for(int i = 0; i < 3 && aiop; ++i)
    {
        if (aiop && aiop->spak[0]) {
            socket_send(aiop->spak[0], (byte_type const*)"k", 1);
        }
    }
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::aiop_addo(aiop_type* aiop, handle_type handle, size_type code, pointer_type data)
{
	// check
	//ems_assert_return_val(aiop && aiop->hash && handle, ems_false);

	// check size
	pthread_mutex_lock(&aiop->hash_lock);
	size_type size = aiop->hash->size();
	pthread_mutex_unlock(&aiop->hash_lock);
	//ems_assert_return_val(size < FD_SETSIZE, ems_false);

	// init aioo
	aioo_type aioo = {0};
	aioo.code = code;
	aioo.data = data;
	aioo.handle = handle;

	// add handle => aioo
	pthread_mutex_lock(&aiop->hash_lock);
	(*aiop->hash)[aioo.handle] = aioo;
	pthread_mutex_unlock(&aiop->hash_lock);

	// the fd
	long_type fd = ((long_type)handle) - 1;

	// enter
	pthread_mutex_lock(&aiop->pfds_lock);

	// update fd max
	if (fd > aiop->sfdm) aiop->sfdm = fd;
	
	// init fds
	if (code & (AIOE_CODE_RECV | AIOE_CODE_ACPT)) FD_SET(fd, &aiop->rfdi);
	if (code & (AIOE_CODE_SEND | AIOE_CODE_CONN)) FD_SET(fd, &aiop->wfdi);
	FD_SET(fd, &aiop->efdi);

	// leave
	pthread_mutex_unlock(&aiop->pfds_lock);

	// spak it
	if (aiop->spak[0] && code) socket_send(aiop->spak[0], (byte_type const*)"p", 1);

	// ok
	return ems_true;
}
CSocketEventSelectEx::void_type CSocketEventSelectEx::aiop_delo(aiop_type* aiop, handle_type handle)
{
	// check
	//ems_assert_return(aiop && aiop->hash && handle);

	// fd
	long_type fd = ((long_type)handle) - 1;

    if (aiop) {
        // enter
        pthread_mutex_lock(&aiop->pfds_lock);
        
        // del fds
        FD_CLR(fd, &aiop->rfdi);
        FD_CLR(fd, &aiop->wfdi);
        FD_CLR(fd, &aiop->efdi);
        
        // leave
        pthread_mutex_unlock(&aiop->pfds_lock);
        
        // del handle => aioo
        pthread_mutex_lock(&aiop->hash_lock);
        aiop->hash->erase(handle);
        pthread_mutex_unlock(&aiop->hash_lock);
        
        // spak it
        if (aiop->spak[0]) socket_send(aiop->spak[0], (byte_type const*)"p", 1);
    }
	
}
CSocketEventSelectEx::bool_type CSocketEventSelectEx::aiop_post(aiop_type* aiop, handle_type handle, size_type code, pointer_type data)
{
	// check
	//ems_assert_return_val(aiop && aiop->hash && handle, ems_false);
	
	// init aioo
	aioo_type aioo = {0};
	aioo.code = code;
	aioo.data = data;
	aioo.handle = handle;

	// set handle => aioo
	pthread_mutex_lock(&aiop->hash_lock);
	(*aiop->hash)[aioo.handle] = aioo;
	pthread_mutex_unlock(&aiop->hash_lock);

	// fd
	long_type fd = ((long_type)handle) - 1;

	// enter
	pthread_mutex_lock(&aiop->pfds_lock);

	// set fds
	if (code & (AIOE_CODE_RECV | AIOE_CODE_ACPT)) FD_SET(fd, &aiop->rfdi); else FD_CLR(fd, &aiop->rfdi);
	if (code & (AIOE_CODE_SEND | AIOE_CODE_CONN)) FD_SET(fd, &aiop->wfdi); else FD_CLR(fd, &aiop->wfdi);
	if ( 	(code & (AIOE_CODE_RECV | AIOE_CODE_ACPT))
		|| 	(code & (AIOE_CODE_SEND | AIOE_CODE_CONN)))
	{
		FD_SET(fd, &aiop->efdi); 
	}
	else 
	{
		FD_CLR(fd, &aiop->efdi);
	}

	// leave
	pthread_mutex_unlock(&aiop->pfds_lock);

	// spak it
	if (aiop->spak[0]) socket_send(aiop->spak[0], (byte_type const*)"p", 1);

	// ok
	return ems_true;
}


CSocketEventSelectEx::dword_type SpakTickCount(void)
{

}


CSocketEventSelectEx::long_type CSocketEventSelectEx::aiop_wait(aiop_type* aiop, aioe_type* list, size_type maxn, long_type timeout)
{
	// check
	//ems_assert_return_val(aiop && list && maxn, -1);
	// init time
	struct timeval t = {0};
	if (timeout > 0)
	{
		t.tv_sec = timeout / 1000;
		t.tv_usec = (timeout % 1000) * 1000;
	}

	// loop
	long_type wait = 0;
	hong_type time = GetTickCount();
	while (!wait && (timeout < 0 || GetTickCount() < time + timeout))
	{
		// enter
		pthread_mutex_lock(&aiop->pfds_lock);

		// init fdo
		size_type sfdm = aiop->sfdm;
		memcpy(&aiop->rfdo, &aiop->rfdi, sizeof(fd_set));
		memcpy(&aiop->wfdo, &aiop->wfdi, sizeof(fd_set));
		memcpy(&aiop->efdo, &aiop->efdi, sizeof(fd_set));

		// leave
		pthread_mutex_unlock(&aiop->pfds_lock);

		// wait
//		ems_trace("select: ..");
		long_type sfdn = select(sfdm + 1, &aiop->rfdo, &aiop->wfdo, &aiop->efdo, timeout >= 0? &t : ems_null);
//		ems_trace("select: %ld", sfdn);
		//ems_assert_return_val(sfdn >= 0, -1);
		
		// spak tick count
		//SpakTickCount();

		// timeout?
		ems_check_return_val(sfdn, 0);
	
		// enter
		pthread_mutex_lock(&aiop->hash_lock);

		// sync
		std::map<handle_type, aioo_type>::iterator itor = aiop->hash->begin();
		std::map<handle_type, aioo_type>::iterator tail = aiop->hash->end();
		for (; itor != tail && wait >= 0 && wait < maxn; ++itor)
		{
            //ems_check_return_val(!loop_bstoped(), -1);
			// the aioo
			aioo_type const& aioo = itor->second;

			// the handle
			handle_type handle = (handle_type)aioo.handle;
			//ems_assert_continue(handle);

			// spak?
			if (handle == aiop->spak[1] && FD_ISSET(((long_type)aiop->spak[1] - 1), &aiop->rfdo))
			{
				// read spak
				char_type spak = '\0';
                long_type lret = socket_recv(aiop->spak[1], (byte_type*)&spak, 1);
				if (lret < 0)
                {
                    wait = -1;
                }

				// trace
//				ems_trace("spak: %c", spak);

				// killed?
				if (lret == 1 && spak == 'k')
                {
                    wait = -1;
                }
				ems_check_break(wait >= 0);

				// continue it
				continue ;
			}

			// filter spak
			ems_check_continue(handle != aiop->spak[1]);

			// the fd
			long_type fd = (long_type)handle - 1;

			// init aioe
			aioe_type aioe = {0};
			aioe.data 	= aioo.data;
			aioe.handle = handle;
			if (FD_ISSET(fd, &aiop->rfdo)) 
			{
				if (aioo.code & AIOE_CODE_ACPT) aioe.code |= AIOE_CODE_ACPT;
				else aioe.code |= AIOE_CODE_RECV;
			}
			if (FD_ISSET(fd, &aiop->wfdo)) 
			{
				if (aioo.code & AIOE_CODE_CONN) aioe.code |= AIOE_CODE_CONN;
				else aioe.code |= AIOE_CODE_SEND;
			}
			if (FD_ISSET(fd, &aiop->efdo) && !(aioe.code & (AIOE_CODE_RECV | AIOE_CODE_SEND))) 
				aioe.code |= AIOE_CODE_RECV | AIOE_CODE_SEND;

			// ok?
			if (aioe.code) list[wait++] = aioe;
		}

		// leave
		pthread_mutex_unlock(&aiop->hash_lock);
	}

	// ok
	return wait;
}
//_NAMESPACE_END

