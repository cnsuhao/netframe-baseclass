#ifndef PPSBASE_NETFRAME_SOCKET_EVENT_SELECTEX_H
#define PPSBASE_NETFRAME_SOCKET_EVENT_SELECTEX_H

/* ////////////////////////////////////////////////////////////////////////////// 
 * includes
 */
#include "Osal.h"
#include "../TCPNetwork.h"
//#include "NetBase.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <map>

//_NAMESPACE_BEGIN
/* ////////////////////////////////////////////////////////////////////////////// 
 * class
 */
class CSocketEventSelectEx:public CLivenet5::CSocketIOBase
{
	/// @name the basic types
	/// @{
public:
	typedef int 							int_type;
	typedef unsigned int					uint_type;
	typedef void 							void_type;
	typedef char 							char_type;
	typedef BOOL 							bool_type;
	typedef long 							long_type;
	typedef unsigned long 					size_type;
	typedef BYTE 							byte_type;
	typedef unsigned short 					uint16_type;
	typedef unsigned int 					uint32_type;
	typedef DWORD							dword_type;
	typedef int 							handle_type;
	typedef void* 							pointer_type;
	typedef void const* 					cpointer_type;
	typedef ems_atomic_t 					atomic_type;
	typedef pthread_t 						loop_type;
	typedef pthread_mutex_t 				lock_type;
#if defined(__64BITS__) && !defined(MACOSX)
	typedef long 							int64_type;
	typedef unsigned long 					uint64_type;
#else
	typedef long long 						int64_type;
	typedef unsigned long long 				uint64_type;
#endif
	typedef int64_type 						hong_type;
	typedef uint64_type 					hize_type;
	/// @}
	
	/// @name the aiop types
	/// @{
public:
	/// the aioe code enum, only for sock
	typedef enum __aioe_code_type
	{
		AIOE_CODE_NONE 		= 0x0000
	, 	AIOE_CODE_CONN 		= 0x0001
	, 	AIOE_CODE_ACPT 		= 0x0002
	,	AIOE_CODE_RECV 		= 0x0004
	,	AIOE_CODE_SEND 		= 0x0008
	, 	AIOE_CODE_EALL 		= AIOE_CODE_RECV | AIOE_CODE_SEND | AIOE_CODE_ACPT | AIOE_CODE_CONN
	,	AIOE_CODE_ONESHOT 	= 0x0010

	} 										aioe_code_type;

	/// the aioo type
	typedef struct __aioo_type
	{
		// the code
		size_type 							code;

		// the data
		pointer_type 						data;

		// the handle 
		handle_type 						handle;

	} 										aioo_type;

	/// the aioe type
	typedef aioo_type 						aioe_type;

	// the aiop type
	typedef struct __aiop_type
	{
		// the fd max
		size_type 							sfdm;

		// the fds
		fd_set 								rfdi;
		fd_set 								wfdi;
		fd_set 								efdi;
		fd_set 								rfdo;
		fd_set 								wfdo;
		fd_set 								efdo;

		// the fds lock
		lock_type 							pfds_lock;

		// the spak
		handle_type 						spak[2];

		// the hash: fd => aioo
		std::map<handle_type, aioo_type>* 	hash;

		// the hash lock
		lock_type 							hash_lock;

	} 										aiop_type;
	/// @}
	
	/// @name the loop types
	/// @{
public:

	/// the conn type
	typedef struct __conn_type
	{
		// the addr
		char_type 							addr[32];

		// the data
		byte_type* 							data;

		// the size
		size_type 							size;

		// the maxn
		size_type 							maxn;

	} 										conn_type;

	/// the conn pool type
	typedef std::map<handle_type, conn_type> pool_type;

	/// @}
	
	/// \name the constructors and destructors
	/// @{
public:			
	CSocketEventSelectEx();
	virtual ~CSocketEventSelectEx();
	/// @}

	/// \name the public interfaces
	/// @{
public:	
	static uint_type 		CreateContextID();
	static void_type 		ReleaseContextID(uint_type fd);

public:	
	int_type 				Start(uint32_type ip,uint16_type port, bool_type bforce = ems_true);
	void_type 				Stop();
	bool_type 				IsActive();

public:	
	virtual uint_type 		SocketConnect(char_type const* addr, uint_type fd = 0);
	virtual bool_type 		SocketSend(uint_type fd, byte_type* data, dword_type size);
	virtual void_type 		SocketClose(uint_type fd);
	/// @}

	/// \name the loop interfaces
	/// @{
private:
    void_type               InnerStop();
	bool_type 				loop_init();
	void_type 				loop_exit();
	long_type 				loop_wait(aioe_type* list, size_type maxn, long_type timeout);
	bool_type 				loop_post(handle_type handle, size_type code, pointer_type data = ems_null);
	void_type 				loop_stop() 						{ ems_atomic_set(&m_stop, 1); 		};
	bool_type 				loop_bstoped() 						{ return ems_atomic_get(&m_stop); 	};
	static pointer_type 	loop_done(pointer_type data);
    pointer_type            loop_impl();
	/// @}
			
	/// \name the conn interfaces
	/// @{
private:
	bool_type 				conn_init(conn_type& conn, char_type const* addr = (char_type const*)ems_null);
	void_type 				conn_exit(conn_type& conn);
	handle_type 			conn_acpt(handle_type sock);
	bool_type 				conn_addr(handle_type sock, char_type* addr, size_type maxn);
	long_type 				conn_send(handle_type sock, byte_type const* data, size_type size);
	/// @}
		
	/// \name the socket interfaces
	/// @{
private:
	static handle_type 		socket_init();
	static bool_type 		socket_pair(handle_type pair[2]);
	static void_type 		socket_exit(handle_type handle);
	static handle_type 		socket_accept(handle_type handle);
	static size_type 		socket_listen(handle_type handle, size_type port, bool_type bforce);
	static long_type 		socket_connect(handle_type handle, char_type const* addr);
	static long_type 		socket_recv(handle_type handle, byte_type* data, size_type size);
	static long_type 		socket_send(handle_type handle, byte_type const* data, size_type size);
	/// @}
	
	/// \name the aiop interfaces
	/// @{
private:	
	static aiop_type* 		aiop_init();
	static void_type 		aiop_exit(aiop_type* aiop);
	static void_type 		aiop_kill(aiop_type* aiop);
	static bool_type 		aiop_addo(aiop_type* aiop, handle_type handle, size_type code, pointer_type data = ems_null);
	static void_type 		aiop_delo(aiop_type* aiop, handle_type handle);
	static bool_type 		aiop_post(aiop_type* aiop, handle_type handle, size_type code, pointer_type data = ems_null);
	static long_type 		aiop_wait(aiop_type* aiop, aioe_type* list, size_type maxn, long_type timeout);


	/// @}

	/// \name the loop members
	/// @{
private:
	atomic_type 			m_stop;
	loop_type 				m_loop;
	handle_type 			m_sock;
	aiop_type* 				m_aiop;
	pool_type 				m_pool;
	lock_type 				m_lock;
	/// @}
};

//_NAMESPACE_END
#endif

