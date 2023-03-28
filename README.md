# complied-SE-indexes

How to run:

g++ master-se.cpp -o ./c-search

./c-search index wsj.xml

g++ output.cpp -o ./c-search

./c-search search 'term'

(base) haxby@oucs1629 program-se % time ./index /Users/haxby/Desktop/complied-SE-indexes-main/WSJ/WSJData/DISKS_1_AND_2.XML
./index   949.49s user 171.98s system 90% cpu 20:41.88 total
(base) haxby@oucs1629 program-se % g++ pp-search.cpp -o ./disks1and2-search                                         
In file included from pp-search.cpp:24:
./disks1and2/includes.h:759598:10: fatal error: sorry, this include generates a translation unit too large for Clang to process.
#include "u_terms/might_h.h"
