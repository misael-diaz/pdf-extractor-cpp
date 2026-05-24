# pdf-extractor-cpp
extracts text from a pdf by leveraging libpoppler

## Drive 

The drive here is to write exploratory code to experiment with libpopple to extract data from medical records. The challenge is that the records come from various institutions and
each institution has more than one format that they use.

## Requirements

Even if your Linux distribution uses `poppler` you still need to install the development files, and you can do so from your terminal session:

```sh
apt install libpoppler-cpp-dev
```

this will install the library and the header files needed to build the application.

## Build

```sh
g++ -std=gnu++11 -I/usr/include/poppler/cpp/ -Wall -Wformat -O0 -g main.cpp -o parser.bin -lpoppler-cpp
```

## Challenges

- **data extraction**: deciding what tool to use to extract the data from the PDF is challenging because there are many alternatives. Chose poppler because it has been battle tested and it require minimal code to work at the byte level.
- **data cleanup**: doing ascii folding simplifies the parsing of the document and also helps with cleaning up data that could confuse the pdf extraction tool
- **timestamps formats**: the sample documents show that timestamps are formatted differently among medical records. This needs to be addressed.

## Features

- **zero copy**: we use a memory map to store the extracted data for performance reasons. This means that the Linux Kernel is giving us fast access to memory without incurring on copies from user to kernel space.
- **lean data**: we need only 7-bits to represent ASCII characters and this means that if there are size limitations (as mentioned in the challenge) by doing this data conversion we are getting rid of the bloat. The document size is reduced to as many bytes as text characters in the document.
- **filters out watermark**: at the byte level the watermarks are easy to identify, filtering out the watermark was achieved by writing simple conditionals. These were applied after doing the ascii folding transformation
