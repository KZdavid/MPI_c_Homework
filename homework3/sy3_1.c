#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char *argv[])
{
	int m, n, k, tmp, i, j, subm, subn, numprocs, myid;
	int *a, *b, *p, *local_a, *local_b;
	double ts, te;
	FILE *fpa, *fpb, *output, *time;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	//set I/O & read data size
	if (myid == 0)
	{
		time = fopen("time", "a");
		output = fopen("output", "a");
		fpa = fopen("A", "r");
		fpb = fopen("B", "r");
		fscanf(fpa, "row=%d", &m);
		fscanf(fpa, "col=%d", &k);
		fscanf(fpb, "row=%d", &tmp);
		fscanf(fpb, "col=%d", &n);
		if (tmp!=k)
		{
			perror("Interior dimension must be conssitent!");
			exit(EXIT_FAILURE);
		}
		
	}

	return 0;
}