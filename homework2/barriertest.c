#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
int npes, myrank;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &npes);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
printf("From process %d out of %d, Hello World!\n", myrank, npes);
MPI_Barrier(MPI_COMM_WORLD);
printf("process %d is finished!\n", myrank);

MPI_Comm comm;
MPI_Group world_group,group;
MPI_Comm_group(MPI_COMM_WORLD,&world_group);
int ranks[5]={0,1,2,3,4};
MPI_Group_incl(world_group,3,ranks,&group);
MPI_Comm_create(MPI_COMM_WORLD,group,&comm);
if(MPI_COMM_NULL!=comm){
printf("subprocess %d starts\n",myrank);
MPI_Barrier(comm);
printf("subprocess %d is finished!\n",myrank);
}
MPI_Finalize();

return 0;
}
