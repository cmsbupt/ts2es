#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG
	//#ifdef LOG
	////#define LOG_NAC //非算术编码的LOG
	////#define LOG_AC //算术编码的LOG
	//#define LOG_DWT_COEFF //打印码流中提取的DWT系数的LOG
	//#define LOG_DWT_DequantCOEFF //打印码流中提取的DWT系数，经过反量化后的LOG
	//#endif

	//#ifdef LOG
		extern FILE * pfLog;
		extern char *cLogFileName;
	//	#ifdef LOG_DWT_COEFF //打印码流中提取的DWT系数的LOG
	//		extern FILE * pfDWT_COEFF_Y_Log;
	//		extern FILE * pfDWT_COEFF_C1_Log;
	//		extern FILE * pfDWT_COEFF_C2_Log;
	//	#endif
	//	#ifdef LOG_DWT_DequantCOEFF //打印码流中提取的DWT系数，经过反量化后的LOG
	//		extern FILE * pfDWT_DequantCOEFF_Log;
	//	#endif
	//	#endif
#endif

//初始化			
void init_log(int argc,char* argv[]);