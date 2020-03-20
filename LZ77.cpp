#include "LZ77.h"
#include <fstream>
#include <queue>
#include <bitset>
#include <unordered_set>
#include <chrono>
#include <sys/stat.h>
#include <cmath>

using namespace std;

LZ77::LZ77() = default;

bool LZ77::hasFailedReading() {
    return this->mFail;
}

//returns number of symbols read
int LZ77::updateBuffer() {
    if (mIfs) {
        mIfs.read(buffer, bufferSize);
        position = 0;
        return mIfs.gcount();
    } else {
        return 0;
    }
}

//tries to read one byte from buffer
//updates buffer if needed
char LZ77::readOneByte() {
    if (position >= bufferLimit) {
        bufferLimit = updateBuffer();
    }
    if (position >= bufferLimit) {
        mFail = true;
        return 0;
    } else {
        return buffer[position++];
    }
}

// finding char sequences matching with next chars
LZ77::triplet LZ77::findMaxMatching(std::deque<char> &q, int index) {
    int pos1, pos2;
    triplet ans(0, 0, 0), other;
    if (index + sizeOfMinSeq <= q.size()) {
        // get hash value for next char sequence
        ll seqVal = getSeqHashVal(q, index);
        for (auto itr = hashMap[seqVal].begin(); itr != hashMap[seqVal].end(); ++itr) {
            // char id in queue is char id in file - deletedCharsFromQueue
            int i = *itr - deletedCharsFromQueue;
            pos1 = i + sizeOfMinSeq;
            pos2 = index + sizeOfMinSeq;
            while (pos2 < q.size() && q.at(pos1) == q.at(pos2)) {
                ++pos1;
                ++pos2;
            }
            other = triplet(mSkipped, index - i, pos1 - i);
            if (ans.getBitsBenefit() < other.getBitsBenefit()) {
                ans = other;
            }
        }
    }
    return ans;
}


void LZ77::compress(const string &st1, const string &st2, int efficiency) {
    mIfs = ifstream(st1, ios::binary);
    mOfs = ofstream(st2, ios::binary);


    if (!mIfs) {
        cout << "Unable to open original file";
        exit(1); // terminate with error
    }

    if (efficiency < 0 || 100 < efficiency) {
        cout << "Efficiency should be from 0 to 100";
        exit(1); // terminate with error
    }

    struct stat stat_buf;
    int rc = stat(st1.c_str(), &stat_buf);
    long sizeOfFile = (rc == 0 ? stat_buf.st_size : -1);
    writingChar = 0;
    writingId = 0;
    efficiency = max(sizeOfMinSeq, (100 - efficiency) * 200);
    double process = 0;
    ll start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    mFail = false;
    position = 0;
    bufferLimit = 0;
    mSkipped = 0;
    deletedCharsFromQueue = 0;

    //input precessing queue
    std::deque<char>* qPtr = new std::deque<char>();
    std::deque<char> q = *(qPtr);

    //string skipped
    string* skippedStrPtr = new string();
    string skippedStr = *(skippedStrPtr);

    hashMap.clear();
    deletedCharsFromQueue = 0;

    //reading first block of bytes
    fillDequeNBytes(q, efficiency);

    //compressed bit portion
    triplet::compressIntVal(*this, efficiency, 0);

    triplet t(0, 0, 0);

    seqVal = getSeqHashVal(q, 0);
    seqVal >>= 8;
    mask = (1L << (8 * sizeOfMinSeq)) - 1;
    oldSeqVal = seqVal;

    int index = 0;
    while (index < q.size()) {
        int qResize = 0;
        triplet t = findMaxMatching(q, index);

        if (t.getBitsBenefit() < minBitsBenefit) {
            mSkipped += 1;
            skippedStr.push_back(q.at(index));
            qResize = 1;
        } else {
            //write triplet
            t.compress(*this);
            if (mSkipped > 0) {
                skippedWrite(skippedStr);
            }
            qResize = t.getLength();
        }

        //update q
        fillDequeNBytes(q, qResize);
        // adding new sequences to hash map
        for (int j = index; j < index + qResize && j + sizeOfMinSeq <= q.size(); ++j) {
            seqVal <<= 8;
            seqVal &= mask;
            seqVal |= (unsigned char)q.at(j + sizeOfMinSeq - 1);
            // real char id in file is id in queue + deletedCharsFromQueue
            hashMap[seqVal].insert(j + deletedCharsFromQueue);
        }
        index += qResize;
        if (index > efficiency) {
            deleteDequeNBytes(q, index - efficiency);
            index = efficiency;
        }
        // print each 5 percents
        if (((index + deletedCharsFromQueue) / (double) sizeOfFile) * 100 - process > 5) {
            process = (((index + deletedCharsFromQueue) / (double) sizeOfFile) * 100);
            process -= std::fmod(process, 5);
            cout << process << "%" << endl;
        }
    }
    if (mSkipped > 0) {
        triplet(mSkipped, 0, 0).compress(*this);
        skippedWrite(skippedStr);
    }
    mOfs << writingChar;
    q.clear();
    hashMap.clear();
    delete qPtr;
    delete skippedStrPtr;
    mOfs.flush();
    int rc2 = stat(st2.c_str(), &stat_buf);
    ll compressedSize = (rc2 == 0 ? stat_buf.st_size : -1);
    cout << "Compressing(compressedSize / inputSize): " << (compressedSize / (double) sizeOfFile * 100) << "%" << endl;
    ll finish = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    cout << "Total compressing time: " << (finish - start) << endl;
    mIfs.close();
    mOfs.close();
    delete [] buffer;
}

void LZ77::skippedWrite(std::string &skippedStr) {
    int p = writingId;
    char c;
    for (int i = 0; i < skippedStr.length(); ++i) {
        c = (i == 0 ? writingChar : (unsigned char) (skippedStr[i - 1]) >> (8 - p)) | (skippedStr[i] << p);
        mOfs << c;
    }
    //put last p bits
    writingChar = ((unsigned char) (skippedStr[skippedStr.length() - 1]) >> (8 - p));

    //clear skippedStr and set mSkipped to 0
    mSkipped = 0;
    skippedStr.clear();
}

void LZ77::decompress(const std::string &st1, const std::string &st2) {
    ll start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    mIfs = ifstream(st1, ios::binary);
    mOfs = ofstream(st2, ios::binary);

    if (!mIfs) {
        cout << "Unable to open compressed file";
        exit(1); // terminate with error
    }

    readingChar = 0;
    readingId = 8;
    int efficiency = triplet::decompressIntVal(*this, 0);
    std::deque<char>* qPtr = new std::deque<char>();
    std::deque<char> q = *(qPtr);

    while (true) {
        triplet t = triplet::decompress(*this);
        if (t.getLength() == 0 && t.getSkipped() == 0) {
            // triplets are off
            break;
        }
        if (t.getSkipped() > 0) {
            int p = readingId;
            char c = 0;
            unsigned char prev = readingChar;
            char next = 0;
            for (int i = 0; i < t.getSkipped(); ++i) {
                next = readOneByte();
                c = ((prev >> p) | (next << (8 - p)));
                prev = next;
                q.push_back(c);
            }
            readingChar = next;
            readingId = p;
        }
        int offset = t.getOffset();
        int length = t.getLength();
        int startId = q.size() - offset;
        // coping match char sequence
        for (int j = startId; j < startId + length; ++j) {
            q.push_back(q.at(j));
        }
        if (q.size() > efficiency) {
            deleteAndWriteDequeNBytes(q, q.size() - efficiency);
        }
    }
    deleteAndWriteDequeNBytes(q, q.size());
    ll finish = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    cout << "Total decompression time: " << (finish - start) << endl;
    mOfs.flush();
    delete qPtr;
}

void LZ77::fillDequeNBytes(std::deque<char> &q, int n) {
    char c;
    for (int i = 0; i < n; ++i) {
        c = readOneByte();
        if (hasFailedReading()) {
            break;
        }
        q.emplace_back(c);
    }
}

void LZ77::deleteDequeNBytes(std::deque<char> &q, int n) {
    for (int i = 0; i < n && !q.empty(); ++i) {
        if (q.size() >= sizeOfMinSeq) {
            oldSeqVal <<= 8;
            oldSeqVal &= mask;
            oldSeqVal |= (unsigned char)q.at(sizeOfMinSeq - 1);
            hashMap[oldSeqVal].erase(deletedCharsFromQueue);
        }
        q.pop_front();
        deletedCharsFromQueue++;
    }
}

void LZ77::deleteAndWriteDequeNBytes(std::deque<char> &q, int n) {
    for (int i = 0; i < n && !q.empty(); ++i) {
        mOfs << q.front();
        q.pop_front();
        deletedCharsFromQueue++;
    }
}


int LZ77::getBit() {
    if (readingId >= 8) {
        readingChar = readOneByte();
        if (hasFailedReading()) {
            return 0;
        }
        readingId = 0;
    }
    return (readingChar >> readingId++) & 1;
}

void LZ77::writeBit(int bit) {
    writingChar |= (bit << writingId++);
    if (writingId == 8) {
        mOfs << writingChar;
        writingChar = 0;
        writingId = 0;
    }
}

// hash value for char sequence
long LZ77::getSeqHashVal(std::deque<char> &q, int id) {
    ll res = 0;
    for (int i = id; i < id + sizeOfMinSeq; ++i) {
        res <<= 8;
        res |= ((unsigned char) q.at(i));
    }
    return res;
}

LZ77::triplet::triplet() = default;

LZ77::triplet::triplet(int skipped, int offset, int length) : mSkipped(skipped), mOffset(offset), mLength(length) {}

void LZ77::triplet::compress(LZ77& lz77) { // reversed order of char or mOffset and mLength bits
    compressIntVal(lz77, mSkipped, 0);
    compressIntVal(lz77, mOffset, 1);
    compressIntVal(lz77, mLength, 2);
}

void LZ77::triplet::compressIntVal(LZ77& lz77, int val, int encryption) { // reversed order of bits
    int option = 0;
    while (val >= (1L << a[encryption][option])) {
        val -= (1 << a[encryption][option]);
        ++option;
    }
    for (int j = 0; j < option; ++j) {
        lz77.writeBit(1);
    }
    lz77.writeBit(0);
    int bits = a[encryption][option];
    for (int i = 0; i < bits; ++i) {
        lz77.writeBit(val % 2);
        val /= 2;
    }
}

LZ77::triplet LZ77::triplet::decompress(LZ77& lz77) {
    int skipped = decompressIntVal(lz77, 0);
    int offset = decompressIntVal(lz77, 1);
    int length = decompressIntVal(lz77, 2);
    return triplet(skipped, offset, length);
}

int LZ77::triplet::decompressIntVal(LZ77& lz77, int encryption) {
    int res = 0;
    int option = 0;
    while (lz77.getBit()) {
        res += (1 << a[encryption][option++]);
    }
    int bits = a[encryption][option];
    for (int i = 0; i < bits; ++i) {
        res += (lz77.getBit() << i);
    }
    return res;
}

int LZ77::triplet::getBitsBenefit() {
    return 8 * mLength - (getIntValSize(mOffset, 1) + getIntValSize(mLength, 2));
}

int LZ77::triplet::getIntValSize(int val, int encryption) {
    int option = 0;
    while (val > (1 << a[encryption][option])) {
        val -= (1 << a[encryption][option]);
        ++option;
    }
    return option + 1 + a[encryption][option];
}


int LZ77::triplet::getSkipped() const {
    return mSkipped;
}

int LZ77::triplet::getOffset() const {
    return mOffset;
}

int LZ77::triplet::getLength() const {
    return mLength;
}

int LZ77::triplet::a[3][6] = {{2, 6, 8, 10, 16, 31},
                              {4, 6, 8, 12, 16, 31},
                              {2, 4, 6, 8, 14, 24}}; // works only for int val (31 is max degree)
