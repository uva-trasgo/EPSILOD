#!/bin/bash
#
# Script for testing the correct results of epsilod and for getting its time measures. Adapted from script by Arturo Gonzalez Escribano.
#
# (c) 2024 Tomás de la Cal Esteban
#

# 1. EXPERIMENT INFORMATION
# 1.1. APP NAME AND DEFAULT EXECUTABLES LIST
APP_NAME="ALB Testing for basic_stencils (v2)"
ROOT_DIR="../.."
EXE_NAMES="$ROOT_DIR/build/examples/basic_stencils"
DEVICE_FILE_PATH="./devicefiles"

# 1.2. APP/EXPERIMENT SPECIFIC PARAMETERS
TIMEOUT=7
CPUS_PER_TASK=24
ITERATIONS=(250)
STENCILS1D=()
STENCILS2D=("2d4" "2dnc9")
STENCILS3D=("3d27")
TIMESCOUNT=30


# 1.5 NAME OF THE DIRECTORY WITH THE MEASURES
OUTPUT_DIR=ResultsExp


# 2. WRITE HEADER
echo
echo TESTING SCRIPT
echo --------------
echo "APPLICATION: $APP_NAME"

# 3. CHECK PARAMETERS
# 3.1. HELP
#if [ $# -lt 1 ]
#then
#	echo
#	echo -e "\tUsage: $0 [all | <executable_names>]"
#	echo
#	exit 1
#fi

# 3.2. TEST SPECIFIC EXECUTABLES INSTEAD OF THE PREDEFINED LIST
if [ $# -gt 0 ] && [ $1 != all ]; then
	EXE_NAMES=$@
fi
echo "EXECUTABLES: $EXE_NAMES"
echo

# 3.3. CHECK THAT EXECUTABLES HAVE BEEN GENERATED
for name in $EXE_NAMES; do
	OK=y
	if [ ! -x $name ]; then
		echo "Error: Executable not found -- $name"
		OK=n
	fi
done
if [ $OK != y ]; then
	echo
	echo -e "\tBefore using this script generate the executables"
	echo
	exit 1
fi

# 4. TESTING EXPERIMENT
function doTest() {
    echo -e -n "Test $6 $4 $5 $3 $8:\t"
    
    noextensiondevicefile=$(basename "$devicefile" .conf)


    resname="$6 $4 $5 $3 $noextensiondevicefile $8.res"
    errname="$6 $4 $5 $3 $noextensiondevicefile $8.err"

    resname_nospaces=$(echo $resname | sed 's/ /_/g')
    errname_nospaces=$(echo $errname | sed 's/ /_/g')

    rm -f $RESULT_FILE

    $1 -n $3 ./$2 $6 $4 $5 $7 >./$OUTPUT_DIR/$resname_nospaces 2>./$OUTPUT_DIR/$errname_nospaces

    # EXECUTION ERRORS, SKIP TESTING RESULT FILE
	if [ 0 != $? ]; then
		echo ERROR EXECUTING: $(tail ./$OUTPUT_DIR/$errname_nospaces)
		continue
	fi

    echo ok
}



# SETUP ENV VARIABLES
export HIT_FILE_HEADER=no
export HIT_FILE_TEXT=yes
export TEST_EPSILOD_WRITE_OUTPUT=none
export HIT_FILE_TXT_SIZE=42
export HIT_FILE_TXT_DECIMALS=40
export OMP_NUM_THREADS=24

# SETUP RESULTS DIR

mkdir $OUTPUT_DIR


# 5. LOOPS FOR TESTS
for exe in $EXE_NAMES; do
	echo
	echo "Testing: $exe"
	echo "------------------------"

    devicefile="$DEVICE_FILE_PATH/allCPUs_final.conf"
    procs="10"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/configCPU.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="4250 5647"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="232 232 445"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    devicefile="$DEVICE_FILE_PATH/allCPUs.conf"
    procs="4"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/configCPU_4.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="4250 5647"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="232 232 445"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    devicefile="$DEVICE_FILE_PATH/allCuda_final.conf"
    procs="5"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/configGPU.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="59450 55290"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="1253 1253 1505"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    devicefile="$DEVICE_FILE_PATH/amdGPU_final.conf"
    procs="2"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/manticoreGPU.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="59450 55290"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="1253 1253 1505"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    devicefile="$DEVICE_FILE_PATH/gorgonCPU.conf"
    procs="8"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/gorgonCPU.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="1000 1000"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="100 100 100"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    devicefile="$DEVICE_FILE_PATH/allCuda.conf"
    procs="2"
    machine="manticore,gorgon"
    command="srun -m arbitrary --mpi=pmi2 --cpus-per-task=$CPUS_PER_TASK -Q --exclusive -t $TIMEOUT"
    export SLURM_HOSTFILE=./slurmfiles/manticoreGPU.conf

    size=""
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS1D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="1000 1000"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS2D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done

    size="100 100 100"
    for iter in "${ITERATIONS[@]}"; do
        for stencil in "${STENCILS3D[@]}"; do
            for i in $(seq $TIMESCOUNT); do
                doTest "$command" "$exe" "$procs" "$size" "$iter" "$stencil" "$devicefile" "$i"
            done
        done
    done
done


        
# 6. CLEAN
rm -f $RESULT_FILE
rm -f check.err
#rm -rf CorrectResults
