#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define TMP_SIZE     1024

unsigned char Tmp_data[TMP_SIZE];


typedef struct 
{
    unsigned int syncword:12;
    unsigned int ID:1;
    unsigned int layer:2;
    unsigned int protection_bit:1;
    unsigned int bitrate_index:4;
    unsigned int sampling_frequency:2;
    unsigned int padding_bit:1;
    unsigned int private_bit:1;
    unsigned int mode:2;
    unsigned int mode_extension:2;
    unsigned int copyright:1;
    unsigned int original_home:1;
    unsigned int emphasis:2;
} FILE_HEADER;



int parse_file_header(FILE *file)
{
    FILE_HEADER *file_hdr = NULL;
    int size;

    size = fread(&Tmp_data, 1, sizeof(FILE_HEADER), file);
    if (size == 0)
    {
        printf("Read EOF @0x%x!\n", ftell(file));
        return -2;
    }

    file_hdr = (FILE_HEADER *)Tmp_data;

    // check error:
    if (file_hdr->syncword != 0x2FF)

    printf("--------------file header--------------\n");
    printf("syncword\t\t%x\n", file_hdr->syncword);
    printf("ID\t\t\t%x\n", file_hdr->ID);
    printf("layer\t\t\t%x\n", file_hdr->layer);
    printf("protection_bit\t\t%x\n", file_hdr->protection_bit);
    printf("bitrate_index\t\t%x\n", file_hdr->bitrate_index);
    printf("sampling_frequency\t%x\n", file_hdr->sampling_frequency);
    printf("padding_bit\t\t%x\n", file_hdr->padding_bit);
    printf("private_bit\t\t%x\n", file_hdr->private_bit);
    printf("mode\t\t\t%x\n", file_hdr->mode);
    printf("mode_extension\t\t%x\n", file_hdr->mode_extension);
    printf("copyright\t\t%x\n", file_hdr->copyright);
    printf("original_home\t\t%x\n", file_hdr->original_home);
    printf("emphasis\t\t%x\n", file_hdr->emphasis);
    printf("\n");
}


int main(int argc, char const *argv[])
{
    FILE *file;

    if (argc < 2)
    {
        printf("Usage: %s [file_name]\n", argv[0]);
        return -1;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf("Open %s error!\n", argv[1]);
        return -1;
    }

    parse_file_header(file);
    parse_xing(file);

    
    fclose(file);


    return 0;
}