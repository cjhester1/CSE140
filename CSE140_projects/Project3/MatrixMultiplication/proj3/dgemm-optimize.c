
//Cj Hester
//I received help on this project from my tutor sundeep kumar sundeep@sundeepkumar.com
//12/9/19
// Pre-Compute Transpose Method

//this was done by setting the for loop : for i = 0 last and putting for j = 0 first, (as it is reveresed in the naive method) , k remained the same
/*
 
 void dgemm(int m, int n, float *A, float *C)
 {
 for (int j = 0; j < m; j++)
 {
 for (int k = 0; k < n; j++)
 {
 for (int i = 0; i < m; i++)
 {
 C[i + j * m] += A[i + k * m] * A[j + k * m];
 
 }
 }
 }
 }
 
 */
// #####################################################################################################################

//loop unrolling method


/*
 
 
 
 
 use loop unrolling to expose register use
 ♦  ... c(i,j)
 += a(i,k) * b(k,j) += a(i+1,k) * b(k,j) += a(i,k) * b(k,j+1)
 c(i+1,j)
 c(i,j+1)
 c(i+1,j+1) += a(i+1,k) * b(k,j+1)
 
 
 Do kk=1,n,stride do ii=1,n,stride
 do j=1,n-2,2
 do i=ii,min(n,ii+stride-1),2
 do k=kk,min(n,kk+stride-1)
 c(i,j)
 c(i+1,j)
 c(i,j+1)
 c(i+1,j+1) += a(i+1,k)* b(k,j+1)
 
 
 
 */
void dgemm(int m, int n, float *A, float *C)
{
    int q;
    float w;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            
            q = 0;
            w = A[i + j * m];
            
            
            
            if (m % 4 != 0) {
                switch (m % 4) {
                    case 3:
                        C[i + 2 * m] += w * A[2 + j * m];
                    case 2:
                        C[i + 1 * m] += w * A[1 + j * m];
                    default:
                        C[i] += w * A[j*m];
                }
                q = m % 4;
            }
            do{
                C[i + q * m] += w * A[q + j * m];
                C[i + (q + 1) * m] += w * A[(q + 1) + j * m];
                C[i + (q + 2) * m] += w * A[(q + 2) + j * m];
                C[i + (q + 3) * m] += w * A[(q + 3) + j * m];
                q += 4;
            } while (q < m);
            
            
            
            
        }
    }
}
// #####################################################################################################################


//Blocking Method

// https://software.intel.com/en-us/articles/cache-blocking-techniques

// I used this website to learn more about utilizing blocking method

/*
 
 http://students.ceid.upatras.gr/~papadakis/mmm/mmm.pdf
 
 
 void matmul(long N, double *a, double *b, double *c)
 {
 
 long i, j, k;
 for (i = 0; i < N; i ++)
 for (j = 0; j < N; j ++)
 for (k = 0; k < N; k ++)
 c[i * N + j] += a[i * N + k] * b[k * N + j];
 
 
 )
 
 
 
 for(i=0;i<num;i++){
 
 for (j=0; j<num; j++){
 for (k=0; k<BLOCK_SIZE; k++){
 } }
 for (m=0; m<BLOCK_SIZE; m++) {
 double sum=0.0;
 for(r=0;r<num;r++){ for(p=0; p<BLOCK_SIZE; p++){
 sum += a[i*BLOCK_SIZE*N + r*BLOCK_SIZE + k*N + p]*
 b_t[j*BLOCK_SIZE*N + r*BLOCK_SIZE + m*N + p];
 } }
 c[i*BLOCK_SIZE*N + j*BLOCK_SIZE + k*N + m] = sum;
 
 }
 
 */

/*
 
 void dgemm( int m, int n, float *A, float *C )
 
 {
 
 int size = m;
 
 for(int i = 0; i < m; i+=size)
 
 for(int j = 0; j < m; j+=size)
 
 for(int k = 0; k < m; k+=size)
 
 for( int aj = 0; aj < size; aj++)
 
 for( int bk = 0; bk < n; bk++ )
 
 for( int ci = 0; ci < size; ci++ )
 
 C[(i + ci) + (j + aj) * m] += A[(i + ci) + (k + bk) * m] * A[( j + aj) + (k + bk) * m];
 
 }
 
 
 */



// #####################################################################################################################


