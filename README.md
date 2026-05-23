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
