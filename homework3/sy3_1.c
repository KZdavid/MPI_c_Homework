#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char *argv[])
{
	int m, n, k, tmp, i, j, l, blockm, blockn, subm, subn, numprocs, myid, parameter[3];
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
		//printf("m=%d,n=%d,k=%d,numprocs=%d,blockm=%d,blockn=%d\n", m, n, k, numprocs, blockm, blockn);
	}

	MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&blockm, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&blockn, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	subm = m / blockm;
	subn = n / blockn;
	local_a = (double*)malloc(sizeof(double)*subm*k);
	local_b = (double*)malloc(sizeof(double)*k*subn);

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
		/*
		p = &(a[0]);
		for (i = 0; i < m*k; i++)
		{
			printf("a[%d]=%f,", i, *p);
			p++;
		}
		printf("\n");
		*/
		
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
		/*
		p = &(b[0]);
		for (i = 0; i < n*k; i++)
		{
			printf("b[%d]=%f,", i, *p);
			p++;
		}
		printf("\n");
		*/
	}

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

	//MPI_Scatter(a, subm*k, MPI_DOUBLE, local_a, subm*k, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//MPI_Barrier(MPI_COMM_WORLD);
	//MPI_Scatter(b, subn*k, MPI_DOUBLE, local_b, subn*k, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//printf("id=%d,blockm=%d,blockn=%d,subm=%d,subn=%d,k=%d\n", myid, blockm, blockn, subm, subn, k);

	if (myid == 0)
	{
		free(a);
	}
	if (myid == 1)
	{
		free(b);
	}
	//printf("id=%d,local_a=%f,local_b=%f\n", myid, local_a[0], local_b[0]);
	MPI_Barrier(MPI_COMM_WORLD);
	local_mp = (double*)malloc(sizeof(double)*subm*subn);
	p = &(local_mp[0]);
	for (i = 0; i < subm; i++)
	{
		pa = &(local_a[i*n]);
		for (j = 0; j < subn; j++)
		{
			pb = &(local_b[j]);
			*p = 0;
			for (l = 0; l < k; l++)
			{
				//printf("id=%d,%f+=%f*%f,", myid, *p, *pa, *pb);
				*p += *pa * *pb;
				pa++; pb++;
			}
			p++; //printf("\n");
		}
	}
	pa = NULL; pb = NULL; p = NULL;
	free(local_a);
	free(local_b);
	//done
	if (myid == 0)
	{
		buf = (double*)malloc(sizeof(double)*m*n);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gather(local_mp, subm*subn, MPI_DOUBLE, buf, subm*subn, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if (myid == 0)
	{
		pbuf = &(buf[0]);
		mp = (double*)malloc(sizeof(double)*m*n);
		//printf("buf=\n%.6f,%.6f,%.6f\n%.6f,%.6f,%.6f\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		for (l = 0; l < numprocs; l++)
		{
			//printf("l=%d,pos=%d\n", l, (l / blockn)*subm*n + (l%blockn)*subn);
			p = &(mp[(l / blockn)*subm*n + (l%blockn)*subn]);
			for (i = 0; i < subm; i++)
			{
				for (j = 0; j < subn; j++)
				{
					*p = *pbuf;
					//printf("i=%d,j=%d,*p=%.6f,*pbuf=%.6f\n", i, j, *p, *pbuf);
					pbuf++; p++;
				}
				p = p + n - subn;
			}
		}
		//printf("mp=\n%.6f,%.6f,%.6f\n%.6f,%.6f,%.6f\n", mp[0], mp[1], mp[2], mp[3], mp[4], mp[5]);
		
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