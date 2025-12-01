#ifndef MXN_MATRIX_H
#define MXN_MATRIX_H

class mxn_Matrix {
 public:

  mxn_Matrix(void);
  mxn_Matrix(int m, int n);
  mxn_Matrix(int n, int m, double **vals);
  mxn_Matrix(const mxn_Matrix &m1);
  ~mxn_Matrix(void);

  int Get_M(void);
  int Get_N(void);
  
  void Assign_Row(int m, double *row);
  void Assign_Col(int n, double *col);
  void Assign_RowCol(int m, int n, double val);

  double *Get_Row(int m);
  double *Get_Col(int n);
  double Get_RowCol(int m, int n);
  mxn_Matrix Diag(void);

  mxn_Matrix Transpose(void);

  mxn_Matrix operator+(const mxn_Matrix& m1);
  mxn_Matrix operator-(const mxn_Matrix& m1);
  mxn_Matrix operator*(double val);
  mxn_Matrix operator*(const mxn_Matrix &m1);
  mxn_Matrix operator=(const mxn_Matrix &m1);
  double *operator[](int row);

  mxn_Matrix Inverse_GJ(void);
  mxn_Matrix Identity(void);

  void Print_Matrix(void);

  int M;
  int N;

 private:

  double **Get_RawValues(void);
  double **Values;
};

#endif
