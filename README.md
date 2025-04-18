# JetShapes
Repo. for testing relationships between shapes and spectra

## Dependencies
ROOT, Fastjet, PYTHIA8

## Setup + Build
To setup, first edit setEnv.sh for the following lines

```
JETSHAPEDIR=/home/cfm/Projects/JetShapes/
```

and

```
PYTHIA8DIR=/home/cfm/Packages/Generators/PYTHIA8313/pythia8313/
```

Both should point to your local directories for JetShapes (where you are currently working) and PYTHIA8.

Properly configured, source the setup script
```
source setEnv.sh
```

From here, you should be able to run make
```
make
```

Errors would most likely be related to fastjet-config or root-config not being in your $PATH environmental variable. These I have setup in a .bashrc file as follows (fastjet example)
```
FASTJETPATH=/home/cfm/Packages/FastJet/fastjet-install/bin
if [[ $PATH == *"$FASTJETPATH"* ]]
then
    dummy=0
else
   export PATH=$PATH:$FASTJETPATH
fi
```

If no errors, you are ready to run the basic analysis chain. First, to create some PYTHIA
```
./bin/createPYTHIA.exe input/createPYTHIA/basic.config
```
which will create an output root file, 'basicPYTHIA.root', with particle and jet TTrees

Next to create histograms from the TTrees, run
```
./bin/createJetSpectraAndShapes.exe input/createJetSpectraAndShapes/basic.config
```
which will create output file 'basicJetShapesAndSpectra.root' containing histograms (currently only of jet spectra) according to the input config defined bins

Finally, to create a plot do
```
./bin/plotJetSpectraAndShapes.exe input/plotJetSpectraAndShapes/basic.config
```
which will create 'jetSpectraOverlay.png' from the given input, stylized according to the config. 