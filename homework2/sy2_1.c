#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int main(int argc, char *argv[])
{
    //parameter
    int i, n = 1000000000;

    FILE *fp;
    srand((unsigned)time(NULL));
    fp = fopen("A", "w");
    fprintf(fp, "%d\n", n);
    for (i = 0; i < 1000; i++)
    {
        fprintf(fp, "%.6f\n", rand() / (float)(RAND_MAX) * 2 - 1);
    }
    fp = fopen("B", "w");
    fprintf(fp, "%d\n", n);
    for (i = 0; i < 1000; i++)
    {
        fprintf(fp, "%.6f\n", rand() / (float)(RAND_MAX) * 2 - 1);
    }
}
