#ifndef _DEBUGPRO_H
#define _DEBUGPRO_H

#ifdef QCOM_4004B

#else
#include <hsf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#endif

typedef enum {		// 兼容Android
	DL_UNKNOWN = 0,
	DL_DEFAULT,
	DL_VERBOSE,
	DL_DEBUG,
	DL_INFO,
	DL_WARN,
	DL_ERROR,
	DL_FATAL,
	DL_SILENT
}LogPriority;

#define DM_JNI		"JNISMART_JNI"
#define DM_JSON		"JNISMART_JSON"
#define DM_SCRIPT	"JNISMART_SCRIPT"
#define DM_PACKET	"JNISMART_PACKET"
#define DM_SOCKET	"JNISMART_SOCKET"

//#define DEBUG_LEVEL		0	// 模块的日志级别比该值小则不输出,为0则输出所有日志

#define uint8 unsigned char
#define uint16 short

#define JD_DEBUG_LEV    10


#if defined(__ANDROID__)
#include<android/log.h>


#define dbg( LEVEL, MODULE, ... ) \
	((MODULE == 0) || (LEVEL < DEBUG_LEVEL)) ? (void)0 : __android_log_print(LEVEL, MODULE, __VA_ARGS__)

#elif defined(_WIN32)


#define _DebugPro_Out(tag, fmt, ...) \
	u_printf("%s-(%s.%d):"fmt"\r\n", tag, __FILE__ + 33, __LINE__, __VA_ARGS__)

#define dbg( LEVEL, MODULE, fmt, ... ) \
	((MODULE == 0) || (LEVEL < DEBUG_LEVEL)) ? (void)0 : _DebugPro_Out(MODULE, fmt, __VA_ARGS__)


#elif defined(__APPLE__)
#define dbg( LEVEL, MODULE, fmt, ... )

#elif defined(QCOM_4004B)
#define _DebugPro_Out(tag, fmt, ...) \
	A_PRINTF("%s-(%s.%d):"fmt"\r\n", tag, __FILE__ + 33, __LINE__, __VA_ARGS__)

#define dbg( LEVEL, MODULE, fmt, ... ) \
	((MODULE == 0) || (LEVEL < DEBUG_LEVEL)) ? (void)0 : _DebugPro_Out(MODULE, fmt, __VA_ARGS__)

#elif (defined(__LPT230__)||defined(__LPT130__)||defined(__LPB130__)||defined(__LPT330__)||defined(__LPB135__))

//#define SHORT_FILE strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__
#define SHORT_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define custom_log(M, ...) do {\
 						   		HF_Debug(JD_DEBUG_LEV,"[%s:%4d] " M "\n", SHORT_FILE, __LINE__, ##__VA_ARGS__);\
                           }while(0==1)

/*#define custom_log(M, ...) do {\
 		u_printf("[%5d|%5d][%-9.9s:%4d] " M "\n", hfsys_get_time(), hfsys_get_memory(), SHORT_FILE, __LINE__, ##__VA_ARGS__);\
 }while(0==1)*/
#else
//#error("Unkown Platform!");
#endif



#endif //_DEBUGPRO_H
