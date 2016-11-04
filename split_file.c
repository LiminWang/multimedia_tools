#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define READ_SIZE   512*1024

#define OUTPUT_FILENAME  "I_am_file_after_split"

static unsigned char Tmp_data[READ_SIZE];


int main(int argc, char **argv) 
{
    FILE *input_file, *output_file;
	unsigned int offset = 0, length = 0;
	const char *file_name;
    
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("\t%s [input_file] [offset(Byte)] [size(Byte)]\n", argv[0]);
        return -1;
    }
	
    file_name = argv[1];
	offset = atoi(argv[2]);
	length = atoi(argv[3]);
	printf("split %s, from %d, %d Bytes.\n",  file_name, offset, length);
	
    input_file = fopen(file_name, "rb");
    if (input_file == NULL)
    {
        printf("Open %s error!\n", file_name);
        return -1;
    }

	output_file = fopen(OUTPUT_FILENAME, "wb");
    if (output_file == NULL)
    {
        printf("Open output file error!\n");
        return -1;
    }
    
	fseek(input_file, offset, SEEK_SET);
	
	while (length)
	{
		unsigned int size = 0, split_size = READ_SIZE;
		
		split_size = (length < READ_SIZE) ? length : READ_SIZE;
		
		// input
		size = fread(Tmp_data, 1, split_size, input_file);
		if (size < split_size)
		{
			printf("Read EOF @0x%x, copy size = %d!\n", ftell(input_file), split_size);
			return -2;
		}

        // output
        if (fwrite(Tmp_data, 1, size, output_file) != size)
        {
            printf("Write output_file %s error!\n", output_file);
            return -1;
        }
        
		length -= size;
		printf(".");
	}
	
    fclose(output_file);
    fclose(input_file);
    
    printf("\nSuccess! output file is: %s\\%s\n", getcwd((char *)Tmp_data, sizeof(Tmp_data)), OUTPUT_FILENAME);
    
    return 0;
}
    
