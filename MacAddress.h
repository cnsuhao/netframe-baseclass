#ifndef __MACADDRESS_H__
#define __MACADDRESS_H__

#pragma once

//网卡地址
class CMacAddress
{
public:
	static CMacAddress GetSelfMacAddress();
	void Clear(){memset(this,0,sizeof(CMacAddress));}
	bool IsEmpty(){CMacAddress address;address.Clear();return address == * this;}
	bool operator == (const CMacAddress & first) const;	
	bool operator >= (const CMacAddress & first) const;
	bool operator <= (const CMacAddress & first) const;
	bool operator < (const CMacAddress & first) const;
	bool operator > (const CMacAddress & first) const;
private:
	BYTE btMac[6];
};
#endif
