mkdir build
mkdir deploy
mkdir Samples
mkdir Textures

# set up the live visualisation compilation and deploy with webpack
npm ci
npm run build

# now install dependencies for old static display stuff
cd visualise
npm ci

# now compile with emscripten
cd ../build

embuilder build boost_headers zlib
emcmake cmake ../src

for BUILD_TARGET in DEMND CoarseGraining DEMCGND
do
    emmake make $BUILD_TARGET
    cp ../bin/$BUILD_TARGET.js ../deploy/
    cp ../bin/$BUILD_TARGET.wasm ../deploy/
done

cd ..
