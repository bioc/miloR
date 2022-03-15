#include "paramEst.h"
#include "utils.h"
#include<RcppArmadillo.h>
#include<RcppEigen.h>
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::depends(RcppEigen)]]
// using namespace Rcpp;

// All functions used in parameter estimation

arma::vec sigmaScoreREML_arma (Rcpp::List pvstar_i, const arma::vec& ystar, const arma::mat& P){
    // Armadillo implementation
    const int& c = pvstar_i.size();
    const int& n = P.n_cols;
    arma::vec reml_score(c);
    // arma::sp_mat _P(P);

    for(int i=0; i < c; i++){
        const arma::mat& _pvi = pvstar_i(i);
        arma::mat P_pvi(n, n);
        P_pvi = P * _pvi; // this is a slow operation <- could we speed this up with sparse matrices?

        double lhs = -0.5 * arma::trace(P_pvi);
        arma::mat mid1(1, 1);
        mid1 = arma::trans(ystar) * P * _pvi * P * ystar;
        double rhs = 0.5 * mid1[0, 0];

        reml_score[i] = lhs + rhs;
    }

    return reml_score;
}


arma::mat sigmaInfoREML_arma (const Rcpp::List& pvstari, const arma::mat& P){
    // REML Fisher/expected information matrix
    const int& c = pvstari.size();
    arma::mat sinfo(c, c);

    // this is a symmetric matrix so only need to fill the upper or
    // lower triangle to make it O(n^2/2) rather than O(n^2)
    for(int i=0; i < c; i++){
        const arma::mat& _ip = pvstari(i);
        arma::mat _ipP = _ip * P;

        for(int j=i; j < c; j++){
            const arma::mat& _jp = pvstari(j);
            // the armadillo implementation is faster than the Eigen.
            // is this always a dense matrix?
            // using the cycling property of a trace doesn't make any discernible difference
            // we only need the diagonal elements of this multiplication - can we reduce the number of
            // operations this way??
            // No - waa
            arma::mat a_ij(P * _ipP * _jp); // this is the biggest bottleneck - it takes >2s!
            double _artr = arma::trace(a_ij);

            sinfo(i, j) = 0.5 * _artr;
            if(i != j){
                sinfo(j, i) = 0.5 * _artr;
            }

        }
    }

    return sinfo;
}


arma::vec sigmaScore (arma::vec ystar, arma::vec beta, arma::mat X, Rcpp::List V_partial, arma::mat V_star_inv){

    int c = V_partial.size();
    int n = X.n_rows;
    arma::vec score(c);
    arma::vec ystarminx(n);
    ystarminx = ystar - (X * beta);

    for(int i=0; i < c; i++){
        arma::mat _ip = V_partial(i);
        double lhs = -0.5 * arma::trace(V_star_inv * _ip);
        arma::mat rhs_mat(1, 1);

        rhs_mat = ystarminx.t() * V_star_inv * _ip * V_star_inv * ystarminx;
        score[i] = lhs + 0.5 * rhs_mat(0, 0);
    }

    return score;
}


arma::mat sigmaInformation (arma::mat V_star_inv, Rcpp::List V_partial){
    int c = V_partial.size();
    int n = V_star_inv.n_cols;
    arma::mat sinfo = arma::zeros(c, c);

    for(int i=0; i < c; i++){
        arma::mat _ip = V_partial(i);
        for(int j=0; j < c; j++){
            arma::mat _jp = V_partial(j);

            arma::mat _inmat(n, n);
            double _tr = 0.0;
            _inmat = V_star_inv * _ip * V_star_inv * _jp;
            _tr = 0.5 * arma::trace(_inmat);

            sinfo(i, j) = _tr;
        }
    }

    return sinfo;
}


arma::vec FisherScore (arma::mat hess, arma::vec score_vec, arma::vec theta_hat){
    // sequentially update the parameter using the Newton-Raphson algorithm
    // theta ~= theta_hat + hess^-1 * score
    // this needs to be in a direction of descent towards a minimum
    int m = theta_hat.size();
    arma::vec theta(m);
    arma::mat hessinv(hess.n_rows, hess.n_cols);
    // hessinv = arma::inv(hess);
    hessinv = arma::inv(hess); // always use pinv? solve() and inv() are most sensitive than R versions

    theta = theta_hat + (hessinv * score_vec);
    return theta;
}


arma::mat coeffMatrix(const arma::mat& X, const arma::mat& Winv, const arma::mat& Z, const arma::mat& Ginv){
    // compute the coefficient matrix from the MMEs
    int c = Z.n_cols;
    int m = X.n_cols;

    arma::mat ul(m, m);
    arma::mat ur(m, c);
    arma::mat ll(c, m);
    arma::mat lr(c, c);

    arma::mat lhs_top(m, m+c);
    arma::mat lhs_bot(c, m+c);
    arma::mat lhs(m+c, m+c);

    ul = X.t() * Winv * X;
    ur = X.t() * Winv * Z;
    ll = Z.t() * Winv * X;
    lr = (Z.t() * Winv * Z) + Ginv;

    lhs_top = arma::join_rows(ul, ur); // join_rows matches the rows i.e. glue columns together
    lhs_bot = arma::join_rows(ll, lr);

    lhs = arma::join_cols(lhs_top, lhs_bot); // join_cols matches the cols, i.e. glue rows together

    return lhs;
}

arma::vec solve_equations (const int& c, const int& m, const arma::mat& Winv, const arma::mat& Zt, const arma::mat& Xt,
                           arma::mat coeffmat, arma::vec beta, arma::vec u, const arma::vec& ystar){
    // solve the mixed model equations
    arma::vec rhs_beta(m);
    arma::vec rhs_u(c);
    arma::mat rhs(m+c, 1);

    arma::vec theta_up(m+c);

    rhs_beta.col(0) = Xt * Winv * ystar;
    rhs_u.col(0) = Zt * Winv * ystar;

    rhs = arma::join_cols(rhs_beta, rhs_u);
    theta_up = arma::inv(coeffmat) * rhs;

    return theta_up;
}

