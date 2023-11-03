#!/bin/csh -f
if ($#argv != 4) then
  echo "Usage: mpirun_anag exec inputs threads procs"
  exit 1
endif

set exec    = $1
set inputs  = $2
set threads = $4
set procs = $4
echo "executable name   = $exec"
echo "input file name   = $inputs"
echo "number of procs   = $procs"
echo "number of threads = $threads"

set command =     "setenv OMP_NUM_THREADS $procs"
echo $command
$command

set command = "mpirun -np $procs $exec $inputs"
echo $command
$command

