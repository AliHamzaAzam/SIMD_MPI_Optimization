//
// Created by Ali Hamza Azam on 02/03/2025.
//

#include "utility.h"
#include <mpi.h>


// Define MPI tags
#define TAG_SEQ 100
#define TAG_FFT 101

// Helper functions for sending and receiving strings via MPI
void sendString(const std::string &str, int dest, int tag) {
    int len = str.size();
    MPI_Send(&len, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    MPI_Send(str.c_str(), len, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
}

std::string recvString(int source, int tag) {
    int len;
    MPI_Recv(&len, 1, MPI_INT, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char* buffer = new char[len + 1];
    MPI_Recv(buffer, len, MPI_CHAR, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    buffer[len] = '\0';
    std::string str(buffer);
    delete[] buffer;
    return str;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // Each process reads its portion of the FASTA file.
    std::vector<std::pair<std::string, std::string>> local_records;
    std::vector<std::string> local_seqs;

    // Rank 0 reads the FASTA file and distributes the records to other processes.
    std::vector<std::pair<std::string, std::string>> full_records;
    if (rank == 0) {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <FASTA_file>" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        full_records = read_fasta(argv[1]);
        if (full_records.empty()) {
            std::cerr << "No sequences were read from the file." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int total = full_records.size();
        int base = total / nprocs;
        int rem = total % nprocs;
        int start = 0;

        for (int p = 0; p < nprocs; ++p) {
            int count = base + (p < rem ? 1 : 0);
            if (p == 0) {
                for (int i = 0; i < count; i++) {
                    local_records.push_back(full_records[i]);
                    local_seqs.push_back(full_records[i].second);
                }
            } else {
                MPI_Send(&count, 1, MPI_INT, p, TAG_SEQ, MPI_COMM_WORLD);
                for (int i = start; i < start + count; i++) {
                    sendString(full_records[i].first, p, TAG_SEQ);
                    sendString(full_records[i].second, p, TAG_SEQ);
                }
            }
            start += count;
        }
    } else {
        int count;
        MPI_Recv(&count, 1, MPI_INT, 0, TAG_SEQ, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int i = 0; i < count; i++) {
            std::string header = recvString(0, TAG_SEQ);
            std::string seq = recvString(0, TAG_SEQ);
            local_records.push_back(std::make_pair(header, seq));
            local_seqs.push_back(seq);
        }
    }

    int local_max = 0;
    for (const auto &s : local_seqs)
        if (s.size() > static_cast<size_t>(local_max))
            local_max = s.size();

    int global_max;
    if (rank == 0) {
        global_max = local_max;
        for (int i = 1; i < nprocs; i++) {
            int tmp;
            MPI_Recv(&tmp, 1, MPI_INT, i, TAG_SEQ, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (tmp > global_max)
                global_max = tmp;
        }
        for (int i = 1; i < nprocs; i++) {
            MPI_Send(&global_max, 1, MPI_INT, i, TAG_SEQ, MPI_COMM_WORLD);
        }
    } else {
        MPI_Send(&local_max, 1, MPI_INT, 0, TAG_SEQ, MPI_COMM_WORLD);
        MPI_Recv(&global_max, 1, MPI_INT, 0, TAG_SEQ, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    size_t common_pad = next_power_of_two(global_max);

    std::unordered_map<char, int> charToIndex = {
        {'A',0}, {'R',1}, {'N',2}, {'D',3}, {'C',4},
        {'Q',5}, {'E',6}, {'G',7}, {'H',8}, {'I',9},
        {'L',10}, {'K',11}, {'M',12}, {'F',13}, {'P',14},
        {'S',15}, {'T',16}, {'W',17}, {'Y',18}, {'V',19},
        {'B',20}, {'Z',21}, {'X',22}
    };

    std::vector<std::vector<std::complex<double>>> localFFT;
    for (const auto &seq : local_seqs) {
        std::vector<int> numeric = seq_to_numeric(seq, charToIndex);
        localFFT.push_back(fft_transform(numeric, common_pad));
    }

    // Gather FFT results at rank 0 using point-to-point communication.
    std::vector<std::vector<std::vector<std::complex<double>>>> fft_results;
    if (rank == 0) {
        fft_results.resize(nprocs);
        fft_results[0] = localFFT;
        for (int p = 1; p < nprocs; ++p) {
            int count;
            MPI_Recv(&count, 1, MPI_INT, p, TAG_FFT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::vector<std::vector<std::complex<double>>> procFFT;
            for (int i = 0; i < count; i++) {
                int vecSize;
                MPI_Recv(&vecSize, 1, MPI_INT, p, TAG_FFT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<double> buf(2 * vecSize);
                MPI_Recv(buf.data(), 2 * vecSize, MPI_DOUBLE, p, TAG_FFT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::vector<std::complex<double>> fftVec(vecSize);
                for (int j = 0; j < vecSize; j++) {
                    fftVec[j] = std::complex<double>(buf[2 * j], buf[2 * j + 1]);
                }
                procFFT.push_back(fftVec);
            }
            fft_results[p] = procFFT;
        }
    } else {
        int count = localFFT.size();
        MPI_Send(&count, 1, MPI_INT, 0, TAG_FFT, MPI_COMM_WORLD);
        for (const auto &fftVec : localFFT) {
            int vecSize = fftVec.size();
            MPI_Send(&vecSize, 1, MPI_INT, 0, TAG_FFT, MPI_COMM_WORLD);
            std::vector<double> buf(2 * vecSize);
            for (int i = 0; i < vecSize; i++) {
                buf[2 * i]     = fftVec[i].real();
                buf[2 * i + 1] = fftVec[i].imag();
            }
            MPI_Send(buf.data(), 2 * vecSize, MPI_DOUBLE, 0, TAG_FFT, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        // Combine the FFT results in the same order as the original records.
        std::vector<std::vector<std::complex<double>>> globalFFT;
        for (int p = 0; p < nprocs; p++) {
            for (auto &vec : fft_results[p])
                globalFFT.push_back(vec);
        }

        // Compute a distance matrix from the FFT results using Pearson correlation.
        size_t n = globalFFT.size();
        std::vector<std::vector<double>> distMatrix(n, std::vector<double>(n, 0.0));
        for (size_t i = 0; i < n; i++) {
            for (size_t j = i + 1; j < n; j++) {
                double corr = fft_correlation(globalFFT[i], globalFFT[j]);
                double dist = 1.0 - corr;
                distMatrix[i][j] = dist;
                distMatrix[j][i] = dist;
            }
        }

        std::cout << "Distance Matrix:" << std::endl;
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                std::cout << distMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        // Perform progressive alignment using the utility function.
        Profile final_profile = progressive_alignment(full_records, distMatrix);
        std::cout << "\nFinal Progressive Alignment:" << std::endl;
        print_alignment(final_profile.headers, final_profile.sequences);
    }

    MPI_Finalize();
    return 0;
}
