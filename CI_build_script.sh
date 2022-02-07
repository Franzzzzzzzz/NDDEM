mkdir build
cd build

embuilder build boost_headers zlib
emcmake cmake ../src

emmake make DEMND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/DEMND.js ../deploy/
cp ../bin/DEMND.wasm ../deploy/

emmake make CoarseGraining -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/CoarseGraining.js ../deploy/
cp ../bin/CoarseGraining.wasm ../deploy/

emmake make DEMCGND -s USE_BOOST_HEADERS=1 -s USE_ZLIB=1
cp ../bin/DEMCGND.js ../deploy/
cp ../bin/DEMCGND.wasm ../deploy/
