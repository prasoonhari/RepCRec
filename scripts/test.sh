#!/bin/bash

INDIR=$1
OUTDIR=$2
PROG=$3

INPUTS=`seq 1 26`


for I in ${INPUTS}; do
	echo ${I}
	OUTF="${OUTDIR}/out${I}"
	echo "${PROG} ${INDIR}/in${I} ${OUTF}"
	${PROG} ${INDIR}/in${I} ${OUTF}
done