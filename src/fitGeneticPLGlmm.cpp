#include<RcppArmadillo.h>
#include<RcppEigen.h>
#include<string>
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::depends(RcppEigen)]]
#include "paramEst.h"
#include "computeMatrices.h"
#include "invertPseudoVar.h"
#include "pseudovarPartial.h"
#include "multiP.h"
#include "inference.h"
#include "utils.h"
using namespace Rcpp;


//' GLMM parameter estimation using pseudo-likelihood with a custom covariance matrix
//'
//' Iteratively estimate GLMM fixed and random effect parameters, and variance
//' component parameters using Fisher scoring based on the Pseudo-likelihood
//' approximation to a Normal loglihood. This function incorporates a user-defined
//' covariance matrix, e.g. a kinship matrix for genetic analyses.
//'
//' @param Z mat - sparse matrix that maps random effect variable levels to
//' observations
//' @param X mat - sparse matrix that maps fixed effect variables to
//' observations
//' @param K mat - sparse matrix that defines the known covariance patterns between
//' individual observations. For example, a kinship matrix will then adjust for the
//' known/estimated genetic relationships between observations.
//' @param muvec vec vector of estimated phenotype means
//' @param offsets vec vector of model offsets
//' @param curr_theta vec vector of initial parameter estimates
//' @param curr_beta vec vector of initial beta estimates
//' @param curr_u vec of initial u estimates
//' @param curr_sigma vec of initial sigma estimates
//' @param curr_G mat c X c matrix of variance components
//' @param y vec of observed counts
//' @param u_indices List a List, each element contains the indices of Z relevant
//' to each RE and all its levels
//' @param theta_conv double Convergence tolerance for paramter estimates
//' @param rlevels List containing mapping of RE variables to individual
//' levels
//' @param curr_disp double Dispersion parameter estimate
//' @param REML bool - use REML for variance component estimation
//' @param maxit int maximum number of iterations if theta_conv is FALSE
//' @param solver string which solver to use - either HE (Haseman-Elston regression) or Fisher scoring
//' @param vardist string which variance form to use NB = negative binomial, P=Poisson
//'
//' @details Fit a NB-GLMM to the counts provided in \emph{y}. The model uses an iterative approach that
//' switches between the joint fixed and random effect parameter inference, and the variance component
//' estimation. A pseudo-likelihood approach is adopted to minimise the log-likelihood of the model
//' given the parameter estimates. The fixed and random effect parameters are estimated using
//' Hendersons mixed model equations, and the variance component parameters are then estimated with
//' the specified solver, i.e. Fisher scoring, Haseman-Elston or constrained Haseman-Elston regression. As
//' the domain of the variance components is [0, +\code{Inf}], any negative variance component estimates will
//' trigger the switch to the HE-NNLS solver until the model converges.
//'
//' @return A \code{list} containing the following elements (note: return types are dictated by Rcpp, so the R
//' types are described here):
//' \describe{
//' \item{\code{FE}:}{\code{numeric} vector of fixed effect parameter estimates.}
//' \item{\code{RE}:}{\code{list} of the same length as the number of random effect variables. Each slot contains the best
//' linear unbiased predictors (BLUPs) for the levels of the corresponding RE variable.}
//' \item{\code{Sigma:}}{\code{numeric} vector of variance component estimates, 1 per random effect variable. For this model the
//' last variance component corresponds to the input \emph{K} matrix.}
//' \item{\code{converged:}}{\code{logical} scalar of whether the model has reached the convergence tolerance or not.}
//' \item{\code{Iters:}}{\code{numeric} scalar with the number of iterations that the model ran for. Is strictly <= \code{max.iter}.}
//' \item{\code{Dispersion:}}{\code{numeric} scalar of the dispersion estimate computed off-line}
//' \item{\code{Hessian:}}{\code{matrix} of 2nd derivative elements from the fixed and random effect parameter inference.}
//' \item{\code{SE:}}{\code{matrix} of standard error estimates, derived from the hessian, i.e. the square roots of the diagonal elements.}
//' \item{\code{t:}}{\code{numeric} vector containing the compute t-score for each fixed effect variable.}
//' \item{\code{COEFF:}}{\code{matrix} containing the coefficient matrix from the mixed model equations.}
//' \item{\code{P:}}{\code{matrix} containing the elements of the REML projection matrix.}
//' \item{\code{Vpartial:}}{\code{list} containing the partial derivatives of the (pseudo)variance matrix with respect to each variance
//' component.}
//' \item{\code{Ginv:}}{\code{matrix} of the inverse variance components broadcast to the full Z matrix.}
//' \item{\code{Vsinv:}}{\code{matrix} of the inverse pseudovariance.}
//' \item{\code{Winv:}}{\code{matrix} of the inverse elements of W = D^-1 V D^-1}
//' \item{\code{VCOV:}}{\code{matrix} of the variance-covariance for all model fixed and random effect variable parameter estimates.
//' This is required to compute the degrees of freedom for the fixed effect parameter inference.}
//' \item{\code{CONVLIST:}}{\code{list} of \code{list} containing the parameter estimates and differences between current and previous
//' iteration estimates at each model iteration. These are included for each fixed effect, random effect and variance component parameter.
//' The list elements for each iteration are: \emph{ThetaDiff}, \emph{SigmaDiff}, \emph{beta}, \emph{u}, \emph{sigma}.}
//' }
//'
//' @author Mike Morgan
//'
//' @examples
//' NULL
//'
//' @name fitGeneticPLGlmm
//'
// [[Rcpp::export]]
List fitGeneticPLGlmm(const arma::mat& Z, const arma::mat& X, const arma::mat& K,
                      arma::vec muvec, arma::vec offsets, arma::vec curr_beta,
                      arma::vec curr_theta, arma::vec curr_u, arma::vec curr_sigma,
                      arma::mat curr_G, const arma::vec& y, List u_indices,
                      double theta_conv,
                      const List& rlevels, double curr_disp, const bool& REML, const int& maxit,
                      std::string solver,
                      std::string vardist){

    // declare all variables
    List outlist(12);
    int iters=0;
    int stot = Z.n_cols;
    const int& c = curr_sigma.size();
    const int& m = X.n_cols;
    const int& n = X.n_rows;
    bool meet_cond = false;
    double constval = 0.0; // value at which to constrain values
    double _intercept = constval; // intercept for HE regression

    // setup matrices
    arma::mat D(n, n);
    D.zeros();
    arma::mat Dinv(n, n);
    Dinv.zeros();

    arma::vec y_star(n);

    arma::mat Vmu(n, n);
    arma::mat W(n, n);
    arma::mat Winv(n, n);

    arma::mat V_star(n, n);
    arma::mat V_star_inv(n, n);
    arma::mat P(n, n);

    arma::mat coeff_mat(m+c, m+c);
    List V_partial(c);
    V_partial = pseudovarPartial_G(Z, K, u_indices);
    // compute outside the loop
    List VP_partial(c);

    arma::vec score_sigma(c);
    arma::mat information_sigma(c, c);
    arma::vec sigma_update(c);
    arma::vec sigma_diff(sigma_update.size());
    sigma_diff.zeros();

    arma::mat G_inv(stot, stot);
    arma::mat Zstar(n, stot);
    arma::mat Gfill(stot, stot);

    arma::vec theta_update(m+stot);
    arma::vec theta_diff(theta_update.size());
    theta_diff.zeros();

    List conv_list(maxit+1);

    // create a uvec of sigma indices
    arma::uvec _sigma_index(c);
    for(int i=0; i < c; i++){
        _sigma_index[i] = i+1;
    }

    // setup vectors to index the theta updates
    // assume always in order of beta then u
    arma::uvec beta_ix(m);
    for(int x=0; x < m; x++){
        beta_ix[x] = x;
    }

    arma::uvec u_ix(stot);
    for(int px = 0; px < stot; px++){
        u_ix[px] = m + px;
    }

    // we only need to invert the Kinship once
    unsigned long _kn = K.n_cols;
    arma::mat Kinv(_kn, _kn);

    // check this isn't singular first - it could be due to a block structure
    double _rcond = arma::rcond(K);
    bool is_singular;
    is_singular = _rcond < 1e-9;

    // check for singular condition
    if(is_singular){
        // first try to invert the top block which should be N/2 x N/2
        Rcpp::warning("Kinship is singular - attempting broad cast inverse");
        double nhalfloat = (double)n/2;
        unsigned int nhalf = nhalfloat;
        Kinv = broadcastInverseMatrix(K, nhalf);
    } else{
        Kinv = arma::inv(K); // this could be very slow
    }

    double pcg_tol = 1e-6;

    bool converged = false;
    while(!meet_cond){
        D.diag() = muvec; // data space
        Dinv = D.i();
        y_star = computeYStar(X, curr_beta, Z, Dinv, curr_u, y, offsets); // data space

        // pick an initial dispersion value close to 1
        curr_disp = 1;

        Vmu = Vmu = computeVmu(muvec, curr_disp, vardist);
        W = computeW(curr_disp, Dinv, vardist);
        Winv = W.i();

        V_star = computeVStar(Z, curr_G, W); // K is implicitly included in curr_G
        V_star_inv = invertPseudoVar(Winv, curr_G, Z);

        if(REML){
            P = computePREML(V_star_inv, X);
            // take the partial derivative outside the while loop, just keep the P*\dVar\dSigma
            VP_partial = pseudovarPartial_P(V_partial, P);

            score_sigma = sigmaScoreREML_arma(VP_partial, y_star, P);
            information_sigma = sigmaInfoREML_arma(VP_partial, P);
        } else{
            List VP_partial = V_partial;
            score_sigma = sigmaScore(y_star, curr_beta, X, VP_partial, V_star_inv);
            information_sigma = sigmaInformation(V_star_inv, VP_partial);
        };

        // choose between HE regression and Fisher scoring for variance components
        // sigma_update is always 1 element longer than the others with HE, but we need to keep track of this
        if(solver == "HE"){
            // try Haseman-Elston regression instead of Fisher scoring
            sigma_update = estHasemanElstonGenetic(Z, P, u_indices, y_star, K);
        } else if (solver == "HE-NNLS"){
            // for the first iteration use the current non-zero estimate
            arma::dvec _curr_sigma(c+1);
            _curr_sigma.fill(constval);

            if(iters > 0){
                _curr_sigma[0] = _intercept;
                _curr_sigma.elem(_sigma_index) = curr_sigma; // is this valid to set elements like this?
            }

            sigma_update = estHasemanElstonConstrainedGenetic(Z, P, u_indices, y_star, K, _curr_sigma, iters);
            _intercept = sigma_update[0];
            sigma_update = sigma_update.tail(c);

        }else if(solver == "Fisher"){
            sigma_update = fisherScore(information_sigma, score_sigma, curr_sigma);
        }

        // if we have negative sigmas then we need to switch solver
        if(any(sigma_update < 0.0)){
            warning("Negative variance components - re-running with NNLS");
            solver = "HE-NNLS";
            // for the first iteration use the current non-zero estimate
            arma::dvec _curr_sigma(c+1, arma::fill::zeros);
            _curr_sigma.fill(constval);

            // if these are all zero then it can only be that they are initial estimates
            if(iters > 0){
                _curr_sigma[0] = _intercept;
                _curr_sigma.elem(_sigma_index) = curr_sigma; // is this valid to set elements like this?
            }
            sigma_update = estHasemanElstonConstrainedGenetic(Z, P, u_indices, y_star, K, _curr_sigma, iters);
        }

        sigma_diff = abs(sigma_update - curr_sigma); // needs to be an unsigned real value

        // update sigma, G, and G_inv
        curr_sigma = sigma_update;
        curr_G = initialiseG_G(u_indices, curr_sigma, K);
        G_inv = invGmat_G(u_indices, curr_sigma, Kinv);

        // Next, solve pseudo-likelihood GLMM equations to compute solutions for beta and u
        // compute the coefficient matrix
        // If using PCG then also use Cholesky factorisation of G - this hasn't been implemented yet
        coeff_mat = coeffMatrix(X, Winv, Z, G_inv); //model space
        // coeff_mat = coeffMatrix(X, Winv, Zstar, Gfill);
        // theta_update = solveEquationsPCG(stot, m, Winv, Zstar.t(), X.t(), coeff_mat, curr_theta, y_star, pcg_tol);
        theta_update = solveEquations(stot, m, Winv, Zstar.t(), X.t(), coeff_mat, curr_beta, curr_u, y_star); //model space

        LogicalVector _check_theta = check_na_arma_numeric(theta_update);
        bool _any_ystar_na = any(_check_theta).is_true(); // .is_true required for proper type casting to bool

        if(_any_ystar_na){
            List this_conv(5);
            this_conv = List::create(_["ThetaDiff"]=theta_diff, _["SigmaDiff"]=sigma_diff, _["beta"]=curr_beta,
                                     _["u"]=curr_u, _["sigma"]=curr_sigma);
            conv_list(iters-1) = this_conv;
            warning("NaN in theta update");
            break;
        }

        theta_diff = abs(theta_update - curr_theta);

        curr_theta = theta_update; //model space
        curr_beta = curr_theta.elem(beta_ix); //model space
        curr_u = curr_theta.elem(u_ix); //model space

        muvec = exp(offsets + (X * curr_beta) + (Z * curr_u)); // data space

        LogicalVector _check_mu_inf = check_inf_arma_numeric(muvec);
        bool _any_mu_inf = any(_check_mu_inf).is_true();

        if(_any_mu_inf){
            List this_conv(5);
            this_conv = List::create(_["ThetaDiff"]=theta_diff, _["SigmaDiff"]=sigma_diff, _["beta"]=curr_beta,
                                     _["u"]=curr_u, _["sigma"]=curr_sigma);
            conv_list(iters-1) = this_conv;
            warning("Inf values in muvec - algorithm is diverging");
            break;
        }

        iters++;

        bool _thconv = false;
        _thconv = all(theta_diff < theta_conv);

        bool _siconv = false;
        _siconv = all(sigma_diff < theta_conv);

        bool _ithit = false;
        _ithit = iters > maxit;

        meet_cond = ((_thconv && _siconv) || _ithit);
        converged = _thconv && _siconv;
        List this_conv(5);
        this_conv = List::create(_["ThetaDiff"]=theta_diff, _["SigmaDiff"]=sigma_diff, _["beta"]=curr_beta,
                                 _["u"]=curr_u, _["sigma"]=curr_sigma);
        conv_list(iters-1) = this_conv;
    }

    // inference
    arma::vec se(computeSE(m, stot, coeff_mat));
    arma::vec tscores(computeTScore(curr_beta, se));
    arma::mat vcov(varCovar(VP_partial, c)); // DF calculation is done in R, but needs this

    outlist = List::create(_["FE"]=curr_beta, _["RE"]=curr_u, _["Sigma"]=curr_sigma,
                           _["converged"]=converged, _["Iters"]=iters, _["Dispersion"]=curr_disp,
                           _["Hessian"]=information_sigma, _["SE"]=se, _["t"]=tscores,
                           _["COEFF"]=coeff_mat, _["P"]=P, _["Vpartial"]=VP_partial, _["Ginv"]=G_inv,
                           _["Vsinv"]=V_star_inv, _["Winv"]=Winv, _["VCOV"]=vcov, _["CONVLIST"]=conv_list);

    return outlist;
}


