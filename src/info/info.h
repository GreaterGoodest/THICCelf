#ifndef INFO
#define INFO

typedef struct target_info
{
	int ph_start;		   //start address for program headers
	int ph_num;			   //program header (ph) count
	int exe_segment_start; //start of executable segment
	int exe_segment_size;
} target_info;

#endif
