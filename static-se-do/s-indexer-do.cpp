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
#include <sys/stat.h>

const char* INDEX_NAME = "./indexs/testdoco.h";
static double k1 = 0.9;
static double b = 0.4;

bool newdoc = false;
typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

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
        } else if (token.length() < 25) {
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

    int m = mkdir("./indexs/", 0777);
    
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
    std::cout << ":)" << std::endl;
    outfile << "const dictionary vocab[] = {\n";
    for (int i = 0; i < keys.size(); i++) {
        outfile << "\t{\"" << keys[i] << "\", i_" << keys[i] << ", " << vocabulary[keys[i]].size() << "},\n";
    }
    outfile << "};\n\n";
    
    file.close();
    outfile.close();
}

int main(int argc, const char *argv[]) {
    if (argc == 2) {
        index(argv[1]);
    }
}
