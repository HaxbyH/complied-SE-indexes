#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

bool newdoc = false;
int docnum = 0;
std::vector<std::string>doc_ids;
typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

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

    std::vector<std::string> arr;
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
            arr.push_back(token);
        }
        line.erase(0, pos + delimiter.length());
    }

    // Remove puncutation of remaining string (last token)
    for (int i = 0; i < puncutation.length(); i++) {
        line.erase(std::remove(line.begin(), line.end(), puncutation.at(i)), 
        line.end());
    }
    if (!line.empty()) {
        arr.push_back(line);
    }
    return arr;

}

void search(const char** words, int numWords) {
    std::cout << "search selected" << "\n";
    for (int i = 2; i < numWords; i++) {
        std::cout << words[i] << "\n";
        // Do Search things
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
            for (char &c: token) {
                c = to_lowercase(c);
            }


            // std::cout << token << "\n";
            postings &currentlist = vocabulary[token];
            if (currentlist.empty() || currentlist[currentlist.size()-1].first != docnum) {
                currentlist.push_back(std::pair<int32_t, int32_t>(docnum, 1));
            } else {
                currentlist[currentlist.size()-1].second++;
            }

        }
    }
    // std::cout << vocabulary.size() << std::endl;
    // for (int i = 0; i < vocabulary.size(); i++) {
    postings &p = vocabulary["a"];
    std::cout << p.size() << std::endl;
    // }
    
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