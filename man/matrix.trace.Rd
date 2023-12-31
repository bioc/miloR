% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/glmm.R
\name{matrix.trace}
\alias{matrix.trace}
\title{Compute the trace of a matrix}
\usage{
matrix.trace(x)
}
\arguments{
\item{x}{A \code{matrix}}
}
\value{
\code{numeric} scalar of the matrix trace.
}
\description{
Exactly what it says on the tin - compute the sum of the matrix diagonal
}
\details{
It computes the matrix trace of a square matrix.
}
\examples{
matrix.trace(matrix(runif(9), ncol=3, nrow=3))

}
\author{
Mike Morgan
}
