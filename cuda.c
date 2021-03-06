//CUDA Program
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda.h>
#include <device_functions.h>
#include <cuda_runtime_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef struct {
	int w;
	int h;
	double* elem;
} Matrix;
#define BLOCK_SIZE 16
__global__ void MatMulKernel(const Matrix, const Matrix, Matrix);
void MatMul(const Matrix A, const Matrix B, Matrix C)
{
	Matrix mA;
	mA.w = A.w; mA.h = A.h;
	size_t size = A.w * A.h * sizeof(double);
	cudaMalloc(&mA.elem, size);
	cudaMemcpy(mA.elem, A.elem, size,
			cudaMemcpyHostToDevice);
	Matrix mB;
	mB.w = B.w; mB.h = B.h;
	size = B.w * B.h* sizeof(double);
	cudaMalloc(&mB.elem, size);
	cudaMemcpy(mB.elem, B.elem, size,
			cudaMemcpyHostToDevice);
	Matrix mC;
	mC.w = C.w; mC.h = C.h;
	size = C.w * C.h * sizeof(double);
	cudaMalloc(&mC.elem, size);
	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);
	dim3 dimGrid(B.w / dimBlock.x, A.h / dimBlock.y);
	MatMulKernel<<<dimGrid, dimBlock>>>(mA, mB, mC);
	cudaMemcpy(C.elem, mC.elem, size,cudaMemcpyDeviceToHost);
	cudaFree(mA.elem);
	cudaFree(mB.elem);
	cudaFree(mC.elem);
}
__global__ void MatMulKernel(Matrix A, Matrix B, Matrix C)
{
	double Cvalue = 0;
	int row = blockIdx.y * blockDim.y + threadIdx.y;
	int col = blockIdx.x * blockDim.x + threadIdx.x;
	for (int e = 0; e < A.width; ++e)
		Cvalue += A.elem[row * A.w + e] * B.elem[e * B.w + col];
	C.elements[row * C.w + col] = Cvalue;	
}
// #define ARRAY_SIZE 256
void dgemm(int n, const double* A, const double* B, double* C)
{	
	int i, j, k;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			C[i*n + j] = 0;
			for (k =0; k < n; k++)
			{
				C[i*n + j] += A[k + i * n] * B[k*n + j];			
			}
		}
	}
}
int main()
{
	int study_cases[]={256,512,768,1024,1280};
	for(int i=0; i<sizeof(study_cases)/sizeof(int); i++)
	{
		for(int j=0;j<5;j++)
		{
		int ARRAY_SIZE = study_cases[i];
		double *A = (double*)calloc(ARRAY_SIZE * ARRAY_SIZE, sizeof(double));
		double *B = (double*)calloc(ARRAY_SIZE * ARRAY_SIZE, sizeof(double));
		double *C1 = (double*)calloc(ARRAY_SIZE * ARRAY_SIZE, sizeof(double));
		double *C2 = (double*)calloc(ARRAY_SIZE * ARRAY_SIZE, sizeof(double));
		for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i++)
		{
			A[i] = rand() % 100;
			B[i] = rand() % 100;
		}
		clock_t t;
		t = clock();
		dgemm(ARRAY_SIZE, A, B, C1);
		t = clock() - t;
		double elapsed_time = ((double)t) / CLOCKS_PER_SEC;
		Matrix M_A = {ARRAY_SIZE, ARRAY_SIZE, A};
		Matrix M_B = {ARRAY_SIZE, ARRAY_SIZE, B};
		Matrix M_C2 = {ARRAY_SIZE, ARRAY_SIZE, C2};
		clock_t t2;
		t2 = clock();
		MatMul(M_A, M_B, M_C2);
		t2 = clock() - t2;
		double elapsed_time2 = ((double)t2) / CLOCKS_PER_SEC;
		for(int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i++)
		{
			if(C1[i] != C2[i])
			{
				printf("Values of C1 and C2 are not equal!");
				return 1;
			}
		}
		printf("> array size :%d, iteration: %d\n",j+1,ARRAY_SIZE);
		printf("\tUnoptimized DGEMM code took %.2f seconds to execute.\n",         elapsed_time);
		printf("\tCUDA MatMull code took %.2f seconds to execute.\n", elapsed_time2);
		free(A);
		free(B);
		free(C1);
		free(C2);
		}
	}
	return 0;
}

