#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MPEG_ES

#define POS     0
#define SIZE    1


#define FRAME_NUM   500
#define FRAME_SIZE  1*1024*1024

static unsigned char Tmp_data[FRAME_SIZE];

struct Nal
{
    unsigned char size[4];
    unsigned char data[0];
};


int parse_nal(FILE *input_file, FILE *output_file)
{
    char file_name[32];
    unsigned char start_code[4] = {0, 0, 0, 1};
    unsigned int i, addr = 0, size = 0;
    struct Frame *p_frame = NULL;

    if (!input_file)
    {
        return -1;
    }

    for (i=0; i<FRAME_NUM; i++)
    {
        addr = ftell(input_file);

        size = fread(Tmp_data, 1, 4, input_file);
        if (size < 4)
        {
            printf("Read EOF @0x%x!\n", ftell(input_file));
            return -2;
        }

        if (fwrite(start_code, 1, 4, output_file) != size)
        {
            printf("Write output_file error!\n");
            return -1;
        }

        size = Tmp_data[0];
        size <<= 8;
        size |= Tmp_data[1];
        size <<= 8;
        size |= Tmp_data[2];
        size <<= 8;
        size |= Tmp_data[3];
        if (size > FRAME_SIZE)
        {
            printf("frame size is too large %d!\n", size);
            return -3;
        }

        if (fread(Tmp_data, 1, size, input_file) < size)
        {
            printf("Read EOF 2 @0x%x!\n", ftell(input_file));
            return -4;
        }
        
        if (fwrite(Tmp_data, 1, size, output_file) != size)
        {
            printf("Write output_file error!\n");
            return -5;
        }

        printf("[%d] pts=0 addr=%x size=%x\n", i, addr, size+4);
    }
}

int main(int argc, char **argv) 
{
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("\t%s [input_file]\n", argv[0]);
        return -1;
    }
    
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL)
    {
        printf("Open %s error!\n", argv[1]);
        return -1;
    }

    if (argc >= 3)
    {
        output_file = fopen(argv[2], "wb");
        if (output_file == NULL)
        {
            printf("Open %s error!\n", argv[2]);
            return -1;
        } 
    }

    parse_nal(input_file, output_file);

    fclose(output_file);
    fclose(input_file);
    
    return 0;
}
    