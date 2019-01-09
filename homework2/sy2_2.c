#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char *argv[])
{
    //var for allocate process and time logging
    int current_process_num, numprocs, myid;
    int *ranks;
    double ts, te;
    FILE *time;

    //var for program and I/O
    int i, chunk_size, m, n, addzero, total_n;
    double *a=NULL, *b=NULL, *local_a=NULL, *local_b=NULL;
    double mp, global_mp, lf;
    FILE *fpa, *fpb, *output;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    MPI_Group world_group, current_group;
    MPI_Comm current_comm;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    //Initialize the time print and output
    if (myid == 0)
    {
        time = fopen("time", "w");
        output = fopen("output", "w");
    }

    ranks = (int*)malloc(numprocs * sizeof(int));
    for (i = 0; i < numprocs; i++) ranks[i] = i;//Used to allocate MPI_Group

    for (current_process_num = 1; current_process_num <= numprocs; current_process_num++)
    {
        //Initialize the Communicator
        //MPI_Barrier(MPI_COMM_WORLD);
        //if(myid==0){printf("cpn=%d,root\n",current_process_num);}
        //else {printf("cpn=%d,id=%d\n",current_process_num,myid);}
        //MPI_Barrier(MPI_COMM_WORLD);
        MPI_Group_incl(world_group, current_process_num, ranks, &current_group);
        MPI_Comm_create(MPI_COMM_WORLD, current_group, &current_comm);
        
        //MPI_Barrier(MPI_COMM_WORLD);
		ts = MPI_Wtime();

        if (MPI_COMM_NULL != current_comm)
        {
            /*********************************************
             **              Program Start              **
             *********************************************/

             //I/O
            if (myid == 0)
            {
                fpa = fopen("A", "r");
                fpb = fopen("B", "r");
                fscanf(fpa, "%d", &m);
                fscanf(fpb, "%d", &n);
                if (m != n) {
                    printf("ERROR: DIMENSIONS MUST BE CONSISTENT!\n");
                    continue;
                }
                chunk_size = ceil(n /(double) current_process_num);
                addzero = chunk_size * current_process_num - n;
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
            MPI_Bcast(&chunk_size, 1, MPI_INT, 0, current_comm);
            local_a = (double*)malloc(sizeof(double)*chunk_size);
            local_b = (double*)malloc(sizeof(double)*chunk_size);
            MPI_Scatter(a, chunk_size, MPI_DOUBLE, local_a, chunk_size, MPI_DOUBLE, 0, current_comm);
            MPI_Scatter(b, chunk_size, MPI_DOUBLE, local_b, chunk_size, MPI_DOUBLE, 0, current_comm);
            //multipliar
            mp = 0; global_mp=0;
            for (i = 0; i < chunk_size; i++)
            {
                if (local_a[i] != 0) {
                    mp = mp + local_a[i] * local_b[i];
                }
            }
            //printf("process%d,mp=%.6f\n",myid,mp);
            MPI_Reduce(&mp, &global_mp, 1, MPI_DOUBLE, MPI_SUM, 0, current_comm);
            if(myid==0){
                fprintf(output, "%.6f\n", global_mp);
				free(a); free(b); a = NULL; b = NULL;
            }
            free(local_a);free(local_b);local_a=NULL;local_b=NULL;
            /*********************************************
             **               Program End               **
             *********************************************/

            //Deconstruct the Communicator
            MPI_Group_free(&current_group);
            MPI_Comm_free(&current_comm);
        }

        //Log and print the time
		te = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
        if (myid == 0)
        {            
            fprintf(time, "%.6f\n", te - ts);
        }

    }
    //close files for time and output
    if (myid == 0)
    {
        fclose(time);
        fclose(output);
    }
    free(ranks);ranks=NULL;
    MPI_Finalize();
    return 0;
}
