#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char *output_fname;
    FILE *fptr_output;

    char extn_secret_file[10];  // extended buffer
    int extn_size;
    long size_secret_file;

} DecodeInfo;

Status do_decoding(DecodeInfo *decInfo);
char decode_byte_from_lsb(char *image_buffer);
long decode_size_from_lsb(char *image_buffer);
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
