#ifndef __TYPE__H__
#define __TYPE__H__
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <time.h>
#include "../../_PSL.h"
#include <map>
#include <list>
#include <set>
#include <string>

using namespace std;
/*
#if defined(__LINUX__)||defined(__APPLE__)
#include <netdb.h>
#include <sys/socket.h>
#endif
*/
//#define __LINUX__
//#define __ANDROID__


//#ifdef ANDROID
//#undef ANDROID
//#endif

#ifdef WIN32
#define tsprintf wsprintf
#define tnsprintf swprintf
#else
#define tsprintf sprintf
#define tnsprintf snprintf
#endif


#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL          0
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define Fseek(x,y,z) _fseeki64((x),(y),(z))
#define Ftell(x) _ftelli64(x)
#else
#include <netdb.h>
#include <sys/socket.h>
typedef hostent HOSTENT;
#define Fseek(x,y,z) fseeko64((x),(y),(z))
#define Ftell(x) ftello64(x)
#ifdef OS_ANDROID
#define fseeko64(stream, offset, origin) fseeko(stream, offset, origin)
#define ftello64(stream) ftello(stream)
#define TEXT(quote) __TEXT(quote)
#define __TEXT(quote) L##quote  
#define FILE_BEGIN              0
#define FILE_CURRENT            1
#define FILE_END                2
#endif

//#define INVALID_HANDLE_VALUE -1
#define HLOCAL void*
#define __stdcall
//#define HANDLE UINT
//#define S_FALSE 1
//#define S_OK 0
#define HRESULT long
#define ESRCH           3


#ifndef WINAPI
#define WINAPI __stdcall
#endif

#endif

//huchen 2013.11.6  为android添加CP_ACP的值
#ifndef WIN32
#define CP_ACP                    0
#endif

#ifndef _DISABLEWARNING4786_4355
#define _DISABLEWARNING4786_4355
#pragma warning( disable : 4786 )
#pragma warning( disable : 4355 )
#endif

#ifndef _ENABLEUSESTL
#define _ENABLEUSESTL
#endif

#ifdef _DEBUG
#define CYASSERT(f)
#else
#define CYASSERT(f)
#endif
#define DPASSERT(x) CYASSERT(x)
const int CONFIGURATIONVALUEBUFFER_SIZE = 1030;


typedef unsigned long long	uint64;
typedef unsigned int		uint32;
typedef unsigned short		uint16;
typedef unsigned char		uint8;

typedef long long	int64;
typedef int			int32;
typedef short		int16;


typedef const char          *LPCSTR, *PCSTR;
typedef char                *LPSTR, *PSTR;
#define nil	0
#ifndef _L
#define _L(x) __L(x)
#ifndef __L
#define __L(x) L##x
#endif
#endif


#ifdef WIN32
#define lstrlen  lstrlenW
#define lstrcpy    lstrcpyW
//#define  wchar_t TCHAR
#define  tstring std::wstring
#ifndef _T
#define _T(s) L##s
#endif



#define ttoi _wtoi
#define tstrchr wcschr
#else
//#define lstrlen  lstrlenA
#define lstrcpy    lstrcpyA
#define  tstring std::string
#ifndef _T
#define _T(s) s
#endif

#define ttoi atoi
#define tstrchr strchr
#endif

#ifdef WIN32
#define __WFUNCTION__ _L(__FUNCTION__)
#define __TFUNCTION__ __WFUNCTION__
#define __WFILE__ _L(__FILE__)
#else
#define __WFUNCTION__ L"function"
#define __TFUNCTION__ __FUNCTION__
#define __WFILE__ L"file"
#define DeleteFile(file) remove(file)
#define DeleteFileA(file) remove(file)
#endif




#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#undef far
#undef near
#undef pascal

#define far
#define near
#define CONST               const

#undef FAR
#undef NEAR

#define FAR far
#define NEAR near

#ifndef MAX_PATH
#define MAX_PATH        260
#endif

#ifdef OS_ANDROID
template<class A,class B>
A min(A a,B b)
{
	return a>b?b:a;
}
template<class A,class B>
A max(A a,B b)
{
	return a>b?a:b;
}
#endif

#ifndef interface
#define interface struct
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
//#include <sys/atomic.h>
#define sprintf_s snprintf
typedef void                *PVOID;
typedef int					INT;
typedef int					BOOL, *PBOOL, *LPBOOL;
typedef unsigned int		UINT, UINT32;
typedef char				CHAR;
typedef short				SHORT;
typedef long				LONG;
typedef unsigned int		DWORD;
typedef unsigned long       ULONG;
typedef long long			INT64, __int64, LONGLONG;
typedef unsigned long long	UINT64, DDWORD, ULONGLONG;
typedef unsigned char		BYTE, UCHAR;
typedef unsigned short		WORD, USHORT;
typedef float				FLOAT;
typedef FLOAT				*PFLOAT;
typedef BYTE				*PBYTE;
typedef BYTE				*LPBYTE;
typedef int					*PINT;
typedef int					*LPINT;
typedef WORD				*PWORD;
typedef WORD				*LPWORD;
typedef int				*LPLONG;
typedef DWORD				*PDWORD;
typedef DWORD				*LPDWORD;
typedef void				*LPVOID;
typedef const void			*LPCVOID;
typedef void				*HANDLE,*HMODULE;
typedef wchar_t             WCHAR;
typedef WCHAR               *PWCHAR;
typedef WCHAR               *LPWCH, *PWCH;
typedef ULONG				*PULONG;
typedef void				*HRSRC;
typedef const WCHAR			*LPCWSTR;

// use for crypt
typedef unsigned int ALG_ID;

typedef unsigned long ULONG_PTR, *PULONG_PTR;

typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTKEY;
typedef ULONG_PTR HCRYPTHASH;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef const char          *LPCSTR;
#ifndef _UNICODE
typedef const char          *LPCTSTR;
typedef char                *LPTSTR;
typedef char                TCHAR;
#else
typedef const wchar_t       *LPCTSTR;
typedef wchar_t             *LPTSTR;
typedef wchar_t             TCHAR;
#endif	// _UNICODE


#define ERROR_SUCCESS       0L
#define NO_ERROR            0L
#define ERROR_CRC           23L
//#define INFINITE            0xFFFFFFFF

#define OutputDebugString   printf
#define stricmp             strcasecmp
#define strcmpi             strcasecmp
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

inline void Sleep(unsigned int x)
{
#if defined(OS_IOS) || defined(__APPLE__)
    if (x<40)
    {
        x = 40;
    }
#endif
    usleep(x*1000);
}

//inline unsigned long GetTickCount()
//{
  //  struct timespec t_spec;
  //  clock_gettime(CLOCK_MONOTONIC, &t_spec);
  //  return t_spec.tv_sec *1000 + t_spec.tv_nsec / 1000000;
//    timeval tv;
//    gettimeofday(&tv, NULL);
//    return tv.tv_sec * 1000 +tv.tv_usec / 1000;
//}

inline long InterlockedCompareExchange(volatile long* Destination, long Exchange, long Comperand)
{
    return __sync_val_compare_and_swap((int32_t*)Destination, Comperand, Exchange);
}

inline long InterlockedIncrement(volatile long* Destination)
{
    return __sync_add_and_fetch((int32_t*)Destination, 1);
}

inline long InterlockedDecrement(volatile long* Destination)
{
    return __sync_sub_and_fetch((int32_t*)Destination, 1);
}

#define memcpy_s(dest, size, src, len) memcpy((dest), (src), (min(size, len)))

#endif	// WIN32

/*inline time_t GetTimeSec()
{
#if defined(WIN32)
    return time(NULL);
#else
    struct timespec t_spec;
    clock_gettime(CLOCK_MONOTONIC, &t_spec);
    return t_spec.tv_sec;
#endif
}*/


#define TRACE printf
//#define ASSERT assert
//#define VERIFY assert



#ifdef WIN32
#define INT64_ARG   "%I64d"
#define UINT64_ARG  "%I64u"
#else
#define INT64_ARG   "%lld"
#define UINT64_ARG  "%llu"
#endif // WIN32

#define XML_UTF8_HEAD   "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
#define XML_GB2312_HEAD "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n"

#ifdef WIN32
#define XML_HEAD	XML_GB2312_HEAD
#else
#define XML_HEAD	XML_GB2312_HEAD
#endif

#ifdef WIN32
#define localtime_r(_time_t,_tm) localtime_s((_tm),(_time_t))
#endif

#ifdef WIN32
//#include <boost/algorithm/string.hpp>
#define ON_LINUX_MAKE_LOWER(str)
#define ON_WINDOWS_MAKE_LOWER(str) boost::algorithm::to_lower(str)
#else
//((stringex&)(str)).makelower()
#define ON_LINUX_MAKE_LOWER(str) boost::algorithm::to_lower(str)
#define ON_WINDOWS_MAKE_LOWER(str)
#endif

//HC 2013.11.8
#if !defined(_TRUNCATE)
#define _TRUNCATE ((size_t)-1)
#endif

//HC add for downlaodengine
#ifndef WIN32
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)

#define WAIT_TIMEOUT                     258L    // dderror

/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

typedef	struct linger	LINGER;

#ifndef _ERRCODE_DEFINED
#define _ERRCODE_DEFINED
typedef int errno_t;
#endif

#endif

//HC add for storage
#ifndef WIN32

typedef ULARGE_INTEGER *PULARGE_INTEGER;




#endif

#endif  // __TYPE__H__
