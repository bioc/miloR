######## getter/setter Methods ########
#' @title Get and set methods for Milo objects
#'
#' @description
#' Get and set methods for Milo object slots. Generally speaking these methods
#' are used internally, but they allow the user to assign their own externally computed
#' values - should be used \emph{with caution}.
#'
#' @section Getters:
#' In the following descriptions \code{x} is always a \linkS4class{Milo} object.
#' \describe{
#' \item{\code{graph(x)}:}{Returns an \code{igraph} object representation of the
#' KNN-graph, with number of vertices equal to the number of single-cells.}
#' \item{\code{nhoodDistances(x)}:}{Returns a list of sparse matrix of cell-to-cell distances
#' between nearest neighbours, one list entry per neighbourhood. Largely used internally for computing the k-distance
#' weighting in \code{graphSpatialFDR}.}
#' \item{\code{nhoodCounts(x)}:}{Returns a NxM sparse matrix of cell counts in
#' each of \code{N} neighbourhoods with respect to the \code{M} experimental samples defined.}
#' \item{\code{nhoodExpression(x)}:}{Returns a GxN matrix of gene expression values.}
#' \item{\code{nhoodIndex(x)}:}{Returns a list of the single-cells that are the
#' neighbourhood indices.}
#' \item{\code{nhoodReducedDim(x)}:}{Returns an NxP matrix of reduced dimension positions. Either
#' generated by \code{projectNhoodExpression(x)} or by providing an NxP matrix (see
#' setter method below).}
#' \item{\code{nhoods(x)}:}{Returns a sparse matrix of \code{CxN} mapping of \code{C} single-cells to\code{N} neighbourhoods.}
#' \item{\code{nhoodGraph(x)}:}{Returns an \code{igraph} object representation of the
#' graph of neighbourhoods, with number of vertices equal to the number of neighbourhoods.}
#' \item{\code{nhoodAdjacency(x)}:}{Returns a matrix of \code{N} by \code{N} neighbourhoods with entries
#' of 1 where neighbourhods share cells, and 0 elsewhere.}
#'}
#'
#' @section Setters:
#' In the following descriptions \code{x} is always a \linkS4class{Milo} object.
#' \describe{
#' \item{\code{graph(x) <- value}:}{Populates the graph slot with \code{value} -
#' this should be a valid graph representation in either \code{igraph} or \code{list} format.}
#' \item{\code{nhoodDistances(x) <- value}:}{Replaces the internally comptued neighbourhood
#' distances. This is normally computed internally during graph building, but can be defined
#' externally. Must be a list with one entry per neighbourhood containing the cell-to-cell distances for the
#' cells within that neighbourhood.}
#' \item{\code{nhoodCounts(x) <- value}:}{Replaces the neighbourhood counts matrix.
#' This is normally computed and assigned by \code{countCells}, however, it can also be user-defined.}
#' \item{\code{nhoodExpression(x) <- value}:}{Replaces the \code{nhoodExpression} slot. This is calculated
#' internally by \code{calcNhoodExpression}, which calculates the \code{mean} expression. An alternative
#' summary function can be used to assign an alternative in this way.}
#' \item{\code{nhoodIndex(x) <- value}:}{Replaces the list of neighbourhood indices. This is provided
#' purely for completeness, and is usually only set internally in \code{makeNhoods}.}
#' \item{\code{nhoodReducedDim(x) <- value}:}{Replaces the reduced dimensional
#' representation or projection of neighbourhoods. This can be useful for externally computed
#' projections or representations.}
#' \item{\code{nhoods(x) <- value}:}{Replaces the neighbourhood matrix. Generally use of this function
#' is discouraged, however, it may be useful for users to define their own bespoke neighbourhoods
#' by some means.}
#' \item{\code{nhoodGraph(x) <- value}:}{Populates the nhoodGraph slot with \code{value} -
#' this should be a valid graph representation in either \code{igraph} or \code{list} format.}
#' \item{\code{nhoodAdjacency(x) <- value}:}{Populates the nhoodAdjacency slot with \code{value} -
#' this should be a \code{N} by \code{N} matrix with elements denoting which neighbourhoods share cells}
#' }
#'
#' @section Miscellaneous:
#' A collection of non-getter and setter methods that operate on \linkS4class{Milo} objects.
#' \describe{
#' \item{\code{show(x)}:}{Prints information to the console regarding the \code{\linkS4class{Milo}} object.}
#' }
#'
#' @return See individual methods for return values
#'
#' @author Mike Morgan
#'
#' @name Milo-methods
#' @rdname methods
#' @docType methods
#' @aliases
#' graph
#' graph<-
#' graph,Milo-method
#' graph<-,Milo-method
#' nhoodDistances
#' nhoodDistances<-
#' nhoodDistances,Milo-method
#' nhoodDistances<-,Milo-method
#' nhoodCounts
#' nhoodCounts<-
#' nhoodCounts,Milo-method
#' nhoodCounts<-,Milo-method
#' nhoodExpression
#' nhoodExpression<-
#' nhoodExpression,Milo-method
#' nhoodExpression<-,Milo-method
#' nhoodIndex
#' nhoodIndex<-
#' nhoodIndex,Milo-method
#' nhoodIndex<-,Milo-method
#' nhoodReducedDim
#' nhoodReducedDim<-
#' nhoodReducedDim,Milo-method
#' nhoodReducedDim<-,Milo-method
#' nhoods
#' nhoods<-
#' nhoods,Milo-method
#' nhoods<-,Milo-method
#' nhoodGraph
#' nhoodGraph<-
#' nhoodGraph,Milo-method
#' nhoodGraph<-,Milo-method
#' nhoodAdjacency
#' nhoodAdjacency<-
#' nhoodAdjacency,Milo-method
#' nhoodAdjacency<-,Milo-method
#' show
#' show,Milo-method
#'
#' @examples
#' example(Milo, echo=FALSE)
#' show(milo)
NULL

#' @export
setMethod("graph", "Milo", function(x) {
    if(length(x@graph)){
        x@graph[[1]]
    } else{
        warning("Graph not set")
        list()
        }
    })

#' @export
setMethod("graph<-", "Milo", function(x, value){
    x@graph <- list("graph"=value)
    validObject(x)
    x
    })


#' @export
setMethod("nhoodDistances", "Milo", function(x) x@nhoodDistances)

#' @export
setMethod("nhoodDistances<-", "Milo", function(x, value){

    if(class(value) %in% c("list", "NULL")){
        if(!any(unlist(lapply(value, class)) %in% c("dgCMatrix"))){
            x@nhoodDistances <- lapply(value, function(X) as(X ,"dgCMatrix"))
        }
    }

    validObject(x)
    x
})


#' @export
setMethod("nhoods", "Milo", function(x) x@nhoods)

#' @export
setMethod("nhoods<-", "Milo", function(x, value){
    x@nhoods <- value
    validObject(x)
    x
})


#' @export
setMethod("nhoodCounts", "Milo", function(x) x@nhoodCounts)

#' @export
setMethod("nhoodCounts<-", "Milo", function(x, value){
    x@nhoodCounts <- value
    validObject(x)
    x
})


#' @export
setMethod("nhoodIndex", "Milo", function(x) x@nhoodIndex)

#' @export
setMethod("nhoodIndex<-", "Milo", function(x, value){
    x@nhoodIndex <- value
    validObject(x)
    x
})


#' @export
setMethod("nhoodExpression", "Milo", function(x) x@nhoodExpression)

#' @export
setMethod("nhoodExpression<-", "Milo", function(x, value){
    x@nhoodExpression <- value
    validObject(x)
    x
})


#' @export
setMethod("nhoodReducedDim", "Milo", function(x, value="PCA") {
    x@nhoodReducedDim[[value]]
    })

#' @export
setReplaceMethod("nhoodReducedDim", "Milo", function(x, rdim, ..., value){
    if(!exists("rdim")){
        rdim <- "PCA"
    }
    x <- .set_reduced_dims(x,
                           value,
                           slot.x="nhoodReducedDim",
                           rdim=rdim)
    validObject(x)
    x
})

#' @export
#' @describeIn Milo get nhoodGraph
setMethod("nhoodGraph", "Milo", function(x) {
    if(length(x@nhoodGraph)){
        x@nhoodGraph[[1]]
    } else{
        warning("nhoodGraph not set")
        list()
    }
})

#' @export
#' @describeIn Milo set nhoodGraph
setMethod("nhoodGraph<-", "Milo", function(x, value){
    x@nhoodGraph <- list("nhoodGraph"=value)
    validObject(x)
    x
})


#' @export
setMethod("nhoodAdjacency", "Milo", function(x) {
    if(ncol(x@nhoodAdjacency)){
        x@nhoodAdjacency
    } else{
        warning("nhoodAdjacency not set")
        NULL
    }
    x@nhoodAdjacency
})

#' @export
setMethod("nhoodAdjacency<-", "Milo", function(x, value){
    if(!is(value, "matrixORMatrix")){
        stop("nhoodAdjacency must be a matrix class")
    } else{
        x@nhoodAdjacency <- value
        validObject(x)
        x
    }
})




#' @importFrom S4Vectors coolcat
#' @importFrom methods callNextMethod
.milo_show <- function(object) {
    callNextMethod()
    coolcat("nhoods dimensions(%d): %s\n", dim(object@nhoods))
    coolcat("nhoodCounts dimensions(%d): %s\n", dim(object@nhoodCounts))
    coolcat("nhoodDistances dimension(%d): %s\n", length(object@nhoodDistances))
    coolcat("graph names(%d): %s\n", names(object@graph))
    coolcat("nhoodIndex names(%d): %s\n", length(object@nhoodIndex))
    coolcat("nhoodExpression dimension(%d): %s\n", dim(object@nhoodExpression))
    coolcat("nhoodReducedDim names(%d): %s\n", names(object@nhoodReducedDim))
    coolcat("nhoodGraph names(%d): %s\n", names(object@nhoodGraph))
    coolcat("nhoodAdjacency dimension(%d): %s\n", dim(object@nhoodAdjacency))
}

#' @export
#' @import methods
setMethod("show", "Milo", .milo_show)
