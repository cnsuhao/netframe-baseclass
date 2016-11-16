#ifndef __LIVE_TCPSOCKET_WRAPER_H__
#define __LIVE_TCPSOCKET_WRAPER_H__

//#define USE_BOOST_ASIO_SOCKET

#ifdef USE_BOOST_ASIO_SOCKET
#include "baseclass/netframe/SocketBoostAsio.h"
#define CTCPSocketWraper	NS_CSocketBoostAsio::CSocketBoostAsio
#else
#ifdef _WINDOWS
#include "baseclass/netframe/eventselectsocketex.h"
#define CTCPSocketWraper	CSocketEventSelectEx
#else
#ifdef __APPLE__
#include "./mobile/EventSelectSocketEx.h"
#define CTCPSocketWraper	CSocketEventSelectEx
#else
#include "./mobile/EventSelectSocketEx.h"
#define CTCPSocketWraper	CSocketEventSelectEx
#endif

#endif //_WINDOWS
#endif//USE_BOOST_ASIO_SOCKET

#endif //__LIVE_TCPSOCKET_WRAPER_H__
