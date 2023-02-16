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

// CHANGE VARIABLES
const size_t NUMDOCS = 700001;
const char* INDEX_NAME = "1000_documents.h";
static double k1 = 0.9;
static double b = 0.4;

bool newdoc = false;
typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

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

// if index exists include 
#if __has_include("1000_documents.h")
#include "1000_documents.h"
#else
#include "emptyindex.h"
#endif

std::vector<std::string> doc_ids;
std::vector<int> doc_lengths;
int docnum = -1;
bool doctag = false;

/* -------------------------
        TO_LOWERCASE
   ------------------------*/
char to_lowercase(char c) 
{
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

/* -------------------------
      REMOVE PUNCTUATION
   ------------------------*/
std::string remove_punc(std::string word) {
    std::string temp = "";
    for (int i = 0; i < word.size(); i++) {
        if ((word[i] >= 'a' && word[i] <= 'z') || (word[i] >= 'A' && word[i] <= 'Z')) {
            temp = temp + word[i];
        }
    }
    return temp;
}

/* ---------------------------
        GET NEXT TOKEN
   --------------------------*/
std::vector<std::string> get_next(std::string line) {
    std::vector<std::string> doc_array;
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;

    // Keep splitting like by " "
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        std::string temp = "";

        // skip doctags
        if (token[0] == '<') {
            doc_array.push_back(token);
            if (token.compare("<DOCNO>")==0) {
                 doctag = true;
            }
        // if last token was <DOCNO>
        } else if (doctag == true && token != "") {
            doc_array.push_back(token);
            doctag = false;
        
        // remove punc
        } else {
            token = remove_punc(token);
            if (!token.empty()) {
                doc_array.push_back(token);
            }
        }
        line.erase(0, pos + delimiter.length());
    }

    // repeat for rest of line
    if (!line.empty()) {
        // if it's a doctag skip
        if (line[0] == '<') {
            doc_array.push_back(line);
            if (line.compare("<DOCNO>")==0) {
                doctag = true;
            }

        // if the last token was <DOCNO>
        } else if (doctag == true) {
            doc_array.push_back(line);
            doctag = false;

        // remove puncuation  
        } else {
            token = remove_punc(line);
            if(!token.empty()) {
                doc_array.push_back(token);
            }
        }

    }
    return doc_array;
}

/* ------------------------
          INDEX
-------------------------*/
void index(const char* input) {

    std::ifstream file;
    file.open(input);

    // return if the file is not valid
    if (!file) {
        std::cout << "File not found!" << "\n";
        return;
    }
    
    // iterate over each line
    std::string line;
    int idoclength = 0;
    while(getline(file, line)) {
        std::vector<std::string> oneline = get_next(line);

        // iterate through tokens
        for (int i = 0; i < oneline.size(); i++) {
            std::string token = oneline[i];

            // grab docID and increases doc numbers
            if (newdoc) {
                doc_ids.push_back(token);
                docnum++;
                newdoc = false;
                continue;
            }
            
            // when we hit DOCNO new document
            if (token.compare("<DOCNO>") == 0) {
                newdoc = true;
            }

            // hit end of file
            if (token.compare("</DOC>") == 0) {
                doc_lengths.push_back(idoclength);

                idoclength = 0;
            }

            // don't index doc tags
            if (token.at(0) == '<') {
                continue;
            }

            // increase the doc length
            idoclength++;

            // convert to lowercase
            for (int i = 0; i < token.size(); i ++) {
                char &c = token[i];
                c = to_lowercase(c);
            }
            // Adding to unordered map
            postings &currentlist = vocabulary[token];
            if (currentlist.empty() || currentlist[currentlist.size()-1].first != docnum) {
                currentlist.push_back(std::pair<int32_t, int32_t>(docnum, 1));
            } else {
                currentlist[currentlist.size()-1].second++;
            }
        }
    }

    // sort keys of array
    std::vector<std::string> keys;
    keys.reserve (vocabulary.size());

    for (auto& it : vocabulary) {
        keys.push_back(it.first);
    }
    std::sort (keys.begin(), keys.end());

    // put doc_ids in file
    std::ofstream outfile;
    outfile.open(INDEX_NAME);

    outfile << "const std::string doc_array[] = {\n";
    for (int i = 0; i < doc_ids.size(); i++) {
        outfile << "\t\"" << doc_ids[i] << "\"" << ",\n";
    }
    outfile << "};\n\n";

    // calculate average document length
    double average_document_length = 0;
    for (int32_t document = 0; document < doc_lengths.size(); document++)
        average_document_length += doc_lengths[document];
    average_document_length /= (double)doc_lengths.size();

    // go through each word finding max and min rsv_score
    double max_rsv = 0;
    double min_rsv = 0;
    for (int q = 0; q < keys.size(); q++) { 
        postings &single = vocabulary[keys[q]];
        for (int i = 0; i < single.size(); i++) {
            
            // calculate rsv
            int tf = single[i].second;
            int d = single[i].first;
            double idf = log((double)doc_lengths.size()/(double)single.size());
            double rsv_score = idf * ((tf * (k1 + 1)) / (tf + k1 * (1 - b + b * (doc_lengths[d] / average_document_length))));
            if (rsv_score < min_rsv) {
                min_rsv = rsv_score;
            } else if (rsv_score > max_rsv) {
                max_rsv = rsv_score;
            }
        }
    }

    for (int q = 0; q < keys.size(); q++) {
        postings &single = vocabulary[keys[q]];
        outfile << "const s_posting i_" << keys[q] << "[] = {"; 
        for (int i = 0; i < single.size(); i++) {
            int tf = single[i].second;
            int d = single[i].first;
            double idf = log((double)doc_lengths.size()/(double)single.size());
            double rsv_score = idf * ((tf * (k1 + 1)) / (tf + k1 * (1 - b + b * (doc_lengths[d] / average_document_length))));
            int impact_score_scaled = (int)(((rsv_score-min_rsv)/max_rsv)*254 + 1);
            outfile << "{" << d << ", " << impact_score_scaled;
            if (i != single.size()-1) {
                outfile << "}, ";
            }
        }
        outfile << "}};\n";
    }    

    outfile << "\n\n";

    // adding vocabulary
    outfile << "const dictionary vocab[] = {\n";
    for (int i = 0; i < keys.size(); i++) {
        outfile << "\t{\"" << keys[i] << "\", i_" << keys[i] << ", " << vocabulary[keys[i]].size() << "},\n";
    }
    outfile << "};\n\n";
    
    file.close();
    outfile.close();
    
}

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

/*---------------------
        MAIN
-----------------------*/
int main(int argc, const char *argv[]) {

    const char* operation = argv[1];

    // If the input is to be indexed
    if (strcmp(operation, "index")==0) {
        if (argc > 3) {
            std::cout << "Insert only one file" << "\n";
        } else {
            index(argv[2]);
        }

    // If the input is to be searched for
    } else if (strcmp(operation, "search")==0) {
        
        // if file ends in txt we are processing more than 1 query
        char buff[5];
        memcpy(buff, &argv[2][strlen(argv[2])-4], 4);
        buff[4] = '\0';

        // if it is a .txt file
        if (strcmp(buff, ".txt")==0) {

            std::ifstream query_file;
            query_file.open(argv[2]);

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
        } else {
            int i = 0;
            for(i = 2;i < argc; i++) {
                argv[i-2] = argv[i];
            }
            argc -= std::min(argc, 2);
            search((char**)argv, argc);
        }
        

    // return error message
    } else {
        std::cout << "ERROR" << std::endl;
    }
}
