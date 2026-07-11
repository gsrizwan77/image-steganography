#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"
#include "types.h"

char decode_byte_from_lsb(char *image_buffer)
{
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
        ch = (ch << 1) | (image_buffer[i] & 1);
    return ch;
}

long decode_size_from_lsb(char *image_buffer)
{
    long size = 0;
    for (int i = 0; i < 32; i++)
        size = (size << 1) | (image_buffer[i] & 1);
    return size;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        char ch = decode_byte_from_lsb(buffer);
        if (ch != magic_string[i])
        {
            printf("ERROR: Magic string mismatch. Not a stego image.\n");
            return e_failure;
        }
    }
    printf("Magic string matched\n");
    printf("Magic string decoded successfully\n");
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->extn_size = (int)decode_size_from_lsb(buffer);
    printf("Decoded the extension and size is successful\n");
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decInfo->size_secret_file = decode_size_from_lsb(buffer);
    printf("Decoded secret file size successfully\n");
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        char ch = decode_byte_from_lsb(buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }
    printf("Secret data decoded successfully\n");
    printf("Decoded the secret file data and size is successful\n");
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("-----Started decoding-----\n");

    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (!decInfo->fptr_stego_image)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }
    printf("Open file is success\n");

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure) return e_failure;
    decode_secret_file_extn_size(decInfo);
    decode_secret_file_extn(decInfo);
    decode_secret_file_size(decInfo);

    static char outname[50];
    if (!decInfo->output_fname)
    {
        sprintf(outname, "output%s", decInfo->extn_secret_file);
        decInfo->output_fname = outname;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (!decInfo->fptr_output)
    {
        printf("ERROR: Unable to create output file\n");
        return e_failure;
    }
    printf("Secret file opened successfully\n");

    decode_secret_file_data(decInfo);

    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);

    printf("Decoding successful\n");
    return e_success;
}
