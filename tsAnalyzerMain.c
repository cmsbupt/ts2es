#include <stdio.h>
/*
本程序是为了学习mpeg2 TS封装
目前最新版的TS封装是2015年发布，支持H.265、H.264
*/
/*
http://blog.sina.com.cn/s/blog_6ca8ed170100qjvd.html
ES流(Elementary Stream，基本流)：数字电视各组成部分编码后所形成的直接表示基本元素内容的流，包含视频、音频或数据的连续码流。
PES流（Paketized Elementary Stream，打包基本码流):是将基本的码流ES流根据需要分成长度不等的数据包，并加上包头就形成了打包的基本码流PES流。
PS (Program Stream，节目流):将一个节目的多个组成部分按照它们之间的互相关系进行组织并加入各组成部分关系描述后的码流。PS流是一种多路复用数字音频、视频等的封装容器，它一个或多个具有共同的时间基准的PES流合并成一个整体流，主要用于节目存储。其包长不固定，且较长，一旦失去同步信息，接收机无法确定下一包的同步位置，会造成失步，导致严重的信息丢失。PS流适用于误码小、信道较好的环境，如演播室、家庭环境和存储介质中。
TS流（Transport Stream，传输流）：是将一个节目的多个组成部分按照它们之间的互相关系进行组织并加入各组成部分关系描述和节目组成信息,并进一步封装成传输包后的码流。TS流是将视频、音频、PSI等数据打包成传输包进行传送。主要用于节目传输。TS的传输包长度固定，一般为188字节。
TS流和PS流是MPEG-2标准中规定的两种输出码流。TS格式中，从视频流的任意一片断开始都可以独立解码，而PS格式不可以。由于TS流具备较强的抵抗传输误码的能力，因此目前在传输媒体中进行传输的MPEG-2码流基本上都采用了TS流的包格式。
TS流的播放：播放前将TS流文件的后缀名该为.mpg或者.mpeg，用可以直接播放MPEG-TS流的播放器（一般的播放器都可以）打开播放即可。
*/
#define LOG
#define TS_WriteToFile 0
#define PCR_TIME_BASE 27000000
#define TS_START_CODE 0x47
#include "types.h"
#include "TS.h"
#include "buffer.h"
#include "log.h"
#define fPATTableFileName "PATTableFile.txt"
#define fPMTTableFileName "PMTTableFile.txt"
#define fPCRTableFileName "PCRTableFile.txt"
#define fProgramFile0Name "program0.txt"
FILE * fPMTTableFile=NULL;
FILE * fPATTableFile=NULL;
FILE * fPCRTableFile=NULL;
FILE * fProgramFile0=NULL;//第一个节目的信息
FILE * pfProgramFileTable[256];;//节目的码流文件 指针数组
FILE * pfProgramFileNameTable[256];;//节目的码流文件名称 指针数组

int main(int argc,char * argv[])
{
	TS_packet_header tmpTSHeader;
	TS_adaptation_field tmpTSAdapt;
	PAT tmpPAT;
	PMT tmpPMT;
	char *filename = argv[1];
	FILE * fInTsFile = fopen(filename,"rb");
	ui8 tmpByte;
	int i,j;
	ui8 buf[188];
	char sTmpFilename[256];
	char sTmpBuf[256];
	int	 fileNum=0;
	int	 fileLen=0;
	int	 packetIdx=0;
	int	 tmpReadByte=0;
	int	 tmpAdajustByte=0;
	FILE * fTmpPacketFile=NULL;
	stBufBitGet * bBuf;
	uint16 PATFind=0;//0表示还没有找到PAT表，不进行具体TS包解析
	uint16 isPESPacket=0;//0表示当前TS包是会话信息包，1表示当前包是PES数据包
						//这个参数会配合TS包头中的句法 payload_unit_start_indicator
						//来决定如何进行后续的解析
	uint16 programTotalNum=0;//节目序号
	uint16 programNumber=0;//节目序号
	uint16 programPID=0;	//节目对应的PMT表的PID
	uint16 programPIDTable[256];	//节目对应的PMT表的PID 列表
	uint16 programNumberTable[256];
	uint16 programPESPIDTable[256][16];	//节目对应的ES流的PID 列表
	FILE * programPESPIDFileTable[256][16];	//节目对应的ES流的PID 对应的文件指针列表
	
	uint16 programVideoGetSPSTable[256][16];	//节目对应的ES流的PID 列表
	uint16 programPIDIdx = 0;//记录当前TS包所属的节目ID序号
	uint16 programPESIdx = 0;//记录当前TS包所属的节目的PES ID序号

	uint16 programPIDDebug = 1012;//记录当前节目需要调试的PID
	if(argc<2){
		printf("usage: tsAnalyzer.exe in.ts\n");
		return -1;
	}
	
	if( fInTsFile == NULL){
		printf("Error: cannot open file %s\n",filename);
	}else
		printf("open file %s\n",filename);
	
	fseek(fInTsFile,0,SEEK_END);
	fileLen =ftell(fInTsFile);
	printf("fileLen %d   packet %d %d \n",fileLen,fileLen/188,fileLen%188);
	fseek(fInTsFile,0,SEEK_SET);

	printf("fInTsFile %X\n",fInTsFile);

	//打开记录 PMT TS包的文件
	fPMTTableFile = fopen(fPMTTableFileName,"wb");
	if( fPMTTableFile == NULL){
		printf("Error: cannot creat file %s\n",fPMTTableFileName);
	}else
		printf("creat file %s\n",fPMTTableFileName);

	//打开记录 PAT TS包的文件
	fPATTableFile = fopen(fPATTableFileName,"wb");
	if( fPATTableFile == NULL){
		printf("Error: cannot creat file %s\n",fPATTableFileName);
	}else
		printf("creat file %s\n",fPATTableFileName);

	//打开记录 PCR TS包的文件
	fPCRTableFile = fopen(fPCRTableFileName,"wb");
	if( fPCRTableFile == NULL){
		printf("Error: cannot creat file %s\n",fPCRTableFileName);
	}else{
		printf("creat file %s\n",fPCRTableFileName);
  }

	//打开记录 第一个节目 TS包的文件
	fProgramFile0 = fopen(fProgramFile0Name,"wb");
	if( fProgramFile0 == NULL){
		printf("Error: cannot creat file %s\n",fProgramFile0Name);
	}else{
		printf("creat file %s\n",fProgramFile0Name);
  }

	init_log(argc,argv);
  //PES 流的文件指针初始化为空
	for(i=0; i< 256; i++ )
	{
		programPESPIDTable[i][15] = 0;
		for(j=0; j< 16; j++ )
		{
			programPESPIDFileTable[i][j]=NULL;
		}
	}
  
  //循环遍历所有TS 包
	//fileLen/188 packetIdx< 222
	for(packetIdx=0; !feof(fInTsFile); packetIdx++)//while( !feof(fInTsFile))//不是文件结束
	{
		isPESPacket = 0;
		fflush(pfLog);
		//printf("packet: %d\n",fileNum);
		sprintf(sTmpFilename,"%s",".\\packet\\");
		//printf("sTmpFilename: %s \n",sTmpFilename);

		sprintf(sTmpBuf,"%d",fileNum++);
		//printf("sTmpBuf: %s \n",sTmpBuf);

		strcat(sTmpFilename,sTmpBuf);
		strcat(sTmpFilename,".log");

		printf("sTmpFilename: %s \t",sTmpFilename); 

		memset(buf,0,188);
		tmpReadByte= fread(buf,1,188,fInTsFile);
		if( tmpReadByte != 188 ){
			printf("fread(buf,1,188,fInTsFile)  read error! return %d bytes. file position %d \n",tmpReadByte,ftell(fInTsFile));
		}else
			printf("fread(buf,1,188,fInTsFile)  read correct! return %d bytes. file position %d \n",tmpReadByte,ftell(fInTsFile));

#if 0
		for(i=0;i<188;i++){
			if(i%16 == 0)
				printf("\n");
			printf("%02X \t ",buf[i]);
		}
		printf("\n");
#endif	

#if TS_WriteToFile
		fTmpPacketFile = fopen(sTmpFilename,"wb");
		if( fTmpPacketFile == NULL){
			printf("Error: cannot open file %s\n",sTmpFilename);
    }
    else{
      printf("open file %s\n", sTmpFilename);
    }
		fwrite(buf,1,188,fTmpPacketFile);
		fflush(fTmpPacketFile);
		fclose(fTmpPacketFile);
#endif		

		bBuf= BufferInit(buf,buf+188,188 );

		//int index=0;
		//while( NextBits(bBuf,8) != TS_START_CODE)
			//ReadBits(bBuf,8);
		//fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
		
		ReadBits(bBuf,8);
		tmpTSHeader.transport_error_indicator		  = ReadBits(bBuf,  1);
		tmpTSHeader.payload_unit_start_indicator	= ReadBits(bBuf,  1);
		tmpTSHeader.transport_priority				    = ReadBits(bBuf,  1);
		tmpTSHeader.PID								            = ReadBits(bBuf, 13);

		//fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
		
		tmpTSHeader.transport_scrambling_control	= ReadBits(bBuf,2);// 等于0时表示没有加密
		tmpTSHeader.adaptation_field_control		  = ReadBits(bBuf,2);
		tmpTSHeader.continuity_counter				    = ReadBits(bBuf,4);

		//fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
		
		//fprintf(pfLog,"TS Packet start at byte: %d\n",ByteStart);
		fprintf(pfLog,"\nTS Idx:\t %d\n",packetIdx);
		fprintf(pfLog,"PID:\t %d\n",tmpTSHeader.PID);
#if 1
		fprintf(pfLog,"transport_error_indicator:\t %d\n",tmpTSHeader.transport_error_indicator);
		fprintf(pfLog,"payload_unit_start_indicator:\t %d\n",tmpTSHeader.payload_unit_start_indicator);
		fprintf(pfLog,"transport_priority:\t %d\n",tmpTSHeader.payload_unit_start_indicator);
#endif		
		
#if 1
		fprintf(pfLog,"transport_scrambling_control:\t %d\n",tmpTSHeader.transport_scrambling_control);
		fprintf(pfLog,"adaptation_field_control:\t %d\n",tmpTSHeader.adaptation_field_control);
		fprintf(pfLog,"continuity_counter:\t %d\n",tmpTSHeader.continuity_counter);
#endif
		fflush(pfLog);
		if( tmpTSHeader.PID == 0)//PAT 
			PATFind=1;
		//如果还没有找到PAT表，则不必继续后面的解析
		if( PATFind == 0)
			continue;

		//如果已经找到PAT表，则查询现有PMT表中节目的PES流的PID，判断当前TS包是
		//属于节目的PES包，还是信息包
		for(i=0; i< programTotalNum; i++ )
		{
			for(j=0; j< programPESPIDTable[i][15]; j++ )
			{
				//查询PMT表
				if( tmpTSHeader.PID == programPESPIDTable[i][j])//节目对应的PES流的PID
				{
					isPESPacket = 1;
					programPIDIdx = i;//记录当前TS包所属的节目ID序号
					programPESIdx = j;//记录当前TS包所属的节目的PES ID序号
					fprintf(pfLog,"program %d: %d PES {",programNumberTable[i],programPESPIDTable[i][15]);
					for(j=0; j< programPESPIDTable[i][15]; j++ )
					{
						fprintf(pfLog,"\t%d",programPESPIDTable[i][j]);
					}
					fprintf(pfLog," }\n");
					break;
				}
			} 
			if( isPESPacket )
					break;			
		}
		fflush(pfLog);
		//在这里有个地方需要注意一下，首先要判断当前TS包是PES包、还是网络信息包（传输PAT、PMT、NIT等信息的TS包）
		//对于PES包，payload_unit_start_indicator为1时，表示当前TS包是一个PES包的开始，PES包通常大于188字节
		//对于网络信息包
		//payload_unit_start_indicator为1时，在前4个字节之后会有一个调整字节，它的数值决定了负载内容的具体开始位置。
		//现在看例子中的数据47 40 00 17 00第五个字节是00，说明紧跟着00之后就是具体的负载内容。
    //如果第5个字节数值为L，则需求读取后续的L个字节
		if( isPESPacket == 0 && tmpTSHeader.payload_unit_start_indicator == 1 ){
			tmpAdajustByte= ReadBits(bBuf,8);//等于1 表明载荷的第一个字节
			fprintf(pfLog,"tmpAdajustByte:\t %d\n",tmpAdajustByte);
			for(i=0; i< tmpAdajustByte; i++ )
			{
				ReadBits(bBuf,8);//
			}
		}
		fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
		
		fflush(pfLog);
		if( tmpTSHeader.adaptation_field_control == 2 ||
			  tmpTSHeader.adaptation_field_control == 3 )
		{ //Table 2-6 – Transport stream adaptation field
			tmpTSAdapt.adaptation_field_length = ReadBits(bBuf,8);
			fprintf(pfLog,"adaptation_field_length: %d\n",tmpTSAdapt.adaptation_field_length);
			if( tmpTSAdapt.adaptation_field_length > 0)
			{
				/*
					discontinuity_indicator 1 bslbf
					random_access_indicator 1 bslbf
					elementary_stream_priority_indicator 1 bslbf
					PCR_flag 1 bslbf
					OPCR_flag 1 bslbf
					splicing_point_flag 1 bslbf
					transport_private_data_flag 1 bslbf
					adaptation_field_extension_flag 1 bslbf
				*/
				tmpTSAdapt.discontinuity_indicator = ReadBits(bBuf,1);
				tmpTSAdapt.random_access_indicator = ReadBits(bBuf,1);
				tmpTSAdapt.elementary_stream_priority_indicator = ReadBits(bBuf,1);
				tmpTSAdapt.PCR_flag = ReadBits(bBuf,1);
				tmpTSAdapt.OPCR_flag = ReadBits(bBuf,1);
				tmpTSAdapt.splicing_point_flag = ReadBits(bBuf,1);
				tmpTSAdapt.transport_private_data_flag = ReadBits(bBuf,1);
				tmpTSAdapt.adaptation_field_extension_flag = ReadBits(bBuf,1);

				fprintf(pfLog,"PCR_flag: %d\n",tmpTSAdapt.PCR_flag);

				if (tmpTSAdapt.PCR_flag == 1) {
					tmpTSAdapt.program_clock_reference_base = ReadBits(bBuf,32);
					tmpTSAdapt.program_clock_reference_base = tmpTSAdapt.program_clock_reference_base<<32;
					tmpTSAdapt.program_clock_reference_base = tmpTSAdapt.program_clock_reference_base + ReadBits(bBuf,1);
					
					fprintf(pfLog,"program_clock_reference_base: %64U\n",tmpTSAdapt.program_clock_reference_base);
					ReadBits(bBuf,6);//保留
					tmpTSAdapt.program_clock_reference_extension = ReadBits(bBuf,9);				
				}
				
				if (tmpTSAdapt.OPCR_flag == 1) {
					tmpTSAdapt.original_program_clock_reference_base  = ReadBits(bBuf,32);
					tmpTSAdapt.original_program_clock_reference_base = tmpTSAdapt.original_program_clock_reference_base<<32;
					tmpTSAdapt.original_program_clock_reference_base = tmpTSAdapt.original_program_clock_reference_base + ReadBits(bBuf,1);					
				}

			}
		}
		else
			tmpTSAdapt.adaptation_field_length =0;
			
		/**/
		fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
		if( tmpTSHeader.PID == 0)//PAT 
		{	
			if( tmpTSAdapt.adaptation_field_length == 0)
			{
				tmpPAT.table_id = ReadBits(bBuf,8);// 8 uimsbf
				fprintf(pfLog,"tmpPAT.table_id: %d\n",tmpPAT.table_id);
				tmpPAT.section_syntax_indicator = ReadBits(bBuf,1);// 1 bslbf
				ReadBits(bBuf,1);//'0' 1 bslbf
				ReadBits(bBuf,2);//reserved 2 bslbf
				tmpPAT.section_length = ReadBits(bBuf,12);// 12 uimsbf
				tmpPAT.transport_stream_id = ReadBits(bBuf,16);// 16 uimsbf
				ReadBits(bBuf,2);// 2 bslbf
				tmpPAT.version_number = ReadBits(bBuf,5);// 5 uimsbf
				tmpPAT.current_next_indicator = ReadBits(bBuf,1);// 1 bslbf
				tmpPAT.section_number = ReadBits(bBuf,8);// 8 uimsbf
				tmpPAT.last_section_number = ReadBits(bBuf,8);// 8 uimsbf
#if 1
				fprintf(pfLog,"section_length:\t %d\n",tmpPAT.section_length);
				fprintf(pfLog,"transport_stream_id:\t %d\n",tmpPAT.transport_stream_id);
				fprintf(pfLog,"version_number:\t %d\n",tmpPAT.version_number);
				fprintf(pfLog,"current_next_indicator:\t %d\n",tmpPAT.current_next_indicator);
				fprintf(pfLog,"section_number:\t %d\n",tmpPAT.section_number);
				fprintf(pfLog,"last_section_number:\t %d\n",tmpPAT.last_section_number);
#endif
				{
					int sectionNum = (tmpPAT.section_length - 8)/4;
					int i;
#if 1
					//fprintf(pfLog,"sectionNum= %d bBuf->TotalBitsLeft:%d  bBuf->TotalBitsLeft/8:%d\n",sectionNum,bBuf->TotalBitsLeft,(bBuf->TotalBitsLeft)/8);
					fprintf(pfLog,"sectionNum= %d TotalBitsLeft:%d\n",sectionNum,bBuf->TotalBitsLeft);
#endif
					programTotalNum = sectionNum;
					for(i=0; i< sectionNum; i++ )
					{
						tmpPAT.program_number =  ReadBits(bBuf,16);
						programNumberTable[i] = tmpPAT.program_number;
#if 1
						fprintf(pfLog,"program_number:%d\n",tmpPAT.program_number);	//节目对应的TS包的PID
						fprintf(fPATTableFile,"program_number:%d\t",tmpPAT.program_number);	//节目对应的TS包的PID
#endif
						ReadBits(bBuf,3);
						if( tmpPAT.program_number == 0){
							tmpPAT.network_PID =  ReadBits(bBuf,13);
#if 1
							fprintf(pfLog,"network_PID:%d\n",tmpPAT.network_PID);	
#endif
						}
						else{
							tmpPAT.program_map_PID =  ReadBits(bBuf,13);
							programPIDTable[i] = tmpPAT.program_map_PID;
#if 1
							fprintf(pfLog,"program_map_PID:%d\n",tmpPAT.program_map_PID);	
							fprintf(fPATTableFile,"program_map_PID:%d\n",tmpPAT.program_map_PID);	
							fflush(fPATTableFile);
#endif
						}

					}
					tmpPAT.CRC_32 =  ReadBits(bBuf,32);
					fprintf(pfLog,"CRC_32: 0x%08X\n",tmpPAT.CRC_32);

					programNumber=tmpPAT.program_number;//节目序号
					programPID=tmpPAT.program_map_PID;	//节目对应的PMT表的PID
				}
			}
			else
				fprintf(pfLog,"adaptation_field_length: %d\n",tmpTSAdapt.adaptation_field_length);
		}
		
		//fprintf(pfLog,"\n  programPID: %d\n",programPID);
		for(i=0; i< programTotalNum; i++ )
		{
			//处理PMT表
			if( tmpTSHeader.PID == programPIDTable[i])//节目对应的PMT表的PID
			{
				int byteLeft=0;
				int programIdx = i; //当前节目的索引序号
				int programESIdx = 0; //当前节目的ES索引序号
				fprintf(pfLog,"\nPMT表 programPID: %d\n",programPIDTable[i]);
				fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
				tmpPMT.table_id  =  ReadBits(bBuf,8);                       //8; //固定为0x02, 表示PMT表  
				tmpPMT.section_syntax_indicator =  ReadBits(bBuf,1);        //1; //固定为0x01  
				tmpPMT.zero =  ReadBits(bBuf,1);                            //1; //0x01  
				tmpPMT.reserved_1  =  ReadBits(bBuf,2);                     //2; //0x03  
				tmpPMT.section_length  =  ReadBits(bBuf,12);                 //12 =  ReadBits(bBuf,13);首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。  
				tmpPMT.program_number  =  ReadBits(bBuf,16);                   //16 =  ReadBits(bBuf,13); 指出该节目对应于可应用的Program map PID  
				fprintf(pfLog,"table_id: %d\n",tmpPMT.table_id);
				fprintf(pfLog,"section_length: %d\n",tmpPMT.section_length);
				fprintf(pfLog,"program_number: %d\n",tmpPMT.program_number);
				tmpPMT.reserved_2      =  ReadBits(bBuf,2);                   //2; //0x03  
				tmpPMT.version_number =  ReadBits(bBuf,5);                    //5; //指出TS流中Program map section的版本号  
				tmpPMT.current_next_indicator  =  ReadBits(bBuf,1);           //1; //当该位置1时，当前传送的Program map section可用；  
				//当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。  
				tmpPMT.section_number     =  ReadBits(bBuf,8);                //8; //固定为0x00  
				tmpPMT.last_section_number  =  ReadBits(bBuf,8);           //8; //固定为0x00  
				tmpPMT.reserved_3  =  ReadBits(bBuf,3);                       //3; //0x07  
				tmpPMT.PCR_PID  =  ReadBits(bBuf,13);                       //13; //指明TS包的PID值，该TS包含有PCR域，  
															//该PCR值对应于由节目号指定的对应节目。  
															//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
				tmpPMT.reserved_4    =  ReadBits(bBuf,4);                     //4; //预留为0x0F  
				tmpPMT.program_info_length =  ReadBits(bBuf,12);//12 //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  
			
				fprintf(pfLog,"version_number: %d\n",tmpPMT.version_number);
				fprintf(pfLog,"current_next_indicator: %d\n",tmpPMT.current_next_indicator);
				fprintf(pfLog,"section_number: %d\n",tmpPMT.section_number);
				fprintf(pfLog,"last_section_number: %d\n",tmpPMT.last_section_number);

				fprintf(pfLog,"PCR_PID: %d\n",tmpPMT.PCR_PID);
				fprintf(pfLog,"program_info_length: %d\n",tmpPMT.program_info_length);

				fprintf(fPCRTableFile,"program_number: %d\t",tmpPMT.program_number);
				fprintf(fPCRTableFile,"PCR_PID: %d\n",tmpPMT.PCR_PID);
				fflush(fPCRTableFile);
				fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
				//12 //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
				if( tmpPMT.program_info_length >0) 			  
				{
					for(i=0; i<tmpPMT.program_info_length; i++)
						ReadBits(bBuf,8); 
				}
				byteLeft = tmpPMT.section_length - tmpPMT.program_info_length - 13;
				while( byteLeft  > 0 ){
					fprintf(pfLog,"\nbyteLeft = %d\n",byteLeft );
					tmpPMT.PMT_Stream  =  ReadBits(bBuf,8);;	//每个元素包含8位, 指示特定PID的节目元素包的类型。
																//该处PID由elementary PID指定  
					tmpPMT.reserved_5  =  ReadBits(bBuf,3);     
					tmpPMT.elementary_PID  =  ReadBits(bBuf,13); //13; 
					tmpPMT.reserved_6   =  ReadBits(bBuf,4);
					tmpPMT.ES_info_length  =  ReadBits(bBuf,12); //12;
					fprintf(pfLog,"PMT_Stream: %d 0x%02X\n",tmpPMT.PMT_Stream,tmpPMT.PMT_Stream);
					fprintf(pfLog,"elementary_PID: %d\n",tmpPMT.elementary_PID);
					fprintf(pfLog,"ES_info_length: %d\n",tmpPMT.ES_info_length);
					fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
					//保存属于当前节目的ES流 的PID号，一路节目会有视频流、音频流，甚至字幕流
					programPESPIDTable[programIdx][programESIdx] = tmpPMT.elementary_PID;
					programESIdx++;
					programPESPIDTable[programIdx][15] = programESIdx;//保存当前节目的ES流数目
					if( tmpPMT.ES_info_length >0) 			  
					{
						for(i=0; i<tmpPMT.ES_info_length; i++)
							ReadBits(bBuf,8); 
					}
					byteLeft = byteLeft - 5 - tmpPMT.ES_info_length;
				}
				tmpPMT.CRC_32 =  ReadBits(bBuf,32); 
				for(i=0; i< programTotalNum; i++ )
				{
					//fprintf(pfLog,"program %d: %d PES {",programNumberTable[i],programPESPIDTable[i][15]);
					fprintf(fPMTTableFile,"program %d: %d PES {",programNumberTable[i],programPESPIDTable[i][15]);
					for(j=0; j< programPESPIDTable[i][15]; j++ )
					{
						//fprintf(pfLog,"\t%d",programPESPIDTable[i][j]);
						fprintf(fPMTTableFile,"\t%d",programPESPIDTable[i][j]);
						if(programPESPIDFileTable[i][j] == NULL){
							char tmpBuff[128];
							sprintf(tmpBuff,"%d",programPESPIDTable[i][j]);
              strcat(tmpBuff, ".log");
              fprintf(fPMTTableFile, "\t PES filename %s", tmpBuff);
							programPESPIDFileTable[i][j] = fopen( tmpBuff,"wb");
							if(programPESPIDFileTable[i][j] == NULL){
								printf("creat %s failed\n",tmpBuff);
							}else{
								fprintf(pfLog,"creat %s success\n",tmpBuff);
							}
						}
					}
					//fprintf(pfLog," }\n");
					fprintf(fPMTTableFile," }\n");
					fflush(fPMTTableFile);
				}
			}
		}
		//如果是PES包 
		if( isPESPacket && tmpTSHeader.payload_unit_start_indicator == 1 && tmpTSHeader.PID == 512){
      //101节目的PES流
			PES tmpPES;
			FILE * fpTmpPES= programPESPIDFileTable[programPIDIdx][programPESIdx];
			fprintf(pfLog,"TotalBitsRead: %d  Byte：%d %d\n",bBuf->TotalBitsRead,(bBuf->TotalBitsRead)/8,(bBuf->TotalBitsRead)%8);
			
			tmpPES.packet_start_code_prefix = ReadBits(bBuf,24);
			tmpPES.stream_id = ReadBits(bBuf,8);
			tmpPES.PES_packet_length = ReadBits(bBuf,16);
			fprintf(pfLog,"packet_start_code_prefix= 0x%02X\n",tmpPES.packet_start_code_prefix);
			fprintf(pfLog,"stream_id: 0x%02X\n",tmpPES.stream_id);
			fprintf(pfLog,"PES_packet_length: %d\n",tmpPES.PES_packet_length);

			if( tmpPES.stream_id  != STID_program_stream_map &&
				  tmpPES.stream_id  != STID_padding_stream &&
				  tmpPES.stream_id  != STID_private_stream_2 &&
				  tmpPES.stream_id  != STID_ECM &&
				  tmpPES.stream_id  != STID_EMM &&
				  tmpPES.stream_id  != STID_program_stream_directory &&
				  tmpPES.stream_id  != STID_DSMCC_stream &&
				  tmpPES.stream_id  != STID_ITU_H_222_1_type_E_stream
				){

				uint8 tmpRead = ReadBits(bBuf,2);
				if( tmpRead != 2 ){//bit 10
					printf("error From if( tmpRead != 2 ){//bit 10 \n");
					exit(0);
				}

				tmpPES.PES_scrambling_control = ReadBits(bBuf,2);
				tmpPES.PES_priority = ReadBits(bBuf,1);
				tmpPES.data_alignment_indicator = ReadBits(bBuf,1);
				tmpPES.copyright = ReadBits(bBuf,1);
				tmpPES.original_or_copy = ReadBits(bBuf,1);
				tmpPES.PTS_DTS_flags = ReadBits(bBuf,2);
				tmpPES.ESCR_flag = ReadBits(bBuf,1);
				tmpPES.ES_rate_flag = ReadBits(bBuf,1);
				tmpPES.DSM_trick_mode_flag = ReadBits(bBuf,1);
				tmpPES.additional_copy_info_flag = ReadBits(bBuf,1);
				tmpPES.PES_CRC_flag = ReadBits(bBuf,1);
				tmpPES.PES_extension_flag = ReadBits(bBuf,1);				
        tmpPES.PES_header_data_length = ReadBits(bBuf, 8);
        fprintf(pfLog, "PES_scrambling_control: %d\n", tmpPES.PES_scrambling_control);
        fprintf(pfLog, "PES_priority: %d\n", tmpPES.PES_priority);
        fprintf(pfLog, "PES_data_alignment_indicator: %d\n", tmpPES.data_alignment_indicator);
        fprintf(pfLog, "PES_copyright: %d\n", tmpPES.copyright);
        fprintf(pfLog, "PES_original_or_copy: %d\n", tmpPES.original_or_copy);
        fprintf(pfLog, "PTS_DTS_flags: %d\n", tmpPES.PTS_DTS_flags);
        fprintf(pfLog, "ESCR_flag: %d\n", tmpPES.ESCR_flag);
        fprintf(pfLog, "ES_rate_flag: %d\n", tmpPES.ES_rate_flag);
        fprintf(pfLog, "DSM_trick_mode_flag: %d\n", tmpPES.DSM_trick_mode_flag);
        fprintf(pfLog, "additional_copy_info_flag: %d\n", tmpPES.additional_copy_info_flag);
        fprintf(pfLog, "PES_CRC_flag: %d\n", tmpPES.PES_CRC_flag);
        fprintf(pfLog, "PES_extension_flag: %d\n", tmpPES.PES_extension_flag);
        fprintf(pfLog, "PES_header_data_length: %d\n", tmpPES.PES_header_data_length);

        if (tmpPES.PTS_DTS_flags == 2) {//'10'
          //'0010' 4 bslbf
          uint8 tmpRead = ReadBits(bBuf, 4);
          if (tmpRead != 2){//bit 0010
            printf("error From if( tmpRead != 4 ){//bit 0010 \n");
            exit(0);
          }
          uint64 tmpTotalPTS = 0;
          uint32 tmpPTS = ReadBits(bBuf, 3);// PTS[32..30] 3 bslbf
          tmpTotalPTS = tmpPTS;
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpPTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalPTS = ((tmpTotalPTS << 15) + tmpPTS);
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpPTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalPTS = ((tmpTotalPTS << 15) + tmpPTS);
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 
          tmpPES.PTS = tmpTotalPTS;
          fprintf(pfLog, "PTS: %I64u\n", tmpPES.PTS);
        }//PTS end


        if (tmpPES.PTS_DTS_flags == 3) {//'11'  PTS + DTS
          //'0011' 4 bslbf
          uint8 tmpRead = ReadBits(bBuf, 4);
          if (tmpRead != 3){//bit 0011
            printf("error From if( tmpRead != 4 ){//bit 0011 \n");
            exit(0);
          }
          uint64 tmpTotalPTS = 0;
          uint32 tmpPTS = ReadBits(bBuf, 3);// PTS[32..30] 3 bslbf
          tmpTotalPTS = tmpPTS;
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpPTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalPTS = ((tmpTotalPTS << 15) + tmpPTS);
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpPTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalPTS = ((tmpTotalPTS << 15) + tmpPTS);
          fprintf(pfLog, "tmpPTS: %u\n", tmpPTS);
          fprintf(pfLog, "tmpTotalPTS: %I64u\n", tmpTotalPTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 
          tmpPES.PTS = tmpTotalPTS;
          fprintf(pfLog, "PTS: %I64u\n", tmpPES.PTS);


          //'0011' 4 bslbf
          tmpRead = ReadBits(bBuf, 4);
          if (tmpRead != 1){//bit 0001
            printf("error From if( tmpRead != 4 ){//bit 0001 \n");
            exit(0);
          }
          uint64 tmpTotalDTS = 0;
          uint32 tmpDTS = ReadBits(bBuf, 3);// PTS[32..30] 3 bslbf
          tmpTotalDTS = tmpDTS;
          fprintf(pfLog, "tmpDTS: %u\n", tmpDTS);
          fprintf(pfLog, "tmpTotalDTS: %I64u\n", tmpTotalDTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpDTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalDTS = ((tmpTotalDTS << 15) + tmpDTS);
          fprintf(pfLog, "tmpDTS: %u\n", tmpDTS);
          fprintf(pfLog, "tmpTotalDTS: %I64u\n", tmpTotalDTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 

          tmpDTS = ReadBits(bBuf, 15);// PTS[29..15] 15 bslbf
          tmpTotalDTS = ((tmpTotalDTS << 15) + tmpDTS);
          fprintf(pfLog, "tmpDTS: %u\n", tmpDTS);
          fprintf(pfLog, "tmpTotalDTS: %I64u\n", tmpTotalDTS);

          tmpRead = ReadBits(bBuf, 1);
          if (tmpRead != 1){//bit 1
            printf("error From marker_bit=1 \n");
            exit(0);
          }//marker_bit 1 bslbf 
          tmpPES.DTS = tmpTotalDTS;
          fprintf(pfLog, "DTS: %I64u\n", tmpPES.DTS);

        }//DTS end
			}
		}
	}
	
	return 0;
}