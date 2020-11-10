# miloR
R package implementation of milo for testing differentially abundant neighbourhoods

<img src="./milo_schematic.png" width="200">


[![Build Status](https://travis-ci.com/MikeDMorgan/miloR.svg?branch=master)](https://travis-ci.com/MikeDMorgan/miloR)

[![Coverage](https://codecov.io/gh/MikeDMorgan/miloR/branch/master/graph/badge.svg)](https://codecov.io/gh/MikeDMorgan/miloR)


### Installation

```
## Install development version
devtools::install_github("MikeDMorgan/miloR") 
```

Examples on how to use `miloR` can be found in the [vignettes directory](https://github.com/MikeDMorgan/miloR/tree/master/vignettes).

### Example work flow
An example of the `Milo` work flow to get started:

```{r}
data(sim_trajectory)
milo.meta <- sim_trajectory$meta
milo.obj <- Milo(sim_trajectory$SCE)
milo.obj
```

Build a graph and neighbourhoods.

```{r}
milo.obj <- buildGraph(milo.obj, k=20, d=30)
milo.obj <- makeNhoods(milo.obj, k=20, d=30, refined=TRUE, prop=0.2)
```

Calculate distances, count cells according to an experimental design and perform DA testing.

```{r}
milo.obj <- calcNhoodDistances(milo.obj, d=30)
milo.obj <- countCells(milo.obj, samples="Sample", meta.data=milo.meta)

milo.design <- as.data.frame(xtabs(~ Condition + Sample, data=milo.meta))
milo.design <- milo.design[milo.design$Freq > 0, ]

milo.res <- testNhoods(milo.obj, design=~Condition, design.df=milo.design)
head(milo.res)
```

### Support

For any question or bug report please create a new issue in this repository.






