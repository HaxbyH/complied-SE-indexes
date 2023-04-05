#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <stdlib.h>
#include <stdint.h>

typedef struct
	{
	uint32_t document_id;
    int impact_score;
	} s_posting;

typedef struct
	{
	const char *term;
	const s_posting *postings_list;
	uint32_t postings_list_length;
	} dictionary;

#include "indexs/testdoco.h"

/* -----------------------
        COMPARE TOKENS
   -----------------------*/
int vocab_compare(const void *a, const void *b) {
    dictionary *left = (dictionary *)a;
    dictionary *right = (dictionary *)b;

    return strcmp(left->term, right->term);
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

/* ----------------------
        SEARCH
-----------------------*/
void search(char** words, int numWords) {

    // numbers of documents in collection
    int documents_in_collection = sizeof(doc_array) / sizeof(std::string);
    static const int topk = 15;
    int **accumulator = new int *[topk];

    int *rsv_scores = new int[documents_in_collection];
    
    int **rsv_pointers = new int *[documents_in_collection];
    int **rsvp = rsv_pointers;
    for (int *pointer = rsv_scores; pointer < rsv_scores + documents_in_collection; pointer++)
	    *rsvp++ = pointer;

    // loop through words in query
    for (int i = 0; i < numWords; i++) {
        dictionary term;
        term.term = words[i];

        dictionary *got = (dictionary *) bsearch(&term, vocab, sizeof(vocab) / sizeof(*vocab), sizeof(*vocab), vocab_compare);

         // if word is in dictionary
        if (got != NULL) {
            int p_length = got->postings_list_length;

            // go through all postings
            for (int i = 0; i < p_length; i++) {
                int is = got->postings_list[i].impact_score;
                int d = got->postings_list[i].document_id;
                rsv_scores[d] += is;

                // if the lowest score in accumulator is empty, insert
                if (accumulator[topk-1] == 0) {
                    insertionSort(accumulator, rsv_pointers[d], topk);

                // if the current score is greater than lowest score in accumulator    
                } else if (rsv_scores[d] > *accumulator[topk-1]) {
                    insertionSort(accumulator, rsv_pointers[d], topk);
                }
            }
        }
    }

    // print out the accumulator
    for (int i = 0; i < topk; i++) {
        if (accumulator[i]!=NULL) {
            std::cout << i << " " << doc_array[accumulator[i] - rsv_scores] << " " <<*accumulator[i] << std::endl;
        }
    }
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
