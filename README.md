# Parallel and distributed programming (semestral work)
Finding tiling over the R grid by I shaped bricks except forbidden squares from set D with maximal cost. Various versions -- sequential, task parallel, data parallel, multiprocess (MPI).

## Input data:
* m, n = size of rectangular grid R[1..m,1..n] composed from m x n squares.
* 1 < i1 = the number of squares composing I brick of length i1 (= form I1).
* i2 = the number of squares composing I brick of length i2 (= form I2), i1 < i2 .
* 0 < c1 = price of placing I1 brick without any collision or penalty.
* c2 = price of placing I1 brick without any collision or penalty, c1 < c2.
* cn < 0 = penalty of square that is not covered by any I brick.
* k < m*n = the number of forbidden squares R.
* D[1..k] = array of coordinates of k forbidden squares inside the R grid.

## Task:
To find tiling over the R grid by I shaped bricks except forbidden squares from set D with maximal cost.
The cost of a tiling, where q squares remain untiled, is

**c1 * number of I1 bricks + c2 * number of I2 bricks - cn * q .**

I1 and I2 bricks can be rotated for tiling, i.e., you can place them horizontaly or verticaly.

## Output of the algorithm:
* Description of a R grid tiling.
For example, a m x n-matrix of integer identifiers where each I brick is represtended by a group of squares with the same unique ID >=1, untiled squares are empty, and forbiden squares are representbed by the 'F' letter.
* Output of the cost, the number and positions of untiled squares.
