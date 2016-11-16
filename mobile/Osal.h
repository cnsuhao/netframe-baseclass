#ifndef __OSAL_H__
#define __OSAL_H__
#include <ctype.h>
#include "OsalMacros.h"
#include "OsalTypedef.h"
#include "baseclass/GetTickCount.h"
#include "OsalApi.h"
//#include "baseclass/stdstring.h"
//#include "OsalEvent.h"
#include "OsalMsgQueue.h"
//#include "OsalHttp.h"
//#include "OsalLog.h"

//#include "public_vars.h"

//for
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct _ems_statistical_data{
		DWORD/*time_t*/  tStartTaskBeginTime;		//Begintime of the start task
		DWORD/*time_t*/  tStartTaskEndTime;		//End time of the start task end
		DWORD/*time_t*/  tBaseInfoDnBeginTime;	//Begin time of the baseinfo download
		DWORD/*time_t*/  tBaseInfoDnEndTime;		//End time of the baseinfo download
		DWORD dwBifSize;						//Bif size in  Bytes
		DWORD/*time_t*/  tBifDnBeginTime;		//Begin time of the Bif download
		DWORD/*time_t*/  tBifDnEndTime;		//End time of the Bif download
		DWORD dwBipSize;						//Bip size in Bytes
		DWORD/*time_t*/  tBipDnBeginTime;			//Begin time of the bip download
		DWORD/*time_t*/  tBipDnEndTime;			//End time of the bip download
		DWORD dwMp4HeaderSize;					//MP4 Header size in Bytes
		DWORD/*time_t*/  tMp4HdDnBeginTime;		//begin time of the Mp4 header download
		DWORD/*time_t*/  tMp4HdDnEndTime;		//end time of the Mp4 header download
		DWORD dw1stCacheSize;					//1st data cache size in Bytes
		DWORD/*time_t*/  tFistCacheDataDnBeginTime; //begin time of the 1st cached data download
		DWORD/*time_t*/  tFistCacheDataDnEndTime;   //end time of the 1st cached data download
	}ems_statistical_data;
	
	typedef struct _ems_statistics_info{
		int bBifCached;					  //@1,bif cached;0,not cached
		unsigned int dwBifSize;          //@bif file size,in Bytes
		unsigned int tDnBifTime;         //@time of the bif download
		
		int bBipCached;					 //@1,bip filed cached;0,not cached
		unsigned int dwBipSize;          //@bip size ,in bytes
		unsigned int tDnBipTime;        //@time of the bip download
		
		int bMp4ZipFileCached;          //@1,cached;0,not cached
		unsigned int dwMp4ZipFile;      //@mp4 zip file size ,in bytes
		unsigned int tDnMp4Header;      //@time of the mp4 header
		
		unsigned int dwMp4HeaderSize;   //@the decompressed mp4 header size, in bytes
		unsigned int tPfv2Mp4Header;    //@time of the pfv file to mp4 header
		
		unsigned int dw1stCacheDataSize;//the 1st cache data download
		unsigned int tDn1stCacheData;   //time of the 1st cache data download
	}ems_statistics_info;
	
	
	extern ems_statistical_data g_emsStatistics;
	extern ems_statistics_info g_emsStatisticsInfo;
	
#ifdef __cplusplus
}
#endif

#endif //__OSAl_H__
