#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <sstream>

// place postings in s_posting struct

struct dictionary {
    std::string term;
    int position;
    int size;
};

const char* INDEX_FILE_NAME = "./s-indexs/wsj1000/";

std::vector<dictionary> vocab;

std::vector<std::string> doc_ids;

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

int main(int argc, const char *argv[]) {

    // read in doc_ids
    std::string doc_id_name = "doc_ids.bin";
    std::ifstream doc_ids_file;
    doc_ids_file.open(INDEX_FILE_NAME + doc_id_name);
    if (!doc_ids_file) {
        std::cout << "no docs" << std::endl;
    }
    std::string p_line;
    while(getline(doc_ids_file, p_line)) {
        doc_ids.push_back(p_line);
    }

    // get in vocabulary
    std::ifstream vocab_file;
    std::string vocab_name = "vocab.bin";
    vocab_file.open(INDEX_FILE_NAME + vocab_name);
    if (!vocab_file) {
        std::cout << "no vocab" << std::endl;
    }
    std::string v_line;
    
    // each line is one entry in vocab
    while(getline(vocab_file, v_line)) {

        // split up line by delimiter
        std::istringstream stream(v_line);
        std::string token;

        // dictionary entry;
        std::string foo[3];
        int i = 0;
        while (getline(stream, token, ' ')) {
            foo[i] = token; 
            i++;
            // std::cout << token << std::endl;
        }

        // create single dictionary entry and add values to it
        dictionary single;
        single.term = foo[0];
        single.position = atoi(foo[1].c_str());
        single.size = atoi(foo[2].c_str());
        vocab.push_back(single);

    }

    int documents_in_collection = doc_ids.size();
    static const int topk = 15;

    int **accumulator = new int *[topk];

    // set up rsv_pointers
    int *rsv_scores = new int[documents_in_collection];
    int **rsv_pointers = new int *[documents_in_collection];
    int **rsvp = rsv_pointers;
    for (int *pointer = rsv_scores; pointer < rsv_scores + documents_in_collection; pointer++)
        *rsvp++ = pointer;
    
    // open postings
    std::ifstream postings_file;
    std::string postings_name = "postings.bin";
    postings_file.open(INDEX_FILE_NAME + postings_name, std::ios::binary);

    // loop through user inputs
    int i = 0;
    for (int i = 1; i < argc; i++) {

        // create temp term
        dictionary term;
        term.term = argv[i];

        // vector to array (for bsearch())
        dictionary* a = vocab.data();

        // binary search newly found array
        dictionary *got = (dictionary *) bsearch(&term, a, vocab.size(), sizeof(*a), vocab_compare);
        
        // if word is in dictionary
        if (got != NULL) {
            postings_file.seekg(got->position);

            for (int i = 0; i < got->size/sizeof(int32_t)/2; i++) {

                int32_t document_id, impact_score;
                postings_file.read((char*)&document_id,sizeof(int32_t));
                postings_file.read((char*)&impact_score,sizeof(int32_t));
                // std::cout << " " << std::endl;
                // std::cout << "doc_id: " <<document_id << std::endl;
                // std::cout << "impact_score: " << impact_score << std::endl;

                // if (rsv_scores[document_id] == NULL) {
                //     std::cout << "null!" << std::endl;
                // }

                rsv_scores[document_id] += impact_score;

                if (accumulator[topk-1] == 0) {
                    insertionSort(accumulator, rsv_pointers[document_id], topk);
                
                } else if (rsv_scores[document_id] > *accumulator[topk-1]) {
                    insertionSort(accumulator, rsv_pointers[document_id], topk);
                }

                // std::cout << "after add: " << rsv_scores[document_id] << std::endl;
                

            }
        }
    }


    // print out accumulator
    std::cout << "\naccumulator" << std::endl;
    for (int i = 0; i < topk; i++) {
        if (accumulator[i]!=NULL) {
            std::cout << i << " " << doc_ids[accumulator[i] - rsv_scores] << " " <<*accumulator[i] << std::endl;

        }
    }
}
