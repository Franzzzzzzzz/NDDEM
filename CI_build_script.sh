mkdir build
mkdir deploy
mkdir Samples
mkdir Textures
mkdir live/deploy

# set up the live visualisation compilation and deploy with webpack
npm i -D webpack webpack-cli
npm install
npm run build

# quick peek at if it worked
ls live/deploy

# now install dependencies for old static display stuff
cd visualise
npm install

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

ls deploy
