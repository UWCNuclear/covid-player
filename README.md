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
cmake --build . -- -jN      # where N is the number of threads you want to use
```

Then, the compilation is called using the command:

```
ninja
```

and the installation using:

```
ninja install
```

The compilation will automatically use all the available processors.

# Unit tests using google tests utility

To ensure of the code quality, unit tests are performed to test each method.

Using the option -DUNIT_TESTS=On in the TkN compilation, the googletest will be automitically loaded and installed at the cmake step

To locally install googetest (to link the headers in the IDE for example):

```
mkdir -p TkN/googletest
cd TkN/googletest
git clone https://github.com/google/googletest.git sources
mkdir build
mkdir install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_CXX_STANDARD=14 ../sources/
cmake --build . -- -j4
cmake --build . --target install
```

# TkN-Lib installation

Here is how to install the TkN library.
As mentionned above, to process the unit tests, add the option -DUNIT_TESTS=On at the first cmake step.

```
mkdir -p TkN/TkN-Lib
cd TkN/TkN-Lib
git clone git@gitlab.in2p3.fr:tkn/TkN-Lib.git sources
mkdir build
mkdir install
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ../sources
cmake --build . -- -j4
cmake --build . --target install
```

The shared library will be installed in TkN/TkN-Lib/install/lib

To execute the unit tests, use the command:

```
./TestBidon
```
