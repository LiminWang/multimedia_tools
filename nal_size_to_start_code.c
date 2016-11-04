#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define FRAME_SIZE  5*1024*1024

static unsigned char Tmp_data[FRAME_SIZE];

#define OUT_FILE_NAME   "output.es"

int main(int argc, char **argv) 
{
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    char input_file_name[256] = {0};
    unsigned int size = 0;
    int i = 0;
    
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("\t%s [frame_file_prefix] [extra_file_name]\n", argv[0]);
        return -1;
    }
    
    output_file = fopen(OUT_FILE_NAME, "wb");
    if (output_file == NULL)
    {
        printf("Open %s error!\n", argv[2]);
        return -1;
    } 

    // extra header
    {
        input_file = fopen(argv[2], "rb");
        if (input_file == NULL)
        {
            printf("Open %s error! please check file name.\n", argv[2]);
            goto err;
        }

        Tmp_data[0] = 0;
        Tmp_data[1] = 0;
        Tmp_data[2] = 0;
        Tmp_data[3] = 1;

        if (fwrite(Tmp_data, 1, 4, output_file) != 4)
        {
            printf("Write error @%d!\n", __LINE__);
            fclose(input_file);
            goto err;
        }

        fseek(input_file, 0, SEEK_END);
        size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);

        if (fread(Tmp_data, 1, size, input_file) < size)
        {
            printf("Read error @%d!\n", __LINE__);
            fclose(input_file);
            goto err;
        }

        if (fwrite(Tmp_data, 1, size, output_file) != size)
        {
            printf("Write error @%d!\n", __LINE__);
            fclose(input_file);
            goto err;
        }

        fclose(input_file);
    }

    // frame
    while (1)
    {
        if (sprintf(input_file_name, argv[1], i) < 0)
        {
            printf("frame_file_prefix is error! please check frame_file_prefix.\n");
            break;
        }

        input_file = fopen(input_file_name, "rb");
        if (input_file == NULL)
        {
            if (i == 0)
            {
                printf("Open %s error! please check file name.\n", input_file_name);
            }
            else
            {
                printf("Done. Cur num = %d\n", i);
            }
            break;
        }

        fseek(input_file, 0, SEEK_END);
        size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);

        if (fread(Tmp_data, 1, size, input_file) < size)
        {
            printf("Read error @%d!\n", __LINE__);
            fclose(input_file);
            break;
        }
        
        Tmp_data[0] = 0;
        Tmp_data[1] = 0;
        Tmp_data[2] = 0;
        Tmp_data[3] = 1;

        if (fwrite(Tmp_data, 1, size, output_file) != size)
        {
            printf("Write error @%d!\n", __LINE__);
            fclose(input_file);
            break;
        }

        fclose(input_file);
        i++;
    }

err:
    fclose(output_file);
    
    return 0;
}
    