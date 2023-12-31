% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/findNhoodGroupMarkers.R
\name{findNhoodGroupMarkers}
\alias{findNhoodGroupMarkers}
\title{Identify post-hoc neighbourhood marker genes}
\usage{
findNhoodGroupMarkers(
  x,
  da.res,
  assay = "logcounts",
  aggregate.samples = FALSE,
  sample_col = NULL,
  subset.row = NULL,
  gene.offset = TRUE,
  subset.nhoods = NULL,
  subset.groups = NULL,
  na.function = "na.pass"
)
}
\arguments{
\item{x}{A \code{\linkS4class{Milo}} object containing single-cell gene expression
and neighbourhoods.}

\item{da.res}{A \code{data.frame} containing DA results, as expected from running
\code{testNhoods}, as a \code{NhoodGroup} column specifying the grouping of neighbourhoods,
as expected from}

\item{assay}{A character scalar determining which \code{assays} slot to extract from the
\code{\linkS4class{Milo}} object to use for DGE testing.}

\item{aggregate.samples}{logical indicating wheather the expression values for cells in the same sample
and neighbourhood group should be merged for DGE testing. This allows to perform testing exploiting the replication structure
in the experimental design, rather than treating single-cells as independent replicates. The function used for aggregation depends on the
selected gene expression assay: if \code{assay="counts"} the expression values are summed, otherwise we take the mean.}

\item{sample_col}{a character scalar indicating the column in the colData storing sample information
(only relevant if \code{aggregate.samples==TRUE})}

\item{subset.row}{A logical, integer or character vector indicating the rows
of \code{x} to use for sumamrizing over cells in neighbourhoods.}

\item{gene.offset}{A logical scalar the determines whether a per-cell offset
is provided in the DGE GLM to adjust for the number of detected genes with
expression > 0.}

\item{subset.nhoods}{A logical, integer or character vector indicating which neighbourhoods
to subset before aggregation and DGE testing (default: NULL).}

\item{subset.groups}{A character vector indicating which groups to test for markers (default: NULL)}

\item{na.function}{A valid NA action function to apply, should be one of
\code{na.fail, na.omit, na.exclude, na.pass}.}
}
\value{
A \code{data.frame} of DGE results containing a log fold change and adjusted
p-value for each aggregated group of neighbourhoods. If \code{return.groups} then
the return value is a list with the slots \code{groups} and \code{dge} containing the
aggregated neighbourhood groups per single-cell and marker gene results, respectively.

\emph{Warning}: If all neighbourhoods are grouped together, then it is impossible to
run \code{findNhoodMarkers}. In this (hopefully rare) instance, this function will return
a warning and return \code{NULL}.
}
\description{
This function will perform differential gene expression analysis on
groups of neighbourhoods. Adjacent and concordantly DA neighbourhoods can be defined using
\code{groupNhoods} or by the user. Cells \emph{between} these
aggregated groups are compared. For differential gene experession based on an input design
\emph{within} DA neighbourhoods see \code{\link{testDiffExp}}.
}
\details{
Using a one vs. all approach, each aggregated group of cells is compared to all others
using the single-cell log normalized gene expression with a GLM
(for details see \code{\link[limma]{limma-package}}), or the single-cell counts using a
negative binomial GLM (for details see \code{\link[edgeR]{edgeR-package}}). When using
the latter it is recommended to set \code{gene.offset=TRUE} as this behaviour adjusts
the model offsets by the number of detected genes in each cell.
}
\examples{
NULL

}
