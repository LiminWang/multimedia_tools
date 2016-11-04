#include <stdio.h>
#include <string.h>
#include <stdlib.h>



static unsigned char *data = NULL;

static const unsigned char sps_flag[] = {0,0,0,1,0x67};


unsigned char *find_sps_flag(unsigned char *data, unsigned int size)
{
	int i;
	int flag_len = sizeof(sps_flag) / sizeof(sps_flag[0]);
	unsigned int consume_size = 0;
	int found = 0;
	
	printf("find data %x size %d\n", data, size);
	
	while (consume_size + flag_len < size)
	{
		found = 1;
		for (i=0; i<flag_len; i++)
		{
			if (data[i] != sps_flag[i])
			{
				found = 0;
			}
		}
		
		if (found)
		{
			printf("consume %d\n", consume_size);
			return data;
		}
		
		data++;
		consume_size++;
	}
	
	return NULL;
}

int save_data_to_file(unsigned char *prefix, unsigned int index, unsigned char *data, unsigned int size)
{
	char filename[1024];
	FILE *output_file = NULL;
	
	printf("save data @ %x size %d.\n", data, size);
	
	sprintf(filename, "%s_%d.264", prefix, index);
	
	output_file = fopen(filename, "wb");
	if (output_file == NULL)
	{
		return -1;
	}
	
    fwrite(data, size, 1, output_file);
    
	fclose(output_file);
	
	return 0;
}

int main(int argc, char **argv) 
{
    FILE *input_file;
	unsigned int filesize = 0, size = 0;
	unsigned int split_size = 0;
	unsigned char *ptr = NULL, *last_ptr = NULL;
	unsigned int idx = 0;
    
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("\t%s [input_file] \n", argv[0]);
        return -1;
    }
    
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL)
    {
        printf("Open %s error!\n", argv[1]);
        return -1;
    }
	
	fseek(input_file, 0L, SEEK_END);
	filesize = ftell(input_file); 	
	fseek(input_file, 0L, SEEK_SET);
	
	printf("filesize = %d\n", filesize);
	
	data = (unsigned char *)malloc(filesize);
	if (fread(data, 1, filesize, input_file) < filesize)
	{
		printf("Read EOF @0x%x!\n", ftell(input_file));
		return -2;
	}
	
	ptr = data;
	
	printf("data in buffer %x size %d!\n", data, filesize);
	while (ptr = find_sps_flag(ptr, filesize))
	{
		if (ptr && last_ptr)
		{
			save_data_to_file(argv[1], idx++, ptr, ptr-last_ptr);
			filesize -= ptr - last_ptr + 1;
		}
		
		ptr++;
		last_ptr = ptr;
	}
	
	save_data_to_file(argv[1], idx, last_ptr, filesize);
	
    fclose(input_file);
    free(data);
    
    return 0;
}
    
