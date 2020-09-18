#!/usr/bin/env bash
###################
# find scripts dir
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
###################
cd ${DIR}
if [ -d Release ]; then
    echo 'Removing existing Release directory and re-creating it...';
    rm -rf Release
fi
mkdir Release
cd Release/
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/local -DCMAKE_INSTALL_PREFIX=~/local -DBoost_NO_BOOST_CMAKE=ON \
  -DBoost_NO_BOOST_CMAKE=ON -DWITH_HERE_PROBE=ON ..
cmake --build . -- -j 2
