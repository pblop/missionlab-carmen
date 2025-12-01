#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "mxn_matrix.h"

mxn_Matrix::mxn_Matrix(void) {
  M = 1;
  N = 1;
  Values = new double *[1];
  Values[0] = new double[1];
  Values[0][0] = 0.0;
}




// create a mxn matrix with 0 as initial values
mxn_Matrix::mxn_Matrix(int m, int n) {
  int cnt, cnt2;

  M = m;
  N = n;

  // initialize the matrix as all 0's
  Values = new double *[M];
  for (cnt = 0; cnt < M; cnt++) {
    Values[cnt] = new double[N];
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      Values[cnt][cnt2] = 0;
    }
  }
}

// create mxn matrix with given initial values
mxn_Matrix::mxn_Matrix(int m, int n, double **vals) {
  int cnt, cnt2;

  M = m;
  N = n;

  // initialize the matrix to the given values
  Values = new double *[M];
  for (cnt = 0; cnt < M; cnt++) {
    Values[cnt] = new double[N];
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      Values[cnt][cnt2] = vals[cnt][cnt2];
    }
  }
}

mxn_Matrix::mxn_Matrix(const mxn_Matrix &m1) {
  int cnt, cnt2;

  M = m1.M;
  N = m1.N;

  // initialize the matrix to the given values
  Values = new double *[M];
  for (cnt = 0; cnt < M; cnt++) {
    Values[cnt] = new double[N];
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      Values[cnt][cnt2] = m1.Values[cnt][cnt2];
    }
  }
}

mxn_Matrix::~mxn_Matrix(void) {
  int cnt;

  for (cnt = 0; cnt < M; cnt++) {
    delete []Values[cnt];
  }
  delete []Values;
}

int mxn_Matrix::Get_N(void) {
  return N;
}

int mxn_Matrix::Get_M(void) {
  return M;
}

double **mxn_Matrix::Get_RawValues(void) {
  return Values;
}


// assign a row to the matrix
void mxn_Matrix::Assign_Row(int m, double *row) {
  int cnt;

  if ((m > M) || (m < 0)) {
    printf("Error, row out of range!\n");
    exit(1);
  }
  
  for (cnt = 0; cnt < N; cnt++) {
    Values[m][cnt] = row[cnt];
  }
}

// assign collumn to matrix
void mxn_Matrix::Assign_Col(int n, double *col) {
  int cnt;

  if ((n > N) || (n < 0)) {
    printf("Error, col out of range!\n");
    exit(1);
  }
  
  for (cnt = 0; cnt < M; cnt++) {
    Values[cnt][n] = col[cnt];
  }
}

// assign an index to the matrix
void mxn_Matrix::Assign_RowCol(int m, int n, double val) {

  Values[m][n] = val;
}


// return a row
double *mxn_Matrix::Get_Row(int m) {
  double *row;

  row = new double[N];
  memcpy((void *)row, (void *)Values[m], sizeof(double) * N);

  return row;
}


// return a collumn
double *mxn_Matrix::Get_Col(int n) {
  int cnt;
  double *col;
  
  col = new double[M];
  for (cnt = 0; cnt < M; cnt++) {
    col[cnt] = Values[cnt][n];
  }

  return col;
}


// return an index
double mxn_Matrix::Get_RowCol(int m, int n) {
  
  return Values[m][n];
}

double *mxn_Matrix::operator[](int row) {
  return Values[row];
}




// assignment operator
mxn_Matrix mxn_Matrix::operator=(const mxn_Matrix &m1) {
  int i, j;

  if (this == &m1) {
    return *this;
  }

  // delete the old values
  for (i = 0; i < M; i++) {
    delete []Values[i];
  }
  delete []Values;  

  // and copy over the new values
  M = m1.M;
  N = m1.N;
  
  Values = new double *[M];
  for (i = 0; i < M; i++) {
    Values[i] = new double[N];
    for (j = 0; j < N; j++) {
      Values[i][j] = m1.Values[i][j];

    }
  }
  return *this;
}


// addition operator
mxn_Matrix mxn_Matrix::operator+(const mxn_Matrix& m1) {
  int i, j;

  if ((m1.M != M) || (m1.N != N)) {
    printf("Can't add matricies, wrong size:  %d x %d     %d x %d\n", m1.M, m1.N, M, N);
    exit(1);
  }
  
  mxn_Matrix sum(M, N);

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      sum.Values[i][j] = m1.Values[i][j] + Values[i][j];
    }
  }

  return sum;
}

// subtraction operator
mxn_Matrix mxn_Matrix::operator-(const mxn_Matrix& m1) {
  int i, j;

  if ((m1.M != M) || (m1.N != N)) {
    printf("Can't subtract matricies, wrong size:  %d x %d     %d x %d\n", m1.M, m1.N, M, N);
    exit(1);
  }
  
  mxn_Matrix dif(M, N);

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      dif.Values[i][j] = Values[i][j] - m1.Values[i][j];
    }
  }

  return dif;
}


// multiplication operator
mxn_Matrix mxn_Matrix::operator*(const double val) {
  int i, j;
  mxn_Matrix mult(M, N);

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult.Values[i][j] = Values[i][j] * val;
    }
  }

  return mult;
}

// prints out the matrix
void mxn_Matrix::Print_Matrix(void) {
  int cnt, cnt2;
  printf("%d x %d\n", M, N);

  for (cnt = 0; cnt < M; cnt++) {
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      
      printf("%f", Values[cnt][cnt2]);
      if (cnt2 + 1 != N) {
	printf(",");
      }
    }
    printf("\n");
  }
}

mxn_Matrix mxn_Matrix::Transpose(void) {
  int cnt, cnt2;

  mxn_Matrix tempmatrix(N, M);

  for (cnt = 0; cnt < M; cnt++) {
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      tempmatrix.Values[cnt2][cnt] = Values[cnt][cnt2];
    }
  }

  return tempmatrix;
}


mxn_Matrix mxn_Matrix::operator*(const mxn_Matrix &m1) {
  int cnt, cnt2, cnt3;

  if (N != m1.M) {
    printf("nxm_Matrix Error: operator*: Invalid row/col sizes:  %dx%d   %dx%d\n", M, N, m1.M, m1.N);
    exit(1);
  }

  mxn_Matrix prod(M, m1.N);

  for (cnt = 0; cnt < M; cnt++) {
    for (cnt2 = 0; cnt2 < m1.N; cnt2++) {
      for (cnt3 = 0; cnt3 < N; cnt3++) {
	prod.Values[cnt][cnt2] += Values[cnt][cnt3] * m1.Values[cnt3][cnt2];
      }
    }
  }

  return prod;
}

mxn_Matrix mxn_Matrix::Identity(void) {
  int cnt;

  if (N != M) {
    printf("mxn_Matrix: Identity: Can't generate identity matrix for %d x %d matrix\n", M, N);
    exit(1);
  }

  mxn_Matrix idmat(N, N);

  for (cnt = 0; cnt < N; cnt++) {
    idmat.Values[cnt][cnt] = 1;
  }

  return idmat;
}
    

mxn_Matrix mxn_Matrix::Inverse_GJ(void) {
  int *indexcol, *indexrow, *pivot;
  int cnt, cnt2, cnt3 = 0, cnt4, cnt5;
  int colindex = 0, rowindex = 0; 
  double largest, dum, pivitinverse, temp;
  double curval;
  int maxindex;
  double *temprow;
  double **rawvals, **rawvals2;
  double subval, normval;


  if (M != N) {
    printf("Error: Inverse_GJ:   Matrix must be square!\n");
    exit(1);
  }

  mxn_Matrix m1(*this);

  //  m1.Print_Matrix();
  //  m2.Print_Matrix();

  int m = M;
  int n = M;
  indexcol = new int[n];                        
  indexrow = new int[n];                        
  pivot = new int[n];                         
  
  rawvals = m1.Get_RawValues();
  
  memset(pivot, 0, sizeof(int)*n);

  for (cnt = 0; cnt < n; cnt++) {            
    
    // find the pivot point
    largest = 0.0;                                 
    for (cnt2 = 0; cnt2 < n; cnt2++) {           
      
      // if we havent found a pivot for this col
      if (pivot[cnt2] != 1) {
	
	// go through each row
	for (cnt3 = 0; cnt3 < n; cnt3++) {
	  
	  // and we havent already worked on this column
	  if (!pivot[cnt3]) {
	    
	    // check to see if it is the largest value (which we will pivot on)
	    if (fabs(m1[cnt2][cnt3]) >= fabs(largest)) {  
	      largest = m1[cnt2][cnt3];      
	      rowindex = cnt2;                      
	      colindex = cnt3;                     
	    }
	  }
	  
	  else if (pivot[cnt3] > 1) {    
	    printf("Error: Inverse_GJ: Singular Matrix\n");
	    exit(1);
	  }
	}
      }
    }
    
    
    // mark that we have pivoted on this col
    pivot[colindex]++;                      
    
    // swap the pivot row to the diagonal
    if (rowindex != colindex) {              
      temprow = rawvals[rowindex];
      rawvals[rowindex] = rawvals[colindex];
      rawvals[colindex] = temprow;
    }	  
    
    // remember for row colindex we swaped col rowindex
    indexrow[cnt] = rowindex;                
    indexcol[cnt] = colindex;
    
    
    //  cant have 0 at a pivot
    if (m1[colindex][colindex] == 0.0) {
      printf("Error: Inverse_GJ: Singular Matrix\n");;
      exit(1);
    }
    
    
    // pivot becomes 1
    normval =  m1[colindex][colindex];
    m1[colindex][colindex] = 1.0;               
     
    // normalize based on the pivots old value
    for (cnt4 = 0; cnt4 < n; cnt4++) {
      m1[colindex][cnt4] *= (1.0 / normval);
    }


    // reduce the rows
    for (cnt5 = 0; cnt5 < n; cnt5++) {          
      if (cnt5 != colindex) {                   
	dum = m1[cnt5][colindex];               
	m1[cnt5][colindex] = 0.0;               
	for (cnt4 = 0; cnt4 < n; cnt4++) {
	  m1[cnt5][cnt4] -= (m1[colindex][cnt4] * dum);
	}
      }
    }
  }
  
  // now we unscrable the the cols based on the order the rows were pivoted
  for (cnt4 = n - 1; cnt4 >= 0; cnt4--) {

    // no need to swap the same
    if (indexrow[cnt4] != indexcol[cnt4]) {  

      // do the swap
      for (cnt3 = 0; cnt3 < n; cnt3++) {    
	temp = m1[cnt3][indexrow[cnt4]];
  	m1[cnt3][indexrow[cnt4]] = m1[cnt3][indexcol[cnt4]];
  	m1[cnt3][indexcol[cnt4]] = temp; 	      
      }
    }
  }
  
  delete []pivot;          
  delete []indexrow;       
  delete []indexcol;     
  
  return m1;
}

mxn_Matrix mxn_Matrix::Diag(void) { 
  int cnt, cnt2;
  mxn_Matrix temp(M,N);
  temp = *this;

  for (cnt = 0; cnt < M; cnt++) {
    for (cnt2 = 0; cnt2 < N; cnt2++) {
      if (cnt != cnt2) {
        temp.Values[cnt][cnt2] = 0.0;
      }
    }
  }

  return temp;
}

