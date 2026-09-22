#include<iostream>
#include<vector>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<queue>
#include<fstream>

using namespace std;

struct pixfreq {
    int pix;
    float freq;
    pixfreq *left = nullptr;
    pixfreq *right = nullptr;
    string code = "";
};

struct huffcode {
    int pix;
    int arrloc;
    float freq;
};

bool compareFrequencies(const huffcode& a, const huffcode& b) {
    return a.freq > b.freq;
}

struct CompareNodes {
    bool operator()(pixfreq* a, pixfreq* b) {
        return a->freq > b->freq;
    }
};

int main() {
    string filename = "image.bin";
    string outputfile = "compressed.bin";

    
    ifstream file(filename, ios::binary | ios::ate);
    if (!file) {
        cout << "Error opening " << filename << "\n";
        return 1;
    }
    
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);
    
    vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        cout << "Error reading file \n";
        return 1;
    }
    file.close();

    unordered_map<unsigned char, int> hist;
    for (unsigned char b : buffer) {
        hist[b]++;
    }
    
    int nodes = hist.size();
    int totalnodes = 2 * nodes - 1;

    vector<pixfreq> byte_freq(totalnodes);
    vector<huffcode> huffcodes(nodes);

    int idx = 0;
    for (auto const& pair : hist) {
        unsigned char byte_val = pair.first;
        int count = pair.second;
        huffcodes[idx] = {byte_val, idx, (float)count / size};
        byte_freq[idx].pix = byte_val;
        byte_freq[idx].freq = (float)count / size;
        idx++;
    }
    
    sort(huffcodes.begin(), huffcodes.end(), compareFrequencies);
    
    int n = 0;
    int nextnode = nodes;
    
    priority_queue<pixfreq*, vector<pixfreq*>, CompareNodes> minHeap;
    for (int i = 0; i < nodes; i++) {
        minHeap.push(&byte_freq[i]);
    }
     
    while (minHeap.size() > 1) {
        pixfreq* leftNode = minHeap.top();  minHeap.pop();
        pixfreq* rightNode = minHeap.top(); minHeap.pop();

        byte_freq[nextnode].pix = 0;
        byte_freq[nextnode].freq = leftNode->freq + rightNode->freq;
        byte_freq[nextnode].left = leftNode;
        byte_freq[nextnode].right = rightNode;

        minHeap.push(&byte_freq[nextnode]);
        nextnode++;
    }
     
    for (int i = totalnodes - 1; i >= nodes; i--) {
        if (byte_freq[i].left != nullptr)
            byte_freq[i].left->code = byte_freq[i].code + "0"; 
        if (byte_freq[i].right != nullptr)
            byte_freq[i].right->code = byte_freq[i].code + "1";
    }
    
    unordered_map<unsigned char, string> code_map;
    for (int m = 0; m < nodes; m++) {
        code_map[byte_freq[m].pix] = byte_freq[m].code;
    }
    
    cout << "Running Compression on .bin file...\n";
    ofstream out(outputfile, ios::binary);
    if (out) {
        unsigned char bit_buffer = 0;
        int bits_in_buffer = 0;

        for (unsigned char b : buffer) {
            string bit_str = code_map[b];

            for (char bit : bit_str) {
                bit_buffer <<= 1;
                if (bit == '1') bit_buffer |= 1;
                bits_in_buffer++;

                if (bits_in_buffer == 8) {
                    out.write(reinterpret_cast<char*>(&bit_buffer), 1);
                    bit_buffer = 0;
                    bits_in_buffer = 0;
                }
            }
        }
        
        if (bits_in_buffer > 0) {
            bit_buffer <<= (8 - bits_in_buffer);
            out.write(reinterpret_cast<char*>(&bit_buffer), 1);
        }
        out.close();
        cout << " -> SUCCESS: new file ->'compressed.bin'.\n";
    }
    return 0;
}
