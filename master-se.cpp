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

// to be replaced
const std::string doc_array[] = {};
const dictionary vocab[] = {};

int n = sizeof(doc_array) / sizeof(doc_array[0]);
std::vector<std::string> doc_ids(doc_array, doc_array + n);
int docnum = doc_ids.size()-1;

/*
   Functions takes in a string and seperates words 
   while removing puncutation 
*/ 

char to_lowercase(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
 
    return c;
}

std::vector<std::string> get_next(std::string line) {

    std::vector<std::string> doc_array;
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    std::string puncutation = "!@#?$%^&*(),.:;'";

    // Keep splitting like by " "
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);

        // Remove puncutation
        for (int i = 0; i < puncutation.length(); i++) {
            char c = puncutation.at(i);
            token.erase(std::remove(token.begin(), token.end(), c), token.end());
        }
        if (!token.empty()) {
            doc_array.push_back(token);
        }
        line.erase(0, pos + delimiter.length());
    }

    // Remove puncutation of remaining string (last token)
    for (int i = 0; i < puncutation.length(); i++) {
        line.erase(std::remove(line.begin(), line.end(), puncutation.at(i)), 
        line.end());
    }
    if (!line.empty()) {
        doc_array.push_back(line);
    }
    return doc_array;

}

int vocab_compare(const void *a, const void *b) {
    dictionary *left = (dictionary *)a;
    dictionary *right = (dictionary *)b;

    return strcmp(left->term, right->term);
}
void process_one_term(int16_t *accumulator, const char *word) {
    dictionary term;
    term.term = word;

    dictionary *got = (dictionary *) bsearch(&term, vocab, sizeof(vocab) / sizeof(*vocab), sizeof(*vocab), vocab_compare);
    if (got != NULL) {
        for (int i = 0; i < got->postings_list_length; i++) {
            accumulator[got->postings_list[i].document_id] += got->postings_list[i].term_frequency;
        }
    }
}

void search(const char** words, int numWords) {
    int16_t accumulator[10] = {};
    // loop through words
    for (int i = 2; i < numWords; i++) {
        process_one_term(accumulator, words[i]);
    }
    for (int doc = 0; doc < 10; doc++) {
        if (accumulator[doc] != 0) {
            std::cout << doc_array[doc] << ": " << accumulator[doc] << std::endl;
        }
    }
}

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

            // don't index doc tags
            if (token.at(0) == '<') {
                continue;
            }

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
    outfile.open("output.cpp");

    std::ifstream infile;
    infile.open("master-se.cpp");

    // adding headers to file
    for (int i = 0; std::getline(infile,line) && i < 29; i++) {
        outfile << line << "\n";
    }

    // addings doc_ids
    outfile << "const std::string doc_array[] = {";
    for (int i = 0; i < doc_ids.size(); i++) {
        outfile << "\"" << doc_ids[i] << "\"" << ",\n";
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

    outfile << "\n";

    // adding vocabulary
    outfile << "const dictionary vocab[] = {\n";
    for (int i = 0; i < keys.size(); i++) {
        outfile << "\t{\"" << keys[i] << "\", i_" << keys[i] << ", " << vocabulary[keys[i]].size() << "},\n";
    }
    outfile << "};\n";
    
    // adding the rest
    std::getline(infile, line);
    while(std::getline(infile, line)) {
        outfile << line << "\n";
    }
    file.close();
    infile.close();
    outfile.close();
    
}

void help() {
    std::cout << "help selected" << "\n";
}

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

    // help operation is used
    } else if (strcmp(operation, "help")==0) {
        help();

    // return error message
    } else {
        std::cout << "error: type - help" << "\n";
    }
}