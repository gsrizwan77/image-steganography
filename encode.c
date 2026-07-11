#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Get image size in bytes */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    printf("width = %u\n", width);
    printf("height = %u\n", height);

    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    printf("Read and validate argument is a success\n");

    if (strstr(argv[2], ".bmp"))
        encInfo->src_image_fname = argv[2];
    else
        return e_failure;

    if (argv[3])
        encInfo->secret_fname = argv[3];
    else
        return e_failure;

    if (argv[4] != NULL && strstr(argv[4], ".bmp"))
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "default.bmp";

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image) { printf("ERROR: Opening source image failed\n"); return e_failure; }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (!encInfo->fptr_secret) { printf("ERROR: Opening secret file failed\n"); return e_failure; }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (!encInfo->fptr_stego_image) { printf("ERROR: Creating stego image failed\n"); return e_failure; }

    printf("Open file is success\n");
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    rewind(encInfo->fptr_src_image);

    long available_capacity = encInfo->image_capacity / 8;  // total bytes we can hide
    long required_capacity  = encInfo->size_secret_file;    // bytes needed for secret

    printf("Available capacity in image : %ld bytes\n", available_capacity);
    printf("Secret file size            : %ld bytes\n", required_capacity);

    if (available_capacity > required_capacity + 100) // +100 for overhead (magic string, extn, size)
    {
        printf("Check capacity is success\n");
        return e_success;
    }

    printf("ERROR: Image does not have enough capacity to store secret file\n");
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char header[54];
    fread(header, 54, 1, fptr_src_image);
    fwrite(header, 54, 1, fptr_dest_image);
    printf("Copied bmp header successfully\n");
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] &= 0xFE;
        image_buffer[i] |= ((data >> (7 - i)) & 1);
    }
    return e_success;
}

Status encode_size_to_lsb(long size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] &= 0xFE;
        imageBuffer[i] |= ((size >> (31 - i)) & 1);
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    printf("OFFSET at %ld before encode magic string\n", ftell(encInfo->fptr_src_image));
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    printf("Encode magic string successfully\n");
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    printf("OFFSET at %ld before encode extension size\n", ftell(encInfo->fptr_src_image));
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    printf("Encoded secret file extn size successfully\n");
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    printf("OFFSET at %ld before encode file extension\n", ftell(encInfo->fptr_src_image));
    for (int i = 0; i < strlen(file_extn); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    printf("Encoded secret file extn is successfully\n");
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    printf("OFFSET at %ld before encode file size\n", ftell(encInfo->fptr_src_image));
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    printf("Encoded secret file size successfully\n");
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char ch;
    printf("OFFSET at %ld before encode file data\n", ftell(encInfo->fptr_src_image));
    rewind(encInfo->fptr_secret);

    while (fread(&ch, 1, 1, encInfo->fptr_secret))
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(ch, buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    printf("Encoded the secret file data successfully\n");
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    printf("OFFSET at %ld before encode copy remaining data\n", ftell(fptr_src));
    char ch;
    while (fread(&ch, 1, 1, fptr_src))
        fwrite(&ch, 1, 1, fptr_dest);
    printf("Copied remaining data successfully\n");
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("-----Started encoding-----\n");

    if (open_files(encInfo) == e_failure) return e_failure;
    if (check_capacity(encInfo) == e_failure) return e_failure;
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) return e_failure;
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure) return e_failure;

    char *ext = strstr(encInfo->secret_fname, ".");
    strcpy(encInfo->extn_secret_file, ext);

    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure) return e_failure;
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure) return e_failure;
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure) return e_failure;
    if (encode_secret_file_data(encInfo) == e_failure) return e_failure;
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) return e_failure;

    printf("Encoding successful\n");
    return e_success;
}
