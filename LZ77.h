#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <unordered_set>
#include <unordered_map>
typedef long long ll;

class LZ77 {
public:
    LZ77();

    void compress(const std::string &st1, const std::string &st2, int s);

    void decompress(const std::string &st1, const std::string &st2);
private:
    std::ifstream mIfs;
    std::ofstream mOfs;
    bool mFail = false;
    const size_t bufferSize = 1024;
    size_t position = 0;
    size_t bufferLimit = 0;
    char *buffer = new char[bufferSize];
    ll mSkipped = 0;
    std::unordered_map<ll, std::unordered_set<ll>> hashMap;
    // min size for matching (from 1 to 8)
    int sizeOfMinSeq = 6;
    ll deletedCharsFromQueue = 0;
    // min bit benefit for triplet
    int minBitsBenefit = 10;
    ll seqVal = 0;
    ll mask = 0;
    ll oldSeqVal = -1;

    // for working with bits
    char readingChar = 0;
    char writingChar = 0;
    int readingId = 8;
    int writingId = 0;

    struct triplet {
        triplet();

        triplet(int, int, int);

        void compress(LZ77&);

        static void compressIntVal(LZ77&, int, int);

        static triplet decompress(LZ77&);

        static int decompressIntVal(LZ77&, int);

        int getBitsBenefit();

        int getSkipped() const;

        int getOffset() const;

        int getLength() const;

        static int a[3][6];

    private:
        int mSkipped;
        int mOffset;
        int mLength;

        int getIntValSize(int, int);
    };

    triplet findMaxMatching(std::deque<char> &, int);

    void fillDequeNBytes(std::deque<char> &, int);

    void deleteDequeNBytes(std::deque<char> &, int);

    void deleteAndWriteDequeNBytes(std::deque<char> &, int);

    long getSeqHashVal(std::deque<char> &q, int id);

    void skippedWrite(std::string&);

    bool hasFailedReading();

    char readOneByte();

    int updateBuffer();

    int getBit();

    void writeBit(int bit);
};

