//
#ifndef TS_H
#define TS_H

#include "types.h"
#include "typedef.h"
//标准文档： T-REC-H.222.0-201206-S!!PDF-E.pdf  
//TS 包结构体，一般是188字节
/*1、TS包包头
TS包的包头提供关于传输方面的信息：同步、有无差错、有无加扰、PCR（节目参考时钟）等标志。
TS包的包头长度不固定，前32比特（4个字节）固定，后面可能跟有自适应字段（适配域）。
32个比特（4个字节）是最小包头。包头的结构固定如下：
*/
struct TS_PACKET{
	uint8 * data;
	int		size;
	uint16	PID;
};
/*
sync_byte （同步字节）：固定为0100 0111 (0x47)；该字节由解码器识别，使包头和有效负载可相互分离。
transport_error_indicator（传输错误指示）：‘1’表示在相关的传输包中至少有一个不可纠正的错误位。当被置1后，在错误被纠正之前不能重置为0。
payload_unit_start_indicator（开始指示）：为1时，在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。因此有效载荷开始的位置应再偏移1+[length]个字节。
transport_priority（传输优先级）：‘1’表明优先级比其他具有相同PID 但此位没有被置‘1’的分组高。
PID：指示存储与分组有效负载中数据的类型。PID 值 0x0000—0x000F 保留。其中0x0000为PAT保留；0x0001为CAT保留；0x1fff为分组保留，即空包。
transport_scrambling_control（加扰控制）：表示TS流分组有效负载的加密模式。空包为‘00’，如果传输包包头中包括调整字段，不应被加密。
adaptation_field_control（适配域控制）：表示包头是否有调整字段或有效负载。‘00’为ISO/IEC未来使用保留；‘01’仅含有效载荷，无调整字段；‘10’ 无有效载荷，仅含调整字段；‘11’ 调整字段后为有效载荷，调整字段中的前一个字节表示调整字段的长度length，有效载荷开始的位置应再偏移[length]个字节。空包应为‘10’。
continuity_counter（连续性计数器）：随着每一个具有相同PID的TS流分组而增加，当它达到最大值后又回复到0。范围为0~15。
*/
typedef struct TS_packet_header
{
uint16 sync_byte;						// : 8;
uint16 transport_error_indicator;		// : 1;
uint16 payload_unit_start_indicator;	// : 1;
uint16 transport_priority;				// : 1;
uint16 PID;								// : 13;
uint16 transport_scrambling_control;	// : 2;
uint16 adaptation_field_control;			// : 2;
uint16 continuity_counter;				// : 4;
} TS_packet_header;
/*
2、TS包净荷部分
TS包中净荷所传输的信息包括两种类型：
1、视频、音频的PES包以及辅助数据；
2、节目专用信息PSI。
当然，TS包也可以是空包。
空包用来填充TS流，可能在重新进行多路复用时被插入或删除。
*/
//标准：2.4.1 Transport stream coding structure and parameters
/*
The PSI tables are carried in the transport stream. There are six PSI tables:
• Program association table (PAT);
• Program map table (MPT);
• Conditional access table (CAT);
• Network information table (NIT);
• Transport stream description table (TSDT);
• IPMP control information table.
*/
/*
PID value 0x0000 is
reserved for the program association table (see Table 2-30).
PID value 0x0001 is reserved for the conditional access table (see Table 2-32). 
PID value 0x0002 is reserved for the transport stream description table (see Table 2-36),
PID value 0x0003 is reserved for IPMP control information table (see ISO/IEC 13818-11) 
PID value 0x0004-0x000F are reserved. 
PID value 0x0010-0x1FFE assigned as network_PID, Program_map_PID, elementary_PID, or for other purposes
PID value 0x1FFF is reserved for null packets (see Table 2-3).
*/

//Table 2-6 – Transport stream adaptation field

typedef struct TS_adaptation_field{
//adaptation_field() {
	ui32 adaptation_field_length;//				8 uimsbf
	ui32 discontinuity_indicator;//					1 bslbf
	ui32 random_access_indicator;//					1 bslbf
	ui32 elementary_stream_priority_indicator;//		1 bslbf
	ui32 PCR_flag;//									1 bslbf
	ui32 OPCR_flag;//								1 bslbf
	ui32 splicing_point_flag;//						1 bslbf
	ui32 transport_private_data_flag;//				1 bslbf
	ui32 adaptation_field_extension_flag;//			1 bslbf
	//if (PCR_flag = = '1') {
	ui64 program_clock_reference_base;//		 33 uimsbf
	//reserved;//		 6 bslbf
	ui32 program_clock_reference_extension;//		 9 uimsbf
	//}
	//if (OPCR_flag = = '1') {
	ui64 original_program_clock_reference_base;//		 33 uimsbf
	//reserved;//		 6 bslbf
	ui32 original_program_clock_reference_extension;//		 9 uimsbf
	//}
	//if (splicing_point_flag = = '1') {
	ui32 splice_countdown;//		 8 tcimsbf
	//}
	//if (transport_private_data_flag = = '1') {
	ui32 transport_private_data_length;//		 8 uimsbf
	//for (i = 0; i < transport_private_data_length; i++) {
	ui32 private_data_byte;//		 8 bslbf
	//}
	//if (adaptation_field_extension_flag = = '1') {
	ui32 adaptation_field_extension_length;//		 8 uimsbf
	ui32 ltw_flag;//		 1 bslbf
	ui32 piecewise_rate_flag;//		 1 bslbf
	ui32 seamless_splice_flag;//		 1 bslbf
	//reserved;//		 5 bslbf
	//if (ltw_flag = = '1') {
	ui32 ltw_valid_flag;//		 1 bslbf
	ui32 ltw_offset;//		 15 uimsbf
	//}
	//if (piecewise_rate_flag = = '1') {
	//reserved;//		 2 bslbf
	ui32 piecewise_rate;//		 22 uimsbf
	//}
	//if (seamless_splice_flag = = '1') {
	ui32 Splice_type;//		 4 bslbf
	ui32 DTS_next_AU[33];//		[32..30] 3 bslbf
	//marker_bit;//		 1 bslbf
	//ui32 DTS_next_AU;//		[29..15] 15 bslbf
	//marker_bit;//		 1 bslbf
	//ui32 DTS_next_AU;//		[14..0] 15 bslbf
	//marker_bit;//		 1 bslbf
	//}
	//for (i = 0; i < N; i++) {
	//reserved;//		 8 bslbf
	//}
	//}
	//for (i = 0; i < N; i++) {
	//stuffing_byte;//		 8 bslbf
	//}
	}TS_adaptation_field;
/*
PID value 0x0000 is
reserved for the program association table (see Table 2-30).
PID value 0x0001 is reserved for the conditional access table (see Table 2-32). 
*/
//PAT表给出节目ID 与 TS PID之间的对应：program_number and the PID 
typedef struct PAT
{
uint16 table_id;						// : 8;
uint16 section_syntax_indicator;		// : 1;
//'0'										 1 bslbf
//reserved									 2 bslbf
uint16 section_length;					// : 12
uint16 transport_stream_id;				// : 16;  用来区分同一个网络中复用的TS码流，值由用户指定
//reserved									 2 bslbf
uint16 version_number;					// : 5;
uint16 current_next_indicator;			// : 1;
uint16 section_number;					// : 8;
uint16 last_section_number;				// : 8;
uint16 program_number;
uint16 network_PID;
uint16 program_map_PID;
uint32 CRC_32;
} PAT;
/*
PAT表中 成员 table_id对应的含义
Value Description
0x00 program_association_section
0x01 conditional_access_section (CA_section)
0x02 TS_program_map_section
0x03 TS_description_section
0x04 ISO_IEC_14496_scene_description_section
0x05 ISO_IEC_14496_object_descriptor_section
0x06 Metadata_section
0x07 IPMP_Control_Information_section (defined in ISO/IEC 13818-11)
*/

//PMT 表结构体  
typedef struct PMT  
{  
uint16 table_id ;//                       : 8; //固定为0x02, 表示PMT表  
uint16 section_syntax_indicator;//        : 1; //固定为0x01  
uint16 zero;//                            : 1; //0x01  
uint16 reserved_1 ;//                     : 2; //0x03  
uint16 section_length ;//                 : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。  
uint16 program_number ;//                   : 16;// 指出该节目对应于可应用的Program map PID  
uint16 reserved_2     ;//                   : 2; //0x03  
uint16 version_number;//                    : 5; //指出TS流中Program map section的版本号  
uint16 current_next_indicator ;//           : 1; //当该位置1时，当前传送的Program map section可用；  
             //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。  
uint16 section_number    ;//                : 8; //固定为0x00  
uint16 last_section_number ;//           : 8; //固定为0x00  
uint16 reserved_3 ;//                       : 3; //0x07  
uint16 PCR_PID ;//                       : 13; //指明TS包的PID值，该TS包含有PCR域，  
											//该PCR值对应于由节目号指定的对应节目。  
											//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
uint16 reserved_4   ;//                     : 4; //预留为0x0F  
uint16 program_info_length;//12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  

uint16 PMT_Stream;  //每个元素包含8位, 指示特定PID的节目元素包的类型。
					//该处PID由elementary PID指定  
uint16 reserved_5 ;//                       : 3; //0x07  
uint16 reserved_6  ;//                      : 4; //0x0F  
uint16 elementary_PID; //	16
uint16 ES_info_length;// 16
uint32 CRC_32;//                            : 32;   
} PMT; 

//PES 包提取

//Table 2-22 – Stream_id assignments
#define STID_program_stream_map			0xBC  //1011 1100
		/*
		NOTE 2 – PES packets of type private_stream_1 and 
		ISO/IEC_13552_stream follow the same PES packet syntax 
		as those for
		Rec. ITU-T H.262 | ISO/IEC 13818-2 video and 
		ISO/IEC 13818-3 audio streams.
		*/
#define STID_private_stream_1			0xBD  //1011 1100
										
#define STID_padding_stream				0xBE  //1011 1100
#define STID_private_stream_2			0xBF  //1011 1100

#define STID_H262_H264_MIN				0xE0  //1110 xxxx
#define STID_H262_H264_MAX				0xEF  //1110 xxxx


#define STID_ECM						0xF0  //1011 1100
#define STID_EMM						0xF1  //1011 1100
#define STID_program_stream_directory	0xFF  //1011 1100
#define STID_DSMCC_stream				0xF2  //1011 1100
#define STID_ITU_H_222_1_type_E_stream	0xBC  //1011 1100
#define STID_metadata_stream			0xFC  //1011 1100

//PMT 表结构体  
typedef struct PES  
{  
uint32 packet_start_code_prefix ;//		24 bslbf
uint16 stream_id ;//					8 uimsbf
uint16 PES_packet_length ;//			16 uimsbf

uint16 PES_scrambling_control ;//		2 bslbf
uint16 PES_priority ;//					1 bslbf
uint16 data_alignment_indicator ;//		1 bslbf
uint16 copyright ;//					1 bslbf
uint16 original_or_copy ;//				1 bslbf
uint16 PTS_DTS_flags ;//				2 bslbf
uint16 ESCR_flag ;//					1 bslbf
uint16 ES_rate_flag ;//					1 bslbf
uint16 DSM_trick_mode_flag ;//			1 bslbf
uint16 additional_copy_info_flag ;//	1 bslbf
uint16 PES_CRC_flag ;//					1 bslbf
uint16 PES_extension_flag ;//			1 bslbf
uint16 PES_header_data_length ;//		8 uimsbf

uint64 PTS ;//[32..30] ;//	 3 bslbf
uint16 marker_bit ;//	 1 bslbf
//uint16 PTS [29..15] ;//	 15 bslbf
//uint16 marker_bit ;//	 1 bslbf
//uint16 PTS [14..0] ;//	 15 bslbf
//uint16 marker_bit ;//	 1 bslbf
uint64 DTS ;//[32..30] ;//	 3 bslbf
uint64 ESCR_base;//[32..30]
uint16 ESCR_extension;//					9 uimsbf
uint32 ES_rate;//							22 uimsbf;
uint16 trick_mode_control;//				3 uimsbf

uint16 field_id;//							2 bslbf
uint16 intra_slice_refresh;//				1 bslbf
uint16 frequency_truncation;//				2 bslbf
uint16 rep_cntrl;//							5 uimsbf

uint16 additional_copy_info;//				7 bslbf
uint16 previous_PES_packet_CRC;//			16 bslbf

uint16 PES_private_data_flag;//				1 bslbf
uint16 pack_header_field_flag;//			1 bslbf
uint16 program_packet_sequence_counter_flag;//	1 bslbf
uint16 P_STD_buffer_flag;//					1 bslbf
uint16 PES_extension_flag_2;//				1 bslbf

uint8 PES_private_data[16];//				128 bslbf
uint8 pack_field_length;//					8 uimsbf

uint8 program_packet_sequence_counter;//			7 uimsbf

uint8 MPEG1_MPEG2_identifier;//				1 bslbf
uint8 original_stuff_length;//				6 uimsbf

uint16 P_STD_buffer_scale;//				1 bslbf
uint16 P_STD_buffer_size;//					13 uimsbf

uint16 PES_extension_field_length;//		7 uimsbf
uint16 stream_id_extension_flag;//			1 bslbf

uint16 stream_id_extension;//				7 uimsbf

uint16 tref_extension_flag;//				1 bslbf

uint64 TREF;//
uint8 PES_packet_data_byte[188];//  PES_packet_data_byte[187]存放的是当前数组中的有效字节数

uint8 padding_byte[188];//  padding_byte[187]存放的是当前数组中的有效字节数
uint32 CRC_32;//                            : 32;   
} PES; 
#endif