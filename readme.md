
🧩 BMP Image Steganography (C Project)
📜 Overview

This project implements image steganography in C — the art of hiding secret data inside an image without visible changes.
It allows you to encode a secret file (like .txt, .pdf, etc.) into a .bmp image, and later decode it back.

🧠 Features

🖼️ Supports 24-bit BMP images.

🔒 Hides any type of secret file inside the image.

🧬 Uses Least Significant Bit (LSB) technique for data hiding.

✅ Includes both encoding and decoding modules.

⚙️ Performs capacity checks to ensure the image can hold the secret data.

💾 Generates a stego.bmp image with embedded data.

├── encode.c             # Contains encoding logic
├── decode.c             # Contains decoding logic
├── encode.h             # Header file for encode module
├── decode.h             # Header file for decode module
├── common.h             # Common definitions and constants (e.g., MAGIC_STRING)
├── types.h              # Custom typedefs (e.g., uint, Status)
├── Makefile             # Build automation script
├── README.md            # Project documentation (this file)
└── sample/
    ├── beautiful.bmp    # Example source image
    ├── secret.txt       # Example secret file
    └── stego.bmp        # Output after encoding

🚀 Usage
🧩 Encoding (Hiding data)
./encode -e <source.bmp> <secret_file> [output_stego.bmp]


Example:

./encode -e sample/beautiful.bmp sample/secret.txt sample/stego.bmp

If the output filename is not provided, it defaults to stego.bmp.

Steps performed:

Opens the source BMP and secret file.

Checks if image has enough capacity.

Copies the BMP header.

Encodes:

Magic string

File extension

File size

Secret file data

Writes encoded data into the new stego image.

🔓 Decoding (Extracting data)

./decode -d <stego.bmp> [output_folder]


Example:

./decode -d sample/stego.bmp


The program will:

Open and skip the BMP header.

Decode the magic string to verify validity.

Decode file extension and size.

Extract and reconstruct the secret file.

Output will be saved as:

decoded_secret.txt

🧮 How It Works (LSB Encoding)

Each byte of the secret data is hidden inside the least significant bits (LSBs) of 8 pixels.

Example:
If secret byte = 01000001 (ASCII 'A'),
then 8 image bytes’ LSBs are replaced with those bits — a change so tiny that the human eye cannot detect it.

🧰 Dependencies

No external libraries needed — uses standard C libraries:

stdio.h

string.h

stdlib.h

🧪 Example Output

Encoding:

Files opened successfully
width = 800
height = 600
INFO: Capacity OK
Copying BMP header
Encoding magic string
Encoding secret file extension size
Encoding secret file extension
Encoding secret file size
Encoding secret file data
Copying remaining image data
Encoding completed successfully!


Decoding:

INFO: Skipped 54-byte BMP header
INFO: Decoded magic string successfully
INFO: Decoded secret file extension (.txt)
INFO: Decoded secret file size (1024 bytes)
INFO: Secret file decoded and saved successfully!

🧑‍💻 Author - Varshini Yadav [varshiniyadav87@gmail.com]
