#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

bool doctag = false;
static double k1 = 0.9;
static double b = 0.4;

bool newdoc = false;
std::vector<std::string> doc_ids;
std::vector<int> doc_lengths;
int docnum = -1;

// File name
const char* INDEX_FILE_NAME = "/Users/haxby/Desktop/complied-SE-indexes-main/standard-se/s-indexs/testdoco/";

typedef std::vector<std::pair<int32_t, int32_t> > postings;
std::unordered_map<std::string, postings> vocabulary;

std::string remove_punc(std::string word) {
    std::string temp = "";
    for (int i = 0; i < word.size(); i++) {
        if ((word[i] >= 'a' && word[i] <= 'z') || (word[i] >= 'A' && word[i] <= 'Z')) {
            temp = temp + word[i];
        }
    }
    return temp;
}

char to_lowercase(char c) 
{
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

/* GET NEXT TOKEN*/
std::vector<std::string> get_next(std::string line) {
    std::vector<std::string> token_array;
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;

    // Keep splitting like by " "
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        std::string temp = "";

        // skip doctags
        if (token[0] == '<') {
            token_array.push_back(token);
            if (token.compare("<DOCNO>")==0) {
                 doctag = true;
            }
        // if last token was <DOCNO>
        } else if (doctag == true && token != "") {
            token_array.push_back(token);
            doctag = false;
        
        // remove punc
        } else {
            token = remove_punc(token);
            if (!token.empty()) {
                token_array.push_back(token);
            }
        }
        line.erase(0, pos + delimiter.length());
    }

    // repeat for rest of line
    if (!line.empty()) {
        // if it's a doctag skip
        if (line[0] == '<') {
            token_array.push_back(line);
            if (line.compare("<DOCNO>")==0) {
                doctag = true;
            }

        // if the last token was <DOCNO>
        } else if (doctag == true) {
            token_array.push_back(line);
            doctag = false;

        // remove puncuation  
        } else {
            token = remove_punc(line);
            if(!token.empty()) {
                token_array.push_back(token);
            }
        }

    }
    return token_array;
}

/* INDEX */
void index(const char* input) {

    // open file
    std::ifstream file;
    file.open(input);

    // return if file not found
    if (!file) {
        std::cout << "File not found!" << std::endl;
        return;
        }

    std::string line;
    int idoclength = 0;
    while(getline(file, line)) {
        std::vector<std::string> oneline = get_next(line);
        for (int i = 0; i < oneline.size(); i++) {
            std::string token = oneline[i];
            
            // grab doc_id and increase doc number
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

            if (token.compare("</DOC>") == 0) {
                doc_lengths.push_back(idoclength);
                idoclength = 0;
            }

            // don't index doc tags
            if(token.at(0) == '<') {
                continue;
            }

            //increase the doc length
            idoclength++;

            for (int i = 0; i < token.size(); i++) {
                char &c = token[i];
                c = to_lowercase(c);
            }

            postings &currentlist = vocabulary[token];
            if (currentlist.empty() || currentlist[currentlist.size()-1].first != docnum) {
                currentlist.push_back(std::pair<int32_t, int32_t>(docnum, 1));
            } else {
                currentlist[currentlist.size()-1].second++;
            }
        }
    }

    // for (int p: doc_lengths) {
    //     std::cout << p << std::endl;
    // }

    std::vector<std::string> keys;
    keys.reserve (vocabulary.size());

    for (auto& it : vocabulary) {
        keys.push_back(it.first);
    }

    std::sort (keys.begin(), keys.end());

    // store doc_ids
    std::string doc_id_name = "doc_ids.bin";
    std::ofstream doc_id_file;
    doc_id_file.open(INDEX_FILE_NAME + doc_id_name);
    for (std::string id : doc_ids) {
        doc_id_file << id << "\n";
    }
    doc_id_file.close();

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
    
    std::ofstream vocab_file;
    std::string vocab_name = "vocab.bin";
    vocab_file.open(INDEX_FILE_NAME + vocab_name);

    std::ofstream postings_file;
    std::string postings_name = "postings.bin";
    postings_file.open(INDEX_FILE_NAME + postings_name, std::ios::binary);

    // run through ordered vocabulary
    for (std::string primary_key : keys) {

        postings &single = vocabulary[primary_key];
        int pointer = postings_file.tellp();
        int size = single.size()*2*sizeof(int32_t);
        // write the postings two memory, each being int32_t
    
        for (int i = 0; i < single.size(); i++) {

            // write document index
            postings_file.write(reinterpret_cast<const char *>(&single[i].first), sizeof(single[i].first));

            // rsv calculation 
            int tf = single[i].second;
            double idf = log((double)doc_lengths.size()/(double)single.size());
            double rsv_score = idf * ((tf * (k1 + 1)) / (tf + k1 * (1 - b + b * (doc_lengths[single[i].first] / average_document_length))));
            int impact_score_scaled = (int)(((rsv_score-min_rsv)/max_rsv)*254 + 1);

            // write rsv score to file in binary
            postings_file.write(reinterpret_cast<const char *>(&impact_score_scaled), sizeof(impact_score_scaled));
        }
        vocab_file << primary_key << " " << pointer << " " << size << "\n";
    }
    vocab_file.close();
    postings_file.close();
}

int main(int argc, const char *argv[]) {
    if (argc == 2) {
        index(argv[1]);
    }
}


