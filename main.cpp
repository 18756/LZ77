#include "LZ77.h"
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <sys/stat.h>

using namespace std;
int main() {
//    (new LZ77())->compress("input.txt", "output.txt", 20000);
//    (new LZ77())->decompress("output.txt", "input2.txt");
//    (new LZ77())->compress("music.mp3", "output.mp3", 20000);
//    (new LZ77())->decompress("output.mp3", "music2.mp3");
    long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::ifstream mIfs("input.txt", ios::binary);
    int c = 0;
    char ch = 0;
    int a = 0;
    while (mIfs) {
        mIfs >> ch;
        c += ch;
        a++;
    }
    long finish = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cout << (finish - start) << endl;
    cout << c;
//    struct stat stat_buf;
//    int rc = stat("input.txt", &stat_buf);
//    cout << (rc == 0 ? stat_buf.st_size : -1);


//    long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    cout << now;
    /*std::unordered_map<int, std::unordered_set<long>> m;
    m[32].insert(1);
    m[32].insert(2);
    m[32].insert(3);
    for (auto itr = m[32].begin(); itr != m[32].end(); ++itr) {
        cout << (*itr) << endl;
    }*/

    return 0;
}

