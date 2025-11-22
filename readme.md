## 🧩 BMP Image Steganography – C Project

## Overview:

Developed a steganography tool in C, enabling secret data to be hidden inside 24-bit BMP images without any visible change. The solution supports encoding various file types (e.g., .txt, .pdf) and accurately restores them using dedicated decoding logic.

## Key Features:

Handles 24-bit BMP images

Hides any secret file (text, binary, etc.) within an image

Implements Least Significant Bit (LSB) data-hiding technique

Offers both encoding and decoding modules

Automatic capacity check to ensure reliability

Outputs stego.bmp with embedded data

## Project Structure:

encode.c / decode.c – Core logic

encode.h / decode.h – Module headers

common.h / types.h – Common constants and typedefs

sample/ – Example images and secret files

## How to Use:

Encoding: ./encode -e <source.bmp> <secret_file> [output_stego.bmp]
Example: ./encode -e sample/beautiful.bmp sample/secret.txt sample/stego.bmp

Decoding: ./decode -d <stego.bmp> [output_folder]
Example: ./decode -d sample/stego.bmp

## How It Works:
Each secret byte is hidden in the LSBs of 8 image bytes, making changes undetectable to the human eye.

## Dependencies:
Standard C libraries only (stdio.h, string.h, stdlib.h).

## Sample Output:
Encoding: Successful validation, encoding steps, and file generation.
Decoding: Header skipping, secret extraction, and file restoration.

Author: Varshini Yadav – varshiniyadav87@gmail.com

#CProgramming #Steganography #EmbeddedSystems #BMP #OpenSource #PortfolioProject

