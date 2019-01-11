#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
	int m, n, k, tmp, i, j, l, blockm, blockn, subm, subn;
	double *a, *b, *p, *pa, *pb, *local_a, *local_b, *mp, *local_mp, *buf, *pbuf;
	FILE *fp, *output, *time;

	//set I/O & read data size
		output = fopen("output", "a");
		fp = fopen("parameter", "r");
		fscanf(fp, "m=%d", &m);
		fscanf(fp, "n=%d", &n);
		fscanf(fp, "k=%d", &k);
		fscanf(fp, "numprocs=%d", &tmp);
		fscanf(fp, "blockm=%d", &blockm);
		fscanf(fp, "blockn=%d", &blockn);
		fclose(fp);
		if (tmp != numprocs)
		{
			perror("Num of process ERROR!\n");
			exit(EXIT_FAILURE);
		}
		if (blockm*blockn != numprocs || m % blockm != 0 || n % blockn != 0)
		{
			perror("Parameter ERROR!\n");
			exit(EXIT_FAILURE);
		}

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

		b = (double *)malloc(sizeof(double)*(n*k));
		fp = fopen("B", "r");
		for (i = 0; i < k; i++)
		{
			p = &(b[i]);
			for (j = 0; j < n; j++)
			{
				fscanf(fp, "%lf", p);
				p += n;
			}
		}

	p = &(mp[0]);
	for (i = 0; i < m; i++)
	{
		pa = &(a[i*n]);
		for (j = 0; j < n; j++)
		{
			pb = &(local_b[j]);
			*p = 0;
			for (l = 0; l < k; l++)
			{
				*p += *pa * *pb;
				pa++; pb++;
			}
			p++;
		}
	}
	pa = NULL; pb = NULL;
	free(local_a);
	free(local_b);
	buf = (double*)malloc(sizeof(double)*m*n);
	MPI_Gather(local_mp, subm*subn, MPI_DOUBLE, buf, subm*subn, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	pbuf = &(buf[0]);
	mp = (double*)malloc(sizeof(double)*m*n);
	for (l = 0; i < numprocs; i++)
	{
		p = &(mp[(l / blockn)*n + (l%blockn)*blockn]);
		for (i = 0; i < subm; i++)
		{
			for (j = 0; j < subn; j++)
			{
				*p = *pbuf;
				pbuf++; p++;
			}
			p = p + n - blockn;
		}
	}

		output = fopen("output", "a");
		fprintf(output, "numprocs=%d,m=%d,n=%d\n", numprocs, m, n);
		fprintf(output, "blockm=%d,blockn=%d\n", blockm, blockn);
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < n; j++)
			{
				fprintf(output, "%.6f\t", *p);
			}
			fprintf(output, "\n");
		}
		free(output);

	p = NULL;


		time = fopen("time", "a");
		fprintf(time, "numprocs=%d,m=%d,n=%d\n", numprocs, m, n);
		fprintf(time, "blockm=%d,blockn=%d\n", blockm, blockn);
		fprintf(time, "time=%.6f", te - ts);


	return 0;
}