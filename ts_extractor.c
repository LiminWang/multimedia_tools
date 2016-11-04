#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define	PKT_SIZE	188

union ts_pkt
{
	struct
	{
		unsigned char sync_byte;
		unsigned char pid[2];
		unsigned char counter;
	}header;
	unsigned char data[PKT_SIZE];
};

static union ts_pkt pkt;

int extract_pid(FILE *src_file, FILE *out_file, unsigned int pid)
{
	unsigned int pkt_pid = 0;
	unsigned int size = 0, offset = 0;
	
	while (1)
	{
		size = fread(&pkt, 1, PKT_SIZE, src_file);
		if (size == 0)
		{
			printf("Read EOF at %d!\n", offset);
			return 0;
		}
		
		if (pkt.header.sync_byte != 0x47) 
		{
			fseek(src_file, ++offset, SEEK_SET);
			printf("packet size is not %d @0x%x!\n", PKT_SIZE, offset);
			continue;
		}
		
		offset += size;
				
		pkt_pid = pkt.header.pid[0];
		pkt_pid <<= 8;
		pkt_pid |= pkt.header.pid[1];
		pkt_pid &= 0x1FFF;
		
		if (pkt_pid == pid)
		{
			//printf("pkt.header.pid = 0x%04x, offset = 0x%08x\n",  pkt.header.pid, offset);
			if (fwrite(&pkt, 1, size, out_file) != size)
			{
				printf("Write out_file error!\n");
				return -1;
			}
		}
	}
	return 0;
}

int main(int argc, char **argv) 
{
	FILE *file, *outfile;
	unsigned int pid = 0;
	
	if (argc < 4)
	{
		printf("Usage:\n");
		printf("\t%s [filename] [pid] [outfile]\n", argv[0]);
		printf("\t(pid is decimal number)\n");
		return -1;
	}
	
	file = fopen(argv[1], "rb");
	if (file == NULL)
	{
		printf("Open %s error!\n", argv[1]);
		return -1;
	}
	
	outfile = fopen(argv[3], "wb");
	if (file == NULL)
	{
		printf("Open %s error!\n", argv[3]);
		return -1;
	}
	pid = atoi(argv[2]);
	
	extract_pid(file, outfile, pid);
	
	fclose(outfile);
	fclose(file);
	
	return 0;
}
