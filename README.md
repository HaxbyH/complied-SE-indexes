# complied-SE-indexes

#### Motivation
---------------

This project explores different methods of sorting the index for a simple search engine, the traditional way, with in this depository is called the baseline, typically store the index in an external file, which is seeked and read during search time. Two alternative methods to this are investigated in this dipository, named the static and program approaches. These are compared to the baseline method.



How to run:

g++ master-se.cpp -o ./c-search

./c-search index wsj.xml

g++ output.cpp -o ./c-search

./c-search search 'term'
