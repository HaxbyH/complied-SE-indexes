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
const size_t NUMDOCS = 100;
const char* INDEX_NAME = "./WSJheaders/testdoco.h";
static double k1 = 0.9;
static double b = 0.4;
double documents_in_collection;
double average_document_length = 0;


bool newdoc = false;
typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

typedef struct
	{
	uint32_t document_id;
	uint16_t term_frequency;
	} s_posting;

typedef struct
	{
	const char *term;
	const s_posting *postings_list;
	uint32_t postings_list_length;
	} dictionary;

typedef struct
    {
    double rsv_sc;
    std::string term_id;
    } accumulator;

// if index exists include 
#if __has_include("WSJheaders/testdoco.h")
#include "WSJheaders/testdoco.h"
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
        } else if (doctag == true) {
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
                // std::cout << idoclength << std::endl;
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

    // adding postings
    for (int i = 0; i < keys.size(); i++) {
        postings &single = vocabulary[keys[i]];
        outfile << "const s_posting i_" << keys[i] << "[] = {";
        for (int i = 0; i < single.size(); i++) {
            outfile << "{" << single[i].first << ", " << single[i].second;
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

    // adding doc lengths
    outfile << "const int doclengths[] = {";
    for (int i = 0; i < doc_lengths.size(); i++) {
        outfile << doc_lengths[i] << ", ";
    }
    outfile << "};\n";
    
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

/* -----------------------
     ADD TO ACCUMULATOR
   -----------------------*/
void process_one_term(accumulator *acc, const char *word) {
    dictionary term;
    term.term = word;

    dictionary *got = (dictionary *) bsearch(&term, vocab, sizeof(vocab) / sizeof(*vocab), sizeof(*vocab), vocab_compare);

    // calculate idf
    int p_length = got->postings_list_length;
    double idf = log(documents_in_collection / p_length);

    if (got != NULL) {
        for (int i = 0; i < p_length; i++) {
            int tf = got->postings_list[i].term_frequency;
            int d = got->postings_list[i].document_id;
            double rsv_score = idf * ((tf * (k1 + 1)) / (tf + k1 * (1 - b + b * (doclengths[d] / average_document_length))));
            // std::cout << rsv_score << std::endl;

            acc[d].rsv_sc += rsv_score;
        }
    }
}

/* ----------------------
        COMPARE RSV
-------------------------*/
int compare_rsv(const void *a, const void *b) {
    accumulator *accumulatorA = (accumulator *)a;
    accumulator *accumulatorB = (accumulator *)b;
    if (accumulatorA->rsv_sc < accumulatorB->rsv_sc) {
        return 1;
    } else {
        return -1;
    }
}

/* ----------------------
        SEARCH
-----------------------*/
void search(const char** words, int numWords) {

    // numbers of documents in collection
    documents_in_collection = sizeof(doclengths) / sizeof(int32_t);

    // average numbers of documents in collection
    for (int32_t document = 0; document < documents_in_collection; document++)
	    average_document_length += doclengths[document];
    average_document_length /= documents_in_collection;

    // accumulator
    accumulator acc[NUMDOCS];

    // worried about speed here
    for (int i = 0; i < documents_in_collection; i++) {
        acc[i].term_id = doc_array[i];
        acc[i].rsv_sc = 0;

    }

    // // loop through words
    for (int i = 2; i < numWords; i++) {
        process_one_term(acc, words[i]);
    }

    qsort(acc, documents_in_collection, sizeof(accumulator), compare_rsv);

    std::string query_id = "PH_ID";
    for(int i = 0; i < documents_in_collection && acc[i].rsv_sc != 0.0; i++) {
        std::cout << query_id << " Q0 " << acc[i].term_id << " " << i+1 << " " << acc[i].rsv_sc << " c-search" << std::endl;
    };

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
        search(argv, argc);

    // return error message
    } else {
        std::cout << "ERROR" << std::endl;
    }
}