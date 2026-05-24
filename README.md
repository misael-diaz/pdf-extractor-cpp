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
- **multiline spanning data**: example some names are broken down into at least one line (tabulated medical record version). Writing the logic to extract the name without failure at seemed looked as if the code needed to use regexs but it turned out that simple logic was all that was needed to tackle this issue.
- **extract physician name**: conceptually extracting the physician name is no harder than extracting the patient data or anything else in the document. This is how the code looks prior to the pitch workshop. Stopping code writing at this point.

## Features

- **zero copy**: we use a memory map to store the extracted data for performance reasons. This means that the Linux Kernel is giving us fast access to memory without incurring on copies from user to kernel space.
- **lean data**: we need only 7-bits to represent ASCII characters and this means that if there are size limitations (as mentioned in the challenge) by doing this data conversion we are getting rid of the bloat. The document size is reduced to as many bytes as text characters in the document.
- **filters out watermark**: at the byte level the watermarks are easy to identify, filtering out the watermark was achieved by writing simple conditionals. These were applied after doing the ascii folding transformation
- **extracts names from a multiline text**: extracts the full patient name even if it spans multiple line (in the tabulated Sura records).
- **extracts physician name**: gets the name of the phsysician from one of the cases.
- **zero dependencies**: poppler is easy to install and frankly should not even be considered a dependency. Unlike Python or other interpreted or jit compiled languages the dependencies always a problem. Projects need maintenance as the language or the ecosystem evolves thereby increasing the burden of the developers. This is a build once application. All that you need the GNU Compiler Collection (GCC) to build it.
- **high performance**: This code can run in a Raspberry Pi if you want, it's a low level code that requires little resources. Pretty much the memory to load the pdf into memory.

## Advantages

You don't need a lot of compute to get a reliable and performant application that gets the job done. If you consider the potential problems of fetching sensitive patient or financial data to LLMs. Business need to be really weigh the benefits or advantages of using AI to handle that data. I am certain that the right way to process the data is by writing performant and reliable code. It's deterministic not susceptible to stochastic or probabilistic based solutions such as machine learning algorithms and LLMs.

Moreover, since there are no LLMs in the loop you get a performance application essentially for free if you compare it to what it takes to make LLM processing more deterministic and reliable. The problem with that approach is not only the costs, but the fact that you are trying to coerce a system that is not meant to be reproducible deterministic. By introducing LLMs into this crucial step you risk loosing the credibility of your clients, this is far more important than revenue any day, for they are the reason for your business to exist. 

## Processing Pipeline

The processing pipeline that this project uses can be best demonstrated by the following diagram:

![pipeline](https://github.com/misael-diaz/pdf-extractor-cpp/blob/953490db23974ec4d507fad4cd9384c4b666ae4a/assets/Pipleline.png)

From the diagram, it is easy to see that the code loads the pdf document into poppler to leverage popplers ability to extract the text as raw bytes. From there we know that the poppler output is going to be Unicode and so for simplicity of our parsing code we pass the data to a unicode to ascii transliterator. Then those bytes are filtered to remove any garbage that can come from watermarks (there is one use case). Since the processing is fast we can afford to pass the filter even if the data doesn't really need it. At that point the data is fed to the parser to extract the patient and physician data that the challenge requires.
