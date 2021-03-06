#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char *argv[])
{
	int m, n, k, tmp, i, j, kk, blockm, blockn, subm, subn, numprocs, myid, parameter[3];
	double *a, *b, *p, *pa, *pb, *local_a, *local_b, *mp, *local_mp, *buf, *pbuf, ts, te;
	FILE *fp, *output, *time;
	MPI_Status status1, status2;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	ts = MPI_Wtime();
	//set I/O & read data size
	if (myid == 0)
	{
		fp = fopen("parameter", "r");
		fscanf(fp, "%d", &m);
		fscanf(fp, "%d", &n);
		fscanf(fp, "%d", &k);
		fscanf(fp, "%d", &tmp);
		fscanf(fp, "%d", &blockm);
		fscanf(fp, "%d", &blockn);
		fclose(fp);
		if (tmp != numprocs)
		{
			printf("Num of process ERROR!\nsuppose:%d,actural:%d\n", tmp, numprocs);
			exit(EXIT_FAILURE);
		}
		if (blockm*blockn != numprocs || m % blockm != 0 || n % blockn != 0)
		{
			perror("Parameter ERROR!\n");
			exit(EXIT_FAILURE);
		}
	}
	//Broadcast data size
	MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&blockm, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&blockn, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	subm = m / blockm;
	subn = n / blockn;
	local_a = (double*)malloc(sizeof(double)*subm*k);
	local_b = (double*)malloc(sizeof(double)*k*subn);
	//procs 0 & 1 read A & B
	if (myid == 0)
	{
		a = (double *)malloc(sizeof(double)*(m*k));
		p = &(a[0]);
		fp = fopen("A", "r");
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < k; j++)
			{
				fscanf(fp, "%lf", p);
				p++;
			}
		}
		fclose(fp);
	}
	if (myid == 1)
	{
		b = (double *)malloc(sizeof(double)*(n*k));
		fp = fopen("B", "r");
		for (i = 0; i < k; i++)
		{
			p = &(b[i]);
			for (j = 0; j < n; j++)
			{
				fscanf(fp, "%lf", p);
				p += k;
			}
		}
		fclose(fp);
	}

	//allocate all data & send them to every proc
	MPI_Barrier(MPI_COMM_WORLD);
	if (myid == 0)
	{
		for (i = 0; i < blockm; i++)
		{
			for (j = 0; j < blockn; j++)
			{
				MPI_Send(a + i * subm*k, subm*k, MPI_DOUBLE, blockn*i + j, 100, MPI_COMM_WORLD);
			}
		}
	}
	MPI_Recv(local_a, subm*k, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD, &status1);
	if (status1.MPI_ERROR)
	{
		printf("recv worng in %d of tag %d", myid, status1.MPI_TAG);
	}
	if (myid == 1)
	{
		for (i = 0; i < blockm; i++)
		{
			for (j = 0; j < blockn; j++)
			{
				MPI_Send(b + j * subn*k, subn*k, MPI_DOUBLE, blockn*i + j, 200, MPI_COMM_WORLD);
			}
		}
	}
	MPI_Recv(local_b, subn*k, MPI_DOUBLE, 1, 200, MPI_COMM_WORLD, &status2);
	if (status2.MPI_ERROR)
	{
		printf("recv worng in %d of tag %d", myid, status2.MPI_TAG);
	}
	if (myid == 0)
	{
		free(a);
	}
	if (myid == 1)
	{
		free(b);
	}

	//Every proc calculate the matrix
	MPI_Barrier(MPI_COMM_WORLD);
	local_mp = (double*)malloc(sizeof(double)*subm*subn);
	p = &(local_mp[0]);
	for (i = 0; i < subm; i++)
	{
		for (j = 0; j < subn; j++)
		{
			pa = &(local_a[i*k]);
			pb = &(local_b[j*k]);
			*p = 0;
			for (kk = 0; kk < k; kk++)
			{
				*p +=*pa * *pb;
				pa++; pb++;
			}
			p++; 
		}
	}
	pa = NULL; pb = NULL; p = NULL;
	free(local_a);
	free(local_b);
	
	//Gather the output together to proc 0
	if (myid == 0)
	{
		buf = (double*)malloc(sizeof(double)*m*n);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gather(local_mp, subm*subn, MPI_DOUBLE, buf, subm*subn, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if (myid == 0)
	{
		//data need to rearange since simple gather is in the wrong order
		pbuf = &(buf[0]);
		mp = (double*)malloc(sizeof(double)*m*n);
		for (kk = 0; kk < numprocs; kk++)
		{
			p = &(mp[(kk / blockn)*subm*n + (kk%blockn)*subn]);
			for (i = 0; i < subm; i++)
			{
				for (j = 0; j < subn; j++)
				{
					*p = *pbuf;
					pbuf++; p++;
				}
				p = p + n - subn;
			}
		}

		//print the results
		output = fopen("output", "a");
		fprintf(output, "numprocs=%d,m=%d,n=%d\n", numprocs, m, n);
		fprintf(output, "blockm=%d,blockn=%d\n", blockm, blockn);
		p = &(mp[0]);
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < n; j++)
			{
				fprintf(output, "%.6f ", *p);
				p++;
			}
			fprintf(output, "\n");
		}
		fprintf(output, "\n");
		fclose(output);
		p = NULL; pbuf = NULL;
		te = MPI_Wtime();

		//print time
		time = fopen("time", "a");
		fprintf(time, "numprocs=%d,m=%d,n=%d\n", numprocs, m, n);
		fprintf(time, "blockm=%d,blockn=%d\n", blockm, blockn);
		fprintf(time, "time=%.6f", te - ts);
		fprintf(time, "\n\n");
		fclose(time);
	}
	MPI_Finalize();

	return 0;
}