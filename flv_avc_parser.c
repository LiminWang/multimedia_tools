#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define LEN		2048


#define FLV_AUDIODATA     8
#define FLV_VIDEODATA     9
#define FLV_SCRIPTDATA    18

#define FLV_AUDIO_PCM     0 // Linear PCM, platform endian
#define FLV_AUDIO_ADPCM   1 // ADPCM
#define FLV_AUDIO_MP3     2 // MP3
#define FLV_AUDIO_PCMLE   3 // Linear PCM, little endian
#define FLV_AUDIO_NELLY16 4 // Nellymoser 16 kHz mono
#define FLV_AUDIO_NELLY8  5 // Nellymoser 8 kHz mono
#define FLV_AUDIO_NELLY   6 // Nellymoser
// 7 = G.711 A-law logarithmic PCM (reserved)
// 8 = G.711 mu-law logarithmic PCM (reserved)
// 9 = reserved
#define FLV_AUDIO_AAC     10 // AAC
#define FLV_AUDIO_SPEEX   11 // Speex
// 14 = MP3 8 kHz (reserved)
// 15 = Device-specific sound (reserved)

//#define FLV_VIDEO_JPEG   1 // non-standard? need samples
#define FLV_VIDEO_H263     2 // Sorenson H.263
#define FLV_VIDEO_SCREEN   3 // Screen video
#define FLV_VIDEO_VP6      4 // On2 VP6
#define FLV_VIDEO_VP6A     5 // On2 VP6 with alpha channel
#define FLV_VIDEO_SCREEN2  6 // Screen video version 2
#define FLV_VIDEO_AVC      7 // AVC
#define FLV_VIDEO_HM62    11 // HM6.2
#define FLV_VIDEO_HM91    12 // HM9.1
#define FLV_VIDEO_HM10    13 // HM10.0
#define FLV_VIDEO_HEVC    14 // HEVC (HM version write to MetaData "HM compatibility")

#define NAL_UNIT_VPS	  32
#define NAL_UNIT_SPS      33
#define NAL_UNIT_PPS      34



#define ERROR_MSG(s)	do { \
	printf("ERR: (%s)(%d) %s", __FUNCTION__, __LINE__, s); \
} while (0)


struct _data {
	unsigned char bit_pos;
	unsigned char *pos;
	unsigned long rest_data;
	unsigned long file_pos;
	unsigned char d[LEN];
} data;

struct _header {
	unsigned char signature[3];
	unsigned char version;
	unsigned char dummy;
	unsigned char audio;
	unsigned char summy2;
	unsigned char video;
	unsigned long dataoffset;
};

struct _tag {
	unsigned long previous_tag_size;
	unsigned char tag_type;
	unsigned long data_size;
	unsigned long timestamp;
	unsigned long timestamp_extended;
	unsigned long stream_id;
	unsigned long data;
};

struct _vdata {
	unsigned char frame_type;
	unsigned char codec_id;
};

struct _avcpacket {
	unsigned char AVCPacketType;
	unsigned long CompositionTime;
};

struct _vc_params_t {
	unsigned long width, height;
	unsigned long profile, level;
	unsigned long nal_length_size;
} params;
	

int data_init(char *file)
{
	int size;
	FILE *f;
	
	f = fopen(file, "rb");
	size = fread(data.d, 1, LEN, f);
	if (size < LEN)
	{
		printf("Read EOF!\n");
		return -1;
	}
	fclose(f);
	
	data.bit_pos = 7;
	data.pos = data.d;
	data.rest_data = LEN;
	
	return 0;
}

unsigned long long bit_read(unsigned long bits)
{
	unsigned long long ret = 0; 
	
	while (bits--)
	{
		ret <<= 1;
		
		if (*data.pos & (1 << data.bit_pos))
		{
			ret |= 1;
		}
		
		if (data.bit_pos == 0)
		{
			data.pos++;
			data.file_pos++;
			data.bit_pos = 7;
			data.rest_data--;
			if (data.rest_data == 0)
			{
				ERROR_MSG("data exhaust!\n");
				exit(-1);
			}
		}
		else
		{
			data.bit_pos--;
		}
	}
	
	return ret;
}

unsigned long long ue_read(void)
{
	int zeros = 0;
	
	while (bit_read(1) == 0)
	{
		zeros++;
	}
	return bit_read(zeros) + ((1 << zeros) - 1);
}

int byte_skip(unsigned long bytes)
{
	while (bytes--)
	{
		data.pos++;
		data.file_pos++;
		data.rest_data--;
		if (data.rest_data == 0)
		{
			ERROR_MSG("data exhaust!\n");
			exit(-1);
		}
	}
	
	return 0;
}

int fpos_jmp(unsigned long fpos)
{
	long bytes = (long)(fpos - data.file_pos);
	
	if (bytes > 0)
	{
		byte_skip(bytes);
	}
	else
	{
		ERROR_MSG("data exhaust!\n");
		exit(-1);
	}
	
	return 0;
}


int read_header(struct _header *h)
{
	if (!h)
	{
		ERROR_MSG("header pointer is NULL!\n");
		return -1;
	}

	memset(h, 0, sizeof(struct _header));

	h->signature[0] = bit_read(8);
	h->signature[1] = bit_read(8);
	h->signature[2] = bit_read(8);
	h->version = bit_read(8);
	h->dummy = bit_read(5);
	h->audio = bit_read(1);
	h->summy2 = bit_read(1);
	h->video = bit_read(1);
	h->dataoffset = bit_read(32);
	
	printf("header:\n");
	printf("signature = %c%c%c\n", h->signature[0], 
			h->signature[1], h->signature[2]);
	printf("version = %d\n", h->version);
	printf("audio = %d\n", h->audio);
	printf("video = %d\n", h->video);
	printf("dataoffset = 0x%X\n", h->dataoffset);
	printf("\n");
	
	return 0;
}

int read_tag(struct _tag *t)
{
	if (!t)
	{
		ERROR_MSG("tag pointer is NULL!\n");
		return -1;
	}

	memset(t, 0, sizeof(struct _tag));
	
	t->previous_tag_size = bit_read(32);
	t->tag_type = bit_read(8);
	t->data_size = bit_read(24);
	t->timestamp = bit_read(24);
	t->timestamp_extended = bit_read(8);
	t->stream_id = bit_read(24);
	t->data = data.file_pos;
	
	printf("tag(0x%X):\n", data.file_pos);
	printf("previous_tag_size = 0x%X\n", t->previous_tag_size);
	printf("tag_type = 0x%X\n", t->tag_type);
	printf("data_size = 0x%X\n", t->data_size);
	printf("timestamp = 0x%X\n", t->timestamp);
	printf("timestamp_extended = 0x%X\n", t->timestamp_extended);
	printf("stream_id = 0x%X\n", t->stream_id);
	printf("\n");
	
	return 0;
}

int read_vdata(struct _vdata *v)
{
	if (!v)
	{
		ERROR_MSG("vdata pointer is NULL!\n");
		return -1;
	}
	
	memset(v, 0, sizeof(struct _vdata));

	v->frame_type = bit_read(4);
	v->codec_id = bit_read(4);
	
	printf("vdata:\n");
	printf("frame_type = 0x%X\n", v->frame_type);
	printf("codec_id = 0x%X\n", v->codec_id);
	printf("\n");
	
	return 0;
}

int read_avcpacket(struct _avcpacket *pkt)
{
	if (!pkt)
	{
		ERROR_MSG("avcpacket pointer is NULL!\n");
		return -1;
	}
	
	memset(pkt, 0, sizeof(struct _vdata));
	
	pkt->AVCPacketType = bit_read(8);
	pkt->CompositionTime = bit_read(24);
}


int ParseSequenceParameterSetHM91(void)
{
	int sps_max_sub_layers_minus1;
	int chroma_format_idc;
	int i;

	bit_read(4);
	sps_max_sub_layers_minus1 = bit_read(3);

	bit_read(1);
	
	bit_read(2);					// XXX_profile_space[]
	bit_read(1);					// XXX_tier_flag[]
	params.profile = bit_read(5);	// XXX_profile_idc[]
	
	bit_read(32);					// XXX_profile_compatibility_flag[][32]
	
	// HM9.1
	bit_read(16);					// XXX_reserved_zero_16bits[]
	
	params.level = bit_read(8);	// general_level_idc
	
	// HM9.1
	for (i = 0; i < sps_max_sub_layers_minus1; i++) {
		int sub_layer_profile_present_flag, sub_layer_level_present_flag;
		
		sub_layer_profile_present_flag = bit_read(1);	// sub_layer_profile_present_flag[i]
		sub_layer_level_present_flag = bit_read(1);		// sub_layer_level_present_flag[i]

		if (sub_layer_profile_present_flag) {
			bit_read(2);			// XXX_profile_space[]
			bit_read(1);			// XXX_tier_flag[]
			bit_read(5);			// XXX_profile_idc[]
			bit_read(32);			// XXX_profile_compatibility_flag[][32]
			bit_read(16);			// XXX_reserved_zero_16bits[]
		}
		if (sub_layer_level_present_flag) {
			bit_read(8);			// sub_layer_level_idc[i]
		}
	}
	
	ue_read();							// seq_parameter_set_id
	
	chroma_format_idc = ue_read(); // chroma_format_idc  // 5FF
	if (chroma_format_idc == 3) {
		bit_read(1);					// separate_colour_plane_flag
	}
	
	params.width  = ue_read();
	params.height = ue_read();

	printf("width = %d, height = %d\n", params.width, params.height);
	
	exit(0);
	
	return 0;
}





void hevc_parse(unsigned char codec_id)
{
	struct _avcpacket pkt;
	unsigned long nal_length_size;
	int numOfSequenceParameterSets;
	int i;
	
	read_avcpacket(&pkt);
	printf("file_pos = 0x%X\n", data.file_pos);
	ERROR_MSG("\n");
	if (bit_read(8) != 1)
	{
		printf("file_pos = 0x%X\n", data.file_pos);
		ERROR_MSG("\n");
		return;
	}
	bit_read(8);
	bit_read(8);
	bit_read(8);
	
	if (bit_read(6) != 63)
	{
		ERROR_MSG("\n");
		return;
	}
	
	nal_length_size = bit_read(2) + 1;
	if (bit_read(3) != 7)
	{
		ERROR_MSG("\n");
		return;
	}
	
	numOfSequenceParameterSets = bit_read(5);
	printf("numOfSequenceParameterSets = 0x%X\n", numOfSequenceParameterSets);

	for (i=0; i<numOfSequenceParameterSets; i++)
	{
		int sps_len = bit_read(16);	// sequenceParameterSetLength
		printf("sps_len = 0x%X\n", sps_len);
		
		if (codec_id >= FLV_VIDEO_HM91 && sps_len > 2)
		{
			unsigned char sps_data = bit_read(8);
			
			if ((sps_data >> 1 & 0x3f) == NAL_UNIT_SPS)
			{
				bit_read(8);
				if (codec_id >= 13)
				{
					//ParseSequenceParameterSet();
				}
				else if (codec_id >= 12)
				{
					ParseSequenceParameterSetHM91();
				}
			}
		}

		//gb.SkipBytes(sps_len);			// sequenceParameterSetNALUnit
	}
	
}

void flv_parse(void)
{
	struct _header h;
	struct _tag tag;
	
	read_header(&h);
	
	while (1)
	{
		read_tag(&tag);
		
		if (tag.tag_type == FLV_SCRIPTDATA)
		{
			
		}
		else if (tag.tag_type == FLV_VIDEODATA) //video
		{
			struct _vdata v;
			read_vdata(&v);
			
			switch (v.codec_id)
			{
				case FLV_VIDEO_AVC:
					break;
				case FLV_VIDEO_HM62:
					break;
				case FLV_VIDEO_HM91:
				case FLV_VIDEO_HM10:
				case FLV_VIDEO_HEVC:
					hevc_parse(v.codec_id);
					break;
				default:
					break;
			}
		}
		fpos_jmp(tag.data + tag.data_size);
	}
}


int main(int argc, char **argv) 
{
	unsigned long a = 0;
	unsigned long long b = 0;
	
	if (argc < 2)
	{
		printf("Usage:\n");
		printf("\t%s [filename]\n", argv[0]);
		return -1;
	}
	
	data_init(argv[1]);
	
	flv_parse();
	
	
	return 0;
}



