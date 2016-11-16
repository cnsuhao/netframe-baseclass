#include "stdafx.h"
#include "TCPSocketWraper.h"

#ifdef USE_BOOST_ASIO_SOCKET
#include "SocketBoostAsio.cpp"
#else
#ifdef _WINDOWS
#include "eventselectsocketex.cpp"
#else
#ifdef __APPLE__
#include "./mobile/EventSelectSocketEx.cpp"
#else
#include "./mobile/EventSelectSocketEx.cpp"
#endif

#endif //_WINDOWS
#endif//USE_BOOST_ASIO_SOCKET