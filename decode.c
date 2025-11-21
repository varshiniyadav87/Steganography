#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Read and validate decoding arguments
 * Input: argc, argv, DecodeInfo structure
 * Output: Fills DecodeInfo with valid file names
 * Description:
 * Validates the stego image file, checks its extension (.bmp),
 * and sets the output secret file name (default or user-provided).
 */
Status read_and_validate_decode_args(int argc,char *argv[] , DecodeInfo *decInfo)
{
    // Validate argument count
    if (argc != 3 && argc != 4)  
    {
        printf("ERROR: Usage: ./decode <stego.bmp> [output_file]\n");
        return e_failure;
    }

    // Validate stego image file (must end with .bmp)
    int len = strlen(argv[2]);
    if (len < 3 || strcmp(argv[2] + len - 4,".bmp") != 0) 
    {
        printf("ERROR: Stego image must be .bmp\n");
        return e_failure;
    }

    //  Copy filename safely into the fixed-size array 
    strcpy(decInfo->stego_image_fname,argv[2]);

    // Set output file name 
    if(argc == 4) 
    {
        //  Copy user-provided output filename safely 
        strcpy(decInfo->secret_fname, argv[3]);
    }
    else 
    {
        // Default file name
        strcpy(decInfo->secret_fname, "output_stego");
    }

    return e_success ;
}

/* Perform decoding process
 * Input: DecodeInfo structure
 * Output: Returns e_success on success, else e_failure
 * Description:
 * Skips the BMP header, validates the magic string,
 * and sequentially decodes the secret file details and data.
 */
Status do_decoding(DecodeInfo *decInfo)
{
printf("INFO: Opening stego image: %s\n", decInfo->stego_image_fname);

    if (skip_bmp_header(decInfo) != e_success)
    {
        printf("ERROR: Failed to skip BMP header\n");
        return e_failure;
    }

    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success)
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }
    printf("INFO: Magic string verified\n");

    if (decode_secret_file_extn_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension file size\n");
        return e_failure;
    }

    if (decode_secret_file_extn(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension\n");
        return e_failure;
    }

    if (decode_secret_file_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret file size\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret data\n");
        return e_failure;
    }

    printf("SUCCESS: Decoding completed! Saved as '%s'\n", decInfo->secret_fname);
    return e_success;
}

/* Skip BMP header
 * Input: DecodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Opens the stego image and moves the file pointer past
 * the 54-byte BMP header to the pixel data region.
 */
Status skip_bmp_header(DecodeInfo *decInfo)
{
    // Open the stego BMP image in binary read mode ("rb")
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");

    // Check if the file was successfully opened
    if (!decInfo->fptr_stego_image)
    {
        // Print error message 
        perror("ERROR: fopen stego image");
        return e_failure;
    }

    // Move the file pointer 54 bytes from the start (SKIP BMP HEADER)
    if (fseek(decInfo->fptr_stego_image, 54L, SEEK_SET) != 0)
    {
        printf("ERROR: Failed to seek past BMP header\n");
        return e_failure;
    }

    printf("INFO: Skipped 54-byte BMP header\n");
    return e_success;
}

/* Decode magic string
 * Input: Expected magic string and DecodeInfo structure
 * Output: Returns e_success if matches, else e_failure
 * Description:
 * Decodes the embedded magic string from the stego image
 * and validates it against the expected string.
 */
Status decode_magic_string(char *magic_string,DecodeInfo *decInfo)
{
    // Get the length of the expected magic string
    int len = strlen(magic_string);

    // Create a buffer to store the decoded magic string.
    char buffer[16] = {0};

    // Extract one byte from every 8 bytes of image data.
    if (decode_data_from_image(buffer, len, decInfo->fptr_stego_image) != e_success)
        return e_failure;

    // Compare the decoded magic string with the original magic string
    if (strcmp(buffer, magic_string) != 0)
    {
        printf("ERROR: Expected magic '%s', got '%s'\n", magic_string, buffer);
        return e_failure;
    }
    return e_success;
}

/* Decode secret file extension size
 * Input: DecodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Reads 32 bytes from the image and extracts the
 * size of the secret file extension from their LSBs.
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    /* Buffer to hold 32 bytes of image data */
    unsigned char buffer[32];

    /* Read 32 bytes */
    if(fread(buffer,1,32,decInfo->fptr_stego_image) != 32)
        return e_failure;

    /* Decode size of file extension */
    if (decode_size_from_lsb(&decInfo->size_secret_file_extn, buffer) != e_success)
        return e_failure;

    printf("DEBUG: Decoded extension size = %ld\n", decInfo->size_secret_file_extn);
    if (decInfo->size_secret_file_extn <= 0 || decInfo->size_secret_file_extn >= MAX_FILE_SUFFIX_) {
        printf("ERROR: invalid decoded extension size %ld\n", decInfo->size_secret_file_extn);
        return e_failure;
    }

    return e_success;
}

/* Decode secret file extension
 * Input: DecodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Decodes the secret file extension from the stego image,
 * creates the output file, and prepares for writing data.
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    /* Clear extension buffer to avoid garbage */
    memset(decInfo->extn_secret_file, 0, sizeof(decInfo->extn_secret_file));

    /* Decode the secret file extension from image */
    if (decode_data_from_image(decInfo->extn_secret_file, (int)decInfo->size_secret_file_extn, decInfo->fptr_stego_image) != e_success) 
    {
        printf("Error: failed to decode secret file extension\n");
        return e_failure;
    }

    /* Null-terminate extension */
    decInfo->extn_secret_file[decInfo->size_secret_file_extn] = '\0';

    /* Append extension to output file name safely */
    strcat(decInfo->secret_fname, decInfo->extn_secret_file);
    printf("INFO: Output file = '%s'\n", decInfo->secret_fname);

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if (!decInfo->fptr_secret)
    {
        perror("ERROR: fopen output file");
        return e_failure;
    }

    return e_success;
}

/* Decode secret file size
 * Input: DecodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Reads 32 bytes from the stego image and decodes
 * the size of the secret file from their LSBs.
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    // Buffer to hold 32 bytes of image data
    unsigned char buffer[32];

    // Read 32 bytes
    if(fread(buffer,1,32,decInfo->fptr_stego_image) != 32) 
        return e_failure;

    // Decode size
    if(decode_size_from_lsb(&decInfo->size_secret_file,buffer) != e_success)
        return e_failure;

    printf("INFO: Secret file size = %ld bytes\n", decInfo->size_secret_file);
    if (decInfo->size_secret_file < 0)
        return e_failure;

    return e_success;
}

/* Decode secret file data
 * Input: DecodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Reads and decodes the secret data from the image
 * and writes it to the reconstructed output file.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    if (decInfo->size_secret_file <= 0)
        return e_failure;
        
    /* Buffer to hold 8 bytes of image data */
    unsigned char buffer[8];

    /* To hold one decoded character */
    char ch;

    if (decInfo->size_secret_file <= 0) 
        return e_failure;

    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        // Read 8 bytes from stego image
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        // Decode one byte from LSBs
        if (decode_byte_from_lsb(&ch, (char *)buffer) != e_success)
            return e_failure;

        // Write decoded byte to secret file
        if (fwrite(&ch, 1, 1, decInfo->fptr_secret) != 1)
            return e_failure;
    }

    return e_success;

    
}

/* Decode data from image
 * Input: Output buffer, data size, and stego image file pointer
 * Output: Returns e_success or e_failure
 * Description:
 * Extracts each byte of data from 8 bytes of image pixels
 * by reading their least significant bits.
 */
Status decode_data_from_image(char *output, int size, FILE *fptr_src_image)
{
    unsigned char img_buffer[8];
    char ch;

    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the image
        if (fread(img_buffer, 1, 8, fptr_src_image) != 8)
        {
            printf("Error: Not enough image data to decode byte %d\n", i);
            return e_failure;
        }

        // Decode one byte
        if (decode_byte_from_lsb(&ch, img_buffer) != e_success)
        {
            printf("Error: Failed to decode byte %d\n", i);
            return e_failure;
        }

        // Store decoded byte
        output[i] = ch;
    }

    output[size] = '\0';  // Null terminate just in case (for strings like file extensions)
    return e_success;
}

/* Decode byte from LSBs
 * Input: Output byte pointer and image buffer
 * Output: Returns e_success or e_failure
 * Description:
 * Extracts 8 bits from 8 bytes' LSBs and reconstructs a character.
 */
Status decode_byte_from_lsb(char *data, unsigned char *image_buffer)
{
    // Initialize to zero
    *data = 0;

    // Decode each bit
    for (int i = 0; i < 8; i++)
        *data |= ((image_buffer[i] & 1) << (7 - i));

    return e_success;
}

/* Decode size from LSBs
 * Input: Pointer to store decoded integer, and buffer
 * Output: Returns e_success or e_failure
 * Description:
 * Extracts a 32-bit integer from the least significant bits of 32 bytes.
 */
Status decode_size_from_lsb(long int *data,unsigned char *buffer)
{
    // Initialize to zero
    *data = 0 ;

    // Decode each bit
    for(int i = 0 ; i < 32 ; i++)
        *data = (*data) | ((buffer[i] & 1) << (31 - i));

    return e_success;
}