#ifndef __OSAL_TYPE_DEF_H__
#define __OSAL_TYPE_DEF_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wchar.h>
#include "basetype.h"

#define FILE_BEGIN              0
#define FILE_CURRENT            1
#define FILE_END                2

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

// [wangrunqing]
#ifndef ems_null
#define ems_null                 (NULL)
#endif

#ifndef ems_false
#define ems_false                (FALSE)
#endif

#ifndef ems_true
#define ems_true                 (TRUE)
#endif
//:~

#ifndef INFINITE
#define INFINITE 0xffffffffu
#endif

#if 0
#ifndef NOMINMAX
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif
#endif

#define	IN
#define	OUT

#define CALLBACK
//#define WINAPI
#define WINAPIV
#define APIENTRY    	    WINAPI
#define APIPRIVATE
#define PASCAL

//#define FAR
//#define NEAR
#define CONST		        const
#define CDECL

#define VOID                void

#if defined(__64BITS__) && !defined(OS_MACOSX)
typedef long				__int64;
typedef long				INT64;
typedef unsigned long		UINT64;
typedef signed long			LONGLONG;
typedef unsigned long		ULONGLONG;
#else
typedef long long           __int64;
typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
#endif
typedef void                *PVOID;
typedef char                CHAR;
typedef short               SHORT;
typedef long                LONG;
typedef SHORT               *PSHORT;  
typedef LONG                *PLONG;    

typedef unsigned char 		UCHAR;
typedef unsigned short 		USHORT;
typedef unsigned long		ULONG;
typedef unsigned int		UINT;

typedef int			        BOOL;

typedef ULONG *			    PULONG;
typedef USHORT *		    PUSHORT;
typedef UCHAR *			    PUCHAR;
typedef char *			    PSZ;
typedef int                	INT;
typedef unsigned int       	*PUINT;

#ifndef __64BITS__
//typedef unsigned long       DWORD;
#else
typedef unsigned int       DWORD;
#endif
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       ULONG_PTR;
typedef ULONG_PTR           SIZE_T;

typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL NEAR           *PBOOL;
typedef BOOL FAR            *LPBOOL;
typedef BYTE NEAR           *PBYTE;
typedef BYTE FAR            *LPBYTE;
typedef int NEAR            *PINT;
typedef int FAR             *LPINT;
typedef WORD NEAR           *PWORD;
typedef WORD FAR            *LPWORD;
typedef int FAR            *LPLONG;
typedef DWORD NEAR          *PDWORD;
typedef DWORD FAR           *LPDWORD;
typedef void FAR            *LPVOID;
typedef CONST void FAR      *LPCVOID;

//typedef unsigned short      WCHAR;
typedef wchar_t             WCHAR;
typedef WCHAR               *PWCHAR;
typedef WCHAR               *LPWCH, *PWCH;
typedef CONST WCHAR         *LPCWCH, *PCWCH;
typedef WCHAR               *NWPSTR;
typedef WCHAR               *LPWSTR, *PWSTR;
typedef CONST WCHAR         *LPCWSTR, *PCWSTR;

typedef CHAR                *PCHAR;
typedef CHAR                *LPCH, *PCH;
typedef CONST CHAR          *LPCCH, *PCCH;
typedef CHAR                *NPSTR;
typedef CHAR                *LPSTR, *PSTR;
typedef CONST CHAR          *LPCSTR, *PCSTR;
typedef char                TCHAR, *PTCHAR;
typedef unsigned char       TBYTE , *PTBYTE ;
typedef LPSTR               LPTCH, PTCH;
typedef LPSTR               PTSTR, LPTSTR;
typedef LPCSTR              LPCTSTR;


#define __TEXT(quote) quote
#define TEXT(quote) __TEXT(quote)

#ifdef L
#undef L
#define L
#endif

typedef int (FAR WINAPI *FARPROC)();
typedef int (NEAR WINAPI *NEARPROC)();
typedef int (WINAPI *PROC)();

typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;
//typedef LONG HRESULT;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#define PALETTEINDEX(i)     ((COLORREF)(0x01000000 | (DWORD)(WORD)(i)))

typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;

typedef PVOID HANDLE;

typedef HANDLE *PHANDLE;
typedef HANDLE NEAR         *SPHANDLE;
typedef HANDLE FAR          *LPHANDLE;
typedef HANDLE              HGLOBAL;
//typedef HANDLE              HLOCAL;
typedef HANDLE              GLOBALHANDLE;
typedef HANDLE              LOCALHANDLE;

typedef WORD                ATOM;

typedef struct hwnd *	    HWND;
typedef struct hdc *	    HDC;
typedef struct hcursor      *HCURSOR;
typedef struct hgdiobj      *HGDIOBJ;
typedef struct hgdiobj      *HBRUSH;
typedef struct hgdiobj      *HPEN;
typedef struct hgdiobj      *HFONT;
typedef struct hgdiobj      *HBITMAP;
typedef struct hgdiobj      *HRGN;
typedef struct hgdiobj      *HPALETTE;
typedef HANDLE		        HICON;
typedef HANDLE		        HINSTANCE;
typedef HANDLE		        HMENU;
typedef HANDLE	            HKEY;
typedef	WORD	            INTERNET_PORT;

#ifndef S_OK
#define S_OK ((HRESULT)0x00000000L)
#endif
#ifndef S_FALSE
#define S_FALSE ((HRESULT)0x00000001L)
#endif
#ifndef	E_FAIL
#define E_FAIL ((HRESULT)0x80000008L)
#endif
#ifndef	NOERROR
#define	NOERROR	0
#endif

#ifndef E_UNEXPECTED
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#endif

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#ifndef	INVALID_HANDLE_VALUE
#define	INVALID_HANDLE_VALUE	((int)-1)
#endif

//typedef	int	SOCKET;

typedef	struct sockaddr	SOCKADDR;
typedef	struct sockaddr *PSOCKADDR;
typedef	struct sockaddr_in	SOCKADDR_IN;
typedef	struct hostent	HOSTENT;
typedef	struct linger	LINGER;

typedef	struct	_WSABUF	{
	ULONG	   len;     /* the length of the buffer */
	char FAR * buf;     /* the pointer to the buffer */
} WSABUF, FAR * LPWSABUF;

#if 0
typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG  HighPart;
	};
	struct {
		DWORD LowPart;
		LONG  HighPart;
	} u;
	LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
	struct {
		DWORD LowPart;
		DWORD HighPart;
	};
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;
	ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
#endif

#ifndef	INVALID_SOCKET
#define	INVALID_SOCKET	((int)-1)
#endif

#define SOCKET_ERROR -1

typedef ULONG_PTR	HCRYPTPROV;
typedef ULONG_PTR 	HCRYPTKEY;
typedef ULONG_PTR 	HCRYPTHASH;
typedef	unsigned int	ALG_ID;


typedef int INT_PTR;

#ifndef ULONG_MAX
#define ULONG_MAX 4294967295u
#endif

#ifndef interface
//typedef struct  interface;
#endif

#define STDMETHOD(x) virtual HRESULT x

#ifndef PURE
#define PURE =0
#endif

#ifndef __stdcall
#define __stdcall
#endif
#define STDMETHODCALLTYPE       __stdcall
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDAPICALLTYPE          __stdcall

#define HMODULE                 HINSTANCE

#define THREAD_PRIORITY_IDLE            -15
#define THREAD_PRIORITY_LOWEST          -2
#define THREAD_PRIORITY_BELOW_NORMAL    -1
#define THREAD_PRIORITY_NORMAL          0
//#define THREAD_PRIORITY_ABOVE_NORMAL    1
//#define THREAD_PRIORITY_HIGHEST         2
#define THREAD_PRIORITY_TIME_CRITICAL   15

#endif //__OSAL_TYPE_DEF_H__
