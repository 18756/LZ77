#include "LZ77.h"
#include <fstream>
#include <queue>
#include <bitset>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <sys/stat.h>

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

triplet LZ77::findMaxMatching(std::deque<char> &q, int index) {
    int pos1, pos2;
    triplet ans(0, 0, 0), other;
    if (index + sizeOfMinSeq <= q.size()) {
        long seqVal = getSeqUniqVal(q, index);
        for (auto itr = hashMap[seqVal].begin(); itr != hashMap[seqVal].end(); ++itr) {
            int i = *itr - deletedCharsFromQueue;
            pos1 = i + sizeOfMinSeq;
            pos2 = index + sizeOfMinSeq;
            /*if (q.at(pos1) != q.at(pos2)) {
                cout << "ERRRRRRRRRRRR" << endl;
            }*/
            while (pos2 < q.size() && q.at(pos1) == q.at(pos2)) {
                ++pos1;
                ++pos2;
            }
            if (pos2 == q.size()) {
                char extraChar = readOneByte();
                while (!hasFailedReading() && q.at(pos1) == extraChar) {
                    ++pos1;
                    q.push_back(extraChar);
                    extraChar = readOneByte();
                }
                if (!hasFailedReading()) {
                    q.push_back(extraChar);
                }
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
    struct stat stat_buf;
    int rc = stat(st1.c_str(), &stat_buf);
    long sizeOfFile = (rc == 0 ? stat_buf.st_size : -1);
    writingChar = 0;
    writingId = 0;
    efficiency = max(sizeOfMinSeq, efficiency);
    double process = 0;
    long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (!mIfs) {
        cout << "Unable to open original file";
        exit(1); // terminate with error
    }

    //input precessing queue
    std::deque<char> q = *(new std::deque<char>());

    //string skipped
    string skippedStr = *(new string());

    hashMap.clear();
    deletedCharsFromQueue = 0;

    //reading first block of bytes
    fillDequeNBytes(q, efficiency);

    //compressed bit portion

    triplet::compressIntVal(*this, efficiency, 0);

    int index = 0;
    while (index < q.size()) {
        int qResize = 0;
        triplet t = findMaxMatching(q, index);
        if (t.getBitsBenefit() <= minBitsBenefit) {
            mSkipped += 1;
            skippedStr.push_back(q.at(index));
            qResize = 1;
        } else {
            //write triplet
            t.compress(*this);
            if (t.getSkipped() > 0) {
                int p = writingId;
                char c;
                for (int i = 0; i < skippedStr.length(); ++i) {
                    if (i == 0) {
                        char a = skippedStr[i] << p;
                        c = (writingChar | a);
                    } else {
                        char b = ((unsigned char) (skippedStr[i - 1]) >> (8 - p));
                        char a = skippedStr[i] << p;
                        c = (a | b);
                    }
                    mOfs << c;
                }

                //put last p bits
                writingChar = ((unsigned char) (skippedStr[skippedStr.length() - 1]) >> (8 - p));

                //clear skippedStr and set mSkipped to 0
                mSkipped = 0;
                skippedStr.clear();
            }

            qResize = t.getLength(); // why it is right
        }

        //update q
        fillDequeNBytes(q, min(qResize, efficiency));
        for (int j = index; j < index + qResize && j + sizeOfMinSeq <= q.size(); ++j) {
            long seqVal = getSeqUniqVal(q, j);
            hashMap[seqVal].insert(j + deletedCharsFromQueue);
        }
        index += qResize;
        if (index > efficiency) {
            deleteDequeNBytes(q, index - efficiency);
            index = efficiency;
        }
        if (((index + deletedCharsFromQueue) / (double)sizeOfFile) * 100 - process > 5) {
            process = ((index + deletedCharsFromQueue) / (double)sizeOfFile) * 100;
            cout << process << "%" << endl;
        }
        mOfs.flush();
    }
    if (mSkipped > 0) {
        triplet(mSkipped, 0, 0).compress(*this);
        //write bitVector and skippedStr to output file
        int p = writingId;
        char c;
        for (int i = 0; i < skippedStr.length(); ++i) {
            if (i == 0) {
                char a = skippedStr[i] << p;
                c = (writingChar | a);
            } else {
                char b = ((unsigned char) (skippedStr[i - 1]) >> (8 - p));
                char a = skippedStr[i] << p;
                c = (a | b);
            }

            mOfs << c;
        }
        writingChar = (unsigned char) (skippedStr[skippedStr.size() - 1]) >> (8 - p);
    }
    mOfs << writingChar;
    q.clear();
    long finish = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cout << "Total compressing time: " << (finish - start) << endl;
    mOfs.flush();
}

void LZ77::decompress(const std::string &st1, const std::string &st2) {
    long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    mIfs = ifstream(st1, ios::binary);
    mOfs = ofstream(st2, ios::binary);
    readingChar = 0;
    readingId = 8;
    if (!mIfs) {
        cout << "Unable to open compressed file";
        exit(1); // terminate with error
    }

    int efficiency = triplet::decompressIntVal(*this, 0);
    std::deque<char> q = *(new std::deque<char>());
    while (true) {
        triplet t = triplet::decompress(*this);
        if (t.getLength() == 0 && t.getSkipped() == 0) {
            break;
        }
        if (t.getSkipped() > 0) {
            int p = readingId;

            char c = 0;
            unsigned char prev = readingChar;
            char next = 0;
            for (int i = 0; i < t.getSkipped(); ++i) {
                next = readOneByte();
                char a = (prev >> p);
                char b = (next << (8 - p));
                c = (a | b);
                prev = next;
                q.push_back(c);
            }
            readingChar = next;
            readingId = p;
        }
        int offset = t.getOffset();
        int length = t.getLength();
        int startId = q.size() - offset;
        for (int j = startId; j < startId + length; ++j) {
            q.push_back(q.at(j));
        }
        if (q.size() > efficiency) {
            deleteAndWriteDequeNBytes(q, q.size() - efficiency);
        }
    }
    deleteAndWriteDequeNBytes(q, q.size());
    long finish = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cout << "Total decompression time: " << (finish - start) << endl;
    mOfs.flush();
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
            long seqVal = getSeqUniqVal(q, 0);
            hashMap[seqVal].erase(deletedCharsFromQueue);
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

long LZ77::getSeqUniqVal(std::deque<char> &q, int id) {
    long res = 0;
    for (int i = id; i < id + sizeOfMinSeq; ++i) {
        res |= ((long)(unsigned char)q.at(i) << (8 * (i - id)));
    }
    return res;
}

triplet::triplet() = default;

triplet::triplet(int skipped, int offset, int length) : mSkipped(skipped), mOffset(offset), mLength(length) {}

void triplet::compress(LZ77 &lz77) { // reversed order of char or mOffset and mLength bits
    compressIntVal(lz77, mSkipped, 0);
    compressIntVal(lz77, mOffset, 1);
    compressIntVal(lz77, mLength, 2);
}

void triplet::compressIntVal(LZ77 &lz77, int val, int encryption) { // reversed order of bits
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

triplet triplet::decompress(LZ77 &lz77) {
    triplet res;
    int skipped = decompressIntVal(lz77, 0);
    int offset = decompressIntVal(lz77, 1);
    int length = decompressIntVal(lz77, 2);
    res = triplet(skipped, offset, length);
    return res;
}

int triplet::decompressIntVal(LZ77 &lz77, int encryption) {
    int res = 0;
    int option = 0;
    while (lz77.getBit()) {
        res += (1 << a[encryption][option++]);
    }
    int bits = a[encryption][option];
    for (int i = 0; i < bits; ++i) {
        res += (1 << i) * lz77.getBit();
    }
    return res;
}

double triplet::getBitsBenefit() {
    return 8 * mLength - (getIntValSize(mOffset, 1) + getIntValSize(mLength, 2));
}

int triplet::getIntValSize(int val, int encryption) {
    int option = 0;
    while (val > (1L << a[encryption][option])) {
        val -= (1 << a[encryption][option]);
        ++option;
    }
    return option + 1 + a[encryption][option];
}

int triplet::getSkipped() const {
    return mSkipped;
}

int triplet::getOffset() const {
    return mOffset;
}

int triplet::getLength() const {
    return mLength;
}

//void triplet::print() {
//    if (c != 128) {
//        cout << "char option: " << c << endl;
//    } else {
//        cout << "second option: mOffset = " << mOffset << " mLength = " << mLength << endl;
//    }
//}

