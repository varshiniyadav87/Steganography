#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    printf("\033[1;36m🖥️  Image dimensions: %u x %u pixels.\033[0m\n", width, height);
    
    // Return image capacity
    return width * height * 3;
}

/* Get file size
 * Input: File pointer fptr
 * Output: Size of the file in bytes
 * Description: 
 * Moves the file pointer to the end to determine the file size using ftell(),
 * then resets the pointer back to the beginning of the file.
 */
uint get_file_size(FILE *fptr)
{
    // Move the file pointer to the end of the file
    fseek(fptr,0,SEEK_END);

    // Get the current position of the file pointer
    long size = ftell(fptr);

    // Reset the file pointer back to the beginning of the file
    fseek(fptr , 0 , SEEK_SET);

    // Return the file size
    return size ;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    printf("\033[1;36m📁 Files opened — preparing environment for data embedding.\033[0m\n");

    // No failure return e_success
    return e_success;
}

/* Read and validate encoding arguments
 * Input: argc, argv, EncodeInfo structure
 * Output: Fills EncodeInfo with valid file names
 * Description: 
 * Checks argument count, validates .bmp files, extracts secret file extension,
 * and sets default output name if not given.
 */
Status read_and_validate_encode_args(int argc,char *argv[] , EncodeInfo *encInfo)
{
    // Check argument count (should be 4 or 6)
    if (argc != 4 && argc != 6)  
        return e_failure;

    // Validate source image file (must end with .bmp)
    int len = strlen(argv[2]) ;
    if (len < 4 || strcmp(argv[2] + len - 4,".bmp") != 0) 
        return e_failure;
    else 
        encInfo->src_image_fname = argv[2];

    // Validate and store secret file details
    char *dot = strchr(argv[3], '.') ;
    if (dot != NULL)
    {
        strcpy(encInfo->extn_secret_file , dot);
        encInfo->secret_fname = argv[3];
    }
    else 
        return e_failure;

    // Set stego image file name (default or user-provided)
    if (argc == 6)
    {
        len = strlen(argv[4]);
        if (len < 4 || strcmp(argv[4] + len - 4, ".bmp") != 0)
            return e_failure;
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    printf("\033[1;36m✅ Validation Passed: All inputs are verified!\033[0m\n");
    return e_success ;
}

/* Check image capacity
 * Input: EncodeInfo structure
 * Output: Returns e_success if image can hold secret data, else e_failure
 * Description:
 * Calculates image capacity and compares it with the total size needed
 * to store the magic string, file extension, file size, and secret data.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if (encInfo->image_capacity < ((strlen(MAGIC_STRING) + 4 + strlen(encInfo->extn_secret_file) + 4 + encInfo->size_secret_file) * 8) + 64)
    {
        printf("\033[1;36m❌ ERROR: Not enough space available!\033[0m\n");
        return e_failure;
    }
    printf("\033[1;36m📊 Capacity Check: Image has sufficient room for the secret payload.\033[0m\n");
    return e_success;
}

/* Perform encoding process
 * Input: EncodeInfo structure
 * Output: Returns e_success on successful encoding, else e_failure
 * Description:
 * Opens files, checks capacity, and performs encoding steps:
 * copying BMP header, embedding magic string, file details, and secret data
 * into the output (stego) image.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    // check if all required files are opened successfully 
    if (open_files(encInfo) != e_success)
        return e_failure;

    // Check if image has enough capacity to hold secret data
    if (check_capacity(encInfo) != e_success)
        return e_failure;

    // Copy BMP header
    printf("\033[1;36m📄 Header copied successfully — canvas ready for steganography.\033[0m\n");    
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;

    // Encode magic string
    printf("\033[1;36m✨ Embedding magic signature to mark presence of hidden data.\033[0m\n");   
    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
        return e_failure;

    // Encode secret file extension size
    printf("\033[1;36m🗂️  Storing secret file extension and size metadata.\033[0m\n");    
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) != e_success)
        return e_failure;

    // Encode secret file extension
    printf("\033[1;36m📑 Writing secret file extension (.txt / .pdf / custom).\033[0m\n");
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
        return e_failure;

    // Encode secret file size
    printf("\033[1;36m📦 Capturing and recording exact secret file size.\033[0m\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
        return e_failure;

    // Encode secret file data
    printf("\033[1;36m🔒 Encoding secret data into pixel bytes, bit by bit.\033[0m\n");
    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;

    // Copy remaining image data
    printf("\033[1;36m📤 Appending untouched image bytes to maintain visual integrity.\033[0m\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;

    printf("\033[1;36m🏆 Steganography successful — hidden data embedded securely.\033[0m\n");

    printf("\033[1;36m🚀 Encoding process completed — your mission is accomplished!\033[0m\n");
    return e_success;
}

/* Encode data into image
 * Input: Data to encode, size of data, source and stego image file pointers
 * Output: Writes encoded data into stego image
 * Description:
 * Reads 8 bytes from the source image and embeds one data byte into their LSBs,
 * repeating for each byte of the input data.
 */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    // Buffer to hold 8 bytes of image data
    unsigned char buffer[8];
    for(int i = 0 ; i < size ; i++)
    {
        // Read 8 bytes from source image
        if(fread(buffer,1,8,fptr_src_image) != 8)
            return e_failure;           

        // Encode one byte of data into those 8 bytes
        if (encode_byte_to_lsb(data[i], buffer) != e_success)
            return e_failure;        

        // Write modified bytes to stego image
        if(fwrite(buffer,1,8,fptr_stego_image) != 8)
            return e_failure;       
    }
    return e_success;
}

/* Encode size to LSBs
 * Input: Integer data, buffer to store encoded bits
 * Output: Returns e_success if successful, else e_failure
 * Description:
 * Encodes a 32-bit integer into the least significant bits of 32 bytes.
 */
Status encode_size_to_lsb(int data, char *buffer)
{
    // Encode each bit of 'data' into the LSB of 32 bytes
    for(int i = 0 ; i < 32 ; i++)
        buffer[i] = (buffer[i] & ~1) | ((data >> (31 - i)) & 1);

    return e_success;
}

/* Encode byte to LSBs
 * Input: One byte of data, image buffer of 8 bytes
 * Output: Returns e_success if successful, else e_failure
 * Description:
 * Encodes 8 bits of a data byte into the LSBs of 8 image bytes.
 */
Status encode_byte_to_lsb(int data, char *image_buffer)
{
    // Encode each bit of 'data' into the LSB of 8 bytes
    for(char i = 0 ; i < 8 ; i++)
        image_buffer[i] = (image_buffer[i] & ~1) | ((data >> (7 - i)) & 1);

    return e_success;
}

/* Copy BMP header
 * Input: Source and destination image file pointers
 * Output: Returns e_success after copying the header
 * Description:
 * Reads the 64-byte BMP header from the source image and writes it
 * unchanged to the destination (stego) image.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Buffer to store BMP header
    char buffer[64];

    // Move to start of source image
    if (fseek(fptr_src_image, 0, SEEK_SET) != 0)
        return e_failure;

    // Read 54-byte BMP header from source
    if(fread(buffer,1,54,fptr_src_image) != 54)
        return e_failure;
    
    // Write header to destination image
    if(fwrite(buffer,1,54,fptr_dest_image) != 54)
        return e_failure;

    return e_success ;
}

/* Encode magic string
 * Input: Magic string and EncodeInfo structure
 * Output: Returns e_success after encoding, else e_failure
 * Description:
 * Encodes the magic string into the image using LSB steganography
 * to mark the start of hidden data in the stego image.
 * Validates inputs before encoding.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Encode the magic string into image data
    if(encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;
    return e_success;
}

/* Encode secret file extension size
 * Input: File extension size and EncodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Encodes the size of the secret file extension into the LSBs
 * of 32 bytes from the source image and writes them to the stego image.
 */
Status encode_secret_file_extn_size(long file_size, EncodeInfo *encInfo)
{
    // Buffer to store 32 bytes from source image
    char buffer[32];
    if(fread(buffer,1,32,encInfo->fptr_src_image) != 32)
        return e_failure;
    
    // Encode file size into 32 bytes
    if(encode_size_to_lsb(file_size,buffer) != e_success)
        return e_failure;
    
    // Write encoded data to stego image
    if(fwrite(buffer,1,32,encInfo->fptr_stego_image) != 32)
        return e_failure;

    return e_success;
}

/* Encode secret file extension
 * Input: File extension string and EncodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Encodes the secret file extension string into the image
 * using least significant bit (LSB) encoding.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // Encode the file extension into image data
    if(encode_data_to_image(file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image) != e_success)
        return e_failure;

    return e_success;
}

/* Encode secret file size
 * Input: Size of the secret file and EncodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Encodes the size of the secret file into the LSBs of 32 bytes
 * from the source image and writes them to the stego image.
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // Buffer to hold 32 bytes of image data
    unsigned char buffer[32];

    // Read 32 bytes from source image
    if(fread(buffer,1,32,encInfo->fptr_src_image) != 32)
        return e_failure;
    
    // Encode file size into LSBs
    if(encode_size_to_lsb(file_size,buffer) != e_success)
        return e_failure;

    // Write encoded data to stego image
    if(fwrite(buffer,1,32,encInfo->fptr_stego_image) != 32)
        return e_failure; 

    return e_success;
}

/* Encode secret file data
 * Input: EncodeInfo structure
 * Output: Returns e_success or e_failure
 * Description:
 * Reads each byte from the secret file, encodes it into the LSBs
 * of 8 bytes from the source image, and writes to the stego image.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Reset secret file pointer to beginning
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    
    // Buffer to hold 8 bytes from image
    unsigned char buffer[8];

    // Holds one byte from secret file
    char data_byte;
    
    // Loop through each byte of the secret file
    for(long i = 0; i < encInfo->size_secret_file; i++)
    {
        // Read one byte from secret file
        if(fread(&data_byte, 1, 1, encInfo->fptr_secret) != 1)
            return e_failure;
        
        // Read 8 bytes from source image
        if(fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;
        
        // Encode the secret byte into LSBs of the image buffer
        if (encode_byte_to_lsb(data_byte, (char *)buffer) != e_success)
            return e_failure;
        
        // Write to stego image
        if(fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }
    
    return e_success;
}

/* Copy remaining image data
 * Input: Source and destination image file pointers
 * Output: Returns e_success or e_failure
 * Description:
 * Copies the remaining pixel data from the source image
 * to the stego image after encoding is complete.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // Check cuurent position
    if (ftell(fptr_dest) == -1) return e_failure;

    // Copy remaining data
    char buffer[4096];
    int n;
    while ((n = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, 1, n, fptr_dest) != n)
            return e_failure;
    }
    return e_success;
}
