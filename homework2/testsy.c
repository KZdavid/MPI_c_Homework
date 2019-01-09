#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
	double t0, t1;

	int i, n, mp, global_mp, chunk_size;
	int *a, *local_a, *b, *local_b;
	int id, p;
	FILE *fp, *fq;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	t0 = MPI_Wtime();
	if (id == 0)
	{
		fp = fopen("A", "r");
		fq = fopen("B", "r");
		fscanf(fp, "%d", &n);
		fscanf(fq, "%d", &n);
		a = (int*)malloc(sizeof(int)*n);
		b = (int*)malloc(sizeof(int)*n);
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	chunk_size = n / p;

	local_a = (int*)malloc(sizeof(int)*chunk_size);
	local_b = (int*)malloc(sizeof(int)*chunk_size);

	if (id == 0)  //根进程
	{
		for (i = 0; i < n; i++) {
			fscanf(fp, "%d", &a[i]);
			fscanf(fq, "%d", &b[i]);
		}
		fclose(fp);
		fclose(fq);
	}
	//分发数据
	MPI_Scatter(a, chunk_size, MPI_INT, local_a, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(b, chunk_size, MPI_INT, local_b, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	mp = 0;
	for (i = 0; i < chunk_size; i++)
	{
		mp = mp + local_a[i] * local_b[i];
	}

	MPI_Reduce(&mp, &global_mp, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	t1 = MPI_Wtime();
	if (id == 0)
	{
		printf("The Multiply is: %d\nTime = %.6e\n", global_mp, t1 - t0);

		free(a);
		a = NULL;
		free(b);
		b = NULL;
	}

	free(local_a);
	local_a = NULL;
	free(local_b);
	local_b = NULL;

	MPI_Finalize();

	return 0;
}
