#include "log.h"

#ifdef LOG
	FILE * pfLog;
	char *cLogFileName;

	//#ifdef LOG_DWT_COEFF //打印码流中提取的DWT系数的LOG
	//	FILE * pfDWT_COEFF_Y_Log;
	//	FILE * pfDWT_COEFF_C1_Log;
	//	FILE * pfDWT_COEFF_C2_Log;
	//#endif
	//#ifdef LOG_DWT_DequantCOEFF //打印码流中提取的DWT系数，经过反量化后的LOG
	//	FILE * pfDWT_DequantCOEFF_Log;
	//#endif
#endif

void init_log(int argc,char* argv[])//argv[]是argc个参数，其中第0个参数是程序的全名， 以后的参数命令行后面跟的是用户输入的参数
{
#ifdef LOG 
		cLogFileName = (char *)malloc(128);
		cLogFileName=strcpy(cLogFileName,argv[1]);
		cLogFileName=strcat(cLogFileName,"_log.txt");
		pfLog=fopen(cLogFileName,"wb");
		if (pfLog == NULL)
		{
			printf("cannot open file\:%s\n",cLogFileName);
			exit(0);
		}
		fprintf(pfLog,"%s\n",cLogFileName);
		free(cLogFileName);

	//#ifdef LOG_DWT_COEFF 
	//	cLogFileName = (char *)malloc(128);
	//	cLogFileName=strcpy(cLogFileName,argv[1]);
	//	cLogFileName=strcat(cLogFileName,"_dwt_coeff_Y_log.txt");
	//	pfDWT_COEFF_Y_Log=fopen(cLogFileName,"wb");
	//	if (!pfDWT_COEFF_Y_Log)
	//	{
	//		printf("cannot open file\:%s\n",cLogFileName);
	//		exit(0);
	//	}
	//	else
	//	{
	//		printf("open file\:%s\n",cLogFileName);
	//	}
	//	cLogFileName=strcpy(cLogFileName,argv[1]);
	//	cLogFileName=strcat(cLogFileName,"_dwt_coeff_C1_log.txt");
	//	pfDWT_COEFF_C1_Log=fopen(cLogFileName,"wb");
	//	if (!pfDWT_COEFF_C1_Log)
	//	{
	//		printf("cannot open file\:%s\n",cLogFileName);
	//		exit(0);
	//	}
	//	else
	//	{
	//		printf("open file\:%s\n",cLogFileName);
	//	}
	//	cLogFileName=strcpy(cLogFileName,argv[1]);
	//	cLogFileName=strcat(cLogFileName,"_dwt_coeff_C2_log.txt");
	//	pfDWT_COEFF_C2_Log=fopen(cLogFileName,"wb");
	//	if (!pfDWT_COEFF_C2_Log)
	//	{
	//		printf("cannot open file\:%s\n",cLogFileName);
	//		exit(0);
	//	}
	//	else
	//	{
	//		printf("open file\:%s\n",cLogFileName);
	//	}
	//	free(cLogFileName);
	//#endif

	//#ifdef LOG_DWT_DequantCOEFF 
	//	cLogFileName = (char *)malloc(128);
	//	cLogFileName=strcpy(cLogFileName,argv[1]);
	//	cLogFileName=strcat(cLogFileName,"_dwt_dequant_coeff_log.txt");
	//	pfDWT_DequantCOEFF_Log=fopen(cLogFileName,"wb");
	//	if (!pfDWT_DequantCOEFF_Log)
	//	{
	//		printf("cannot open file\:%s\n",cLogFileName);
	//		exit(0);
	//	}
	//	free(cLogFileName);
	//#endif
#endif
}