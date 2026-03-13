#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define N 8

// function which abort the MPI environment due to a problem
void Abort(MPI_Comm comm, int rc, const char* wrong_message)
{
    if (rc != 0)
    {
        perror(wrong_message);
        MPI_Abort(comm, rc); // Abort the communicator in case of error
    }
}

int menu()
{
    printf("Menu: \n");
    printf("1. Continue \n");
    printf("2. Exit \n");

    printf("\n");

    int choice;
    printf("Type a choice: \n");
    scanf("%d",&choice);

    while(choice < 1 || choice > 2)
    {
        printf("\n");

        printf("Error choice!!! \n");
        printf("Type a choice: \n");
        scanf("%d",&choice);
    }
    printf("\n");

    return choice;
}

typedef struct
{
    int value;
    int rank;
}DiscoveryElement;

int main(int argc,char** argv)
{
    int rc;
    int process_rank;
    int total_processes;
    int lines_per_process;
    int elements_per_process;
    int root_process = 0; // process 0
    int choice;
    int base_count;
    int remainder;
    int offset;
    int local_max;
    int local_sum;
    int resultAB;
    int global_position;
    int local_position;
    int received_position;
    int tag1 = 1;
    MPI_Status status;
    int row;
    int column;
    int start_index;
    int left; // left processor
    int right; // right processor
    int *send_data; // sending data between processes
    int *receive_data; // receiving data between processes

    // Every process stores in a struct its local max and position
    DiscoveryElement local_element;
    // This variable will include the global max and its process
    DiscoveryElement global_element;
    int rank_global_max;

    int *A;
    int *B;
    int *C;
    int *C_local; // local array for every process
    int *RowC; // row of array C
    int *D;
    int *D_local; // local array for every process
    int *RowD; // row of array D
    int *SumCD; // Final sum of arrays C,D
    int *SubSumCD; // Sub sum of arrays C,D
    int *SubProductCD; // sub-multiplication arrays C,D
    int *ProductCD; // multiplication arrays C,D
    int *sub_multiplication_CB; // Sub multiplication of arrays C,B
    int *multiplication_CB; // Multiplication of arrays C,B
    int *sendcounts; // array shows how elements each process will calculate 
    int *displs; // array shows the offset for each process

    FILE *fpC; // file pointer for array C
    const char* filenameC = "numbersC.txt"; // file for array C

    FILE *fpD; // file pointer for array D
    const char* filenameD = "numbersD.txt"; // file for array D

    FILE *fpB; // file pointer for array B
    const char* filenameB = "numbersB.txt"; // file for array B

    FILE *fpA; // file pointer for array A
    const char* filenameA = "numbersA.txt"; // file for array A

    rc = MPI_Init(&argc,&argv); // Initialization of MPI environment
    Abort(MPI_COMM_WORLD,rc,"MPI_Init");

    rc = MPI_Comm_rank(MPI_COMM_WORLD,&process_rank); // ranks of processes
    Abort(MPI_COMM_WORLD,rc,"MPI_Comm_rank");

    rc = MPI_Comm_size(MPI_COMM_WORLD,&total_processes); // amount of processes
    Abort(MPI_COMM_WORLD,rc,"MPI_Comm_size");

    // We decide how many lines each process will calculate

    base_count = N / total_processes;
    remainder = N % total_processes;

    if(process_rank < remainder)
    {
        lines_per_process = base_count + 1;
    }
    else
    {
        lines_per_process = base_count;
    }

    // array shows how elements each process will calculate
    sendcounts = (int*) malloc(total_processes*sizeof(int));
    if(sendcounts == NULL)
    {   
        fprintf(stderr,"Not enough memory \n");
        fprintf(stderr,"No memory allocation for array sendcounts \n");
        exit(EXIT_FAILURE);
    }

    // array shows how elements each process will calculate
    displs = (int*) malloc(total_processes*sizeof(int));
    if(displs == NULL)
    {   
        fprintf(stderr,"Not enough memory \n");
        fprintf(stderr,"No memory allocation for array displs \n");
        exit(EXIT_FAILURE);
    }

    if(process_rank == root_process)
    {
        // array C
        C = (int*) malloc(N*N * sizeof(int));
        if(C == NULL)
        {   
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array C \n");
            exit(EXIT_FAILURE);
        }

        // array D
        D = (int*) malloc(N*N * sizeof(int));
        if(D == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array D \n");
            exit(EXIT_FAILURE);
        }
    }

    choice = 1;
    while(choice == 1)
    {
        // We define arrays sendcounts and displs for all processes
        offset = 0;
        for(int process=0; process<total_processes; process++)
        {
            int lines_process;
            if(process < remainder)
            {
                lines_process = base_count + 1;
            }
            else
            {
                lines_process = base_count;
            }

            sendcounts[process] = lines_process * N;
            displs[process] = offset;
            offset += sendcounts[process];
        }

        if(process_rank == root_process)
        {
            // QUESTION A

            // create file for array C
            fpC = fopen(filenameC,"r");
            if(fpC == NULL) // if file descriptor doesn't bind memory for file
            {
                perror("Error opening file for reading");
                fprintf(stderr,"Error with file %s \n",filenameC);
                exit(EXIT_FAILURE);
            }

            // create file for array D
            fpD = fopen(filenameD,"r");
            if(fpD == NULL) // if file descriptor doesn't bind memory for file
            {
                perror("Error opening file for reading");
                fprintf(stderr,"Error with file %s \n",filenameD);
                fclose(fpC); // close file for array C because we opened this file previously
                exit(EXIT_FAILURE);
            }

            printf("Array C dimensions %dx%d \n",N,N);
            printf("Array C takes data from file %s \n",filenameC);
            for(int i=0; i<N; i++)
            {
                for(int j=0; j<N; j++)
                {
                    fscanf(fpC,"%d",&C[i*N+j]);
                }
            }
            for(int i=0; i<N; i++)
            {
                for(int j=0; j<N; j++)
                {
                    printf("%d ",C[i*N+j]);
                }
                printf("\n");
            }

            printf("\n");

            printf("Array D dimensions %dx%d \n",N,N);
            printf("Array D takes data from file %s \n",filenameD);
            for(int i=0; i<N; i++)
            {
                for(int j=0; j<N; j++)
                {
                    fscanf(fpD,"%d",&D[i*N+j]);
                }
            }
            for(int i=0; i<N; i++)
            {
                for(int j=0; j<N; j++)
                {
                    printf("%d ",D[i*N+j]);
                }
                printf("\n");
            }

            printf("\n");

            // All processes have the same amount lines  
            //lines_per_process = N / total_processes;

            fclose(fpC);
            fclose(fpD);
        }

        C_local = (int*) malloc(lines_per_process*N * sizeof(int));
        if(C_local == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for local array C \n");
            exit(EXIT_FAILURE);
        }

        /* Root process scatters data of array C in other processes in equal portions.
        Root process scatters array C by lines. */
        rc = MPI_Scatterv(C,sendcounts,displs,MPI_INT,C_local,lines_per_process*N,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Scatterv");

        D_local = (int*) malloc(lines_per_process*N * sizeof(int));
        if(D_local == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for local array D \n");
            exit(EXIT_FAILURE);
        }

        /* Root process scatters data of array D in other processes in equal portions.
        Root process scatters array D by lines. */
        rc = MPI_Scatterv(D,sendcounts,displs,MPI_INT,D_local,lines_per_process*N,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Scatterv");

        SubSumCD = (int*) malloc(lines_per_process*N * sizeof(int));
        if(SubSumCD == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array SubSumCD \n");
            exit(EXIT_FAILURE);
        }
        /* Every process calculates its lines. Columns remains stable.
        Every process calculates a sub-sum of final result. */
        for(int i=0; i<lines_per_process; i++)
        {
            for(int j=0; j<N; j++)
            {
                SubSumCD[i*N+j] = C_local[i*N+j] + D_local[i*N+j];
            }
        }

        // Every process finds its local max and position
        local_max = SubSumCD[0];
        local_position = 0;
        for(int i=0; i<lines_per_process; i++)
        {
            for(int j=0; j<N; j++)
            {
                if(SubSumCD[i*N+j] > local_max)
                {
                    local_max = SubSumCD[i*N+j];
                    local_position = i*N+j;
                }
            }
        }

        // Position of local max in final array
        global_position = displs[process_rank] + local_position;

        local_element.value = local_max;
        local_element.rank = process_rank; 

        /* We gather all local maximums in root process.
        Root process finds the global max. */
        rc = MPI_Reduce(&local_element,&global_element,1,MPI_2INT,MPI_MAXLOC,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Reduce");

        if(process_rank == root_process)
        {
            rank_global_max = global_element.rank;
        }
        // Root process sends the rank which have the global max to other processes
        rc = MPI_Bcast(&rank_global_max,1,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Bcast");

        // Position of global max
        if(process_rank == root_process)
        {
            // if root process found the global max
            if(rank_global_max == root_process)
            {
                // Root process has the final max
                received_position = global_position;
            }
            else // if other process except from root process finds the global max
            {
                // Root process receives the position of max from process which found the max
                rc = MPI_Recv(&received_position,1,MPI_INT,rank_global_max,tag1,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD,rc,"MPI_Recv");
            }
        }
        // if other process except from root process finds the global max
        else if(process_rank == rank_global_max)
        {
            // This process sends its position of max in root process
            rc = MPI_Send(&global_position,1,MPI_INT,root_process,tag1,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD,rc,"MPI_Send");
        }

        // Root process displays the global max, its process and its position
        if(process_rank == root_process)
        {
            row = received_position / N; // row of global max
            column = received_position % N; // column of global max

            printf("Process %d found global max:%d in row:%d and column:%d \n",
            global_element.rank,global_element.value,row,column);
            printf("\n");
        }

        if(process_rank == root_process)
        {
            // Root process binds memory for final array 
            SumCD = (int*) malloc(N*N * sizeof(int));
            if(SumCD == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for array SumCD \n");
                exit(EXIT_FAILURE);
            }
        }

        // Root process gathers all sub-sums of processes in array SumCD (final result)
        /*rc = MPI_Gather(SubSumCD,lines_per_process*N,MPI_INT,SumCD,lines_per_process*N,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Gather");*/

        // Root process gathers all sub-sums of processes in array SumCD (final result)
        rc = MPI_Gatherv(SubSumCD,lines_per_process*N,MPI_INT,SumCD,sendcounts,displs,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Gatherv");

        // Root process displays the final array
        if(process_rank == root_process)
        {
            printf("Array SumCD dimensions %dx%d: \n",N,N);
            for(int i=0; i<N; i++)
            {
                for(int j=0; j<N; j++)
                {
                    printf("%d ",SumCD[i*N+j]);
                }
                printf("\n");
            }

            // release memory allocation
            free(SumCD);

            printf("\n");
        }

        // release memory
        free(D_local);
        free(SubSumCD);

        // QUESTION B
        B = (int*) malloc(N*sizeof(int));
        if(B == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array B \n");
            exit(EXIT_FAILURE);
        }
    
        if(process_rank == root_process)
        {
            // create file for array B
            fpB = fopen(filenameB,"r");
            if(fpB == NULL) // if file descriptor doesn't bind memory for file
            {
                perror("Error opening file for reading");
                fprintf(stderr,"Error with file %s \n",filenameB);
                exit(EXIT_FAILURE);
            }

            printf("Array B dimensions %dx1 \n",N);
            printf("Array B takes data from file %s \n",filenameB);
            for(int i=0; i<N; i++)
            {
                fscanf(fpB,"%d",&B[i]);
            }

            for(int i=0; i<N; i++)
            {
                printf("%d \n",B[i]);
            }
            printf("\n");
        }

        // Root process sends array B to other processes
        rc = MPI_Bcast(B,N,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Bcast");

        // Every process implements its local calculations

        sub_multiplication_CB = (int*) malloc(lines_per_process*sizeof(int));
        if(sub_multiplication_CB == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array sub multiplicationCB \n");
            exit(EXIT_FAILURE);
        }

        for(int i=0; i<lines_per_process; i++)
        {
            sub_multiplication_CB[i] = 0;
            for(int j=0; j<N; j++)
            {
                sub_multiplication_CB[i] += C_local[i*N+j] * B[j];
            }
        }

        free(C_local);

        // Root process defines the final array for the multiplication of C,B
        if(process_rank == root_process)
        {
            multiplication_CB = (int*) malloc(N*sizeof(int));
            if(multiplication_CB == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for array multiplicationCB \n");
                exit(EXIT_FAILURE);
            }
        }
    
        // We define arrays sendcounts and displs for array multiplications_CB
        offset = 0;
        for(int process=0; process<total_processes; process++)
        {
            int lines_process;
            if(process < remainder)
            {
                lines_process = base_count + 1;
            }
            else
            {
                lines_process = base_count;
            }

            // Every process calculates 'lines_process' elements and not 'lines_process*N' 
            sendcounts[process] = lines_process;
            displs[process] = offset;
            offset += sendcounts[process];
        }

        // Root process gathers elements from all processes 
        rc = MPI_Gatherv(sub_multiplication_CB,lines_per_process,MPI_INT,multiplication_CB,sendcounts,displs,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Gatherv");

        if(process_rank == root_process)
        {
            printf("Multiplication of arrays C,B: \n");
            for(int i=0; i<N; i++)
            {
                printf("%d \n",multiplication_CB[i]);
            }
            printf("\n");

            free(multiplication_CB);
        }

        // release memory
        free(sub_multiplication_CB);

        // QUESTION C
        A = (int*) malloc(N*sizeof(int));
        if(A == NULL)
        {
            fprintf(stderr,"Not enough memory \n");
            fprintf(stderr,"No memory allocation for array A \n");
            exit(EXIT_FAILURE);
        }

        if(process_rank == root_process)
        {
            // create file for array A
            fpA = fopen(filenameA,"r");
            if(fpA == NULL) // if file descriptor doesn't bind memory for file
            {
                perror("Error opening file for reading");
                fprintf(stderr,"Error with file %s \n",filenameA);
                exit(EXIT_FAILURE);
            }

            printf("Array A dimensions 1x%d \n",N);
            printf("Array A takes data from file %s \n",filenameA);
            for(int i=0; i<N; i++)
            {
                fscanf(fpA,"%d",&A[i]);
            }

            for(int i=0; i<N; i++)
            {
                printf("%d \n",A[i]);
            }
            printf("\n");
        }

        // Root process sends array A to other processes
        rc = MPI_Bcast(A,N,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Bcast");

        // Amount of elements for every process
        if(process_rank < remainder)
        {
            elements_per_process = base_count + 1;
        }
        else
        {
            elements_per_process = base_count;
        }

        // Every process calculates its starting index from displs
        start_index = displs[process_rank];

        // Every process calculates its sum
        local_sum = 0;
        for(int i=0; i<elements_per_process; i++)
        {
            local_sum += A[start_index + i] * B[start_index + i];
        }

        // Root process gathers all sub-sums of processes and it enforces sum for all sub-sums
        resultAB = 0;
        rc = MPI_Reduce(&local_sum,&resultAB,1,MPI_INT,MPI_SUM,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Reduce");

        if(process_rank == root_process)
        {
            printf("Multiplication of arrays A,B: %d \n",resultAB);
            printf("\n");
        }

        // release memory
        free(A);
        free(B);

        // QUESTION D
        if(N == total_processes)
        {
            // We bind memory for row array C
            RowC = (int*) malloc(N*sizeof(int));
            if(RowC == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for row array C \n");
                exit(EXIT_FAILURE);
            }

            // Root process scatters array C to other processes by lines.
            rc = MPI_Scatter(C,N,MPI_INT,RowC,N,MPI_INT,root_process,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD,rc,"MPI_Scatter");


            // We bind memory for row array D
            RowD = (int*) malloc(N*sizeof(int));
            if(RowD == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for row array D \n");
                exit(EXIT_FAILURE);
            }

            // Root process scatters array D to other processes by lines.
            rc = MPI_Scatter(D,N,MPI_INT,RowD,N,MPI_INT,root_process,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD,rc,"MPI_Scatter");

            // We define for every process the left(previous) and right(next) process
            if(process_rank == 0)
            {
                left = total_processes - 1;
                right = process_rank + 1;
            }
            else if(process_rank == total_processes - 1)
            {
                left = process_rank - 1;
                right = 0;
            }
            else
            {
                left = process_rank - 1;
                right = process_rank + 1;
            }

            // We bind memory for sub-multiplication of arrays C,D 
            SubProductCD = (int*) malloc(N*sizeof(int));
            if(SubProductCD == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for array SubProductCD \n");
                exit(EXIT_FAILURE);
            }

            for(int j=0; j<N; j++)
            {
                // We initialize the array SubProductCD
                SubProductCD[j] = 0;
            }

            for(int j=0; j<N; j++)
            {
                /* We calculate the partial contribution (sum) 
                RowC[process_rank][process_rank] * RowD[process_rank][j] */
                SubProductCD[j] += RowC[0] * RowD[j];
            }

            // All processes will exchange rows of array D

            // We bind memory for array send_data 
            send_data = (int*) malloc(N*sizeof(int));
            if(send_data == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for array send_data \n");
                exit(EXIT_FAILURE);
            }

            // We bind memory for array receive_data 
            receive_data = (int*) malloc(N*sizeof(int));
            if(receive_data == NULL)
            {
                fprintf(stderr,"Not enough memory \n");
                fprintf(stderr,"No memory allocation for array receive_data \n");
                exit(EXIT_FAILURE);
            }

            memcpy(send_data,RowD,N*sizeof(int)); // The current process will send its line
            for(int step=1; step<total_processes; step++)
            {
                // Every process sends its line in left(previous) process
                rc = MPI_Send(send_data,N,MPI_INT,left,tag1,MPI_COMM_WORLD);
                Abort(MPI_COMM_WORLD,rc,"MPI_Send");

                // Every process receives the next line from right(next) process
                rc = MPI_Recv(receive_data,N,MPI_INT,right,tag1,MPI_COMM_WORLD,&status);
                Abort(MPI_COMM_WORLD,rc,"MPI_Recv");

                // memory copy receive_data in RowD
                memcpy(RowD,receive_data,N*sizeof(int));

                // row of array RowC
                row = step;

                // Calculation of partial contribution
                for(int j=0; j<N; j++)
                {
                    SubProductCD[j] += RowC[row] * RowD[j];
                }

                // memory RowD in send_data
                memcpy(send_data,RowD,N*sizeof(int));
            }

            if(process_rank == root_process)
            {
                // We bind memory for array ProductCD 
                ProductCD = (int*) malloc(N*N * sizeof(int));
                if(ProductCD == NULL)
                {
                    fprintf(stderr,"Not enough memory \n");
                    fprintf(stderr,"No memory allocation for array ProductCD \n");
                    exit(EXIT_FAILURE);
                }
            }

            // Root process gathers contributions from all processes
            rc = MPI_Gather(SubProductCD,N,MPI_INT,ProductCD,N,MPI_INT,root_process,MPI_COMM_WORLD);
            Abort(MPI_COMM_WORLD,rc,"MPI_Gather");

            if(process_rank == root_process)
            {
                printf("Multiplication of arrays C,D: \n");
                for(int i=0; i<N; i++)
                {
                    for(int j=0; j<N; j++)
                    {
                        printf("%d ",ProductCD[i*N+j]);
                    }
                    printf("\n");
                }
                printf("\n");

                // release memory
                free(ProductCD);
            }

            // release memory
            free(RowC);
            free(RowD);
            free(SubProductCD);
            free(send_data);
            free(receive_data);
        }
        


        if(process_rank == root_process)
        {
            choice = menu(); // We display the menu
        }

        // Root process sends the choice in all processes
        rc = MPI_Bcast(&choice,1,MPI_INT,root_process,MPI_COMM_WORLD);
        Abort(MPI_COMM_WORLD,rc,"MPI_Bcast");
    }

    // release memory allocation
    free(sendcounts);
    free(displs);

    if(process_rank == root_process)
    {
        // release memory allocation
        free(C);
        free(D);
    }

    MPI_Finalize(); // Termination MPI environment

    return 0;
}
