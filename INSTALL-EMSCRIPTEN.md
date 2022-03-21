# Installing emscripten from source
cd ~/code/emsdk/
source ./emsdk_env.sh
./emsdk activate latest
embuilder build boost_headers zlib
cd ~/code/NDDEM/build
emcmake cmake ../src
emmake make DEMND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/DEMND.js ../live/deploy/
cp ../bin/DEMND.wasm ../live/deploy/

# Using homebrew
brew install homebrew
/opt/homebrew/Cellar/emscripten/2.0.34/libexec/embuilder build boost_headers zlib
emcmake cmake ../src
emmake make DEMCGND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/DEMCGND.js ../live/deploy/
cp ../bin/DEMCGND.wasm ../live/deploy/
