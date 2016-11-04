#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define TS_SIZE     (192)
unsigned char Tmp_data[TS_SIZE];


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
    unsigned char time_stamp[4];
    unsigned char sync_word;
    unsigned char pid[2];
    unsigned char continuity_counter;
};

struct adaptation_field
{
    unsigned char adaptation_field_length;
    unsigned char discontinuity_indicator;
};


int convert_ts_packet(FILE *input_file, FILE *output_file)
{
    int size = 0;
    struct TS_header *p_ts_packet = NULL;

    if (!input_file || !output_file)
    {
        return -1;
    }

    while (1)
    {
        //input
        size = fread(Tmp_data, 1, TS_SIZE, input_file);
        if (size == 0)
        {
            printf("Read EOF @0x%x!\n", ftell(input_file));
            return -2;
        }

        p_ts_packet = (struct TS_header *)Tmp_data;
        if (p_ts_packet->sync_word != 0x47)
        {
            printf("sync word error @0x%x!\n", ftell(input_file));
            fseek(input_file, -187L, SEEK_CUR);
            continue;
        }

        // output
        if (fwrite(Tmp_data + 4, 1, TS_SIZE - 4, output_file) != TS_SIZE - 4)
        {
            printf("Write output_file error!\n");
            return -1;
        }
    }
}

int main(int argc, char **argv) 
{
    FILE *input_file, *output_file;
    
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("\t%s [input_file] [output_file]\n", argv[0]);
        return -1;
    }
    
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL)
    {
        printf("Open %s error!\n", argv[1]);
        return -1;
    }
    output_file = fopen(argv[2], "wb");
    if (output_file == NULL)
    {
        printf("Open %s error!\n", argv[2]);
        return -1;
    }

    convert_ts_packet(input_file, output_file);

    fclose(output_file);
    fclose(input_file);
    
    return 0;
}
    