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

This section is dedicated to documenting the challenges that I encountered during the initial phase before I even decided to commit to developing a solution for this challenge.

- **data extraction**: deciding what tool to use to extract the data from the PDF is challenging because there are many alternatives. Chose poppler because it has been battle tested and it require minimal code to work at the byte level.
- **data cleanup**: doing ascii folding simplifies the parsing of the document and also helps with cleaning up data that could confuse the pdf extraction tool
- **timestamps formats**: the sample documents show that timestamps are formatted differently among medical records. This needs to be addressed.
- **multiline spanning data**: example some names are broken down into at least one line (tabulated medical record version). Writing the logic to extract the name without failure at seemed looked as if the code needed to use regexs but it turned out that simple logic was all that was needed to tackle this issue.
- **extract physician name**: conceptually extracting the physician name is no harder than extracting the patient data or anything else in the document. This is how the code looks prior to the pitch workshop. Stopping code writing at this point.

If you ask me the hardest part was identifying if I can tackle the challenge in a reasonable amount of time by leveraing my expertise and background. Another difficult point was the programming language choice. At first I was considering to use Golang because I thought I could drive more of the project beyond the data extraction. But as I saw that the data extraction was the biggest hurdle I knew that I had to work with a language that I am more familiar with such as C/C++ or Python. At first I thought it would be great to use Python but that did not go along with my personal goal of building a project with zero dependencies for this event. As I discussed ideas with AI about the alternatives the AI mentioned some libraries that I have used from Python for text processing. As it is common in Python the hardcore libraries are written in C/C++ and Python provides the needed wrappers. I asked AI to generate some code snippets to look at what it would be like to extract raw data from a pdf with a battle tested library. That's when I decided to go all out with C++ and poppler. If you look at the implementation you will know that I am mostly using C++ to interact with poppler and then I switch to low level C code (raw pointers) to get the maximum performance possible with little ceremony from my part. The main reason here was focusing on the problem and know that if there's an error is because of something that I wrote and this is why the debugger is one of the most important tools used to develop this application. It would have been great to write memory safe C++ code but frankly most of the time I am writing C code and again this was not the time to write fancy code. Just write functional code.

Along the line of writing functional code you will notice that I did not bother to refactor code into functions. I saw the patterns but again had to weigh the benefits of speed writing and locality of reference (the developer locality of reference) against writing clean code. I will not be evaluated by the structure of the code, I will be evaluated by the ability of the code to solve the problem.

## Features

Lists the most important features that I was able to implement during the event timeline:

- **zero copy**: we use a memory map to store the extracted data for performance reasons. This means that the Linux Kernel is giving us fast access to memory without incurring on copies from user to kernel space.
- **lean data**: we need only 7-bits to represent ASCII characters and this means that if there are size limitations (as mentioned in the challenge) by doing this data conversion we are getting rid of the bloat. The document size is reduced to as many bytes as text characters in the document.
- **filters out watermark**: at the byte level the watermarks are easy to identify, filtering out the watermark was achieved by writing simple conditionals. These were applied after doing the ascii folding transformation
- **extracts names from multiline text**: extracts the full patient name even if it spans multiple line (in the tabulated Sura records).
- **extracts physician name**: gets the name of the phsysician from one of the cases. Had been given more time I would have had more cases completed. However I had to weigh the number of features against my ability to pitch the work done the next day.
- **zero dependencies**: poppler is easy to install and frankly should not even be considered a dependency. Unlike Python or other interpreted or jit compiled languages the dependencies are always a problem. Projects need maintenance as the language or the ecosystem evolves thereby increasing the burden on the developers that maintain it. This is a build once application. All that you need the GNU Compiler Collection (GCC) to build it.
- **high performance**: This code can run in a Raspberry Pi if you want, it's a low level code that requires little resources. Pretty much the memory to load the pdf into memory.

## Advantages

You don't need a lot of compute to get a reliable and performant application that gets the job done. If you consider the potential problems of fetching sensitive patient or financial data to LLMs. Business need to be really careful and weigh the benefits or advantages of using AI to handle the data extraction and processing. I am certain that the right way to process the data is by writing performant and reliable code. It's deterministic not susceptible to stochastic or probabilistic based solutions such as machine learning algorithms and LLMs.

The build is simple, all that you need is to install poppler, this document shows you how, and to build with the C++ compiler provided by GCC or LLVM if you prefer. Simplicity is an advantage, you don't have to deal with massive dependencies. Sometimes dependency handling entails compartmentalization with Docker to isolate the application and that is more work and introduces yet another layer to be concerned about to deploy the solution. The build time is reasonable if you compare it against the time it requires to build Rust applications. (There's the problem of the borrow checker which makes the development much bumpy and strenous, if you are looking for development speed unless you are a pro Rust developer choose something else and that's what I did.) This is a lean implementation so even if C++ is not considered a fast build project for this one it is because we do not leverage the complexities introduced by the STL.

Moreover, since there are no LLMs in the loop you get a performance application essentially for free if you compare it to what it takes to make LLM processing more deterministic and reliable. The problem with that approach is not only the costs, but the fact that you are trying to coerce a system that is not meant to be reproducible deterministic. By introducing LLMs into this crucial step you risk loosing the credibility of your clients, this is far more important than revenue any day, for they are the reason for your business to exist. 

Open source code. I am releasing my code under the MIT License so that it will be easy for anyone to use this code to address similar problems. The other advantage is that tools like this when made open source a community can grow from it and become more than their original intention or use.

## Broader Impact

This approach of writing low level code to extract sensitive data from a PDF document can be applied not only for the health sector but also to other sectors as well such as finance (realiable data extraction from bank summaries), law sector, manufacturing, government, and academic settings.

The implementation shows that with simple logic it is possible to extract data from complex pdf documents even if they contain a certain degree of noise or garbage data.

## Future Work

It would have been great to write this tool so that it processes all the documents recursively down a directory hierarchy. However I focused on the core functionality. I can envision that this feature can be implemented by piping the result of the find command using simple file extension matching such as `find -iname '*.pdf'`. We know that the output of the command is going to be a list separated by newlines and so it could be either put all into memory or processed one line at a time if you want to be conservative with memory resources. Meaning you expect a lot of documents (as in from a dedicated fileserver or network filesystem).

It would also have been great to store the data in a SQL database. I would have chosen SQLite because it requires minimal configuration and it's just one more library to link to this code. This would be the connection point with the microservice architecture of the client. Bearing this in mind this is were we would stop and say if a document was not suitable for processing by poppler that it requires human revision instead of the automation workflow. The code would continue processing the rest of the documents.

I can envision this code to be scheduled by a simple cronjob that can be set by a system administrator or an experienced Linux user. In fact an LLM can even help guide a newcomer to Linux on how to setup the cronjob. Probably it would be wise to run this application daily I do not expect it to take long to process a batch of document because it is performant.

The best tools are the tools that do just their job and that's it. I do not see more functionality needed for this application other than to extend it for other documents or new layouts.

## Maintenance

This code does not require high maintenance it is essentially a build it once and run it application. I do see the need to extend it (not rewrite it) for new documents. However, the programming is really basic since it uses basic pattern matching provided by GLIBC. I did not see the need to use regular expressions regexs to process the raw data. It all boils down to pointer arithmetic, once you are comfortable with that as a programmer you can work on the development of any low level application. 

## Processing Pipeline

The processing pipeline that this project uses can be best demonstrated by the following diagram:

![pipeline](https://github.com/misael-diaz/pdf-extractor-cpp/blob/953490db23974ec4d507fad4cd9384c4b666ae4a/assets/Pipleline.png)

From the diagram, it is easy to see that the code loads the pdf document into poppler to leverage popplers ability to extract the text as raw bytes. From there we know that the poppler output is going to be Unicode and so for simplicity of our parsing code we pass the data to a unicode to ascii transliterator. Then those bytes are filtered to remove any garbage that can come from watermarks (there is one use case). Since the processing is fast we can afford to pass the filter even if the data doesn't really need it. At that point the data is fed to the parser to extract the patient and physician data that the challenge requires.
