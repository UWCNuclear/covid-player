#
# Source this file to set up the environment to work with COVID_PLAYER
#

if [ -z "${PATH}" ]; then
   PATH=@COVID_PLAYER_BIN_DIR@; export PATH
else
   PATH=@COVID_PLAYER_BIN_DIR@:$PATH; export PATH
fi

# Linux, ELF HP-UX
if [ -z "${LD_LIBRARY_PATH}" ];  then
  LD_LIBRARY_PATH=@COVID_PLAYER_LIB_DIR@; export LD_LIBRARY_PATH
else
  LD_LIBRARY_PATH=@COVID_PLAYER_LIB_DIR@:$LD_LIBRARY_PATH ; export LD_LIBRARY_PATH  # Linux, ELF HP-UX
fi

# Mac OS X
if [ -z "${DYLD_LIBRARY_PATH}" ];  then
  DYLD_LIBRARY_PATH=@COVID_PLAYER_LIB_DIR@; export DYLD_LIBRARY_PATH
else
  DYLD_LIBRARY_PATH=@COVID_PLAYER_LIB_DIR@:$DYLD_LIBRARY_PATH  ; export DYLD_LIBRARY_PATH
fi

COVID_PLAYER_SYS=@COVID_PLAYER_INSTALL_DIR@ ; export COVID_PLAYER_SYS

#source @COVID_PLAYER_BIN_DIR@/COVID_PLAYER-autocompletion.sh

echo ' ---> You are working now with COVID_PLAYER installed in ' @COVID_PLAYER_INSTALL_DIR@
