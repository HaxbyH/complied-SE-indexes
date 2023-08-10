# complied-SE-indexes

## Motivation
This project explores different methods of sorting the index for a simple search engine, the traditional way in this depository is called the baseline, typically storing the index in an external file, which is seeked and read during search time. Two alternative methods are investigated in this depository: the static and program approaches. Compiling the search engine holds promise regarding a speed increase due to compiler time optimisations. A compiled search engine also means the final search engine is a single, easily transported executable file.

## Static Search Engine

The core idea of the static search engine is to compile the postings as constant static variables. Doing this means each variable is stored in the data segment of memory rather than being initialised and placed onto the heap during execution. This is done by the indexer of the search engine producing header files which can be compiled with the search engine. In the static search engine the document names, postings, and vocabulary are all storage in const C arrays.

## Program Search Engine
The program search engine is unwinding the loop performed when processing a postings list. This loop is being unwound by explicitly writing operations that must be done to the scores array for each term in the vocabulary. Just as the static search engine moves data from being placed on the heap intothe data segment of the program’s memory, the program search engine is moving data into the code segment of the program’s memory. To create the source code for the program search engine, the indexer writes an individual function for every term in the vocabulary. These functions hold the exact values that will be added to the scores array, and will add to the scores array if the function is called.

## Document Ordered and Impact Ordered
All types of search engines are made with both document ordered and impacted ordered approaches, these are indicated by io and do. io for impact ordered and do for document ordered.


