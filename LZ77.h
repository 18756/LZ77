#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <unordered_set>
#include <unordered_map>

class LZ77;

struct triplet {
    triplet();

    triplet(int, int, int);

    void compress(LZ77 &);

    static void compressIntVal(LZ77 &, int, int);

    static triplet decompress(LZ77 &);

    static int decompressIntVal(LZ77 &, int);

    double getBitsBenefit();

    int getIntValSize(int, int);

    void print();

    int getSkipped() const;

    int getOffset() const;

    int getLength() const;

    constexpr static const int a[3][6] = {{1, 3, 7, 8, 10, 12},
                                          {1, 2, 4, 8, 16, 32},
                                          {1, 2, 4, 8, 16, 32}}; // work only for int val

private:
    int mSkipped;
    int mOffset;
    int mLength;
};

class LZ77 {
public:
    LZ77();

    bool hasFailedReading();

    char readOneByte();

    int updateBuffer();

    void compress(const std::string &st1, const std::string &st2, int s);

    void decompress(const std::string &st1, const std::string &st2);

    int getBit();

    void writeBit(int bit);
private:
    std::ifstream mIfs;
    std::ofstream mOfs;
    bool mFail = false;
    const size_t bufferSize = 1024;
    int position = 0;
    int bufferLimit = 0;
    char *buffer = new char[bufferSize];
    int mSkipped = 0;
    std::unordered_map<long, std::unordered_set<int>> hashMap;
    int sizeOfMinSeq = 6;
    int deletedCharsFromQueue = 0;
    int minBitsBenefit = 10;

    triplet findMaxMatching(std::deque<char> &, int);

    void fillDequeNBytes(std::deque<char> &, int);

    void deleteDequeNBytes(std::deque<char> &, int);

    void deleteAndWriteDequeNBytes(std::deque<char> &, int);

    long getSeqUniqVal(std::deque<char> &, int);

    char readingChar = 0;
    char writingChar = 0;
    int readingId = 8;
    int writingId = 0;

};

