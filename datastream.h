// DataStream.h: interface for the CDataStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATASTREAM_H__D90A2534_EA73_4BEA_8B7E_87E59A3D1D26__INCLUDED_)
#define AFX_DATASTREAM_H__D90A2534_EA73_4BEA_8B7E_87E59A3D1D26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef WIN32
#include <windows.h>
#endif

#if defined(LIVENET5)
#if !defined(_WINDOWS_) && !defined(_WINDOWS)
#include "../OsalTypedef.h"
#endif
#else
//#include "basetype.h"
#endif

#include <stdio.h> 
#include <assert.h> 
#include <string>
#include <vector>
#include <list>

using namespace std;

#ifndef  __ALIGN_4__
#ifdef __APPLE__
#define __ALIGN_4__
#else
#ifdef ANDROID
#define __ALIGN_4__
#endif //ANDROID
#endif //__APPLE__
#endif //__ALIGN_4__

#define writestring write_string
#define readstring read_string
//_NAMESPACE_BEGIN
/*! jiangyong, 2012-5-28   15:20
*	TODO:      tstring已经在tstring.h定义过了避免重复定义
*/
//#include "tstringex.h"
//typedef basic_string<TCHAR, char_traits<TCHAR>,	allocator<TCHAR> > tstring;

//数据流操作函数
class CDataStream  
{
public :
	CDataStream(BYTE * szBuf,int isize)
	{
        if((NULL ==szBuf)||(0 == isize))
            m_GoodBit = false;
        else
        {
		    m_GoodBit = true;
		    m_isize = isize;
		    buffer = szBuf;
		    current = buffer;
        }
	}
	CDataStream(char * szBuf,int isize)
	{
if((NULL ==szBuf)||(0 == isize))
            m_GoodBit = false;
        else
        {
		    m_GoodBit = true;
		    m_isize = isize;
		    buffer = (BYTE*)szBuf;
		    current = buffer;
        }
	}
	~CDataStream()
	{
	}
	void clear()
	{
		current = buffer;
		if(current)
			current[0]=0;
	}
	//此函数不动态增加内存,一次打印的数据长度不应该超过缓冲区的三分之一,否则可能导致失败
	/*bool printf(const char * format,...)
	{
		if(current)
		{
			if(current - buffer > (m_isize*2)/3)
				return false;
			va_list argPtr ;
			va_start( argPtr, format ) ;
			int count = vsprintf( current, format, argPtr ) ;
			va_end( argPtr );
			current += count ;
			return true;
		}
		return false;
	}*/
	//此函数拷贝字符串
	bool strcpy(const char * szStr)
	{
		if(current&&szStr)
		{
			int ilen = (int)strlen((LPCSTR)szStr);
			if((m_isize-(current - buffer)) < (ilen +2))
				return false;
			memcpy(current,szStr,ilen+1);
			current += ilen;
			return true;
		}
		return false;
	}
	BYTE * getcurrentpos()
	{
		return current;
	}
	bool move(int ilen)//当前指针向后移动ilen
	{

		assert((current + ilen) <= (buffer + m_isize));
		if(m_GoodBit && (current + ilen) <= (buffer + m_isize))
		{
			current += ilen;
		}else{
			m_GoodBit	= false;
		}

		return m_GoodBit;

	}
	void reset()
	{
		current = buffer;
	}
	void push()
	{
		m_stack.push_back(current);
	}
	void pop()
	{
		current = *m_stack.rbegin();
		m_stack.pop_back();
	}
	BYTE readbyte()
	{
		assert((current + 1) <= (buffer + m_isize));
		if(m_GoodBit && (current + 1) <= (buffer + m_isize))
		{
			current ++;
			return *(current-1);
		}
		m_GoodBit = false;
		//return (BYTE)-1;
		return 0;// PGP, 2010-7-13   15:54	返回0更合适
	}
	void writebyte(BYTE btValue)
	{
		assert((current + 1) <= (buffer + m_isize));
		if(m_GoodBit && (current + 1) <= (buffer + m_isize))
		{
			*current = btValue;
			current ++; 	
		}
		else
			m_GoodBit = false;

	}
	WORD readword()
	{
		assert((current + 2) <= (buffer + m_isize));
		if(m_GoodBit && (current + 2) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			current +=2;
			return *((WORD*)(current-2));
  #else
			/* FIXME: assume all data read out is in little endian */
			current += 2;
			return bswap_16((unsigned short)*((WORD*)(current - 2)));
  #endif
#else // __ALIGN_4__
			unsigned short tmp_var = 0;
			unsigned char  *p_tmp = (unsigned char*)&tmp_var;
  #ifndef WORDS_BIGENDIAN
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			current += 2;
  #else
			p_tmp[1] = *current;
			p_tmp[0] = *(current + 1);
			current += 2;
  #endif
			return tmp_var;
#endif
		}

		m_GoodBit = false;
		//return (WORD)-1;
		return 0;// PGP, 2010-7-13   15:54	返回0更合适
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	/* no swaping version */
	WORD readword_ns()
	{
#ifndef __LINUX__
		assert((current + 2) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
		{
			current +=2;
			return *((WORD*)(current-2));
		}

		m_GoodBit = false;
		return (WORD)-1;
	}
#else // __ALIGN_4__
	/* no swaping version */
	WORD readword_ns()
	{
#ifndef __LINUX__
		assert((current + 2) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
		{
			unsigned short tmp_var = 0;
			unsigned char *p_tmp = (unsigned char*)&tmp_var;
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			current += 2;
			return tmp_var;
		}

		m_GoodBit = false;
		return (WORD)-1;
	}
#endif
#endif // WORDS_BIGENDIAN
	void writeword(WORD wValue)
	{		
#ifndef __LINUX__
		assert((current + 2) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			*((WORD*)current) = wValue;
  #else
			/* FIXME: assume all data should be stored in little endian */
			*((WORD*)current) = bswap_16((unsigned short)wValue);
  #endif
#else // __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			unsigned char *p_tmp = (unsigned char*)&wValue;
			*current = p_tmp[0];
			*(current + 1) = p_tmp[1];
  #else
			unsigned char *p_tmp = (unsigned char*)&wValue;
			*current = p_tmp[1];
			*(current + 1) = p_tmp[0];
  #endif
#endif
			current +=2;
		}
		else
			m_GoodBit = false;
	}

#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	/* no swaping version */
	void writeword_ns(WORD wValue)
	{		
#ifndef __LINUX__
		assert((current + 2) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
		{
			*((WORD*)current) = wValue;
			current +=2;
		}
		else
			m_GoodBit = false;
	}
#else //__ALIGN_4__
	void writeword_ns(WORD wValue)
	{		
#ifndef __LINUX__
		assert((current + 2) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
		{
			unsigned char        *p_tmp = (unsigned char*)&wValue;
			*current = p_tmp[0];
			*(current + 1) = p_tmp[1];
			current +=2;
		}
		else
			m_GoodBit = false;
	}
#endif
#endif //WORDS_BIGENDIAN

	float readfloat()
	{
#ifndef __LINUX__
		assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + sizeof(float)) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
			current +=sizeof(float);
  #ifndef WORDS_BIGENDIAN
			return *((float*)(current-sizeof(float)));
  #else
			int midval=bswap_32(*(int*)(current-sizeof(float)));
			return *((float*)&midval);
			/* FIXME: assume sizeof(float) = 4 */
	//		return bswap_float(*((float*)(current-sizeof(float))));
  #endif
#else // __ALIGN_4__
			float tmp_var = 0.0f;
			unsigned char  *p_tmp = (unsigned char*)&tmp_var;
			int i_tmp;
  #ifndef WORDS_BIGENDIAN
			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				p_tmp[i_tmp] = *(current + i_tmp);
  #else
			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				p_tmp[i_tmp] = *(current + sizeof(float) - i_tmp -1);
  #endif
			current +=sizeof(float);
			return tmp_var;
#endif // __ALIGN_4__
		}
		m_GoodBit = false;
		return 0;
		
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	/* no swaping version */
	float readfloat_ns()
	{
#ifndef __LINUX__
		assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
		if(m_GoodBit && (current + sizeof(float)) <= (buffer + m_isize))
		{
			current +=sizeof(float);
			return *((float*)(current-sizeof(float)));
		}
		m_GoodBit = false;
		return 0;
		
	}
#else // __ALIGN_4__
	float readfloat_ns()
	{
#ifndef __LINUX__
		assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + sizeof(float)) <= (buffer + m_isize))
		{
			float tmp_var = 0.0f;
			unsigned char  *p_tmp = (unsigned char*)&tmp_var;
			int i_tmp;
			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				p_tmp[i_tmp] = *(current + i_tmp);
			current +=sizeof(float);
			return tmp_var;

		}
		m_GoodBit = false;
		return 0;
		
	}
#endif
#endif //WORDS_BIGENDIAN

	void writefloat(float fValue)
	{
		assert((current + sizeof(float)) <= (buffer + m_isize));
		if((current + sizeof(float)) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			*((float*)current) = fValue;
  #else
			*current = bswap_32(*(int*)&fValue);
			
			/* FIXME: assume sizeof(float) = 4 */
//			*((float*)current) = bswap_float(fValue);
  #endif
#else  // __ALIGN_4__
			unsigned char *p_tmp = (unsigned char*)&fValue;
			int i_tmp = 0;

  #ifndef WORDS_BIGENDIAN
			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				*(current + i_tmp) = p_tmp[i_tmp];
  #else
			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				*(current + i_tmp) = p_tmp[sizeof(float) - i_tmp - 1];
  #endif
#endif
			current +=sizeof(float);
		}
		else
			m_GoodBit = false;
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	void writefloat_ns(float fValue)
	{
#ifndef __LINUX__
		assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
		if((current + sizeof(float)) <= (buffer + m_isize))
		{
			*((float*)current) = fValue;
			current +=sizeof(float);
		}
		else
			m_GoodBit = false;
	}
#else // __ALIGN_4__
	void writefloat_ns(float fValue)
	{
#ifndef __LINUX__
		assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
		if((current + sizeof(float)) <= (buffer + m_isize))
		{
			unsigned char *p_tmp = (unsigned char*)&fValue;
			int i_tmp = 0;

			for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
				*(current + i_tmp) = p_tmp[i_tmp];
			current +=sizeof(float);
		}
		else
			m_GoodBit = false;
	}
#endif
#endif // WORDS_BIGENDIAN

	int readint()
	{
		assert((current + sizeof(int)) <= (buffer + m_isize));
		if((current + sizeof(int)) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
			current +=sizeof(int);
  #ifndef WORDS_BIGENDIAN
			return *((int*)(current-sizeof(int)));
  #else
			return bswap_32(*((int*)(current-sizeof(int))));
  #endif //WORDS_BIGENDIAN
#else //__ALIGN_4__
			unsigned int tmp_var;
                        unsigned char *p_tmp = (unsigned char*)&tmp_var;
  #ifndef WORDS_BIGENDIAN
                        p_tmp[0] = *current;
                        p_tmp[1] = *(current + 1);
                        p_tmp[2] = *(current + 2);
                        p_tmp[3] = *(current + 3);
  #else
                        p_tmp[3] = *current;
                        p_tmp[2] = *(current + 1);
                        p_tmp[1] = *(current + 2);
                        p_tmp[0] = *(current + 3);
  #endif //WORDS_BIGENDIAN
			//current +=4;
			current +=sizeof(int);
                        return tmp_var;
#endif //__ALIGN_4__
		}
		m_GoodBit = false;
		return 0;

	}
	void writeint(int iValue)
	{
		assert((current + sizeof(int)) <= (buffer + m_isize));
		if((current + sizeof(int)) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			*((int*)current) = iValue;
  #else
			*((int*)current) = bswap_32(iValue);
  #endif
#else
  #ifndef WORDS_BIGENDIAN
			unsigned char *p_tmp = (unsigned char*)&iValue;

                        *current = p_tmp[0];
                        *(current + 1) = p_tmp[1];
                        *(current + 2) = p_tmp[2];
                        *(current + 3) = p_tmp[3];
  #else
			unsigned char *p_tmp = (unsigned char*)&iValue;

                        *current = p_tmp[3];
                        *(current + 1) = p_tmp[2];
                        *(current + 2) = p_tmp[1];
                        *(current + 3) = p_tmp[0];
  #endif
#endif
			current +=sizeof(int);
		}
		else
			m_GoodBit = false;
	}
	DWORD readdword()
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 4) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
			current +=4;
  #ifndef WORDS_BIGENDIAN
			return *((DWORD*)(current-4));
  #else
			return bswap_32(*((DWORD*)(current-4)));
  #endif
#else
			unsigned int tmp_var;
			unsigned char *p_tmp = (unsigned char*)&tmp_var;
  #ifndef WORDS_BIGENDIAN
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			p_tmp[2] = *(current + 2);
			p_tmp[3] = *(current + 3);
  #else
			p_tmp[3] = *current;
			p_tmp[2] = *(current + 1);
			p_tmp[1] = *(current + 2);
			p_tmp[0] = *(current + 3);
  #endif
			current +=4;
			return tmp_var;
#endif
		}
		m_GoodBit = false;
		return 0;
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	DWORD readdword_ns()
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if(m_GoodBit && (current + 4) <= (buffer + m_isize))
		{
			current +=4;
			return *((DWORD*)(current-4));
		}
		m_GoodBit = false;
		return 0;
	}
#else  // __ALIGN_4__ 
	DWORD readdword_ns()
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if((current + 4) <= (buffer + m_isize))
		{
			unsigned int tmp_var;
			unsigned char *p_tmp = (unsigned char*)&tmp_var;
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			p_tmp[2] = *(current + 2);
			p_tmp[3] = *(current + 3);
			current += 4;
			return tmp_var;
		}
		m_GoodBit = false;
		return 0;
	}
#endif
#endif //WORDS_BIGENDIAN

	void writedword(DWORD dwValue)
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if((current + 4) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			*((DWORD*)current) = dwValue;
  #else
			*((DWORD*)current) = bswap_32(dwValue);
  #endif
#else  // __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			unsigned char *p_tmp = (unsigned char*)&dwValue;

			*current = p_tmp[0];
			*(current + 1) = p_tmp[1];
			*(current + 2) = p_tmp[2];
			*(current + 3) = p_tmp[3];
  #else
			unsigned char *p_tmp = (unsigned char*)&dwValue;

			*current = p_tmp[3];
			*(current + 1) = p_tmp[2];
			*(current + 2) = p_tmp[1];
			*(current + 3) = p_tmp[0];
  #endif
#endif
			current +=4;
		}
		else
			m_GoodBit = false;
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	void writedword_ns(DWORD dwValue)
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if((current + 4) <= (buffer + m_isize))
		{
			*((DWORD*)current) = dwValue;
			current +=4;
		}
		else
			m_GoodBit = false;
	}
#else //__ALIGN_4__
	void writedword_ns(DWORD dwValue)
	{
#ifndef __LINUX__
		assert((current + 4) <= (buffer + m_isize));
#endif
		if((current + 4) <= (buffer + m_isize))
		{
			unsigned char *p_tmp = (unsigned char*)&dwValue;

			*current = p_tmp[0];
			*(current + 1) = p_tmp[1];
			*(current + 2) = p_tmp[2];
			*(current + 3) = p_tmp[3];
			current +=4;
		}
		else
			m_GoodBit = false;
	}

#endif
#endif //WORDS_BIGENDIAN

	__int64 readint64()
	{
		assert((current + 8) <= (buffer + m_isize));
		if(m_GoodBit && (current + 8) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
			current +=8;
  #ifndef WORDS_BIGENDIAN
			return *((__int64*)(current-8));
  #else
			return bswap_64(*((__int64*)(current-8)));
  #endif
#else // __ALIGN_4__
			__int64 tmp_var = 0ll;
			unsigned char *p_tmp = (unsigned char*)&tmp_var;
  #ifndef WORDS_BIGENDIAN
			/*
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			p_tmp[2] = *(current + 2);
			p_tmp[3] = *(current + 3);
			p_tmp[4] = *(current + 4);
			p_tmp[5] = *(current + 5);
			p_tmp[6] = *(current + 6);
			p_tmp[7] = *(current + 7);
			*/
			for (int i = 0; i < 8; i++) {
				p_tmp[i] = *(current + i);
			}
  #else
			/*
			p_tmp[0] = *(current + 7);
			p_tmp[1] = *(current + 6);
			p_tmp[2] = *(current + 5);
			p_tmp[3] = *(current + 4);
			p_tmp[4] = *(current + 3);
			p_tmp[5] = *(current + 2);
			p_tmp[6] = *(current + 1);
			p_tmp[7] = *(current);
			*/
			for (int i = 0; i < 8; i++) {
				p_tmp[i] = *(current + 7 - i);
			}
  #endif
			current +=8;
			return tmp_var;
#endif
		}
		
		m_GoodBit = false;
		//return (__int64)-1;
		return 0;// PGP, 2010-7-13   15:54	返回0更合适
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	__int64 readint64_ns()
	{
#ifndef __LINUX__
		assert((current + 8) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 8) <= (buffer + m_isize))
		{
			current +=8;
			return *((__int64*)(current-8));
		}
		
		m_GoodBit = false;
		//return (__int64)-1;
		return 0;// PGP, 2010-7-13   15:54	返回0更合适
	}
#else // __ALIGN_4__
	__int64 readint64_ns()
	{
#ifndef __LINUX__
		assert((current + 8) <= (buffer + m_isize));
#endif
		if(m_GoodBit &&(current + 8) <= (buffer + m_isize))
		{
			__int64 tmp_var = 0ll;
			unsigned char *p_tmp = (unsigned char*)&tmp_var;
			/*
			p_tmp[0] = *current;
			p_tmp[1] = *(current + 1);
			p_tmp[2] = *(current + 2);
			p_tmp[3] = *(current + 3);
			p_tmp[4] = *(current + 4);
			p_tmp[5] = *(current + 5);
			p_tmp[6] = *(current + 6);
			p_tmp[7] = *(current + 7);
			*/
			for (int i = 0; i < 8; i++) {
				p_tmp[i] = *(current + i);
			}
			current += 8;
			return tmp_var;
		}
		
		m_GoodBit = false;
		//return (__int64)-1;
		return 0;// PGP, 2010-7-13   15:54	返回0更合适
	}
#endif
#endif // WORDS_BIGENDIAN

	void writeint64(__int64 iValue)
	{
		assert((current + 8) <= (buffer + m_isize));
		if((current + 8) <= (buffer + m_isize))
		{
#ifndef __ALIGN_4__
  #ifndef WORDS_BIGENDIAN
			*((__int64*)current) = iValue;
  #else
			*((__int64*)current) = bswap_64(iValue);
  #endif
#else // __ALIGN_4__
			unsigned char *p_tmp = (unsigned char*)&iValue;
  #ifndef WORDS_BIGENDIAN
			/*
			*(current) = p_tmp[0];
			*(current + 1) = p_tmp[1];
			*(current + 2) = p_tmp[2];
			*(current + 3) = p_tmp[3];
			*(current + 4) = p_tmp[4];
			*(current + 5) = p_tmp[5];
			*(current + 6) = p_tmp[6];
			*(current + 7) = p_tmp[7];
			*/
			for (int i = 0; i < 8; i++) {
				*(current + i) = p_tmp[i];
			}
  #else
			/*
			*(current) = p_tmp[7];
			*(current + 1) = p_tmp[6];
			*(current + 2) = p_tmp[5];
			*(current + 3) = p_tmp[4];
			*(current + 4) = p_tmp[3];
			*(current + 5) = p_tmp[2];
			*(current + 6) = p_tmp[1];
			*(current + 7) = p_tmp[0];
			*/
			for (int i = 0; i < 8; i++) {
				*(current + i) = p_tmp[7 - i];
			}
  #endif
#endif
			current +=8;
		}
		else
			m_GoodBit = false;
	}
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
	void writeint64_ns(__int64 iValue)
	{
#ifndef __LINUX__
		assert((current + 8) <= (buffer + m_isize));
#endif
		if((current + 8) <= (buffer + m_isize))
		{
			*((__int64*)current) = iValue;
			current +=8;
		}
		else
			m_GoodBit = false;
	}
#else // __ALIGN_4__
	void writeint64_ns(__int64 iValue)
	{
#ifndef __LINUX__
		assert((current + 8) <= (buffer + m_isize));
#endif
		if((current + 8) <= (buffer + m_isize))
		{
			unsigned char *p_tmp = (unsigned char*)&iValue;
			/*
			*(current) = p_tmp[0];
			*(current + 1) = p_tmp[1];
			*(current + 2) = p_tmp[2];
			*(current + 3) = p_tmp[3];
			*(current + 4) = p_tmp[4];
			*(current + 5) = p_tmp[5];
			*(current + 6) = p_tmp[6];
			*(current + 7) = p_tmp[7];
			*/
			for (int i = 0; i < 8; i++) {
				*(current + i) = p_tmp[i];
			}
			current +=8;
		}
		else
			m_GoodBit = false;
	}
#endif
#endif // WORDS_BIGENDIAN

	BYTE * readdata(DWORD dwLen)
	{
		assert((current + dwLen) <= (buffer + m_isize));
		if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
		{
			current +=dwLen;
			return (BYTE*)(current-dwLen);
		}
		
		m_GoodBit = false;
		return NULL;
	}
	//增加安全接口,当读内存越界时,返回错误
	//2008.5.5, Add by YP.
 	bool readdata(DWORD dwLen,BYTE * pbyData)
 	{
 		if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
 		{
 			memcpy(pbyData,current,dwLen);
 			current +=dwLen;
 			return true;
 		}
 		m_GoodBit = false; //add by pgp
 		return false;
 	}
	bool readdata(BYTE * pbyData,DWORD dwLen)
	{
		if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
		{
			memcpy(pbyData,current,dwLen);
			current +=dwLen;
			return true;
		}
		m_GoodBit = false; //add by pgp
		return false;
	}
	void writedata(BYTE const * pData,DWORD dwLen)
	{
		assert((current + dwLen) <= (buffer + m_isize));
		if((current + dwLen) <= (buffer + m_isize))
		{
			memcpy(current,pData,dwLen);		
			current +=dwLen;
		}
		else
			m_GoodBit = false;
	}

	// PGP, 2010-9-10   17:51	读UTF8字符串
	char * read_utf8_string()
	{
		int ilen = 0;
		int buf_left = leavedata();
		bool good = false;
		for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
		{
			if(0==current[ilen])
			{
				good	= true;
				break;
			}
		}
		if(!good)
		{
			m_GoodBit	= false;
			return NULL;
		}
		char * szRes = (char*)current;
		if(m_GoodBit && (current + ilen) <= (buffer + m_isize))
		{
			current +=(ilen+1);
			return szRes;
		}
		m_GoodBit = false;
		return NULL;
	}
	wchar_t* read_wstring()
	{
		int ilen = 0;
		int buf_left = leavedata()/sizeof(wchar_t);
		bool good = false;
		for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
		{
			if(0==((wchar_t*)current)[ilen])
			{
				good	= true;
				break;
			}
		}
		if(!good)
		{
			m_GoodBit	= false;
			return NULL;
		}
		wchar_t * szRes = (wchar_t*)current;
		if(m_GoodBit && (current + ilen*sizeof(wchar_t)+2) <= (buffer + m_isize))
		{
			current +=(ilen*sizeof(wchar_t)+2);
			return szRes;
		}
		m_GoodBit = false;
		return NULL;
	}
	char * read_string()
	{
		int ilen = 0;
		int buf_left = leavedata();
		bool good = false;
		for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
		{
			if(0==current[ilen])
			{
				good	= true;
				break;
			}
		}
		if(!good)
		{
			m_GoodBit	= false;
			return "";
		}
		char * szRes = (char*)current;
		if(m_GoodBit && (current + ilen+1) <= (buffer + m_isize))
		{
			current +=(ilen+1);
			return szRes;
		}
		m_GoodBit = false;
		return "";
	}
	
	//add by lh 原因：原来的strcpy函数操作时没有将0Copy进去，函数名没有对应
	bool write_string(const char * szStr)
	{
		if(current&&szStr)
		{
			int ilen = (int)strlen(szStr);
            if(ilen>0)
            {
			    if((m_isize-(current - buffer)) < (ilen +1))	//原来为(ilen +2)
				    return false;
			    //memcpy(current,szStr,ilen+1);
			    //current += (ilen+1);				//修改原来的current += ilen
			    //return true;
			    // PANGUIPIN, 2011-4-18   16:59	拷贝字符串时，不管字符串是否为空，都要拷贝一个0进去
			    memcpy(current,szStr,ilen);
			    current += (ilen);	
            }
		}
		writebyte(0);
		return true;
	}
	bool write_utf8_string(const char * szStr)
	{
		if(current&&szStr)
		{
			int ilen = (int)strlen((LPCSTR)szStr);
            if(ilen>0)
            {
			    if((m_isize-(current - buffer)) < (ilen +1))	//原来为(ilen +2)
				    return false;
    // 			memcpy(current,szStr,ilen+1);
    // 			current += (ilen+1);				//修改原来的current += ilen
    // 			return true;
			    // PANGUIPIN, 2011-4-18   16:59	拷贝字符串时，不管字符串是否为空，都要拷贝一个0进去
			    //否则连续拷贝多个字符串，有些是空的，当读时就会有错误
			    memcpy(current,szStr,ilen);
			    current += (ilen);	
            }
		}
		writebyte(0);
		return true;
	}
	bool write_wstring(const wchar_t * szStr)
	{
		if(current&&szStr)
		{
			int ilen = (int)wcslen(szStr);
            if(ilen>0)
            {
			    if((m_isize-(current - buffer)) < (ilen*2+2))
				    return false;
    // 			memcpy(current,szStr,ilen*2+2);
    // 			current += (ilen*2+2);				
    // 			return true;
			    memcpy(current,szStr,ilen*2);
			    current += (ilen*2);
            }
		}
		writeword(0);
		return true;
	}

	//标示操作是否成功
	/*! Simon.M.Lu, 2007-11-22   11:10
	*	设置good_bit
	*/
	void good_bit(bool flag){m_GoodBit=flag;}
	bool good_bit()
	{
		return m_GoodBit;
	}

	//add end 
	int size()
	{
		return (int)(current-buffer);
	}
	int leavedata()//缓冲区剩余字节
	{
		return m_isize-size();
	}
	const BYTE * getbuffer(){return buffer;}
//	const char * getbuffer(){return (char*)buffer;}
	int getbufferlength(void)const{return m_isize;}
protected :
	//用来对读写操作检查是否成功,当失败时设置为false,对应的数据返回为0
	bool 	m_GoodBit;

	BYTE* buffer;
	BYTE* current;
	int m_isize;
	list<BYTE*> m_stack;//堆栈
};


//增加流操作符
//use:
//	CDataStream cdrIn(pBuff, len);
//	cdrIn >> dwLen >> ucFlag;
// 	assert(cdrIn.good_bit());		//check

//	CDataStream cdOut(pBuff, len);
//	cdOut << dwLen << ucFlag;
// 	assert(cdOut.good_bit());		//check

//输入流
#ifdef WIN32
inline CDataStream & operator >> (CDataStream &is, DWORD & x)
{
	x = is.readdword();
	return  is;
}
#endif
/*! panguipin, 2012-5-25   14:08
*	添加unsigned int
*/
 inline CDataStream & operator >> (CDataStream &is, unsigned int & x)
 {
	 x = is.readdword();
	 return  is;
 }
inline CDataStream & operator >> (CDataStream &is, WORD & x)
{
	x = is.readword();
	return  is;
}
inline CDataStream & operator >> (CDataStream &is, BYTE & x)
{
	x = is.readbyte();
	return  is;
}

inline CDataStream & operator >> (CDataStream &is, float & x)
{
	x = is.readfloat();
	return  is;
}
inline CDataStream & operator >> (CDataStream &is, __int64 & x)
{
	x = is.readint64();
	return  is;
}

inline CDataStream & operator >> (CDataStream &is, int & x)
{
	x = is.readint();
	return  is;
}

inline CDataStream & operator >> (CDataStream &is, UINT64 & x)
{
	x = is.readint64();
	return  is;
}
inline CDataStream & operator >> (CDataStream &is, string & x)
{
	char * pstr = is.read_utf8_string();
	if(pstr)//如果是空指针,赋值给string会崩溃
		x = pstr;
	return  is;
}
inline CDataStream & operator >> (CDataStream &is, wstring & x)
{
	wchar_t * pstr = is.read_wstring();
	if(pstr)//如果是空指针,赋值给string会崩溃
		x = pstr;
	return  is;
}


//输出流
#ifdef WIN32
inline CDataStream & operator << (CDataStream &os, const DWORD & x)
{
	os.writedword(x);
	return  os;
}
#endif
/*! panguipin, 2012-5-25   14:08
*	添加unsigned int
*/
 inline CDataStream & operator << (CDataStream &os, const unsigned int & x)
 {
	 os.writedword(x);
	 return  os;
 }
inline CDataStream & operator << (CDataStream &os, const WORD & x)
{
	os.writeword(x);
	return  os;
}
inline CDataStream & operator << (CDataStream &os, const BYTE & x)
{
	os.writebyte(x );
	return  os;
}

inline CDataStream & operator << (CDataStream &os, const float & x)
{
	os.writefloat(x);
	return  os;
}
inline CDataStream & operator << (CDataStream &os, const __int64 & x)
{
	os.writeint64(x);
	return  os;
}
inline CDataStream & operator << (CDataStream &os, const int & x)
{
	os.writeint(x);
	return  os;
}

inline CDataStream & operator << (CDataStream &os, const UINT64 & x)
{
	os.writeint64(x);
	return  os;
}

inline CDataStream & operator << (CDataStream &os, const string & x)
{
	os.write_utf8_string(x.c_str());
	return  os;
}
inline CDataStream & operator << (CDataStream &os, const wstring & x)
{
	os.write_wstring(x.c_str());
	return  os;
}
//使用
//CInputDataStream dsIn(pBuf, bufLen);
//dsIn >> dwLen1 >> ucFlag1;
// 	assert(dsIn.good_bit());		//check

//数据流操作函数,输入
class CInputDataStream : public CDataStream
{
public:
	CInputDataStream(BYTE * szBuf, int isize)
		:CDataStream(szBuf, isize)
	{
	}
	CInputDataStream & operator >> (DWORD & dwValue)
	{
		dwValue = readdword();
		return *this;
	}
	
	CInputDataStream & operator >> (WORD & wValue)
	{
		wValue = readword();
		return *this;
	}
	CInputDataStream & operator >> (BYTE & ucValue)
	{
		ucValue = readbyte();
		return *this;
	}

	CInputDataStream & operator >> (float & fValue)
	{
		fValue = readfloat();
		return *this;
	}
	CInputDataStream & operator >> (__int64 & i64Value)
	{
		i64Value = readint64();
		return *this;
	}
	CInputDataStream & operator >> (string & strValue)
	{
		strValue = read_string();
		return *this;
	}
};

//输出流,增加内存自动分配功能及流操作符重载
//使用
//COutputDataStream dsOut;
//dsOut << (DWORD)0 << BYTE('Z') << (WORD)wTest;
//assert(dsOut.good_bit());		//check

class COutputDataStream : public CDataStream
{
public:
	#define CHUNK 1024
	COutputDataStream()
		:CDataStream((BYTE*)NULL, 0),
		pOutBuf(NULL)
	{
		//预分配1024的内存
		pOutBuf = new BYTE[CHUNK];
		buffer =  pOutBuf;
		current = buffer;	
		m_isize = CHUNK;
	}
	//预分配指定的缓冲区大小
	COutputDataStream(int isize)
		:CDataStream((BYTE*)NULL, isize)
		,pOutBuf(NULL)
	{
		pOutBuf = new BYTE[isize];
		buffer =  pOutBuf;
		current = buffer;	
		m_isize = isize;
	}

	COutputDataStream(BYTE * szBuf, int isize);
	//	:CDataStream(szBuf, isize),
	//	pOutBuf(NULL)
	//{
	//}

	~COutputDataStream()
	{
		if(NULL != pOutBuf)
		{
			delete [] pOutBuf;
		}
		pOutBuf = NULL;
		buffer = NULL;
		m_isize = 0;
	}

	inline void ReAllocMem(size_t nLen = 0)
	{
		//分配内存增倍
		DWORD dwSize = m_isize * 2;
		size_t nSize = current - buffer;	//原来的消息长度
		m_isize = max(dwSize, static_cast<DWORD>(nLen+nSize));

		bool bDelete =true;
		if(NULL == pOutBuf)
			bDelete = false;
		//重分配内存
		//vctBuff.resize(m_isize);
		pOutBuf = new BYTE[m_isize];

		//原来的数据Copy到的新缓冲区
		memcpy(pOutBuf, buffer, nSize);

		//重新设置缓冲位置等
		if(bDelete)
			delete [] buffer;			//删除旧的

		buffer =  pOutBuf;
		current = buffer + nSize;
	}

	COutputDataStream& operator << ( DWORD & dwValue )
	{
		if((current + sizeof(dwValue)) > (buffer + m_isize))
		{
			ReAllocMem();
		}
		writedword(dwValue);
		return *this;
	}
	
	void writedata(BYTE * pData,DWORD dwLen)
	{
		if((current + dwLen) > (buffer + m_isize))
		{
			ReAllocMem(dwLen);
		}
		memcpy(current,pData,dwLen);		
		current += dwLen;
	}

	COutputDataStream& operator << ( WORD & wValue )
	{
		if((current + sizeof(WORD)) > (buffer + m_isize))
		{
			ReAllocMem();
		}
		writeword(wValue);
		return *this;
	}

	COutputDataStream& operator << ( BYTE & ucValue )
	{
		if((current + sizeof(BYTE)) > (buffer + m_isize))
		{
			ReAllocMem();
		}
		writebyte(ucValue);
		return *this;
	}

	COutputDataStream& operator << ( float & fValue )
	{
		if((current + sizeof(float)) > (buffer + m_isize))
		{
			ReAllocMem();
		}
		writefloat(fValue);
		return *this;
	}

	COutputDataStream& operator << ( __int64 & i64Value )
	{
		if((current + sizeof(__int64)) > (buffer + m_isize))
		{
			ReAllocMem();
		}
		writeint64(i64Value);
		return *this;
	}
#if 0
	COutputDataStream& operator << ( tstring & strValue )
	{
//		USES_CONVERSION;
		if(current)
		{
			int ilen = static_cast<int>(strValue.size());
			if((m_isize-(current - buffer)) < (ilen +1))
			{
				ReAllocMem((current - buffer));
			}
		}

		//writestring(T2CA(strValue.c_str()));
		write_wstring(strValue.c_str());
		return *this;
	}
#endif
private:
	//vector<char>	vctBuff;
	BYTE * 	pOutBuf;

};


class CNetworkByteOrder
{
public:
	static unsigned short int convert(unsigned short int iValue)
	{
		unsigned short int iData;
		((BYTE*)&iData)[0] = ((BYTE*)&iValue)[1];
		((BYTE*)&iData)[1] = ((BYTE*)&iValue)[0];
		return iData;
	}
	static int convert(int iValue)
	{
		int iData;
		((BYTE*)&iData)[0] = ((BYTE*)&iValue)[3];
		((BYTE*)&iData)[1] = ((BYTE*)&iValue)[2];
		((BYTE*)&iData)[2] = ((BYTE*)&iValue)[1];
		((BYTE*)&iData)[3] = ((BYTE*)&iValue)[0];
		return iData;
	}
	static __int64 convert(__int64 iValue)
	{
		__int64 iData;
		((BYTE*)&iData)[0] = ((BYTE*)&iValue)[7];
		((BYTE*)&iData)[1] = ((BYTE*)&iValue)[6];
		((BYTE*)&iData)[2] = ((BYTE*)&iValue)[5];
		((BYTE*)&iData)[3] = ((BYTE*)&iValue)[4];
		((BYTE*)&iData)[4] = ((BYTE*)&iValue)[3];
		((BYTE*)&iData)[5] = ((BYTE*)&iValue)[2];
		((BYTE*)&iData)[6] = ((BYTE*)&iValue)[1];
		((BYTE*)&iData)[7] = ((BYTE*)&iValue)[0];
		return iData;
	}

};

//_NAMESPACE_END

#endif // !defined(AFX_DATASTREAM_H__D90A2534_EA73_4BEA_8B7E_87E59A3D1D26__INCLUDED_)
