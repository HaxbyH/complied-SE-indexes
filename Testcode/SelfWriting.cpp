#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
using namespace std;

int main () {

  fstream file;
  file.open("SelfWritingcopy.cpp");
  char charName;
  int p;

  while(file.get(charName)) {
    // std::cout << charName << "\n";
    if (charName == '/') {
      p = file.tellp();
      std::cout << file.tellp() << "\n";
    }
  }

  file.clear();
  file.seekg(p-1);

  std::cout << file.tellg() << "\n";
  std::cout << p << "\n";
  file << "int x = 10;"  << "\n";

    /**/


  // myfile.close();
  return 0;

}
