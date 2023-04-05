#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

#include "indexs/100000documents/doc_ids.h"

const int documents_in_collection = sizeof(doc_array) / sizeof(std::string);

static const int topk = 15;
static int32_t **accumulator = new int32_t *[topk];

// int32_t *accumulator = new int32_t [15]();
int32_t *rsv_scores = new int32_t[documents_in_collection]();
// = new int32_t[20]();

typedef struct dictionary {
    std::string term;
    void (*func_pointer)();

    // std::function<void> func;
} dictionary;

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

void accValid(int* n) {
    if (accumulator[topk-1] == 0) {
        insertionSort(accumulator, n, topk);
    } else if (*n > *accumulator[topk-1]) {
        insertionSort(accumulator, n, topk);
    }
}

#include "indexs/100000documents/functions.h"
#include "indexs/100000documents/vocab.h"

int compare_rsv(const void *a, const void *b) {
    int **first = (int **)a;
    int **second = (int **)b;
    return **first < **second ? 1 : **first == **second ? *first < *second ? 1 : *first == *second ? 0 : -1 : -1;
}

int vocab_compare(const void *a, const void *b) {
    dictionary *left = (dictionary *)a;
    dictionary *right = (dictionary *)b;
    return left->term.compare(right->term);
}

int search(char** words, int numWords) {

    accumulator = new int32_t *[topk];
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
    }

    for (int i = 0; i < topk; i++) {
        if (accumulator[i]!=NULL) {
            std::cout << i << " " << doc_array[accumulator[i] - rsv_scores] << " " <<*accumulator[i] << std::endl;
        }        
        // std::cout << *accumulator[i] << std::endl;
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
