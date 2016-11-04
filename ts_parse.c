#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define	TS_SIZE		188
#define PES_SIZE	1024

unsigned char Tmp_data[TS_SIZE];
const unsigned char *TMP_FILE = "pes.tmp";


struct PES_header
{
	unsigned char packet_start_code_prefix[3];
	unsigned char stream_id;
	unsigned char PES_packet_length[2];
	unsigned char control_byte;
	unsigned char flags;
	unsigned char PES_header_data_length;
};

struct TS_header
{
	unsigned char sync_word;
	unsigned char pid[2];
	unsigned char continuity_counter;
};

struct adaptation_field
{
	unsigned char adaptation_field_length;
	unsigned char discontinuity_indicator;
};


int parse_ts_packet(FILE *f)
{
	int size = 0;
	struct TS_header *p_ts_packet = NULL;
	struct adaptation_field *p_adaptation_field = NULL;
	unsigned char adaptation_field_control = 0;

	if (!f)
	{
		return -1;
	}

	while (1)
	{
		size = fread(&Tmp_data, 1, TS_SIZE, f);
		if (size == 0)
		{
			printf("Read EOF @0x%x!\n", ftell(f));
			return -2;
		}

		p_ts_packet = (struct TS_header *)Tmp_data;
		if (p_ts_packet->sync_word != 0x47)
		{
			printf("sync word error @0x%x!\n", ftell(f));
			fseek(f, -187L, SEEK_CUR);
			continue;
		}

		adaptation_field_control = (p_ts_packet->continuity_counter >> 4) & 0x3;
		if (adaptation_field_control == 0x2 || adaptation_field_control == 0x3)
		{
			p_adaptation_field = (struct adaptation_field *)(Tmp_data + sizeof(struct TS_header));
			if (p_adaptation_field->adaptation_field_length > 0)
			{
				unsigned char discontinuity_indicator = p_adaptation_field->discontinuity_indicator & 0x80;

				//printf("discontinuity_indicator %d @0x%x\n", discontinuity_indicator, ftell(f)-188+5);
				if (discontinuity_indicator)
				{
					printf("----------------------discontinuity_indicator %d @0x%x!\n", discontinuity_indicator, ftell(f));
				}

			}
		}
	}

}

int main(int argc, char **argv) 
{
	FILE *file, *outfile;
	
	if (argc < 2)
	{
		printf("Usage:\n");
		printf("\t%s [filename]\n", argv[0]);
		printf("\t(pid is decimal number)\n");
		return -1;
	}
	
	{
		file = fopen(argv[1], "rb");
		if (file == NULL)
		{
			printf("Open %s error!\n", argv[1]);
			return -1;
		}

		parse_ts_packet(file);
		
		fclose(file);
	}
	
	return 0;
}
