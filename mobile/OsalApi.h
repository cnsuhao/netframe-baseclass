#pragma once
//#include <Windows.h>
#include <string>
#include "basetype.h"
//#if defined __LINUX__ && !(defined PF_IQIYI_TV)
#include "OsalTypedef.h"
//#endif


//#define tstring std::string

#define OSAL_SAFE_DELETE_ARRAY(p)    if(p) { delete[] (p); (p) = 0; }
#ifdef NDEBUG
  //#define TRACE
#else
  #define TRACE EMSTrace
#endif
//#define VERIFY

//typedef int32 time_t;
#ifdef WIN32
#define lstrlen strlen
#define lstrlenA strlen
#define lstrcmp strcmp
#define lstrcpy strcpy
#define lstrcat strcat
#define lstrcmpi _stricmp
#define stricmp _stricmp
#define strnicmp _strnicmp
#else
bool RemoveDirectory(const char * szPath);
bool CreateDirectory(const char * szPath);
bool GetMacAddr(char *mac);
#define strnicmp strncasecmp
#endif

#define	mSleep(ms)	usleep(1000 * (ms))


std::string stringFormat(const char *fmt, ...);
std::wstring wstringFormat(const wchar_t *fmt, ...);

std::string ws2s(const std::wstring& ws);
std::wstring s2ws(const std::string& s);

std::wstring OsalUtf8ToUnicode(const char * pszUtf8Str);
std::string  OsalUnicodeToUtf8(const wchar_t* pwszStr);

//Platform::String^ GetIPString(uint32 ip);
//Platform::String^ StrFormat(const char *fmt, ...);

void StringTrim(std::string& str);
std::string StringTokenize(std::string src, const char *szDelimiters, int &iStart);
void StrToLower(string & strSrc, string & strDest);

//uint32  GetPortFromString(Platform::String^ portStr);
//uint32  GetIPFromeHostName(Windows::Networking::HostName^ host);

int URLParse(const char * szURL, std::string& strServiceType, std::string& strServer, std::string& strObject, WORD& nPort);

//unsigned int GetTickCount();
//time_t time(time_t *t);

//std::wstring GetGuidStr();

bool OsalGetProp(const wchar_t *container, const wchar_t *key, const wchar_t *default_val, wchar_t *prop, int size);
bool OsalSetProp(const wchar_t *container, const wchar_t *key, const wchar_t *value);
int OsalGetPropInt(const wchar_t *container, const wchar_t *key, int default_val);


int PPSGetPrivateProfileStringW( const wchar_t *section, const wchar_t *key, const wchar_t * default_value, wchar_t *value, unsigned int size, const wchar_t *file);
int PPSGetPrivateProfileIntW( const wchar_t *section, const wchar_t *key,int default_value, const wchar_t *file);
int PPSWritePrivateProfileStringW( const wchar_t *section, const wchar_t *key,const wchar_t *value, const wchar_t *file);

int PPSGetPrivateProfileString( const char *section, const char *key, const char * default_value, char *value, unsigned int size, const char *file);
int PPSGetPrivateProfileInt( const char *section, const char *key,int default_value, const char *file);
int PPSWritePrivateProfileString( const char *section, const char *key,const char *value, const char *file);

#ifndef MAX
#define MAX(x, y)  ((x > y)? x: y)
#endif
#ifndef MIN
#define MIN(x, y)  ((x < y)? x: y)
#endif


#if defined(ANDROID)||defined(__APPLE__)
// [wangrunqing]: atomic operations
typedef volatile long ems_atomic_t;
#if defined(__GNU__)||defined(__APPLE__)
#define ems_atomic_get(a) 									((LONG)__sync_val_compare_and_swap((ems_atomic_t*)(a), 0, 0))
#define ems_atomic_set(a, v) 								__sync_lock_test_and_set((ems_atomic_t*)(a), (LONG)(v))
#define ems_atomic_set0(a) 									__sync_lock_test_and_set((ems_atomic_t*)(a), 0)
#define ems_atomic_pset(a, p, v) 							__sync_val_compare_and_swap((ems_atomic_t*)(a), (LONG)(p), (LONG)(v))
#define ems_atomic_fetch_and_set0(a) 						((LONG)__sync_lock_test_and_set((ems_atomic_t*)(a), 0))
#define ems_atomic_fetch_and_set(a, v) 						((LONG)__sync_lock_test_and_set((ems_atomic_t*)(a), (LONG)(v)))
#define ems_atomic_fetch_and_pset(a, p, v) 					((LONG)__sync_val_compare_and_swap((ems_atomic_t*)(a), (LONG)(p), (LONG)(v)))
#define ems_atomic_fetch_and_inc(a) 						((LONG)__sync_fetch_and_add((ems_atomic_t*)(a), 1))
#define ems_atomic_fetch_and_dec(a) 						((LONG)__sync_fetch_and_sub((ems_atomic_t*)(a), 1))
#define ems_atomic_inc_and_fetch(a) 						((LONG)__sync_add_and_fetch((ems_atomic_t*)(a), 1))
#define ems_atomic_dec_and_fetch(a) 						((LONG)__sync_sub_and_fetch((ems_atomic_t*)(a), 1))
#elif defined(WIN32)
#define ems_atomic_get(a) 									ems_atomic_fetch_and_pset(a, 0, 0)
#define ems_atomic_set(a, v) 								ems_atomic_fetch_and_set(a, v)
#define ems_atomic_set0(a) 									ems_atomic_fetch_and_set(a, 0)
#define ems_atomic_pset(a, p, v) 							ems_atomic_fetch_and_pset(a, p, v)
#define ems_atomic_fetch_and_set0(a) 						ems_atomic_fetch_and_set(a, 0)
static inline LONG ems_atomic_fetch_and_set(ems_atomic_t* a, LONG v)
{
	return (LONG)InterlockedExchange((LONG volatile*)a, v);
}
static inline LONG ems_atomic_fetch_and_pset(ems_atomic_t* a, LONG p, LONG v)
{
	return (LONG)InterlockedCompareExchange((LONG volatile*)a, v, p);
}
static inline LONG ems_atomic_fetch_and_inc(ems_atomic_t* a)
{
	LONG o = ems_atomic_get(a);
	InterlockedIncrement((LONG volatile*)a);
	return o;
}
static inline LONG ems_atomic_fetch_and_dec(ems_atomic_t* a)
{
	LONG o = ems_atomic_get(a);
	InterlockedDecrement((LONG volatile*)a);
	return o;
}
static inline LONG ems_atomic_inc_and_fetch(ems_atomic_t* a)
{
	return (LONG)InterlockedIncrement((LONG volatile*)a);
}
static inline LONG ems_atomic_dec_and_fetch(ems_atomic_t* a)
{
	return (LONG)InterlockedDecrement((LONG volatile*)a);
}
#else
#endif

// align
#define ems_align2(x) 					(((x) + 1) >> 1 << 1)
#define ems_align4(x) 					(((x) + 3) >> 2 << 2)
#define ems_align8(x) 					(((x) + 7) >> 3 << 3)
#define ems_align(x, b) 				(((x) + ((b) - 1)) & ~((b) - 1))

// [wangrunqing]: the number of entries in the array
#define ems_arrayn(x) 					((sizeof((x)) / sizeof((x)[0])))

// [wangrunqing]: static assert
#define ems_assert_static(x) 			do { typedef int __ems_static_assert__[(x)? 1 : -1]; } while(0)

// [wangrunqing]: assert
#define ems_assert(x)					do { if (!(x)) { EMSDBG("[wrq]: [assert]: expr: %s, at file: %s\n", #x, __FILE__);   } } while(0)
#define ems_assert_return(x)			do { if (!(x)) { EMSDBG("[wrq]: [assert]: expr: %s, at file: %s\n", #x, __FILE__);  return ; } } while(0)
#define ems_assert_return_val(x, v)		do { if (!(x)) { EMSDBG("[wrq]: [assert]: expr: %s, at file: %s\n", #x, __FILE__);  return (v); } } while(0)
#define ems_assert_break(x)				{ if (!(x)) { EMSDBG("[wrq]: [assert]: expr: %s, at file: %s\n", #x, __FILE__);  break ; } }
#define ems_assert_continue(x)			{ if (!(x)) { EMSDBG("[wrq]: [assert]: expr: %s, at file: %s\n", #x, __FILE__);  continue ; } }

// [wangrunqing]: check
#define ems_check_return(x)				do { if (!(x)) { return ; } } while(0)
#define ems_check_return_val(x, v)		do { if (!(x)) { return (v); } } while(0)
#define ems_check_break(x)				{ if (!(x)) { break ; } }
#define ems_check_continue(x)			{ if (!(x)) { continue ; } }

/* ///////////////////////////////////////////////////////////////////////
 * [wangrunqing]: the bits operations
 *
 * @note: reference from the tbox library
 */

#define ems_bits_set1(p, i) 			do {((BYTE*)(p))[(i) >> 3] |= (0x1 << (7 - ((i) & 7)));} while (0)
#define ems_bits_set0(p, i) 			do {((BYTE*)(p))[(i) >> 3] &= ~(0x1 << (7 - ((i) & 7)));} while (0)
#define ems_bits_bset(p, i) 			(((BYTE*)(p))[(i) >> 3] & (0x1 << (7 - ((i) & 7))))

#define ems_bits_get_u64_le(p) 			((UINT64)*((p) + 7) << 56 | (UINT64)*((p) + 6) << 48 | (UINT64)*((p) + 5) << 40 | (UINT64)*((p) + 4) << 32 | (UINT64)*((p) + 3) << 24 | (UINT64)*((p) + 2) << 16 | (UINT64)*((p) + 1) << 8 | (UINT64)*(p))
#define ems_bits_get_u64_be(p) 			((UINT64)*(p) << 56 | (UINT64)*((p) + 1) << 48 | (UINT64)*((p) + 2) << 40 | (UINT64)*((p) + 3) << 32 | (UINT64)*((p) + 4) << 24 | (UINT64)*((p) + 5) << 16 | (UINT64)*((p) + 6) << 8 | (UINT64)*((p) + 7))

// optimization for gcc
#if defined(__GNUC__) && __GNUC__ >= 4

# 	define ems_bits_swap_u32(x) 		__builtin_bswap32(x)
# 	define ems_bits_swap_u64(x) 		__builtin_bswap64(x)

# 	define ems_bits_cl0_u32_be(x) 		((x)? (DWORD)__builtin_clz((UINT32)(x)) : 32)
# 	define ems_bits_cl0_u32_le(x) 		((x)? (DWORD)__builtin_ctz((UINT32)(x)) : 32)
# 	define ems_bits_cl0_u64_be(x) 		((x)? (DWORD)__builtin_clzll((UINT64)(x)) : 64)
# 	define ems_bits_cl0_u64_le(x) 		((x)? (DWORD)__builtin_ctzll((UINT64)(x)) : 64)

# 	define ems_bits_cb1_u32(x) 			((x)? (DWORD)__builtin_popcount((UINT32)(x)) : 0)
# 	define ems_bits_cb1_u64(x) 			((x)? (DWORD)__builtin_popcountll((UINT64)(x)) : 0)

# 	define ems_bits_fb1_u32_le(x) 		((x)? (DWORD)__builtin_ffs((UINT32)(x)) - 1 : 32)
# 	define ems_bits_fb1_u64_le(x) 		((x)? (DWORD)__builtin_ffsll((UINT64)(x)) - 1 : 64)
#endif

// cl0, count leading bit 0
#ifndef ems_bits_cl0_u32_be
# 	define ems_bits_cl0_u32_be(x) 		ems_bits_cl0_u32_be_inline(x)
#endif
#ifndef ems_bits_cl0_u32_le
# 	define ems_bits_cl0_u32_le(x) 		ems_bits_cl0_u32_le_inline(x)
#endif
#ifndef ems_bits_cl0_u64_be
# 	define ems_bits_cl0_u64_be(x) 		ems_bits_cl0_u64_be_inline(x)
#endif
#ifndef ems_bits_cl0_u64_le
# 	define ems_bits_cl0_u64_le(x) 		ems_bits_cl0_u64_le_inline(x)
#endif

// cl1, count leading bit 1
#ifndef ems_bits_cl1_u32_be
# 	define ems_bits_cl1_u32_be(x) 		ems_bits_cl0_u32_be(~(UINT32)(x))
#endif
#ifndef ems_bits_cl1_u32_le
# 	define ems_bits_cl1_u32_le(x) 		ems_bits_cl0_u32_le(~(UINT32)(x))
#endif
#ifndef ems_bits_cl1_u64_be
# 	define ems_bits_cl1_u64_be(x) 		ems_bits_cl0_u64_be(~(UINT64)(x))
#endif
#ifndef ems_bits_cl1_u64_le
# 	define ems_bits_cl1_u64_le(x) 		ems_bits_cl0_u64_le(~(UINT64)(x))
#endif

// cb1, count bit 1
#ifndef ems_bits_cb1_u32
# 	define ems_bits_cb1_u32(x) 			ems_bits_cb1_u32_inline(x)
#endif
#ifndef ems_bits_cb1_u64
# 	define ems_bits_cb1_u64(x) 			ems_bits_cb1_u64_inline(x)
#endif

// cb0, count bit 0
#ifndef ems_bits_cb0_u32
# 	define ems_bits_cb0_u32(x) 			((x)? (DWORD)ems_bits_cb1_u32(~(UINT32)(x)) : 32)
#endif
#ifndef ems_bits_cb0_u64
# 	define ems_bits_cb0_u64(x) 			((x)? (DWORD)ems_bits_cb1_u64(~(UINT64)(x)) : 64)
#endif

/* fb0, find the first bit 0
 *
 * find bit zero by little endian, fb0(...11101101) == 1
 * find bit zero by big endian, fb0(...11101101) == 27
 */
#ifndef ems_bits_fb0_u32_be
# 	define ems_bits_fb0_u32_be(x) 		((x)? ems_bits_cl0_u32_be(~(UINT32)(x)) : 0)
#endif
#ifndef ems_bits_fb0_u32_le
# 	define ems_bits_fb0_u32_le(x) 		((x)? ems_bits_cl0_u32_le(~(UINT32)(x)) : 0)
#endif
#ifndef ems_bits_fb0_u64_be
# 	define ems_bits_fb0_u64_be(x) 		((x)? ems_bits_cl0_u64_be(~(UINT64)(x)) : 0)
#endif
#ifndef ems_bits_fb0_u64_le
# 	define ems_bits_fb0_u64_le(x) 		((x)? ems_bits_cl0_u64_le(~(UINT64)(x)) : 0)
#endif

// fb1, find the first bit 0
#ifndef ems_bits_fb1_u32_be
# 	define ems_bits_fb1_u32_be(x) 		((x)? ems_bits_cl0_u32_be(x) : 32)
#endif
#ifndef ems_bits_fb1_u32_le
# 	define ems_bits_fb1_u32_le(x) 		((x)? ems_bits_cl0_u32_le(x) : 32)
#endif
#ifndef ems_bits_fb1_u64_be
# 	define ems_bits_fb1_u64_be(x) 		((x)? ems_bits_cl0_u64_be(x) : 64)
#endif
#ifndef ems_bits_fb1_u64_le
# 	define ems_bits_fb1_u64_le(x) 		((x)? ems_bits_cl0_u64_le(x) : 64)
#endif

/* ///////////////////////////////////////////////////////////////////////
 * cl0
 */
static inline DWORD ems_bits_cl0_u32_be_inline(UINT32 x)
{
    ems_check_return_val(x, 32);
    
	DWORD n = 31;
	if (x & 0xffff0000) { n -= 16; 	x >>= 16; 	}
	if (x & 0xff00) 	{ n -= 8; 	x >>= 8; 	}
	if (x & 0xf0) 		{ n -= 4; 	x >>= 4; 	}
	if (x & 0xc) 		{ n -= 2; 	x >>= 2; 	}
	if (x & 0x2) 		{ n--; 					}
    
    return n;
}
static inline DWORD ems_bits_cl0_u32_le_inline(UINT32 x)
{
    ems_check_return_val(x, 32);
    
	DWORD n = 31;
	if (x & 0x0000ffff) { n -= 16; 	} else x >>= 16;
	if (x & 0x00ff) 	{ n -= 8; 	} else x >>= 8;
	if (x & 0x0f) 		{ n -= 4; 	} else x >>= 4;
	if (x & 0x3) 		{ n -= 2; 	} else x >>= 2;
	if (x & 0x1) 		{ n--; 		}
    
    return n;
}
static inline DWORD ems_bits_cl0_u64_be_inline(UINT64 x)
{
    ems_check_return_val(x, 64);
    
	DWORD n = ems_bits_cl0_u32_be((UINT32)(x >> 32));
	if (n == 32) n += ems_bits_cl0_u32_be((UINT32)x);
    
    return n;
}
static inline DWORD ems_bits_cl0_u64_le_inline(UINT64 x)
{
    ems_check_return_val(x, 64);
    
	DWORD n = ems_bits_cl0_u32_le((UINT32)x);
	if (n == 32) n += ems_bits_cl0_u32_le((UINT32)(x >> 32));
    
    return n;
}

/* ///////////////////////////////////////////////////////////////////////
 * cb1
 */
static inline DWORD ems_bits_cb1_u32_inline(UINT32 x)
{
    ems_check_return_val(x, 0);
    
#if 0
	/*
	 * 0x55555555 = 01010101010101010101010101010101
	 * 0x33333333 = 00110011001100110011001100110011
	 * 0x0f0f0f0f = 00001111000011110000111100001111
	 * 0x00ff00ff = 00000000111111110000000011111111
	 * 0x0000ffff = 00000000000000001111111111111111
	 */
    
	x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x & 0x0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f);
	x = (x & 0x00ff00ff) + ((x >> 8) & 0x00ff00ff);
	x = (x & 0x0000ffff) + ((x >> 16) & 0x0000ffff);
#elif 0
	// mit hackmem count
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4)) & 0x0f0f0f0f;
	x = x + (x >> 8);
	x = x + (x >> 16);
	x &= 0x7f;
#elif 0
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4) & 0x0f0f0f0f);
	x = (x * 0x01010101) >> 24;
#elif 0
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4) & 0x0f0f0f0f) % 255;
#else
	x = x - ((x >> 1) & 0x77777777) - ((x >> 2) & 0x33333333) - ((x >> 3) & 0x11111111);
	x = (x + (x >> 4) & 0x0f0f0f0f);
	x = (x * 0x01010101) >> 24;
#endif
    
    return x;
}
static inline DWORD ems_bits_cb1_u64_inline(UINT64 x)
{
    ems_check_return_val(x, 0);
    
#if 0
	x = x - ((x >> 1) & 0x5555555555555555L);
	x = (x & 0x3333333333333333L) + ((x >> 2) & 0x3333333333333333L);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fL;
	x = x + (x >> 8);
	x = x + (x >> 16);
	x = x + (x >> 32);
	x &= 0x7f;
#else
	x = x - ((x >> 1) & 0x7777777777777777ULL) - ((x >> 2) & 0x3333333333333333ULL) - ((x >> 3) & 0x1111111111111111ULL);
	x = (x + (x >> 4) & 0x0f0f0f0f0f0f0f0fULL);
	x = (x * 0x0101010101010101ULL) >> 56;
#endif
    
    return (DWORD)x;
}

/* ///////////////////////////////////////////////////////////////////////
 * spinlock
 */

// the spinlock type
typedef ems_atomic_t 	ems_spinlock_t;

/*! init spinlock
 *
 * @param lock 		the lock
 *
 * @return 			ems_true or ems_false
 */
static inline BOOL ems_spinlock_init(ems_spinlock_t* lock)
{
	// init
	*lock = 0;
    
	// ok
	return ems_true;
}

/*! exit spinlock
 *
 * @param lock 		the lock
 */
static inline void ems_spinlock_exit(ems_spinlock_t* lock)
{
	// exit
	*lock = 0;
}

/*! enter spinlock
 *
 * @param lock 		the lock
 */
static inline void ems_spinlock_enter(ems_spinlock_t* lock)
{
	// init tryn
	volatile DWORD tryn = 5;
    
	// lock it
	while (ems_atomic_fetch_and_pset((ems_atomic_t*)lock, 0, 1))
	{
		if (!tryn--)
		{
			// yield the processor
		#ifdef WIN32
			Sleep(1);
		#else
			sched_yield();
		#endif
            
			// reset tryn
			tryn = 5;
		}
	}
}

/*! try to enter spinlock
 *
 * @param lock 		the lock
 *
 * @return 			ems_true or ems_false
 */
static inline BOOL ems_spinlock_enter_try(ems_spinlock_t* lock)
{
	// try lock it
	return ems_atomic_fetch_and_pset((ems_atomic_t*)lock, 0, 1)? ems_false : ems_true;
}

/*! leave spinlock
 *
 * @param lock 		the lock
 */
static inline void ems_spinlock_leave(ems_spinlock_t* lock)
{
	// leave
    *((ems_atomic_t*)lock) = 0;
}

#endif //WIN32

void Busurecliendid();
