#' @title
#' Milo class definition
#'
#' @description
#' The class definition container to hold the data structures required for the Milo workflow.
#'
#' @slot graph An igraph object that represents the kNN graph
#' @slot nhoods A CxN binary sparse matrix mapping cells to the neighbourhoods they belong to
#' @slot nhoodDistances An list of PxN sparse matrices of Euclidean distances
#' between vertices in each neighbourhood, one matrix per neighbourhood
#' @slot nhoodCounts An NxM sparse matrix of cells counts in each neighourhood
#' across M samples
#' @slot nhoodIndex A list of the index vertices for each neighbourhood
#' @slot nhoodExpression An GxN matrix of genes X neighbourhoods containing
#' average gene expression levels across cells in each neighbourhood
#' @slot nhoodReducedDim a list of reduced dimensional representations of
#' neighbourhoods, including projections into lower dimension space
#' @slot nhoodGraph an igraph object that represents the graph of neighbourhoods
#' @slot .k A hidden slot that stores the value of k used for graph building
#'
#' @returns A Milo class object - see object builder help pages for details
#'
#' @importClassesFrom Matrix dgCMatrix dsCMatrix dgTMatrix dgeMatrix ddiMatrix sparseMatrix
setClassUnion("matrixORMatrix", c("matrix", "dgCMatrix", "dsCMatrix", "ddiMatrix",
                                  "dgTMatrix", "dgeMatrix"))
setClassUnion("characterORNULL", c("character", "NULL"))
setClassUnion("listORNULL", c("list", "NULL"))
setClassUnion("numericORNULL", c("numeric", "NULL"))
#' @aliases Milo
#' @rdname Milo
#'
#' @export
#' @importFrom SingleCellExperiment SingleCellExperiment
#' @importFrom S4Vectors SimpleList
setClass("Milo",
    contains = "SingleCellExperiment",
    slots=c(
        graph = "list", # this should be a list or an igraph object
        nhoods = "matrixORMatrix", # this should be a matrix
        nhoodDistances = "listORNULL", # this should be a matrix
        nhoodCounts = "matrixORMatrix", # this should be a matrix
        nhoodIndex = "list", # used to store nhood indices
        nhoodExpression = "matrixORMatrix", # this should be NA or a matrix
        nhoodReducedDim = "list", # this should be a list
        nhoodGraph = "list", # this should be an igraph object (I'm copying from the graph slot)
        nhoodAdjacency = "matrixORMatrix", # to save on computing adjacency multiple times
        .k = "numericORNULL" # must be an integer or not set
        ),
    prototype = list(
        graph = list(),
        nhoods = Matrix::Matrix(0L, sparse=TRUE),
        nhoodDistances = NULL,
        nhoodCounts = Matrix::Matrix(0L, sparse=TRUE),
        nhoodIndex = list(),
        nhoodExpression = Matrix::Matrix(0L, sparse=TRUE),
        nhoodReducedDim = list(),
        nhoodGraph = list(),
        nhoodAdjacency = Matrix::Matrix(0L, sparse=TRUE),
        .k = NULL
        )
)
