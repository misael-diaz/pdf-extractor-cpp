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
