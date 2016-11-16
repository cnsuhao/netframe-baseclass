#include "stdafx.h"
#include <nb30.h>
#include "MacAddress.h"

class CAutoLoadDll
{
public:
	CAutoLoadDll(const char * szDllname)
	{	
		hinstLib = LoadLibrary((LPCSTR)szDllname);
	}
	~CAutoLoadDll()
	{
		if(hinstLib)
			FreeLibrary(hinstLib);
	}
	FARPROC GetProcAddress(const char * szProcname)
	{
		if(hinstLib)
			return ::GetProcAddress(hinstLib,szProcname); 
		else
			return NULL;
	}
public:
	HINSTANCE hinstLib;     
};

CMacAddress CMacAddress::GetSelfMacAddress()
{
	typedef struct _ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff [30];
	}ASTAT, * PASTAT;
	ASTAT Adapter; 
	int nStep = 0;
	CMacAddress selfMacAddr;
	try
	{		
		memset(selfMacAddr.btMac,0,sizeof(CMacAddress));
		nStep = 1;
		NCB ncb;
		UCHAR uRetCode;
		LANA_ENUM lana_enum;
		memset(&ncb,0,sizeof(ncb));
		nStep = 2;
		ncb.ncb_command = NCBENUM;	
		ncb.ncb_buffer = (unsigned char *) &lana_enum;
		ncb.ncb_length = sizeof(lana_enum);	
		//定义Netbios函数类型
		typedef  UCHAR  (WINAPI * NETBIOS)(PNCB pncb);
		//加载NetApi32.dll
		CAutoLoadDll netapidll("NetApi32.dll");	
		//获取Netbios函数地址
		NETBIOS fnNetbios = (NETBIOS)netapidll.GetProcAddress("Netbios");
		nStep = 3;
		//如果获取失败就直接标识调用失败
		if(fnNetbios)
		{
			nStep = 4;
			uRetCode = fnNetbios( &ncb );
		}
		else
		{
			nStep = 20;
			uRetCode = 1;
		}
		if(uRetCode==0 && lana_enum.length>0)
		{
			nStep = 5;
			UCHAR cNum = lana_enum.lana[0];
			ncb.ncb_command = NCBRESET;
			ncb.ncb_lana_num = cNum;
			uRetCode = fnNetbios( &ncb );
			nStep = 6;
			memset( &ncb, 0, sizeof(ncb) );
			nStep = 7;
			ncb.ncb_command = NCBASTAT;
			lstrcpy( (char *)ncb.ncb_callname,"* ");
			nStep = 8;
			ncb.ncb_buffer = (unsigned char *) &Adapter;
			ncb.ncb_length = sizeof(Adapter);
			ncb.ncb_lana_num = cNum;
			uRetCode = fnNetbios( &ncb );
			nStep = 9;
			if(uRetCode==0)
			{
				nStep = 10;
				memcpy(&selfMacAddr.btMac,Adapter.adapt.adapter_address,6);
				nStep = 11;
			}
		}
	}
	catch(...)
	{
		selfMacAddr.Clear();
	//	CThpMiscHelper::Log2File("c:\\pps_debug","%s",nStep);
	}
	return selfMacAddr;
}
bool CMacAddress::operator == (const CMacAddress & first) const
{
	return memcmp(this,&first,sizeof(CMacAddress))==0?true:false;	
}
bool CMacAddress::operator >= (const CMacAddress & first) const
{
	return memcmp(this,&first,sizeof(CMacAddress))>=0?true:false;	
}
bool CMacAddress::operator <= (const CMacAddress & first) const
{
	return memcmp(this,&first,sizeof(CMacAddress))<=0?true:false;
}
bool CMacAddress::operator < (const CMacAddress & first) const
{
	return memcmp(this,&first,sizeof(CMacAddress))<0?true:false;	
}
bool CMacAddress::operator > (const CMacAddress & first) const
{
	return memcmp(this,&first,sizeof(CMacAddress))>0?true:false;	
}