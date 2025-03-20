//
// Created by Ali Hamza Azam on 03/03/2025.
//

#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <complex>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <map>
#include <cctype>

using namespace std;

// Data structure for Needleman-Wunsch result
struct NWResult {
    std::string aligned_A;
    std::string aligned_B;
    double score{};
};

#define MAT_SIZE 23
#define GAP_OPEN 5.5 // gap open penalty
#define GAP_EXT  2.0 // gap extension penalty
#define MINF     -1e9


// Substitution matrix for amino acids
int gon250mt[23][23] = {
    {  24,  -6,  -3,  -3,   5,  -2,   0,   5,  -8,  -8, -12,  -4,  -7, -23,   3,  11,   6, -36, -22,   1,   0,   0,   0}, // A
    {  -6,  47,   3,  -3, -22,  15,   4, -10,   6, -24, -22,  27, -17, -32,  -9,  -2,  -2, -16, -18, -20,   0,   0,   0}, // R
    {  -3,   3,  38,  22, -18,   7,   9,   4,  12, -28, -30,   8, -22, -31,  -9,   9,   5, -36, -14, -22,   0,   0,   0}, // N
    {  -3,  -3,  22,  47, -32,   9,  27,   1,   4, -38, -40,   5, -30, -45,  -7,   5,   0, -52, -28, -29,   0,   0,   0}, // D
    {   5, -22, -18, -32, 115, -24, -30, -20, -13, -11, -15, -28,  -9,  -8, -31,   1,  -5, -10,  -5,   0,   0,   0,   0}, // C
    {  -2,  15,   7,   9, -24,  27,  17, -10,  12, -19, -16,  15, -10, -26,  -2,   2,   0, -27, -17, -15,   0,   0,   0}, // Q
    {   0,   4,   9,  27, -30,  17,  36,  -8,   4, -27, -28,  12, -20, -39,  -5,   2,  -1, -43, -27, -19,   0,   0,   0}, // E
    {   5, -10,   4,   1, -20, -10,  -8,  66, -14, -45, -44, -11, -35, -52, -16,   4, -11, -40, -40, -33,   0,   0,   0}, // G
    {  -8,   6,  12,   4, -13,  12,   4, -14,  60, -22, -19,   6, -13,  -1, -11,  -2,  -3,  -8,  22, -20,   0,   0,   0}, // H
    {  -8, -24, -28, -38, -11, -19, -27, -45, -22,  40,  28, -21,  25,  10, -26, -18,  -6, -18,  -7,  31,   0,   0,   0}, // I
    { -12, -22, -30, -40, -15, -16, -28, -44, -19,  28,  40, -21,  28,  20, -23, -21, -13,  -7,   0,  18,   0,   0,   0}, // L
    {  -4,  27,   8,   5, -28,  15,  12, -11,   6, -21, -21,  32, -14, -33,  -6,   1,   1, -35, -21, -17,   0,   0,   0}, // K
    {  -7, -17, -22, -30,  -9, -10, -20, -35, -13,  25,  28, -14,  43,  16, -24, -14,  -6, -10,  -2,  16,   0,   0,   0}, // M
    { -23, -32, -31, -45,  -8, -26, -39, -52,  -1,  10,  20, -33,  16,  70, -38, -28, -22,  36,  51,   1,   0,   0,   0}, // F
    {   3,  -9,  -9,  -7, -31,  -2,  -5, -16, -11, -26, -23,  -6, -24, -38,  76,   4,   1, -50, -31, -18,   0,   0,   0}, // P
    {  11,  -2,   9,   5,   1,   2,   2,   4,  -2, -18, -21,   1, -14, -28,   4,  22,  15, -33, -19, -10,   0,   0,   0}, // S
    {   6,  -2,   5,   0,  -5,   0,  -1, -11,  -3,  -6, -13,   1,  -6, -22,   1,  15,  25, -35, -19,   0,   0,   0,   0}, // T
    { -36, -16, -36, -52, -10, -27, -43, -40,  -8, -18,  -7, -35, -10,  36, -50, -33, -35, 142,  41, -26,   0,   0,   0}, // W
    { -22, -18, -14, -28,  -5, -17, -27, -40,  22,  -7,   0, -21,  -2,  51, -31, -19, -19,  41,  78, -11,   0,   0,   0}, // Y
    {   1, -20, -22, -29,   0, -15, -19, -33, -20,  31,  18, -17,  16,   1, -18, -10,   0, -26, -11,  34,   0,   0,   0}, // V
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // B
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // Z
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}  // X
};

// Mapping for protein characters
const unordered_map<char,int> protein_index = {
    {'A',0}, {'R',1}, {'N',2}, {'D',3}, {'C',4}, {'Q',5}, {'E',6},
    {'G',7}, {'H',8}, {'I',9}, {'L',10}, {'K',11}, {'M',12}, {'F',13},
    {'P',14}, {'S',15}, {'T',16}, {'W',17}, {'Y',18}, {'V',19},
    {'B',20}, {'Z',21}, {'X',22}
};

// Needleman–Wunsch alignment with affine gap penalties
NWResult needleman_wunsch(const std::string &s1, const std::string &s2) {
    // Convert sequences to uppercase; substitute non-standard chars with 'X'
    std::string s1_up = s1, s2_up = s2;
    for (char &c : s1_up) {
        c = std::toupper(c);
        if (protein_index.find(c) == protein_index.end())
            c = 'X';
    }
    for (char &c : s2_up) {
        c = std::toupper(c);
        if (protein_index.find(c) == protein_index.end())
            c = 'X';
    }

    int n = s1_up.size();
    int m = s2_up.size();

    // Create DP matrices for match (M), gap in s2 (Ix) and gap in s1 (Iy)
    std::vector<std::vector<double>> M(n+1, std::vector<double>(m+1, MINF));
    std::vector<std::vector<double>> Ix(n+1, std::vector<double>(m+1, MINF));
    std::vector<std::vector<double>> Iy(n+1, std::vector<double>(m+1, MINF));

    // Traceback matrices: 0 = from M, 1 = from Ix, 2 = from Iy
    std::vector<std::vector<int>> ptrM(n+1, std::vector<int>(m+1, -1));
    std::vector<std::vector<int>> ptrIx(n+1, std::vector<int>(m+1, -1));
    std::vector<std::vector<int>> ptrIy(n+1, std::vector<int>(m+1, -1));

    // Initialization
    M[0][0] = 0;
    Ix[0][0] = Iy[0][0] = MINF;

    // Use reduced penalty for end gaps
    double end_gap_penalty = GAP_EXT;

    for (int i = 1; i <= n; i++) {
        M[i][0] = MINF;
        Ix[i][0] = -((i == n ? end_gap_penalty : GAP_OPEN) + (i-1)*GAP_EXT);
        Iy[i][0] = MINF;
        ptrIx[i][0] = 1; // came from Ix
    }
    for (int j = 1; j <= m; j++) {
        M[0][j] = MINF;
        Iy[0][j] = -((j == m ? end_gap_penalty : GAP_OPEN) + (j-1)*GAP_EXT);
        Ix[0][j] = MINF;
        ptrIy[0][j] = 2; // came from Iy
    }

    // Fill matrices
    for (int i = 1; i <= n; i++){
        for (int j = 1; j <= m; j++){
            // Get substitution score
            int idx1 = protein_index.at(s1_up[i-1]);
            int idx2 = protein_index.at(s2_up[j-1]);
            double score = gon250mt[idx1][idx2] / 10.0; // Normalize score

            // Calculate M[i][j]
            double from_M = M[i-1][j-1];
            double from_Ix = Ix[i-1][j-1];
            double from_Iy = Iy[i-1][j-1];
            int from_state = 0;
            double max_prev = from_M;
            if (from_Ix > max_prev) { max_prev = from_Ix; from_state = 1; }
            if (from_Iy > max_prev) { max_prev = from_Iy; from_state = 2; }
            M[i][j] = max_prev + score;
            ptrM[i][j] = from_state;

            // Adjust gap penalties for terminal gaps
            double gap_open_penalty = GAP_OPEN;
            double gap_ext_penalty = GAP_EXT;

            if (i == n || j == m) {
                gap_open_penalty = end_gap_penalty;
            }

            // Calculate Ix[i][j] = gap in s2
            double cand1 = M[i-1][j] - (gap_open_penalty + gap_ext_penalty);
            if (double cand2 = Ix[i-1][j] - gap_ext_penalty; cand1 >= cand2) {
                Ix[i][j] = cand1;
                ptrIx[i][j] = 0; // from M
            } else {
                Ix[i][j] = cand2;
                ptrIx[i][j] = 1; // from Ix
            }

            // Calculate Iy[i][j] = gap in s1
            double cand3 = M[i][j-1] - (gap_open_penalty + gap_ext_penalty);
            if (double cand4 = Iy[i][j-1] - gap_ext_penalty; cand3 >= cand4) {
                Iy[i][j] = cand3;
                ptrIy[i][j] = 0; // from M
            } else {
                Iy[i][j] = cand4;
                ptrIy[i][j] = 2; // from Iy
            }
        }
    }

    // Choose best score from M, Ix, Iy
    double final_score = M[n][m];
    int state = 0; // 0: M, 1: Ix, 2: Iy
    if (Ix[n][m] > final_score) { final_score = Ix[n][m]; state = 1; }
    if (Iy[n][m] > final_score) { final_score = Iy[n][m]; state = 2; }

    // Backtracking
    int i = n, j = m;
    std::string aligned_A, aligned_B;
    while(i > 0 || j > 0) {
        if(state == 0) { // from M
            int prev = ptrM[i][j];
            aligned_A.push_back(s1_up[i-1]);
            aligned_B.push_back(s2_up[j-1]);
            i--; j--;
            state = prev;
        } else if(state == 1) { // from Ix
            int prev = ptrIx[i][j];
            aligned_A.push_back(s1_up[i-1]);
            aligned_B.push_back('-');
            i--;
            state = (prev == 0 ? 0 : 1);
        } else { // from Iy
            int prev = ptrIy[i][j];
            aligned_A.push_back('-');
            aligned_B.push_back(s2_up[j-1]);
            j--;
            state = (prev == 0 ? 0 : 2);
        }
    }
    reverse(aligned_A.begin(), aligned_A.end());
    reverse(aligned_B.begin(), aligned_B.end());

    NWResult result;
    result.aligned_A = aligned_A;
    result.aligned_B = aligned_B;
    result.score = final_score;
    return result;
}

// Read sequences from a FASTA file
vector<pair<string,string>> read_fasta(const string &filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error: Cannot open file: " << filename << endl;
        return {};
    }
    vector<pair<string,string>> records;
    string line, header, seq;
    while(getline(infile, line)) {
        if(line.empty()) continue;
        if(line[0] == '>') {
            if(!seq.empty()){
                records.emplace_back(header, seq);
                seq.clear();
            }
            header = line.substr(1);
        } else {
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            seq += line;
        }
    }
    if(!seq.empty())
        records.emplace_back(header, seq);
    return records;
}

// Get next power of 2 for FFT
static inline size_t next_power_of_two(const size_t n) {
    size_t p = 1;
    while(p < n) p <<= 1;
    return p;
}

// Fast Fourier Transform implementation
void fft(vector<complex<double>> &data, const bool inverse = false) {
    size_t n = data.size();
    if(n <= 1) return;
    size_t j = 0;
    for(size_t i = 1; i < n; i++){
        size_t bit = n >> 1;
        for(; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if(i < j)
            swap(data[i], data[j]);
    }
    for(size_t len = 2; len <= n; len <<= 1){
        const double theta = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        complex<double> wlen(cos(theta), sin(theta));
        for(size_t i = 0; i < n; i += len){
            complex<double> w(1.0, 0.0);
            for(size_t k = 0; k < len/2; k++){
                complex<double> u = data[i+k];
                complex<double> t = w * data[i+k+len/2];
                data[i+k] = u + t;
                data[i+k+len/2] = u - t;
                w *= wlen;
            }
        }
    }
    if(inverse){
        for(size_t i = 0; i < n; i++){
            data[i] /= n;
        }
    }
}

// Convert sequence to numeric vector for FFT
vector<int> seq_to_numeric(const string &seq, const unordered_map<char,int> &charToIndex) {
    vector<int> numeric(seq.size());
    for(size_t i = 0; i < seq.size(); i++){
        numeric[i] = charToIndex.at(seq[i]);
    }
    return numeric;
}

// Compute FFT transform with padding
vector<complex<double>> fft_transform(const vector<int> &numericSeq, const size_t commonLength) {
    vector<complex<double>> data(commonLength, complex<double>(0.0,0.0));
    for(size_t i = 0; i < numericSeq.size(); i++){
        data[i] = complex<double>(numericSeq[i], 0.0);
    }
    fft(data, false);
    return data;
}

// Calculate Pearson correlation
double pearson_correlation(const vector<double> &x, const vector<double> &y) {
    if(x.size() != y.size() || x.empty()) return 0.0;
    double mean_x = 0.0, mean_y = 0.0;
    for(size_t i = 0; i < x.size(); i++){
        mean_x += x[i];
        mean_y += y[i];
    }
    mean_x /= x.size();
    mean_y /= y.size();
    double numerator = 0.0, denom_x = 0.0, denom_y = 0.0;
    for(size_t i = 0; i < x.size(); i++){
        const double dx = x[i] - mean_x;
        const double dy = y[i] - mean_y;
        numerator += dx * dy;
        denom_x += dx*dx;
        denom_y += dy*dy;
    }
    const double denominator = sqrt(denom_x*denom_y);
    if(denominator < 1e-12) return 0.0;
    return numerator/denominator;
}

// Compute correlation from FFT results
double fft_correlation(const vector<complex<double>> &fft_A, const vector<complex<double>> &fft_B) {
    vector<double> realA(fft_A.size()), realB(fft_B.size());
    for(size_t i = 0; i < fft_A.size(); i++){
        realA[i] = fft_A[i].real();
        realB[i] = fft_B[i].real();
    }
    return pearson_correlation(realA, realB);
}

// Build distance matrix using FFT correlation
vector<vector<double>> construct_distance_matrix(const vector<string> &seqs) {
    unordered_map<char,int> char_to_index;
    string chars;
    for(const auto &s : seqs)
        chars += s;
    sort(chars.begin(), chars.end());
    chars.erase(unique(chars.begin(), chars.end()), chars.end());
    int idx = 0;
    for(char c : chars)
        char_to_index[c] = idx++;

    size_t max_seq_len = 0;
    for(const auto &s : seqs)
        if(s.size() > max_seq_len) max_seq_len = s.size();
    const size_t common_pad = next_power_of_two(max_seq_len);

    vector<vector<complex<double>>> fftList(seqs.size());
    for(size_t i = 0; i < seqs.size(); i++){
        vector<int> numeric = seq_to_numeric(seqs[i], char_to_index);
        fftList[i] = fft_transform(numeric, common_pad);
    }
    const size_t n = seqs.size();
    vector<vector<double>> distMatrix(n, vector<double>(n, 0.0));
    for(size_t i = 0; i < n; i++){
        for(size_t j = i+1; j < n; j++){
            const double corr = fft_correlation(fftList[i], fftList[j]);
            const double dist = 1.0 - corr;
            distMatrix[i][j] = dist;
            distMatrix[j][i] = dist;
        }
    }
    return distMatrix;
}

// Simple wrapper for pairwise alignment
NWResult align_pair(const string &s1, const string &s2) {
    return needleman_wunsch(s1, s2);
}

// Profile structure for multiple sequence alignment
struct Profile {
    vector<string> headers;
    vector<string> sequences;
};

// Align two profiles by propagating gap patterns
Profile align_profiles(const Profile &pA, const Profile &pB) {
    // Use first sequence of each profile as representative
    NWResult nw = align_pair(pA.sequences[0], pB.sequences[0]);
    Profile merged;
    // Propagate alignment to profile A sequences
    for(size_t i = 0; i < pA.sequences.size(); i++){
        string orig = pA.sequences[i];
        string new_seq;
        size_t pos = 0;
        for(char c : nw.aligned_A) {
            if(c == '-') new_seq.push_back('-');
            else { new_seq.push_back(orig[pos]); pos++; }
        }
        merged.headers.push_back(pA.headers[i]);
        merged.sequences.push_back(new_seq);
    }
    // Propagate alignment to profile B sequences
    for(size_t i = 0; i < pB.sequences.size(); i++){
        string orig = pB.sequences[i];
        string new_seq;
        size_t pos = 0;
        for(char c : nw.aligned_B) {
            if(c == '-') new_seq.push_back('-');
            else { new_seq.push_back(orig[pos]); pos++; }
        }
        merged.headers.push_back(pB.headers[i]);
        merged.sequences.push_back(new_seq);
    }
    return merged;
}

// Progressive alignment using UPGMA clustering
Profile progressive_alignment(const vector<pair<string,string>> &records, const vector<vector<double>> &dist_matrix) {
    const size_t n = records.size();
    // Initialize one profile per sequence
    map<int, Profile> profiles;
    for (size_t i = 0; i < n; i++){
        Profile p;
        p.headers.push_back(records[i].first);
        p.sequences.push_back(records[i].second);
        profiles[i] = p;
    }
    int nextID = n;
    map<int, vector<int>> clusters;
    for (size_t i = 0; i < n; i++){
        clusters[i] = {static_cast<int>(i)};
    }
    // UPGMA merging loop
    while (clusters.size() > 1) {
        double min_dist = numeric_limits<double>::infinity();
        int bestA = -1, bestB = -1;
        for(auto itA = clusters.begin(); itA != clusters.end(); ++itA){
            for(auto itB = next(itA); itB != clusters.end(); ++itB){
                double sum_dist = 0.0;
                int count = 0;
                for (const int memA : itA->second) {
                    for (const int memB : itB->second) {
                        int i = memA, j = memB;
                        if(i > j) swap(i, j);
                        sum_dist += dist_matrix[i][j];
                        count++;
                    }
                }
                if(const double avgDist = sum_dist / count; avgDist < min_dist){
                    min_dist = avgDist;
                    bestA = itA->first;
                    bestB = itB->first;
                }
            }
        }
        // Merge profiles for bestA and bestB
        const Profile merged_profile = align_profiles(profiles[bestA], profiles[bestB]);
        vector<int> new_members = clusters[bestA];
        new_members.insert(new_members.end(), clusters[bestB].begin(), clusters[bestB].end());
        clusters[nextID] = new_members;
        profiles[nextID] = merged_profile;
        clusters.erase(bestA);
        clusters.erase(bestB);
        profiles.erase(bestA);
        profiles.erase(bestB);
        nextID++;
    }
    return profiles.begin()->second;
}

// Print alignment in blocks with formatting
void print_alignment(const vector<string> &headers, const vector<string> &alignment, const size_t block_size = 50) {
    if(alignment.empty()) return;
    const size_t alnLength = alignment[0].size();

    // Right-pad sequences with dots (instead of gaps) for MSF format
    vector<string> padded_alignment = alignment;
    for (auto &seq : padded_alignment) {
        if (const size_t last_non_gap = seq.find_last_not_of('-'); last_non_gap != string::npos && last_non_gap + 1 < seq.size()) {
            for (size_t i = last_non_gap + 1; i < seq.size(); i++) {
                if (seq[i] == '-') {
                    seq[i] = '.';
                }
            }
        }
    }

    // Print in blocks
    for(size_t start = 0; start < alnLength; start += block_size) {
        const size_t end = min(start + block_size, alnLength);
        for(size_t i = 0; i < alignment.size(); i++){
            cout << headers[i];
            if(headers[i].size() < 10)
                cout << string(10 - headers[i].size(), ' ');

            string seq = padded_alignment[i].substr(start, end - start);
            replace(seq.begin(), seq.end(), '-', '.');

            string spaced;
            for (size_t j = 0; j < seq.size(); j++) {
                if (j > 0 && j % 10 == 0) {
                    spaced += ' ';
                }
                spaced += seq[j];
            }

            cout << " " << spaced << "\n";
        }
        cout << "\n";
    }
}
#endif //UTILITY_H
