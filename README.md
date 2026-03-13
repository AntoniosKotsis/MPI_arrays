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
**EXECUTION EXAMPLES:**

N = 8, p = 8:

Array C dimensions 8x8 
Array C takes data from file numbersC.txt 
1 2 3 4 30 31 32 33 
5 6 7 8 34 35 36 37 
9 10 11 12 38 39 40 41 
13 14 15 16 42 43 44 45 
17 18 19 20 46 47 48 49 
21 22 23 24 50 51 52 53 
25 26 27 28 54 55 56 57 
29 30 31 32 58 59 60 61 

Array D dimensions 8x8 
Array D takes data from file numbersD.txt 
17 18 19 20 100 101 102 103 
21 22 23 24 110 111 112 113 
25 26 27 28 114 115 116 117 
29 30 31 32 118 119 120 121 
80 81 82 83 200 201 202 203 
84 85 86 87 204 205 206 207 
88 89 90 91 208 209 210 211 
92 93 94 95 212 213 214 215 

Process 7 found global max:276 in row:7 and column:7 

Array SumCD dimensions 8x8: 
18 20 22 24 130 132 134 136 
26 28 30 32 144 146 148 150 
34 36 38 40 152 154 156 158 
42 44 46 48 160 162 164 166 
97 99 101 103 246 248 250 252 
105 107 109 111 254 256 258 260 
113 115 117 119 262 264 266 268 
121 123 125 127 270 272 274 276 

Array B dimensions 8x1 
Array B takes data from file numbersB.txt 
100 
-4 
5 
7 
17 
25 
3 
8 

Multiplication of arrays C,B: 
1780 
2424 
3068 
3712 
4356 
5000 
5644 
6288 

Array A dimensions 1x8 
Array A takes data from file numbersA.txt 
10 
-20 
19 
456 
9 
11 
89 
99 

Multiplication of arrays A,B: 5854 

Multiplication of arrays C,D: 
11106 11242 11378 11514 27110 27246 27382 27518 
10975 11143 11311 11479 29208 29376 29544 29712 
10876 11076 11276 11476 31536 31736 31936 32136 
10809 11041 11273 11505 33896 34128 34360 34592 
10774 11038 11302 11566 36288 36552 36816 37080 
14297 14593 14889 15185 44186 44482 44778 45074 
17852 18180 18508 18836 51966 52294 52622 52950 
21439 21799 22159 22519 59778 60138 60498 60858 

Menu: 
1. Continue 
2. Exit 

Type a choice: 
1

Array C dimensions 8x8 
Array C takes data from file numbersC.txt 
1 2 3 4 30 31 32 33 
5 6 7 8 34 35 36 37 
9 10 11 12 38 39 40 41 
13 14 15 16 42 43 44 45 
17 18 19 20 46 47 48 49 
21 22 23 24 50 51 52 53 
25 26 27 28 54 55 56 57 
29 30 31 32 58 59 60 61 

Array D dimensions 8x8 
Array D takes data from file numbersD.txt 
17 18 19 20 100 101 102 103 
21 22 23 24 110 111 112 113 
25 26 27 28 114 115 116 117 
29 30 31 32 118 119 120 121 
80 81 82 83 200 201 202 203 
84 85 86 87 204 205 206 207 
88 89 90 91 208 209 210 211 
92 93 94 95 212 213 214 215 

Process 7 found global max:276 in row:7 and column:7 

Array SumCD dimensions 8x8: 
18 20 22 24 130 132 134 136 
26 28 30 32 144 146 148 150 
34 36 38 40 152 154 156 158 
42 44 46 48 160 162 164 166 
97 99 101 103 246 248 250 252 
105 107 109 111 254 256 258 260 
113 115 117 119 262 264 266 268 
121 123 125 127 270 272 274 276 

Array B dimensions 8x1 
Array B takes data from file numbersB.txt 
100 
-4 
5 
7 
17 
25 
3 
8 

Multiplication of arrays C,B: 
1780 
2424 
3068 
3712 
4356 
5000 
5644 
6288 

Array A dimensions 1x8 
Array A takes data from file numbersA.txt 
10 
-20 
19 
456 
9 
11 
89 
99 

Multiplication of arrays A,B: 5854 

Multiplication of arrays C,D: 
11106 11242 11378 11514 27110 27246 27382 27518 
10975 11143 11311 11479 29208 29376 29544 29712 
10876 11076 11276 11476 31536 31736 31936 32136 
10809 11041 11273 11505 33896 34128 34360 34592 
10774 11038 11302 11566 36288 36552 36816 37080 
14297 14593 14889 15185 44186 44482 44778 45074 
17852 18180 18508 18836 51966 52294 52622 52950 
21439 21799 22159 22519 59778 60138 60498 60858 

Menu: 
1. Continue 
2. Exit 

Type a choice: 
2
