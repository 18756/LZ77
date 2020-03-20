#include <map>
#include <cstring>
#include "LZ77.h"

int main (int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "You must specify option -compress or -decompress";
        return 1;
    } else {
        if (0 != strcmp(argv[1], "-compress") && 0 != strcmp(argv[1], "-decompress")) {
            std::cout << "Incorrect option, option must be -compress or -decompress";
            return 1;
        }
        if (strcmp(argv[1], "-compress") == 0) {
            if (argc != 5) {
                std::cout << "Wrong number of arguments. Must be 3.";
            } else {
            	LZ77* lz77 = new LZ77();
                lz77->compress(argv[2], argv[3], std::stoi(argv[4]));
            	delete lz77;
            }
        } else if (strcmp(argv[1], "-decompress") == 0) {
            if (argc != 4) {
                std::cout << "Wrong number of arguments. Must be 2.";
            } else {
            	LZ77* lz77 = new LZ77();
                lz77->decompress(argv[2], argv[3]);
                delete lz77;
            }
        }
    }
}

