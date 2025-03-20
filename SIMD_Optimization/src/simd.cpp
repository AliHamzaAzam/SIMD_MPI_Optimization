//
// Created by Ali Hamza Azam on 18/02/2025.

//
#include <arm_neon.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc/_malloc.h>
#include <mach/mach_time.h>
#include <simd/simd.h>

int N = 512;

void scalar_2Dimplementation(float **A, float **B, float **C) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = A[j][i] * B[i][j];
        }
    }
}

void scalar_1Dimplementation(const float *A, const float *B, float *C) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i*N + j] = A[j*N + i] * B[i*N + j];
        }
    }
}

void simd_implementation_neon(float **A, float **B, float **C) {
    float *A_T_flat;
    posix_memalign(reinterpret_cast<void **>(&A_T_flat), 16, N * N * sizeof(float));
    auto** A_T = new float*[N];
    for (int i = 0; i < N; ++i) {
        A_T[i] = &A_T_flat[i * N];
    }

    // Transpose A into A_T using SIMD
    for (int i = 0; i < N; i += 4) {
        for (int j = 0; j < N; j += 4) {
            const float32x4_t row0 = vld1q_f32(A[i] + j);
            const float32x4_t row1 = vld1q_f32(A[i + 1] + j);
            const float32x4_t row2 = vld1q_f32(A[i + 2] + j);
            const float32x4_t row3 = vld1q_f32(A[i+3] + j);

            const float32x4x2_t tmp1 = vzipq_f32(row0, row2);
            const float32x4x2_t tmp2 = vzipq_f32(row1, row3);

            const float32x4x2_t result1 = vzipq_f32(tmp1.val[0], tmp2.val[0]);
            const float32x4x2_t result2 = vzipq_f32(tmp1.val[1], tmp2.val[1]);

            vst1q_f32(A_T[j] + i, result1.val[0]);
            vst1q_f32(A_T[j+1] + i, result1.val[1]);
            vst1q_f32(A_T[j+2] + i, result2.val[0]);
            vst1q_f32(A_T[j+3] + i, result2.val[1]);
        }
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; j += 4) {
            float32x4_t a = vld1q_f32(A_T[i] + j);
            float32x4_t b = vld1q_f32(B[i] + j);
            vst1q_f32(C[i] + j, vmulq_f32(a, b));
        }
    }

    free(A_T_flat);
    delete[] A_T;
}

void simd_implementation_apple(float **A, float **B, float **C) {
  auto **A_T = new float *[N];
  float *A_T_flat;
  posix_memalign(reinterpret_cast<void **>(&A_T_flat), 16, N * N * sizeof(float));
  for (int i = 0; i < N; ++i) {
    A_T[i] = &A_T_flat[i * N];
  }

  // Transpose A
  for (int i = 0; i < N; i += 4) {
    for (int j = 0; j < N; j += 4) {
      simd_float4 row0 = *(simd_float4 *)&A[i][j];
      simd_float4 row1 = *(simd_float4 *)&A[i + 1][j];
      simd_float4 row2 = *(simd_float4 *)&A[i + 2][j];
      simd_float4 row3 = *(simd_float4 *)&A[i + 3][j];

      A_T[j][i] = row0[0];
      A_T[j][i + 1] = row1[0];
      A_T[j][i + 2] = row2[0];
      A_T[j][i + 3] = row3[0];

      A_T[j + 1][i] = row0[1];
      A_T[j + 1][i + 1] = row1[1];
      A_T[j + 1][i + 2] = row2[1];
      A_T[j + 1][i + 3] = row3[1];

      A_T[j + 2][i] = row0[2];
      A_T[j + 2][i + 1] = row1[2];
      A_T[j + 2][i + 2] = row2[2];
      A_T[j + 2][i + 3] = row3[2];

      A_T[j + 3][i] = row0[3];
      A_T[j + 3][i + 1] = row1[3];
      A_T[j + 3][i + 2] = row2[3];
      A_T[j + 3][i + 3] = row3[3];
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j += 4) {
      simd_float4 a_row = *(simd_float4 *)&A_T[i][j];
      simd_float4 b_row = *(simd_float4 *)&B[i][j];
      simd_float4 result = a_row * b_row;
      *(simd_float4 *)&C[i][j] = result;
    }
  }

  free(A_T_flat);
  delete[] A_T;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s N implementation\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    if (N % 4 != 0) {
        printf("N must be a multiple of 4\n");
        return 1;
    }
    char* implementation = argv[2];

    // Allocate contiguous memory blocks for matrices
    float *A_flat, *B_flat, *C_flat;
    posix_memalign(reinterpret_cast<void **>(&A_flat), 16, N * N * sizeof(float));
    posix_memalign(reinterpret_cast<void **>(&B_flat), 16, N * N * sizeof(float));
    posix_memalign(reinterpret_cast<void **>(&C_flat), 16, N * N * sizeof(float));

    // Create 2D arrays for row pointers
    auto** A = new float*[N];
    auto** B = new float*[N];
    auto** C = new float*[N];
    for (int i = 0; i < N; ++i) {
        A[i] = &A_flat[i * N];
        B[i] = &B_flat[i * N];
        C[i] = &C_flat[i * N];
    }

    // Initialize matrices
    srand(0);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = static_cast<float>(rand()) / RAND_MAX;
            B[i][j] = static_cast<float>(rand()) / RAND_MAX;
        }
    }

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    // Start time
    uint64_t start = mach_absolute_time();

    // Run the implementation 100 times to get an average time
    for (int i = 0; i < 100; i++) {
        if (strcmp(implementation, "neon") == 0) {
            simd_implementation_neon(A, B, C);
        } else if (strcmp(implementation, "apple") == 0) {
            simd_implementation_apple(A, B, C);
        } else if (strcmp(implementation, "scalar1D") == 0) {
            scalar_1Dimplementation(A_flat, B_flat, C_flat);
        } else if (strcmp(implementation, "scalar2D") == 0) {
            scalar_2Dimplementation(A, B, C);
        } else {
            printf("Invalid implementation\n");
            free(A_flat); free(B_flat); free(C_flat);
            delete[] A; delete[] B; delete[] C;
            return 1;
        }
    }

    // End time
    uint64_t end = mach_absolute_time();
    uint64_t elapsed = end - start;

    // Convert elapsed ticks to nanoseconds
    uint64_t elapsedNano = elapsed * timebase.numer / timebase.denom;


    printf("%s, %d, %.3llu\n", implementation, N, elapsedNano / 100);

    // Free memory
    free(A_flat);
    free(B_flat);
    free(C_flat);
    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
}