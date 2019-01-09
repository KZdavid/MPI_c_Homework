#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
	//var for allocate process and time logging
	int numprocs, myid;
	double ts, te;
	//var for program and I/O
	int i, chunk_size, m, n, addzero, total_n;
	double *a = NULL, *b = NULL, *local_a = NULL, *local_b = NULL;
	double mp, global_mp, lf;
	FILE *fpa, *fpb;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	printf("%d after init", myid);

	//Initialize the time print and output
	ts = MPI_Wtime();
	if (myid == 0)
	{
		fpa = fopen("A", "r");
		fpb = fopen("B", "r");
		fscanf(fpa, "%d", &m);
		fscanf(fpb, "%d", &n);
		n = m * (m > n) + n * (n >= m);
		chunk_size = ceil((double)((double)n / (double)numprocs));
		addzero = chunk_size * numprocs - n;
		total_n = n + addzero;
		a = (double*)malloc(sizeof(double)*total_n);
		b = (double*)malloc(sizeof(double)*total_n);
		for (i = 0; i < addzero; i++)
		{
			a[i] = 0; b[i] = 0;
		}
		for (i = 0; i < n; i++)
		{
			fscanf(fpa, "%lf", &lf); a[addzero + i] = lf;
			fscanf(fpb, "%lf", &lf); b[addzero + i] = lf;
		}
		fclose(fpa);
		fclose(fpb);
	}
	printf("%d is ready", myid);

	MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	local_a = (double*)malloc(sizeof(double)*chunk_size);
	local_b = (double*)malloc(sizeof(double)*chunk_size);
	MPI_Scatter(a, chunk_size, MPI_DOUBLE, local_a, chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Scatter(b, chunk_size, MPI_DOUBLE, local_b, chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	printf("%d scatter finished", myid);
	//multipliar
	mp = 0; global_mp = 0;
	for (i = 0; i < chunk_size; i++)
	{
		if (local_a[i] != 0) {
			mp = mp + local_a[i] * local_b[i];
		}
	}
	printf("process%d,mp=%.6f\n", myid, mp);
	MPI_Reduce(&mp, &global_mp, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	if (myid == 0) {
		printf("inner_product = %.16f\n", global_mp);
		free(a); free(b); a = NULL; b = NULL;
	}
	free(local_a); free(local_b); local_a = NULL; local_b = NULL;
	te = MPI_Wtime();
	//Log and print the time
	if (myid == 0)
	{
		printf("time = %.6f\n", te - ts);
	}
	MPI_Finalize();
	return 0;
}
