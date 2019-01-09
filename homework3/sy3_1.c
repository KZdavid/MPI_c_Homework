#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char *argv[])
{
	int m, n, k, tmp, i, j, subm, subn, numprocs, myid;
	int *a, *b, *p, *local_a, *local_b;
	double ts, te;
	FILE *fpa, *fpb, *fp, *output, *time;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	//set I/O & read data size
	if (myid == 0)
	{
		time = fopen("time", "a");
		output = fopen("output", "a");
		fp = fopen("parameter", "r");
		fscanf(fp, "m=%d", &m);
		fscanf(fp, "n=%d", &n);
		fscanf(fp, "k=%d", &k);
		fscanf(fp, "numprocs=%d", &tmp);
		fscanf(fp, "subm=%d", &subm);
		fscanf(fp, "subn=%d", &subn);
		if (tmp!=numprocs)
		{
			perror("Num of process ERROR!\n");
			exit(EXIT_FAILURE);
		}
		if (subm*subn != numprocs || m % subm != 0 || n % subn != 0)
		{
			perror("Parameter ERROR!\n");
			exit(EXIT_FAILURE);
		}

	}

	return 0;
}