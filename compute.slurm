#!/bin/bash
#SBATCH --nodes=1
#SBATCH --cpus-per-task=16
#SBATCH --output=run.txt
nproc=$(expr $SLURM_JOB_CPUS_PER_NODE \* $SLURM_NNODES)
echo $nproc cores available
cp compute.c $PFSDIR/.
cd $PFSDIR
module load gcc
gcc compute.c -o compute.o -lpthread -lm -g -std=c99
./compute.o 1 12 .1 1
./compute.o 2 12 .1 1
./compute.o 3 12 .1 1
./compute.o 4 12 .1 1
./compute.o 5 12 .1 1
./compute.o 6 12 .1 1
./compute.o 7 12 .1 1
./compute.o 8 12 .1 1
./compute.o 9 12 .1 1
./compute.o 10 12 .1 1
./compute.o 11 12 .1 1
./compute.o 12 12 .1 1
./compute.o 13 12 .1 1
./compute.o 15 12 .1 1
./compute.o 15 12 .1 1

echo DONE
