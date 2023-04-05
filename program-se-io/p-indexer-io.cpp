#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sys/stat.h>

bool doctag = false;
static double k1 = 0.9;
static double b = 0.4;

bool newdoc = false;
std::vector<std::string> doc_ids;
std::vector<int> doc_lengths;
int docnum = -1;

// File name
std::string INDEX_FILE_NAME = "./indexs/testdoco/";
std::string uterms_name = "/u_terms";

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
        
        // this bit needs to be added to all search engines
        } else if (token.length() < 25) {
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

bool p_comparison(const std::pair<int,int> &a,const std::pair<int,int> &b)
{
       return a.second>b.second;
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

    //create directorys
    int m = mkdir("./indexs/", 0777);
    int md = mkdir(INDEX_FILE_NAME.c_str(), 0777);
    int mdu = mkdir((INDEX_FILE_NAME + uterms_name).c_str(), 0777);

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

            if (token.size() < 25) {
                postings &currentlist = vocabulary[token];
                if (currentlist.empty() || currentlist[currentlist.size()-1].first != docnum) {
                    currentlist.push_back(std::pair<int32_t, int32_t>(docnum, 1));
                } else {
                    currentlist[currentlist.size()-1].second++;
                }
            }
        }
    }

    std::vector<std::string> keys;
    keys.reserve (vocabulary.size());

    for (auto& it : vocabulary) {
        keys.push_back(it.first);
    }

    std::sort (keys.begin(), keys.end());

    // store doc_ids
    std::string doc_id_name = "doc_ids.bin";

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

    // run through ordered vocabulary - updating tf to impact_score
    for (std::string primary_key : keys) {
        postings &single = vocabulary[primary_key];
    
        for (int i = 0; i < single.size(); i++) {

            // rsv calculation 
            int tf = single[i].second;
            double idf = log((double)doc_lengths.size()/(double)single.size());
            double rsv_score = idf * ((tf * (k1 + 1)) / (tf + k1 * (1 - b + b * (doc_lengths[single[i].first] / average_document_length))));
            int impact_score_scaled = (int)(((rsv_score-min_rsv)/max_rsv)*254 + 1);
            single[i].second = impact_score_scaled;

        }
    }

    // open vocab file
    std::ofstream vocab_file;
    std::string vocab_name = "vocab.h";
    vocab_file.open(INDEX_FILE_NAME + vocab_name);

    // open vocab function file
    std::ofstream function_file;
    std::string function_name = "functions.h";
    function_file.open(INDEX_FILE_NAME + function_name);
    
    // open includes file
    std::ofstream includes_file;
    std::string includes_name = "includes.h";
    includes_file.open(INDEX_FILE_NAME + includes_name);

    // create start of vocab 
    vocab_file << "const dictionary vocab[] = {\n";

    // run through vocab again
    for (std::string primary_key : keys) {
        postings &single_postings = vocabulary[primary_key];

        // add includes path
        includes_file << "#include \"u_terms/" << primary_key <<"_h.h\"\n";

        // add to vocab
        vocab_file << "\t{\"" << primary_key << "\", &f_" << primary_key << "},\n"; 

        // add to functions file
        function_file << "void f_" + primary_key + "() {\n";

        // sort the impact scores by descending order
        std::sort(single_postings.begin(),single_postings.end(), p_comparison);

        std::ofstream postings_file;
        std::string postings_name = "u_terms/" + primary_key + "_h.h";
        postings_file.open(INDEX_FILE_NAME + postings_name);

        int past_impact = 256;
        for (int i = 0; i < single_postings.size(); i++) {

            // if the impact score has changed
            if (single_postings[i].second != past_impact) {
                // add the new function to the function list for term
                if (past_impact != 256) {
                    postings_file << "}\n";
                }
                function_file << "\t" << primary_key << "_" << single_postings[i].second << "();\n";

                // create new function header
                postings_file << "void " << primary_key << "_" << single_postings[i].second << "() {\n";
            } 

            postings_file << "\trsv_scores[" << single_postings[i].first << "] += " << single_postings[i].second << ";\n";
            postings_file << "\taccValid(&rsv_scores[" << single_postings[i].first << "]);\n";
            // if (single_postings[i].second != past_impact) {
            //     postings_file << "}\n";
            // }
            past_impact = single_postings[i].second;
        }
        postings_file << "}\n";

        function_file << "}\n\n";
        postings_file.close();
        // return;
    
    }

    std::ofstream docs_file;
    std::string docs_name = "doc_ids.h";
    docs_file.open(INDEX_FILE_NAME + docs_name);

    docs_file << "const std::string doc_array[] = {\n";
    for (std::string x : doc_ids) {
        docs_file << "\t\""<< x << "\",\n";
    }

    docs_file << "};\n";
    vocab_file << "};\n";

    docs_file.close();
    vocab_file.close();
    function_file.close();
    includes_file.close();
}

int main(int argc, const char *argv[]) {
    if (argc == 2) {
        index(argv[1]);
    }
}


