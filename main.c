#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *arg)
{
    if (strcmp(arg, "-e") == 0)
        return e_encode;
    else if (strcmp(arg, "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("  Encode: %s -e <src.bmp> <secret.txt> <stego.bmp>\n", argv[0]);
        printf("  Decode: %s -d <stego.bmp> <output.txt>\n", argv[0]);
        return 1;
    }

    OperationType op = check_operation_type(argv[1]);

    if (op == e_encode)
    {
        EncodeInfo encInfo;
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Invalid encoding arguments\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_success)
            printf("INFO: Encoding completed successfully. Stego file: %s\n", encInfo.stego_image_fname);
        else
            fprintf(stderr, "ERROR: Encoding failed\n");
    }
    else if (op == e_decode)
    {
        DecodeInfo decInfo;
        decInfo.stego_image_fname = argv[2];
        if (argc >= 4)
            decInfo.output_fname = argv[3];
        else
            decInfo.output_fname = "output.txt";

        if (do_decoding(&decInfo) == e_success)
            printf("INFO: Decoding completed successfully. Output file: %s\n", decInfo.output_fname);
        else
            fprintf(stderr, "ERROR: Decoding failed\n");
    }
    else
    {
        fprintf(stderr, "ERROR: Unsupported operation. Use -e or -d\n");
        return 1;
    }

    return 0;
}
