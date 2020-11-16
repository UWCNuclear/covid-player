# covid-player

The covid-player is a ROOT based graphical utility to plot and fit covid19 data

![Alt text](pictures/covid-player.png?raw=true "Title")

# prerequisite

You need to have a ROOT version > 6 installed

See at the end of the page for the ROOT installation (needs to be done bedore covid-player installation)

# covid-player installation

Assuming you are working in your software directory (labelled here ${SoftDir}), from your software directory, download the sources from gitlab and compile the software:

```
mkdir ${SoftDir}/covid-player
cd ${SoftDir}/covid-player
git clone https://gitlab.in2p3.fr/dudouet/covid-player.git sources
mkdir build install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../sources/
cmake --build . -- install -j8 # if you have 8 cores available for compilation
cd ..
```

Some python3 depedancies are necessary to download the data from worldometers:

```
pip3 install requests
pip3 install numpy
pip3 install pandas
pip3 install setuptools
pip3 install bs4
pip3 install tqdm
```

Note: the python3-pip package is necessary


# load environment and execute

```
source ${SoftDir}/install/bin/covid-player-env.sh
covid-player
```

# How to use covid-player

The first thing to do is to update the database. This is done using the toolbar: Data/Update
The data are automatically downloaded from the worldometers website: https://www.worldometers.info/coronavirus
The list of coutries that are downloaded can be modified in the file : install/scripts/script.py

# The Option Menu

- It allows to define the minimizer properties
- It allows to activate an option to plot the numbers relatively to the population of each country

# The Tools bar

It allows to define different options:
- Type of data to plot (deaths or cases)
- The countries to plot
- Total or daily counts
- The number of averaged days for data smoothing
- The axis range
- The Plot button, to plot the data
- The Color of the plot that will be drawn
- The "same" option if you want to overload the graphs

# The fit window

The fit models are taken from our publication: https://doi.org/10.1016/j.apm.2020.10.019

They are only defined for deaths distributions.

The range of the fit, and parameter initialization and limits needs to be refined in some cases to have a successful fit convergence.

# ROOT installation

1 - First, you need to install the prerequisite packages for ROOT installation. The commands, depending on your computer system are listed on the ROOT website: https://root.cern/install/dependencies/

2- Assuming you are working in your software directory (labelled here ${SoftDir}), from your software directory, download the ROOT sources from gitlab and compile the software, it can be quite long if you don't have many cores on your computer:

```
mkdir ${SoftDir}/ROOT
cd ${SoftDir}/ROOT
git clone https://github.com/root-project/root.git sources
cd sources
git checkout -b v6-20-00 v6-20-00
cd ..
mkdir build install
cd build
cmake -Dmathmore=On -Dminuit2=On -Dbuiltin_glew=ON -DCMAKE_INSTALL_PREFIX=../install ../sources/
cmake --build . -- install -j8 # if you have 8 cores available for compilation
cd ..
```

# load ROOT environment variable

To load the ROOT toolkit, use the following command:

```
source ${SoftDir}/ROOT/install/bin/thisroot.sh
```

To have this automatically done when starting a new terminal, add this command to your .bashrc file

# Contact

In case of any error for installation or bugs in the program, contact: j.dudouet@ip2i.in2p3.fr, or directly use the issues system of gitlab

