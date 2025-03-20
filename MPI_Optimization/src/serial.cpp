//
// Created by Ali Hamza Azam on 02/03/2025.
//

#include "utility.h"

// Main function
int main(const int argc, char* argv[]){
    if(argc < 2){
        cerr << "Usage: " << argv[0] << " <FASTA_file>" << endl;
        return 1;
    }
    const string filename = argv[1];
    const bool print_output = argv[2] ? false : true;
    const vector<pair<string,string>> records = read_fasta(filename);
    if(records.empty()){
        cerr << "No sequences were read from the file." << endl;
        return 1;
    }
    vector<string> seqs;
    for(auto &r : records){
        seqs.push_back(r.second);
    }
    vector<vector<double>> dist_matrix = construct_distance_matrix(seqs);
    cout << "Distance Matrix:" << "\n";
    for(size_t i = 0; i < dist_matrix.size(); i++){
        for(size_t j = 0; j < dist_matrix[i].size(); j++){
            cout << dist_matrix[i][j] << (j+1 < dist_matrix[i].size() ? " " : "");
        }
        cout << "\n";
    }
    cout << "\n";


    // Sort the final profile by headers
    if (print_output) {
        Profile final_profile = progressive_alignment(records, dist_matrix);
        vector<pair<string, string>> sorted_pairs;
        for (size_t i = 0; i < final_profile.headers.size(); i++) {
            sorted_pairs.emplace_back(final_profile.headers[i], final_profile.sequences[i]);
        }
        sort(sorted_pairs.begin(), sorted_pairs.end());
        final_profile.headers.clear();
        final_profile.sequences.clear();
        for (auto& p : sorted_pairs) {
            final_profile.headers.push_back(p.first);
            final_profile.sequences.push_back(p.second);
        }

        cout << "Final Progressive Alignment:" << "\n";
        print_alignment(final_profile.headers, final_profile.sequences, 50);
    }
    return 0;
}