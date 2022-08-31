#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>

bool newdoc = false;
typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

// doc ids
std::string doc_array[] = {};

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

void search(const char** words, int numWords) {
    std::cout << "search selected" << "\n";
    for (std::string x: doc_ids) {
        std::cout << x << "\n";
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

    // print out doc_ids and tf for word
    postings &p = vocabulary["the"];
    for (int i = 0; i < p.size(); i++) {
        std::cout << doc_ids[p[i].first] << ": " << p[i].second << std::endl;

    }

    // put doc_ids in file
    std::ofstream outfile;
    outfile.open("output.cpp");

    std::ifstream infile;
    infile.open("master-se.cpp");

    // adding headers to file
    for (int i = 0; std::getline(infile,line) && i < 14; i++) {
        outfile << line << "\n";
    }

    // addings doc_ids
    outfile << "std::string doc_array[] = {";
    for (int i = 0; i < doc_ids.size(); i++) {
        outfile << "\"" << doc_ids[i] << "\"" << ",\n";
    }
    outfile << "};\n\n";
    
    outfile << "//Index Here";
    outfile << "\n";
    outfile << "//Vocab Here";

    // adding the rest
    while(std::getline(infile, line)) {
        outfile << line << "\n";
    }
    
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

    /* Possibily add delete*/

    // return error message
    } else {
        std::cout << "error: type - help" << "\n";
    }
}