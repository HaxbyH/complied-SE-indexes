#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <sstream>

std::vector<std::string> doc_ids;

// place postings in s_posting struct
typedef struct
    {
    uint32_t document_id;
    int impact_score;
    } s_postings;

typedef struct
    {
    const char *term;
    const int position;
    const int size;
    } dictionary;

std::vector<dictionary> vocab;

int main(int argc, const char *argv[]) {

    // read in doc_ids
    std::ifstream doc_ids_file;
    doc_ids_file.open("./s-indexs/testdoco/doc_ids.bin");
    std::string p_line;
    while(getline(doc_ids_file, p_line)) {
        doc_ids.push_back(p_line);
    }

    // get in vocabulary
    std::ifstream vocab_file;
    vocab_file.open("./s-indexs/testdoco/vocab.bin");
    std::string v_line;
    // each line is one entry in vocab
    while(getline(vocab_file, v_line)) {

        // split up line by delimiter
        std::istringstream stream(v_line);
        std::string token;

        dictionary entry;

        int i = 0;
        while (getline(stream, token, ' ')) {
            std::cout << token << std::endl;
        }

        // char *ptr = strtok (v_line, " ");
        
        
        // std::cout << v_line << std::endl;
    }
    


    
    // doc_ids.seekg(144);
    // int value;
    // int size = 80;
    // //   std::cout << 72/sizeof(int32_t);
    // for(int i = 0; i < size/sizeof(int32_t)/2; i++) {
    //     int first;
    //     doc_ids.read((char*)&first,sizeof(int32_t));

    //     int second;
    //     doc_ids.read((char*)&second,sizeof(int32_t));


    //     std::cout << first << " " << second << std::endl;
        //first
        //second
    // }
}
