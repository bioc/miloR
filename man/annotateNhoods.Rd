% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/annotateNhoods.R
\name{annotateNhoods}
\alias{annotateNhoods}
\title{Add annotations from colData to DA testing results}
\usage{
annotateNhoods(x, da.res, coldata_col, subset.nhoods = NULL)
}
\arguments{
\item{x}{A \code{\linkS4class{Milo}} object containing single-cell gene expression
and neighbourhoods.}

\item{da.res}{A \code{data.frame} containing DA results, as expected from running
\code{testNhoods}.}

\item{coldata_col}{A character scalar determining which column of \code{colData(x)} stores
the annotation to be added to the neighbourhoods}

\item{subset.nhoods}{A character, numeric or logical vector that will subset the annotation to the specific nhoods. If
a character vector these should correspond to row names of \code{nhoodCounts}. If a logical vector then
these should have the same \code{length} as \code{nrow} of \code{nhoodCounts}. If numeric, then these are assumed
to correspond to indices of \code{nhoodCounts} - if the maximal index is greater than \code{nrow(nhoodCounts(x))}
an error will be produced. This is necessary if \code{testNhoods} was run using \code{subset.nhoods=...}.}
}
\value{
A \code{data.frame} of model results (as \code{da.res} input) with two new columns: (1) \code{coldata_col} storing
the assigned label for each neighbourhood; (2) \code{coldata_col_fraction} storing the fraction of cells in the neighbourhood with
the assigned label.
}
\description{
This function assigns a categorical label to neighbourhoods in the differential abundance results
data.frame (output of \code{testNhoods}), based on the most frequent label among cells in each
neighbourhood. This can be useful to stratify DA testing results by cell types or samples.
Also the fraction of cells carrying that label is stored.
}
\details{
For each neighbourhood, this calculates the most frequent value of \code{colData(x)[coldata_col]}
among cells in the neighbourhood and assigns that value as annotation for the neighbourhood, adding a column in the
\code{da.res} data.frame. In addition, a \code{coldata_col_fraction} column will be added, storing the fraction of cells
carrying the assigned label. While in practice neighbourhoods are often homogeneous, one might choose to remove an
annotation label when the fraction of cells with the label is too low (e.g. below 0.6).
}
\examples{

NULL

}
\author{
Emma Dann
}
