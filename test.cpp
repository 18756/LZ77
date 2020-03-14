#include <iostream>
#include <deque>
#include <bitset>
#include <algorithm>

using namespace std;

class triplet {
public:
    triplet() = default;

    triplet(int distance, int length) : distance(distance), length(length), c(128) {}

    triplet(char c) : c(c) {}

    void compress(deque<bool>& deque) { // reversed order of char or distance and length bits
        if (c != 128) {
            deque.push_back(false);
            bitset<8> bitSet(c);
            for (int i = 0; i < 8; ++i) {
                deque.push_back(bitSet[i]);
            }
        } else {
            deque.push_back(true);
            compressIntVal(deque, distance);
            compressIntVal(deque, length);
        }
    }

    static void compressIntVal(deque<bool>& deque, int val) { // reversed order of bits
        int option = 0;
        --val;
        while (val > (1L << a[option])) {
            val -= (1 << a[option]);
            ++option;
        }
        deque.insert(deque.end(), option, true);
        deque.push_back(false);
        int bits = a[option];
        for (int i = 0; i < bits; ++i) {
            deque.push_back(val % 2 == 1);
            val /= 2;
        }
    }

    static char getOneChar(deque<bool>& deque) {
        char res = 0;
        for (int i = 0; i < min(8, (int)deque.size()); ++i) {
            res += (1 << i) * (deque.at(i));
        }
        deque.erase(deque.begin(), deque.begin() + min(8, (int)deque.size()));
        return res;
    }

    static void merge(deque<bool>& deque1, deque<bool>& deque2) {
        deque1.insert(deque1.begin(), deque2.begin(), deque2.end());
    }

    static triplet decompress(deque<bool>& deque) {
        int wasRead = 0;
        triplet res;
        if (!deque.front()) {
            ++wasRead;
            char c = 0;
            for (int i = 0; i < 8; ++i, ++wasRead) {
                c += (1 << i) * (deque.at(wasRead));
            }
            res = triplet(c);
        } else {
            ++wasRead;
            int distance = decompressIntVal(deque, wasRead);
            int length = decompressIntVal(deque, wasRead);
            res = triplet(distance, length);
        }
        deque.erase(deque.begin(), deque.begin() + wasRead);
        return res;
    }

    static int decompressIntVal(deque<bool> &deque, int &id) {
        int res = 1;
        int option = 0;
        while (deque.at(id)) {
            res += (1 << a[option]);
            ++id;
            ++option;
        }
        ++id; // zero bit
        int bits = a[option];
        for (int i = 0; i < bits; ++i, ++id) {
            res += (1 << i) * (deque.at(id));
        }
        return res;
    }

    static void charToBinary(deque<bool>& deque, char c) {
        for (int i = 0; i < 8; ++i) {
            deque.push_back(c % 2 == 1);
            c /= 2;
        }
    }

    double getAmountSize() {
        if (c != 128) {
            return 9;
        } else {
            return (1 + getIntValSize(distance) + getIntValSize(length)) / (double)length;
        }
    }

    int getIntValSize(int val) {
        int option = 0;
        --val;
        while (val > (1L << a[option])) {
            val -= (1 << a[option]);
            ++option;
        }
        return option + 1 + a[option];
    }

    void print() {
        if (c != 128) {
            cout << "char option: " << c << endl;
        } else {
            cout << "second option: distance = " << distance << " length = " << length << endl;
        }
    }

    constexpr static const int a[6] = {1, 2, 4, 8, 16, 32}; // work only for int val

    int distance;
    int length;
    int c = 128; // if c = 128 then distance and length is main else c is main
};

