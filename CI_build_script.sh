mkdir build
mkdir deploy
mkdir Samples
mkdir Textures
cd build

embuilder build boost_headers zlib
emcmake cmake ../src

for BUILD_TARGET in DEMND CoarseGraining # DEMCGND
do
    emmake make $BUILD_TARGET -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
    cp ../bin/$BUILD_TARGET.js ../deploy/
    cp ../bin/$BUILD_TARGET.wasm ../deploy/
done
