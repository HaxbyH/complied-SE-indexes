#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

// int32_t *accumulator = new int32_t [15]();
int32_t *rsv_scores; 
// = new int32_t[20]();

typedef struct dictionary {
    std::string term;
    void (*func_pointer)();

    // std::function<void> func;
} dictionary;

#include "testdoco/includes.h"
#include "testdoco/functions.h"
#include "testdoco/vocab.h"

int vocab_compare(const void *a, const void *b) {
    dictionary *left = (dictionary *)a;
    dictionary *right = (dictionary *)b;
    return left->term.compare(right->term);
}

void insertionSort(int *arr[], int* n, const int topk) {

    // check if document is already in acc, if it is take out of accumulator
    for(size_t i = 0; i < topk; i++) {
        if(arr[i]==n) {
            arr[i] = NULL;
            break;
        }
    }

    int* temp = n;
    int* j;
    int i = 0;

    // run through size of acc
    while (i < topk) {
        // reached end, place in array
        if (arr[i]==NULL) {
            arr[i] = temp;
            break;
        // if bigger, place then swap
        } else if (*arr[i] < *temp) {
            j = arr[i];
            arr[i] = temp;
            temp = j;
        }
        i++;
    }
}

int search(char** words, int numWords) {

    const int documents_in_collection = 11;
    static const int topk = 15;

    int32_t **accumulator = new int32_t *[topk];

    // set up rsv_pointers
    rsv_scores = new int32_t[documents_in_collection]();
    int32_t **rsv_pointers = new int32_t *[documents_in_collection]();
    int32_t **rsvp = rsv_pointers;
    for (int32_t *pointer = rsv_scores; pointer < rsv_scores + documents_in_collection; pointer++)
        *rsvp++ = pointer;
    
    // go through all words
    for (int i = 0; i < numWords; i++) {
        dictionary term;
        term.term = words[i];

        int vocab_length = sizeof(vocab) / sizeof(dictionary);
        dictionary *got = (dictionary *) bsearch(&term, vocab, vocab_length, sizeof(*vocab), vocab_compare);
        if (got != NULL) {
            got->func_pointer();
        }

        // at this point how do we track what rsv pointers to put into the accumulator?
    }

    // do qsort here
    // then print with rsv_pointers
    // then test on more and more data
    for (int i = 0; i < documents_in_collection; i++) {
        std::cout << rsv_scores[i] << std::endl;
    }

    return 0;
}

int main(int argc, const char *argv[]) {

    // if file ends in txt we are processing more than 1 query
    char buff[5];
    memcpy(buff, &argv[1][strlen(argv[1])-4], 4);
    buff[4] = '\0';
    // std::cout << buff << std::endl;

    // if it is a .txt file
    if (strcmp(buff, ".txt")==0) {
        clock_t now;
        for (int iteration = 0; iteration < 1; iteration ++) {
            // if (iteration == 1) {
            //     std::cout << "clock_started" << std::endl;
            //     now = clock();
            // }

            std::ifstream query_file;
            query_file.open(argv[1]);

            if (query_file) {
                // get line
                char line[100];
                while(query_file.getline(line, 100)) {
                    
                    // get line by " "
                    int words_size = 0;
                    char* words[10];
                    char** words_pointer = words;
                    char* p;

                    // add word to char*[]
                    p = strtok(line, " ");
                    while(p != NULL) {
                        words[words_size++] = p;
                        p = strtok(NULL, " ");
                    }
                    search(words_pointer, words_size);
                    std::cout << " " << std::endl;
                }
            }
            // if (iteration == 1) {
            //     now = clock() - now;
            //     std::cout << "time taken (secs): " <<(float)now/CLOCKS_PER_SEC << std::endl;
            // }
        
        }
    } else {
        search((char**)argv, argc);
        std::cout << " " << std::endl;
        // clock_t now = clock();
        // search((char**)argv, argc);
        // now = clock() - now; 
        // std::cout << float(now)/CLOCKS_PER_SEC << std::endl;

    }
}