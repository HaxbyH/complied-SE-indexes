#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <sstream>
#include <ctime>

// place postings in s_posting struct

struct dictionary {
    std::string term;
    int position;
    int size;
};

const char* INDEX_FILE_NAME = "/Users/haxby/Desktop/complied-SE-indexes-main/standard-se/s-indexs/disk1and2/";

// loaded on start up
std::vector<dictionary> vocab;
std::vector<std::string> doc_ids;
std::ifstream postings_file;

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

void search(char** words, int numWords) {
    const int documents_in_collection = doc_ids.size();
    static const int topk = 15;

    int32_t **accumulator = new int32_t *[topk];

    // set up rsv_pointers
    int32_t *rsv_scores = new int32_t[documents_in_collection]();
    int32_t **rsv_pointers = new int32_t *[documents_in_collection];
    int32_t **rsvp = rsv_pointers;
    for (int32_t *pointer = rsv_scores; pointer < rsv_scores + documents_in_collection; pointer++)
        *rsvp++ = pointer;

    // loop through user inputs
    for (int i = 0; i < numWords; i++) {

        // return;
        // create temp term
        dictionary term;
        term.term = words[i];

        // vector to array (for bsearch())
        dictionary* a = vocab.data();

        // binary search newly found array
        dictionary *got = (dictionary *) bsearch(&term, a, vocab.size(), sizeof(*a), vocab_compare);
        
        // return;
        // if word is in dictionary
        if (got != NULL) {
            postings_file.seekg(got->position);

            for (int i = 0; i < got->size/sizeof(int32_t)/2; i++) {
                // std::cout << "i" << std::endl;

                int32_t document_id, impact_score;
                
                postings_file.read((char*)&document_id,sizeof(int32_t));
                postings_file.read((char*)&impact_score,sizeof(int32_t));
                
                rsv_scores[document_id] += impact_score;

                if (accumulator[topk-1] == 0) {
                    insertionSort(accumulator, rsv_pointers[document_id], topk);
                
                } else if (rsv_scores[document_id] > *accumulator[topk-1]) {
                    insertionSort(accumulator, rsv_pointers[document_id], topk);
                }
            }
        }
    }


    // print out accumulator
    for (int i = 0; i < topk; i++) {
        if (accumulator[i]!=NULL) {
            std::cout << i << " " << doc_ids[accumulator[i] - rsv_scores] << " " <<*accumulator[i] << std::endl;

        }
    }
}

int main(int argc, const char *argv[]) {

    std::string doc_id_name = "doc_ids.bin";
    std::string vocab_name = "vocab.bin";
    std::string postings_name = "postings.bin";

    std::ifstream doc_ids_file;
    std::ifstream vocab_file;

    // read in doc_ids
    doc_ids_file.open(INDEX_FILE_NAME + doc_id_name);
    // std::cout << INDEX_FILE_NAME + doc_id_name << std::endl;
    if (!doc_ids_file) {
        std::cout << "no docs" << std::endl;
    }
    std::string p_line;
    while(getline(doc_ids_file, p_line)) {
        doc_ids.push_back(p_line);
    }

    // get in vocabulary
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

    // open postings file
    postings_file.open(INDEX_FILE_NAME + postings_name, std::ios::binary);

    // check if command line is a file

    // if file ends in txt we are processing more than 1 query
    char buff[5];
    memcpy(buff, &argv[1][strlen(argv[1])-4], 4);
    buff[4] = '\0';
    // std::cout << buff << std::endl;

    // if it is a .txt file
    if (strcmp(buff, ".txt")==0) {
        clock_t now;
        for (int iteration = 0; iteration < 2; iteration ++) {
            if (iteration == 1) {
                std::cout << "clock_started" << std::endl;
                now = clock();
            }

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
            if (iteration == 1) {
                now = clock() - now;
                std::cout << "time taken (secs): " <<(float)now/CLOCKS_PER_SEC << std::endl;
            }
        
        }
    } else {
        search((char**)argv, argc);
        clock_t now = clock();
        search((char**)argv, argc);
        now = clock() - now; 
        std::cout << float(now)/CLOCKS_PER_SEC << std::endl;

    }
}

