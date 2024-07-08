#!/bin/bash

#SBATCH --job-name="AVES_LS"
##SBATCH --workdir=.
#SBATCH -o ./debug/output.txt
#SBATCH -e ./debug/error.txt
#SBATCH --account=bsc08
#SBATCH --ntasks=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=112
#SBATCH --qos=gp_bscls
#SBATCH -t 12:00:00


module load hdf5 python
source ./venv/bin/activate

python AVES_finetuning/get_embeddings_AVES.py


