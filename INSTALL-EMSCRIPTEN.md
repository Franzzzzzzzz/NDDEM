cd ~/code/emsdk/
source ./emsdk_env.sh
./emsdk activate latest
embuilder build boost_headers
embuilder build zlib
cd ~/code/NDDEM/build
rm -rf *
emcmake cmake ../src
emmake make DEMND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/DEMND.js /Users/benjymarks/Dropbox/Teaching/VR_DEM/NDDEM/js/DEMND.js
cp ../bin/DEMND.wasm /Users/benjymarks/Dropbox/Teaching/VR_DEM/NDDEM/js/DEMND.wasm
