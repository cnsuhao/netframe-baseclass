#ifndef	__OSAL_MACROS_H__
#define	__OSAL_MACROS_H__

//Defind PPS_MOBILE when code used in mobile platform 
//                  exclude android tv
#if defined __LINUX__ && !(defined PF_IQIYI_TV)
#define PPS_MOBILE
#endif

/* 
 * define the following macros to support big-endian system 
 */
//#define WORDS_BIGENDIAN     1
//#define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
/* 
 * add macros for debug 
 */
#undef	__LINUX_DEBUG__
#ifdef	__LINUX_DEBUG__
#define	TRACE(args...)	printf(args)
#define TRACE1(args...) printf(args)
#else
//#define	TRACE(args...)
//#define TRACE1(args...)
#endif


/*
 * Define to support DBGPRINT
 */
//#define	__DEBUG_PRINTF__
#ifdef	__DEBUG_PRINTF__
#define DBGPRINT(format,args...) printf(format, ##args)
#else
#define DBGPRINT(format,args...)
#endif

#ifdef __DEBUG_PRINTF__
#define DbgPrint(cond,format,args...) do {	\
	if(cond) {	\
		printf("***[%s]@[line:%d]::",__FUCNTION__,__LINE__);\
		printf(format,##args);\
	}\
}while(0)
#else
#define DbgPrint(cond,format,args...)
#endif


/*
 * Define for PPS released android
 * NOTE: following macros may rely on it, put it at top
 */
#ifdef OS_ANDROID
//define it in 'jni/Config.mk'
//#define PPS_ANDROID_KEY
#endif


/* 
 * Define to skip verify 
 */
#if (defined OS_IOS) || (defined __PLATFORM_PC__) || (defined PPS_ANDROID_KEY) || (defined OS_MACOSX)
#ifndef __SKIP_DEVICE_VERIFY__
#define __SKIP_DEVICE_VERIFY__
#endif
#endif


/*
 * Define Url prefix 
 */
#if (defined OS_IOS) || (defined OS_ANDROID) || (defined OS_MACOSX)
 // IOS, Android
#define __URL_PPS__
#elif (defined __PLATFORM_PC__)
 // PC Linux
 #define __URL_PPS_CRYPT__
 #else
 // Common case (STB)
 #define __URL_TVOD__
 #endif
 

/*
 * Define to enable long session id
 * NOTE: server only support IOS and PPS released Android
 */
#if (defined OS_IOS) || (defined PPS_ANDROID_KEY)
#define __LONG_SESSION_ID__
#endif


/* 
 * Define to disable pthread_join function
 */
#define DISABLE_PTHREAD_JOIN 1


#ifdef DISABLE_PTHREAD_JOIN
#ifndef ENABLE_PTHREAD_DETACH
#define ENABLE_PTHREAD_DETACH
#endif
#endif


/*
 * Define for windows compatibility
 */
#ifdef _WINDOWS
#define	MAX_PATH	260
#define _MAX_PATH       MAX_PATH
#endif


/*
 * Define Http timeout value
 */
#define HTTP_TIMEOUT	            60


/*
 * Define to support NTP
 * NOTE: p2p may affected by this macro
 */
//#define __NTP__


/*
 * Define for pthread stack size setting
 */
#define	OSAL_SET_PTHREAD_ATTRIBUTE
#define	OSAL_PTHREAD_STACKSIZE	(1024*1024)	//set to 1MB temporary

//Chrome base thread create set stacksize
#define CHROME_BASE_THREAD_SET_STACK
#define CHROME_BASE_THREAD_STACKSIZE    (1024*1024)     //set to 1MB temporary

/*
 * Define Thread Sche Policy 
 */
#ifdef __PLATFORM_C2__
#define	OSAL_PTHREAD_SCHED_POLICY	SCHED_RR
#define	OSAL_PTHREAD_SCHED_PRIORITY	1
#endif


/*
 * In some mobile platforms, local store cannot been written in for normal user,so can not store any files in
 * when clinent is running 
 */
#ifdef __NO_PGF__
#ifndef NO_LOCAL_FILE
//#define NO_LOCAL_FILE //if defined NO_LOCAL_FILE ,DONOT to store any files in client's local store
#endif
#endif //__NO_PGF__


/*
 * NAND or SD is damageable block device, if they are written repeatly too much,they will be die out;
 * if Page Save File(PGF ) is stored in such devices,it needs to decrease the wirting times ASAP;
 * so that many changes for the head or index area of the *.pgf just cached in memory, if the cached 
 * time or blocks changed number is above some threshold, write the cached info to the *.pgf
 */
#ifndef PGF_IN_DAMAGEABLE_DEVICE
#ifdef NGBOX
#define PGF_IN_DAMAGEABLE_DEVICE 
#endif
#endif

#ifdef PGF_IN_DAMAGEABLE_DEVICE
#ifndef CACHED_BLOCKS_THRESHOLD
#define CACHED_BLOCKS_THRESHOLD 15 //@
#endif
#ifndef CACHED_TICKS_THRESHOLD
#define CACHED_TICKS_THRESHOLD	800000//@800s
#endif
#endif


/*
 * Define PGF name
 */
#ifdef PGF_IN_DAMAGEABLE_DEVICE
#define PGF_CACHE_NAME          "ems_flash.cache"	//@cache file stored in NAND,SD,...damageable flash devices
#else
#define PGF_CACHE_NAME          "ems.cache"
#endif


/* 
 * Define bif and index file name
 */
#define BIF_FILE_POSTFIX        ".bif"
#define FIX_DATA_POSTFIX        ".dat"

/*define extend name for Bip file*/
#define BIP_FILE_EXT   ".bip"    

/*define extend name for the mp4 header file*/
#define MP4_HEADER_FILE_EXT ".hdr"

/* 
 * Define tracker group name
 */
#define LOCAL_SERVER_CFG 	"serblf.cfg" 


/*
 * Define PGF size
 */
#ifdef PGF_IN_DAMAGEABLE_DEVICE //#if PGF_IN_DAMAGEABLE_DEVICE
  #ifndef CACHE_RESIZE
    #define	PPSVOD_FIX_STORAGE_SIZE	(900 << 20)	// 900MB
  #endif
#else	//#else PGF_IN_DAMAGEABLE_DEVICE
  #ifndef __PLATFORM_CANMORE__
  /* Define Disk Storage Size used to Cache Datastream */
    #if (defined __PLATFORM_PC__) || (defined OS_MACOSX)
      // PC
      #define	PPSVOD_FIX_STORAGE_SIZE	(1 << 30)	// 1GB
    #else
      #if (defined __PLATFORM_REALTEK__) || (defined __PLATFORM_MSTAR__)
        // Realtek, Mstar
        #define	PPSVOD_FIX_STORAGE_SIZE	(22 << 20)	// 22MB
      #else
        // Common case
        #define	PPSVOD_FIX_STORAGE_SIZE	(30 << 20)	// 30MB(default)
      #endif
    #endif //__PLATFORM_PC__
  #endif //__PLATFORM_CANMORE__
#endif //#endif PGF_IN_DAMAGEABLE_DEVICE


/* 
 * Revision info for Baseinfo and Validate during post message 
 * Revision number is vodnet.dll's revision under windows 
 */
#if defined OS_IOS && defined FOR_QIYI
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	1
  #define	VODNET_REV_3	21
  #define	VODNET_REV_4    446	
  #define	LINUX_VODRES_REV	"2.1.21.446"
#elif defined OS_IOS
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	1
  #define	VODNET_REV_3	16
  #define	VODNET_REV_4    666	
  #define	LINUX_VODRES_REV	"2.1.16.666"
#elif defined OS_MACOSX 
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	2
  #define	VODNET_REV_3	17
  #define	VODNET_REV_4	667
  #define	LINUX_VODRES_REV	"2.2.17.667"
#elif defined OS_ANDROID && defined FOR_QIYI
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	3
  #define	VODNET_REV_3	23
  #define	VODNET_REV_4    447
  #define	LINUX_VODRES_REV	"2.3.26.447"
#elif defined OS_ANDROID
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	3
  #define	VODNET_REV_3	16
  #define	VODNET_REV_4	668
  #define	LINUX_VODRES_REV	"2.3.13.668"
#elif defined _WINRT_DLL
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
  // WP8
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	4
  #define	VODNET_REV_3	14
  #define	VODNET_REV_4	669
  #define	LINUX_VODRES_REV	"2.4.13.669"
#else
  // Win8
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	5
  #define	VODNET_REV_3	15
  #define	VODNET_REV_4	670
  #define	LINUX_VODRES_REV	"2.5.15.670"
#endif
#else
 //Default Linux
  #define	VODNET_REV_1	2
  #define	VODNET_REV_2	6
  #define	VODNET_REV_3	16
  #define	VODNET_REV_4	671
  #define	LINUX_VODRES_REV	"2.6.16.671"
#endif

/*
 * Revision info for LiveNet PPS
 * Revision number is Livenet3.dll's revision under windows 
 * */
#ifdef OS_IOS
#define LIVENET_REV_1   1
#define LIVENET_REV_2   1
#define LIVENET_REV_3   11
#define LIVENET_REV_4   1260
#elif defined OS_ANDROID
#define LIVENET_REV_1   1
#define LIVENET_REV_2   2
#define LIVENET_REV_3   12
#define LIVENET_REV_4   1261
#elif defined OS_MACOSX
#define LIVENET_REV_1   1
#define LIVENET_REV_2   3
#define LIVENET_REV_3   13
#define LIVENET_REV_4   1262
#endif

/*
 * Define Client Name post to Heart Server for statics caculation.
 * */
#ifdef OS_IOS 
  #define LIVE_POST_CLIENT_NAME  "iOS_ppstream" 
#elif defined OS_ANDROID
  #define LIVE_POST_CLIENT_NAME  "Android_ppstream" 
#elif defined OS_MACOSX
  #define LIVE_POST_CLIENT_NAME  "MacOSX_ppstream" 
#else
  /*Default as Linux Client*/
  #define LIVE_POST_CLIENT_NAME  "Linux_ppstream" 
#endif


/*
 * Define this macro to enable downloading 
 */
#define LINUX_DOWNLOAD_ENABLE 1


/* 
 * macros for block manager 
 */
#ifdef	__PLATFORM_PC__
  // PC
  #define	HF_BLOCKS	4	// hookfile cache block number
  #define	BM_BLOCKS	5	// block manager cache block number
#else
  #ifdef __PLATFORM_REALTEK__
    // Realtek
    #define	HF_BLOCKS	1
    #define	BM_BLOCKS	4
  #else
    #ifndef NGBOX
      // Common case
      #define	HF_BLOCKS	2
      #define	BM_BLOCKS	4
    #else
      // NGBOX
      #define	HF_BLOCKS	4
      #define	BM_BLOCKS	5
    #endif
  #endif
#endif

#ifdef __NO_PGF__      //__NO_PGF__
#ifdef OS_ANDROID
#define MAX_HF_CACHED_BLKS 5
#else
  #if (BM_BLOCKS<3)    //hook file cached max block num,at least >=3
    #define MAX_HF_CACHED_BLKS 3
  #else
    //#define MAX_HF_CACHED_BLKS ((BM_BLOCKS)+1)    //hook file cached max block num,at least >=3, 
    #define MAX_HF_CACHED_BLKS (BM_BLOCKS)          //hook file cached max block num,at least >=3, 
  #endif
#endif  
#endif//__NO_PGF__


/* 
 * macros to control uploading. Define this macro to disable uploading to other nodes. 
 */
#if defined(__PLATFORM_CANMORE__) || defined(__PLATFORM_PC__) || defined(NGBOX) 
// Do nothing here
#elif defined OS_ANDROID
 #define	LINUX_VIP_FLAG  	
#elif defined OS_IOS 
 #define	LINUX_VIP_FLAG  	
#else
  #ifdef LINUX_DOWNLOAD_ENABLE
  // Do nothing here
  #else
  #define	LINUX_VIP_FLAG
  #endif
#endif

/* 
 * define this macro to enable tracker authentication 
 */
//#define LINUX_TRACKER_AUTH 1

/* 
 * macros for ems.conf path, library path, cache path
 */
#ifndef NO_LOCAL_FILE
  /* Keep local file */
  #if (!defined OS_ANDROID) && (!defined ANDROID_STATIC)
    #ifndef OS_IOS
      // Common case
      #define PPS_DEFAULT_ROOT_PATH "/tmp/ems/"
    #else
      // IOS
      #define PPS_DEFAULT_ROOT_PATH "/var/mobile/ems"
    #endif

    #ifndef __PLATFORM_REALTEK__
      #ifndef __PLATFORM_TELECHIPS__
        // Common case
        #define PPS_ETC_FILE_DEFAULT_PATH  "/etc/ems.conf"
      #else
        // Telechips, NGBOX
        #define PPS_ETC_FILE_DEFAULT_PATH  "/opt/setup/ems.conf"
      #endif
    #else
      // Realtek
      #define   PPS_ETC_FILE_DEFAULT_PATH  "/usr/local/etc/dvdplayer/ems.conf"
    #endif //__PLATFORM_REALTEK__
  #else
    // Android
#ifdef FOR_QIYI
    #define PPS_DEFAULT_ROOT_PATH  "/data/data/com.qiyi.video/files/QIYIVideoP2P/"
    #define PPS_DEFAULT_LIB_PATH   "/data/data/com.qiyi.video/files/QIYIVideoP2P/" 
#else
    #define PPS_DEFAULT_ROOT_PATH  "/data/data/tv.pps/ppscache/"//"/sdcard/ems/" //"/cache/ems/"
    #define PPS_DEFAULT_LIB_PATH   "/data/data/tv.pps/lib/"
#endif
  #endif // OS_ANDROID & ANDROID_STATIC
  //extern char PPS_ETC_FILE[];
#else
  #ifndef OS_IOS
    // Common case
    #define PPS_DEFAULT_LIB_PATH "/opt/PPStream/pclib"
  #else
    // IOS
    #define PPS_DEFAULT_LIB_PATH "/var/mobile/ems"
  #endif

  #ifdef __PLATFORM_PC__
    // PC
    #define PPS_DEFAULT_ROOT_PATH "/root/.ems/"
  #else
    // Common case
    #define PPS_DEFAULT_ROOT_PATH "/tmp/.ems/"
  #endif

  // Common case
  #define  PPS_ETC_FILE_DEFAULT_PATH "ems.conf"
#endif
//extern char *PPS_ROOT_PATH;


/* 
 * Define xml file down load url 
 */
#ifdef __PLATFORM_PC__
    #define DOWNLOAD_XML_URL        "tv.ppstream.com"
    #define	PPSVOD_PL_XML_WEBSITE	"tv.ppstream.com"
#else
  #ifndef MLTV_URL
    #define DOWNLOAD_XML_URL        "tv.lettv.com"
    #define PPSVOD_PL_XML_WEBSITE   "plist.lettv.com"
  #else
    //ML_TV foriegn version  
    #define DOWNLOAD_XML_URL        "mltv.lettv.com"
    #define PPSVOD_PL_XML_WEBSITE   "mltv.lettv.com"
  #endif //MLTV_URL
#endif


/*
 * For playlist downloading
 */
#define	PPSVOD_PL_CLASSES_XML	"channelforstb.php"


/*
 * Define to enable update
 */
//#define __ENABLE_UPDATE


/* 
 * Version number of current release, used for update 
 */
#undef PPS_LINUX_UPDATE_VERSION_1
#undef PPS_LINUX_UPDATE_VERSION_2
#undef PPS_LINUX_UPDATE_VERSION_3
#undef PPS_LINUX_UPDATE_VERSION_4
#define PPS_LINUX_UPDATE_VERSION_1 0
#define PPS_LINUX_UPDATE_VERSION_2 0
#define PPS_LINUX_UPDATE_VERSION_3 1
#define PPS_LINUX_UPDATE_VERSION_4 570
#define PPS_VERSION_STRING "0.1.661"


/* 
 * define update interval
 */
#define	PPSVOD_UPDATE_CLASSES_TIME	(30 * 60) // 30 minutes


/*
 * Define update config file name
 */
#define EMS_UPDATE_LOCAL_CONF   "/emsupdate.ini"


/*
 * For update verification
 */
#define EMS_VERIFIED_SHMSZ      16
#define EMS_VERIFIED_KEY        0x5678
#define EMS_VERIFIED_SHM_WORD   "vdot"


/* 
 * For pps libraries update 
 */
//#define PPSVOD_UPDATE_SITE_NAME "update.111222.cn"
#define PPSVOD_UPDATE_SITE_NAME " "
//#define PPSVOD_UPDATE_SITE_PATH "product/tvbox/changhong/update.ini"
#define PPSVOD_UPDATE_SITE_PATH " "


/*
 * Define for windows compatibility
 */
#ifdef	_DEBUG
#define	DEBUG_NEW	new
#endif


/*
 * Define for windows compatibility
 */
#ifndef _T
#define _T(x)		x
#endif


/* 
 * macros about hash, porting from wincrypt.h 
 */
// Algorithm classes
#define	ALG_CLASS_HASH	(4 << 13)
// Algorithm classes
#define	ALG_CLASS_ANY	(0)
// Algorithm types
#define	ALG_TYPE_ANY	(0)
// Hash sub ids
#define	ALG_SID_MD5	3
#define	ALG_SID_SHA	4
#define	ALG_SID_SHA1	4
// algorithm identifier definitions
#define	CALG_MD5	(ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD5)
#define	CALG_SHA1	(ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA1)


/* 
 * For window compatibility
 * The following macros are moved from stdafx.h 
 */
// 不使用udp的接收缓冲队列
 #define  UDP_NOTUSE_SENDLIST
// // 不使用udp的发送缓存队列
 #define  UDP_NOTUSE_RECVLIST
// #define SHOW_MOREPPS_DEBUGINFO
#define	CALG_MD5_UseCHash

//In FilePeerSession.cpp to select Peer
//It takes lots of CPU, so disable it temply.
//#define  PEER_SELECT

//In VodUploadControl.cpp to control Vod Upload
//It takes lots of CPU, so disable it temply. 
//#define USE_VOD_UPLOAD

#endif
