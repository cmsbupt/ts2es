#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG
	//#ifdef LOG
	////#define LOG_NAC //�����������LOG
	////#define LOG_AC //���������LOG
	//#define LOG_DWT_COEFF //��ӡ��������ȡ��DWTϵ����LOG
	//#define LOG_DWT_DequantCOEFF //��ӡ��������ȡ��DWTϵ�����������������LOG
	//#endif

	//#ifdef LOG
		extern FILE * pfLog;
		extern char *cLogFileName;
	//	#ifdef LOG_DWT_COEFF //��ӡ��������ȡ��DWTϵ����LOG
	//		extern FILE * pfDWT_COEFF_Y_Log;
	//		extern FILE * pfDWT_COEFF_C1_Log;
	//		extern FILE * pfDWT_COEFF_C2_Log;
	//	#endif
	//	#ifdef LOG_DWT_DequantCOEFF //��ӡ��������ȡ��DWTϵ�����������������LOG
	//		extern FILE * pfDWT_DequantCOEFF_Log;
	//	#endif
	//	#endif
#endif

//��ʼ��			
void init_log(int argc,char* argv[]);