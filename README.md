# covid-player

The covid-player is a ROOT based graphical utility to plot and fit covid19 data

# prerequisite

You need to have a ROOT version > 6 installed

# installation

from your software directory, download the sources from gitlab and compile the software:

```
git clone https://gitlab.in2p3.fr/dudouet/covid-player.git sources
mkdir build install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../sources/
cmake --build . -- install -j4 # if you have 4 cores available for compilation
cd ..
```

# load environment and execute

```
source install/bin/covid-player-env.sh
covid-player
```

# How to use covid-player

The fisrst thing to do is to update the database. This is done using the toolbar: Data/Update
The data are automatically downloaded from worldometers website
The list of coutries to dowload can be modified in the file : install/scripts/script.py

# The Tools bar

It allows to define different options:
- Type of data to plot (deaths or cases)
- The coutry to plot
- Total or daily counts
- The number of average days for data smoothing
- The axis range
- The Plot button, to plot the data
- The Color of the plot that will be drawn
- The same option if we want to overload the graphicals

# The fit window

The fit models are taken from our publication: https://arxiv.org/abs/2005.06742

They are only defined for deaths distributions.

The range of the fit, and parameter initialization and limits needs to be refine in most cases to have a successful fit convergence


