/* 
    Name            : Varshini
    Date            : 14/11/2025
    Description     : This project implements image steganography in C — the art of hiding secret data inside an image without visible changes.
                      It allows you to encode a secret file (like .txt, .pdf, etc.) into a .bmp image, and later decode it back.
*/


#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

int main(int argc , char *argv[])
{
    // Structure to hold encoding related info
    EncodeInfo encInfo;     

    // Structure to hold decoding related info
    DecodeInfo decInfo;     

    // Check operation type from argv
    OperationType op_type = check_operation_type(argv);
    if (op_type == e_encode)
    {
        if (read_and_validate_encode_args(argc,argv,&encInfo) == e_success)
        {
            do_encoding(&encInfo);  // Perform encoding

            // Close all opened files after encoding
            fclose(encInfo.fptr_src_image);
            fclose(encInfo.fptr_secret);
            fclose(encInfo.fptr_stego_image);
        }
        else
        {
            printf("Validation unsuccessful\n");
            return 0;
        }
    }
    else if(op_type == e_decode)
    {
        if (read_and_validate_decode_args(argc,argv,&decInfo) == e_success)
        {
            do_decoding(&decInfo);  // Perform decoding

            // Close all opened files after encoding
            fclose(decInfo.fptr_stego_image);
            fclose(decInfo.fptr_secret);
        }
        else
        {
            printf("Validation unsuccessful\n");
            return 0;
        }
    }
    else 
    {
        printf("Unsupported\n");
    }
    return 0;
}

// Determines the operation type based on command line argument
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e") == 0) 
        return e_encode;        // Encode operation selected
    else if(strcmp(argv[1],"-d") == 0) 
        return e_decode;        // Decode operation selected
    else
        return e_unsupported;   // Unsupported operation
}