#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

using namespace std;

// Cache parameters (from textbook)
static const int NUM_LINES = 1024;
static const int BLOCK_WORDS = 4;
static const int BYTE_OFFSET_BITS = 2;
static const int WORD_OFFSET_BITS = 2;
static const int INDEX_BITS = 10;
static const int TAG_BITS = 18;
static const int MEMORY_LATENCY = 4;

// Address field extraction
static const uint32_t INDEX_SHIFT = BYTE_OFFSET_BITS + WORD_OFFSET_BITS;
static const uint32_t TAG_SHIFT = INDEX_SHIFT + INDEX_BITS;

static const uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1;
static const uint32_t TAG_MASK = (1u << TAG_BITS) - 1;

// FSM states
enum class State : uint8_t {
    IDLE,
    COMPARE_TAG,
    WRITE_BACK,
    ALLOCATE
};

const char* stateName(State s) {
    switch (s) {
        case State::IDLE: return "IDLE";
        case State::COMPARE_TAG: return "COMPARE_TAG";
        case State::WRITE_BACK: return "WRITE_BACK";
        case State::ALLOCATE: return "ALLOCATE";
    }
    return "";
}

// Cache line structure
struct CacheLine {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    uint32_t data[BLOCK_WORDS] = {};
};

// CPU request
struct CPURequest {
    bool isWrite;
    uint32_t address;
    uint32_t wdata;
    string label;
};

// Simple memory model with delay
class MainMemory {
    map<uint32_t, uint32_t> store;

public:
    void readBlock(uint32_t blockAddr, uint32_t out[BLOCK_WORDS]) {
        for (int i = 0; i < BLOCK_WORDS; i++) {
            uint32_t addr = blockAddr * BLOCK_WORDS + i;
            out[i] = store.count(addr) ? store[addr] : (0xA0000000u | addr);
        }
    }

    void writeBlock(uint32_t blockAddr, uint32_t in[BLOCK_WORDS]) {
        for (int i = 0; i < BLOCK_WORDS; i++) {
            store[blockAddr * BLOCK_WORDS + i] = in[i];
        }
    }
};

// FSM controller
class CacheController {
    State state = State::IDLE;
    CacheLine cache[NUM_LINES];
    MainMemory mem;

    int memTimer = 0;
    uint32_t buffer[BLOCK_WORDS];

public:
    int getIndex(uint32_t addr) {
        return (addr >> INDEX_SHIFT) & INDEX_MASK;
    }

    uint32_t getTag(uint32_t addr) {
        return (addr >> TAG_SHIFT) & TAG_MASK;
    }

    void run(vector<CPURequest> requests) {
        int cycle = 0;
        int reqIndex = 0;
        bool busy = false;
        CPURequest current;

        while (true) {
            cycle++;
            cout << "\nCycle " << cycle << " | State: " << stateName(state) << endl;

            // Fetch new request when idle
            if (state == State::IDLE && !busy && reqIndex < requests.size()) {
                current = requests[reqIndex++];
                busy = true;
                cout << "Request: " << current.label << endl;
                state = State::COMPARE_TAG;
            }

            switch (state) {

            case State::COMPARE_TAG: {
                int idx = getIndex(current.address);
                uint32_t tag = getTag(current.address);
                CacheLine &line = cache[idx];

                bool hit = line.valid && line.tag == tag;

                if (hit) {
                    cout << "HIT\n";

                    if (current.isWrite) {
                        line.data[0] = current.wdata;
                        line.dirty = true;
                        cout << "Write → dirty set\n";
                    } else {
                        cout << "Read → data = " << line.data[0] << endl;
                    }

                    busy = false;
                    state = State::IDLE;

                } else {
                    cout << "MISS\n";

                    if (line.valid && line.dirty) {
                        state = State::WRITE_BACK;
                    } else {
                        state = State::ALLOCATE;
                    }
                    memTimer = MEMORY_LATENCY;
                }
                break;
            }

            // Write dirty block back to memory
            case State::WRITE_BACK:
                cout << "Write-back...\n";
                if (--memTimer == 0) {
                    int idx = getIndex(current.address);
                    CacheLine &line = cache[idx];

                    uint32_t blockAddr =
                        ((line.tag << INDEX_BITS) | idx);

                    mem.writeBlock(blockAddr, line.data);
                    line.dirty = false;

                    state = State::ALLOCATE;
                    memTimer = MEMORY_LATENCY;
                }
                break;

            // Fetch new block from memory
            case State::ALLOCATE:
                cout << "Allocate...\n";
                if (--memTimer == 0) {
                    int idx = getIndex(current.address);
                    uint32_t tag = getTag(current.address);

                    uint32_t blockAddr =
                        current.address >> (BYTE_OFFSET_BITS + WORD_OFFSET_BITS);

                    mem.readBlock(blockAddr, buffer);

                    CacheLine &line = cache[idx];
                    line.valid = true;
                    line.dirty = false;
                    line.tag = tag;

                    for (int i = 0; i < BLOCK_WORDS; i++) {
                        line.data[i] = buffer[i];
                    }

                    state = State::COMPARE_TAG;
                }
                break;

            default:
                break;
            }

            if (!busy && reqIndex >= requests.size() && state == State::IDLE)
                break;
        }
    }
};

int main() {
    vector<CPURequest> tests = {
        {false, 0x00001230, 0, "TC1: Read Miss"},
        {false, 0x00001230, 0, "TC2: Read Hit"},
        {true,  0x00001238, 42, "TC3: Write Hit"},
        {false, 0x00002230, 0, "TC4: Dirty Miss"}
    };

    CacheController cache;
    cache.run(tests);

    return 0;
}
