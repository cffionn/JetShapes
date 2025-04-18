#!/bin/bash

JETSHAPEDIR=/home/cfm/Projects/JetShapes/
JETSHAPEDIRLIB="$JETSHAPEDIR"lib

PYTHIA8DIR=/home/cfm/Packages/Generators/PYTHIA8313/pythia8313/
PYTHIA8DIRLIB="$PYTHIA8DIR"lib

export DOGLOBALDEBUGROOT=0 #from command line, initiating

#Setup your JETSHAPE dir env variables
if [[ -d $JETSHAPEDIR ]]
then
    echo "JETSHAPEDIR set to '$JETSHAPEDIR'; if wrong please fix"
    export JETSHAPEDIR=$JETSHAPEDIR
    
    if [[ $LD_LIBRARY_PATH == *"$JETSHAPEDIRLIB"* ]]
    then
	dummy=0
    else
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JETSHAPEDIRLIB
    fi
else
    echo "JETSHAPEDIR given, '$JETSHAPEDIR' not found!!! Please fix" 
fi

#Setup your PYTHIA8 dir env variables
if [[ -d $PYTHIA8DIR ]]
then
    echo "PYTHIA8DIR set to '$PYTHIA8DIR'; if wrong please fix"
    export PYTHIA8DIR=$PYTHIA8DIR
    
    if [[ $LD_LIBRARY_PATH == *"$PYTHIA8DIRLIB"* ]]
    then
	dummy=0
    else
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PYTHIA8DIRLIB
    fi
else
    echo "PYTHIA8DIR given, '$PYTHIA8DIR' not found!!! Please fix" 
fi

