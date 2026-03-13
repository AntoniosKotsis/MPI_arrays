# MPI_arrays
Management arrays with library MPI

**Common Design for All Tasks:**

1. We define arrays **C** and **D**.
2. Processor 0 will either read the arrays’ data from user input or from a file.
   We will use the second option for N=8.

**Implementation:**

1. Define 2D integer arrays **C** and **D** with dimensions NxN.
2. Define two file descriptors for arrays **C** and **D**.
   Define two variables for the filenames that contain the data.
   Use `fopen` to open the files in read mode.
3. Check for errors with the file descriptors.
   If an error occurs, terminate the program using `exit`.
   If the second file descriptor fails, close the first one and exit.
4. For each array, use `fscanf` to read the data from the file into the array.

---

**Menu:**

**Design:**

1. Create a menu with 2 options: continue or exit.
2. The user chooses whether to continue or not.
3. If an invalid value is entered, display an error message and request input again until valid.
4. Processor 0 reads the user’s choice.
5. Processor 0 sends the choice to all other processes.
6. The other processes receive the user’s choice.

**Note:**
If processor 0 does not send the user’s “exit” choice (option 2), processor 0 will finish while the other processors continue, which may cause a deadlock.
Thus, processor 0 must broadcast the user’s choice to all processors to ensure all either continue or exit.

**Implementation:**

1. Implement a `menu` function with no parameters that returns the user’s choice.
2. Use `scanf` to read the user’s selection.
3. Use a `while` loop to validate input.
4. Call the `menu` function on processor 0.
5. Use `MPI_Bcast` to broadcast the user’s choice from processor 0 to all other processes.

---

**Task I – Sum of Matrices C and D:**

**Design:**

1. Compute how many rows of **C** and **D** each processor will handle.
2. Processor 0 distributes the arrays based on row count.
3. Each processor calculates the rows of the resulting array that correspond to it.
   Example: processor 0 handles rows 1–2 of the matrices.
4. Processor 0 gathers all processors’ results.
5. Processor 0 displays the final result.

**Implementation:**

1. Divide N by the number of processes (assume N is a multiple of the process count).
2. Use `MPI_Scatterv` to distribute arrays based on row count.
3. Use `for` loops on local arrays to compute local results.
4. Use `MPI_Gatherv` to collect results at processor 0.
5. Processor 0 displays the final array.

All arrays are dynamically allocated using `malloc`.

**Subtask a:**
Use `MPI_Scatterv` and `MPI_Gatherv` to distribute and collect varying amounts of data per process.

**Subtask b – Find Global Maximum:**

**Design:**

1. Each processor finds its local maximum and its position in the local array.
2. Each processor computes the position of its local maximum in the final array using offsets.
3. All processors send their local maximum and position to processor 0.
   Processor 0 finds the global maximum and the processor that found it.
4. The processor that found the global maximum sends the position to processor 0.
5. Processor 0 displays the processor number, row, and column of the global maximum.

**Implementation:**

1. Use nested `for` loops to access local array elements.
2. Compute local maximum position and offset with the `displs` array.
3. Use `MPI_Reduce` to find the global maximum.
4. Use `MPI_Bcast` to inform all processors which processor found the global maximum.
5. The processor with the global maximum sends its position using `MPI_Send`. Processor 0 receives it with `MPI_Recv`.
6. Compute row = position / N, column = position % N.

---

**Task II – Multiplication of C and B:**

**Design:**

1. Define array **B** with N elements.
2. Processor 0 reads **B** from a file.
3. Processor 0 broadcasts **B** to all processors.
4. Each processor computes its local part of the multiplication.
5. Processor 0 gathers local results.
6. Processor 0 computes and displays the final array.

**Implementation:**

1. Dynamically allocate **B** with `malloc`.
2. Open a file pointer for **B** and read values into the array.
3. Use `MPI_Bcast` to broadcast **B**.
4. Each process computes its local results in a loop.
5. Use `MPI_Gatherv` to collect results at processor 0.
6. Processor 0 displays the final array.

---

**Task III – Dot Product of A and B:**

**Design:**

1. Declare array **A**.
2. Processor 0 reads **A** from a file.
3. Broadcast **A** to all processors.
4. Compute how many elements each process handles.
5. Determine the starting index for each process.
6. Each process computes its local sum.
7. Processor 0 collects all local sums and computes the final sum.

**Implementation:**

1. Dynamically allocate **A** using `malloc`.
2. Read **A** from a file using a file pointer.
3. Broadcast **A** using `MPI_Bcast`.
4. Each process calculates its local sum using its assigned elements.
5. Use `MPI_Reduce` to sum local sums at processor 0.

---

**Task IV – Ring Algorithm for Multiplying C and D:**

**Design:**

* Only execute if N = number of processors.

1. Each processor receives one row of **C** and **D** based on its rank.
2. Compute the partial contribution to the resulting array.
3. Perform circular exchange of **D**’s rows.

   * Each processor receives the next row from the next processor.
   * Each processor sends its row to the previous processor.
4. Compute each element `[rank][next_row]` using the received row.

**Implementation:**

1. Check condition N = number of processors; if not, display an error and continue.
2. Free previous `C_local` and `D_local`, reallocate with `malloc` for N elements.
3. Use `MPI_Scatter` to distribute rows of **C** and **D**.
4. Allocate `ProductCD` dynamically, initialize to zero.
5. Compute partial contributions for the local row.
6. Allocate `SubProductCD`, `send_data`, `receive_data` dynamically.
7. Use a loop to compute contributions for remaining rows.
8. Processor 0 gathers all partial contributions with `MPI_Gather`.
9. Processor 0 displays the final matrix.

**Special case when N % p = 0:**

* Distribute N/p rows per process.
* Multiply blocks of rows instead of single elements.
* Send/receive (N/p)*N elements.

---

**Sample Executions (English):**

**N = 8, p = 4:**

```
Array C dimensions 8x8
Array C read from file numbersC.txt
1 2 3 4 30 31 32 33
5 6 7 8 34 35 36 37
...
Array D dimensions 8x8
Array D read from file numbersD.txt
17 18 19 20 100 101 102 103
...
Process 3 found global max: 276 at row 7, column 7

Array SumCD dimensions 8x8
18 20 22 24 130 132 134 136
...

Array B dimensions 8x1
Array B read from file numbersB.txt
100
-4
5
...

Multiplication of arrays C*B
1780
2424
...

Array A dimensions 1x8
Array A read from file numbersA.txt
10
-20
19
...

Multiplication of arrays A*B
1100

Menu:
1. Continue
2. Exit
Type a choice:
1
...
Type a choice:
2
```
N = 8, p = 2:

(Similar output; global max found by process 1, etc.)

N = 8, p = 8:

(Similar output; global max found by process 7, includes multiplication of C*D.)
