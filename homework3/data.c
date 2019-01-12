#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int i,j,m, n, k, blockm, blockn;
	FILE *fp;
	printf("m,n,k=\n");
	scanf("%d", &m);
	scanf("%d", &n);
	scanf("%d", &k);
	printf("blockm,blockn=\n");
	scanf("%d", &blockm);
	scanf("%d", &blockn);
	fp = fopen("parameter", "w");
	fprintf(fp,"%d\n%d\n%d\n%d\n%d\n%d\n", m, n, k, blockm*blockn, blockm, blockn);
	fclose(fp);
	srand((unsigned)time(NULL));
	fp = fopen("A", "w");
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < k; j++)
		{
			fprintf(fp, "%f ", rand() / (float)(RAND_MAX) * 2 - 1);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	fp = fopen("B", "w");
	for (i = 0; i < k; i++)
	{
		for (j = 0; j < n; j++)
		{
			fprintf(fp, "%f ", rand() / (float)(RAND_MAX) * 2 - 1);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}