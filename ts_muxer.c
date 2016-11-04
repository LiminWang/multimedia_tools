#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define	TS_SIZE		188
#define PES_SIZE	1024

unsigned char Tmp_data[PES_SIZE];
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


int packing_PES(FILE *src_file, FILE *out_file, unsigned int stream_id)
{
	struct PES_header pes_header;
	unsigned int read_size = PES_SIZE - sizeof(struct PES_header) + 4;
	int first_pkt = 1;
	unsigned int size = 0, offset = 0;
	
	while (1)
	{
		size = fread(&Tmp_data, 1, read_size, src_file);
		if (size == 0)
		{
			printf("Read EOF!\n", offset);
			return 0;
		}
	
		// PES header
		{
			pes_header.packet_start_code_prefix[0] = 0x00;
			pes_header.packet_start_code_prefix[1] = 0x00;
			pes_header.packet_start_code_prefix[2] = 0x01;
			pes_header.stream_id = stream_id;
			pes_header.PES_packet_length[0] = (PES_SIZE >> 8) & 0xFF;
			pes_header.PES_packet_length[1] = PES_SIZE & 0xFF;
			pes_header.control_byte = 0x80;
			if (first_pkt)
			{
				pes_header.control_byte |= (1<<2);
				first_pkt = 0;
			}
			pes_header.flags = 0;
			pes_header.PES_header_data_length = 0;
			
			if (fwrite(&pes_header, 1, sizeof(struct PES_header), out_file) != sizeof(struct PES_header))
			{
				printf("Write out_file error!\n");
				return -1;
			}
		}
		
		// data
		{
			if (fwrite(&Tmp_data, 1, read_size, out_file) != read_size)
			{
				printf("Write out_file error!\n");
				return -1;
			}
		}
	}
}


int packing_TS(FILE *src_file, FILE *out_file, unsigned int pid)
{
	struct TS_header ts_header;
	unsigned int read_size = TS_SIZE - sizeof(struct TS_header);
	int first_pkt = 1;
	unsigned int size = 0, counter = 0;
	
	while (1)
	{
		size = fread(&Tmp_data, 1, read_size, src_file);
		if (size == 0)
		{
			printf("Read EOF!\n");
			return 0;
		}
	
		// TS header
		{
			ts_header.sync_word = 0x47;
			ts_header.pid[0] = (pid >> 8) & 0xFF;
			ts_header.pid[1] = pid & 0xFF;
			//printf("pid= 0x%04x, ts_header.pid[1]=%02x\n", pid, ts_header.pid[1]);
			if (first_pkt)
			{
				ts_header.pid[0] |= (1<<6);
				first_pkt = 0;
			}
			ts_header.continuity_counter = counter++;
			ts_header.continuity_counter |= (1<<4);
			counter %= 16;
			
			if (fwrite(&ts_header, 1, sizeof(struct TS_header), out_file) != sizeof(struct TS_header))
			{
				printf("Write out_file error!\n");
				return -1;
			}
		}
		
		// data
		{
			if (fwrite(&Tmp_data, 1, read_size, out_file) != read_size)
			{
				printf("Write out_file error!\n");
				return -1;
			}
		}
	}
}


int main(int argc, char **argv) 
{
	FILE *file, *outfile;
	
	if (argc < 3)
	{
		printf("Usage:\n");
		printf("\t%s [filename] [outfile]\n", argv[0]);
		printf("\t(pid is decimal number)\n");
		return -1;
	}
	
	// Add PES header
	{
		file = fopen(argv[1], "rb");
		if (file == NULL)
		{
			printf("Open %s error!\n", argv[1]);
			return -1;
		}
		outfile = fopen(TMP_FILE, "wb");
		if (file == NULL)
		{
			printf("Open %s error!\n", TMP_FILE);
			return -1;
		}

		packing_PES(file, outfile, 0xc0);
		
		fclose(outfile);
		fclose(file);
	}
	
	// Add TS pkt header
	{
		file = fopen(TMP_FILE, "rb");
		if (file == NULL)
		{
			printf("Open %s error!\n", TMP_FILE);
			return -1;
		}
		outfile = fopen(argv[2], "wb");
		if (file == NULL)
		{
			printf("Open %s error!\n", argv[2]);
			return -1;
		}
		
		packing_TS(file, outfile, 0x1100);
		
		fclose(outfile);
		fclose(file);
	}
	
	
	return 0;
}
