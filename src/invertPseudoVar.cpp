#include<RcppArmadillo.h>
#ifdef _OPENMP
#include <omp.h>
#endif
// [[Rcpp::depends(RcppArmadillo)]]
#include "invertPseudoVar.h"
using namespace Rcpp;

arma::mat invertPseudoVar(const arma::mat& A, const arma::mat& B, const arma::mat& Z,
                          const arma::mat& ZtA){
    int c = B.n_cols;
    int n = A.n_cols;

    arma::mat omt(n, n);
    arma::mat mid(c, c);
    arma::mat ZB(n, c);

    // test some openmp parallelisation - saves ~2s on n=1000 with ~50 threads
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            ZB = Z * B;
        }

        #pragma omp section
        {
            mid = arma::eye<arma::mat>(c, c) + ZtA * ZB;
        }
    }

    double _rcond = arma::rcond(mid);
    if (_rcond < 1e-12) {
        Rcpp::warning("Pseudovariance component matrix is computationally singular");
        arma::mat midinv = arma::pinv(mid); // no guarantee on PD - use pseudoinverse
        omt = A - A * ZB * (midinv * ZtA); // stack multiplications like this appear to be slow
    } else{
        arma::mat midinv = arma::inv(mid); // no guarantee on PD.
        // this is hard to speed up - main bottleneck
        omt = A - A * ZB * (midinv * ZtA);
    }

    return omt;
}


arma::mat kRankOneUpdates(const arma::mat& Vinv, const arma::mat& B){
    // use the sum of k-rank one updates to compute the inverse of
    // the pseudo-covariance matrix, given we have the inverse from
    // iteration i-1 - done by k successive updates to the rows of Vinv
    // the vector u=column vector of zeros, except 1 in the entry for
    // the kth position for k in {1, 2, ..., n}
    // the vector v^T = the kth row of the matrix B containing the updates
    // from the parameter estimation

    const int n = B.n_rows;
    arma::mat vupdate = Vinv;
    arma::mat _vup(n, n, arma::fill::zeros);

    for(int k=0; k < n; k++){
        // select the kth row of B
        arma::drowvec vt_k = B.row(k); // automatically a row vector, i.e. v^T
        arma::uvec u = arma::zeros<arma::uvec>(n);
        u[k] = 1;

        _vup = rankOneUp(Vinv, u, vt_k);
        vupdate.row(k) = _vup.row(k);
    }

    return vupdate;
}


arma::mat rankOneUp(const arma::mat& A, const arma::uvec& u, const arma::drowvec& v){
    // compute the update inverse for A, u, v^T

    arma::dvec Au = A*u;
    arma::drowvec vA = v * A;

    // error in here
    return A - (((Au * vA))/arma::as_scalar(1 + vA*u));

}



