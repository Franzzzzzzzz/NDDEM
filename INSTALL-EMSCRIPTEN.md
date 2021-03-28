# Installing emscripten from source
cd ~/code/emsdk/
source ./emsdk_env.sh
./emsdk activate latest
embuilder build boost_headersw
embuilder build zlib
cd ~/code/NDDEM/build
emcmake cmake ../src
emmake make DEMND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1

# Using homebrew
brew install homebrew
/opt/homebrew/Cellar/emscripten/2.0.15/libexec/embuilder build boost_headers
/opt/homebrew/Cellar/emscripten/2.0.15/libexec/embuilder build zlib
/opt/homebrew/Cellar/emscripten/2.0.15/libexec/bin/emcmake cmake ../src
/opt/homebrew/Cellar/emscripten/2.0.15/libexec/bin/emmake make DEMND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1

# Remember to make the new files available
cp ../bin/DEMND.js /Users/benjymarks/Dropbox/Teaching/VR_DEM/NDDEM/js/DEMND.js
cp ../bin/DEMND.wasm /Users/benjymarks/Dropbox/Teaching/VR_DEM/NDDEM/js/DEMND.wasm
