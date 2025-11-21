#ifndef DECODE_H
#define DECODE_H

/* Contains user defined types */
#include "types.h" 

/* Maximum length for file extension */
#define MAX_FILE_SUFFIX_ 50


/* Structure to hold all data required for decoding process */

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char stego_image_fname[1000];
    FILE *fptr_stego_image;

    /* Secret File Info */
    char secret_fname[1000];
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX_];
    long size_secret_file_extn;
    long size_secret_file;
} DecodeInfo;


/* Decoding function prototype */

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);

/* Perform the Decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Skip bmp image header */
Status skip_bmp_header(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(char * ,DecodeInfo *decInfo);

/*Decode secret file extenstion size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode function, which does the real Decoding */
Status decode_data_from_image(char *data, int size, FILE *fptr_src_image);

/* Decode a byte from LSB of image data array */
Status decode_byte_from_lsb(char *data, unsigned char *image_buffer);

/* Decode an integer size value from LSB of image data array */
Status decode_size_from_lsb(long int *data,unsigned char *buffer);

#endif

