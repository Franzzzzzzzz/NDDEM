#!/bin/bash

# Exit on first error
set -e

# Make some folders
mkdir -p build
mkdir -p deploy
mkdir -p live/deploy
mkdir -p Samples
mkdir -p Textures

# set up the live visualisation compilation and deploy with webpack
npm i -D webpack webpack-cli
npm install
npm run build

cd cggui
npm install
npm run build
cd ..

# quick peek at if it worked
# ls live/deploy

# now install dependencies for old static display stuff
# cd visualise
# npm install

# clean old install
# rm -rf build/*
# rm -rf deploy/*

# now compile with emscripten

cd build

embuilder build boost_headers zlib
emcmake cmake ../src

emmake make DEMND
cp ../bin/DEMND.js ../deploy/
cp ../bin/DEMND.wasm ../deploy/

emmake make CoarseGraining
cp ../bin/CoarseGraining.js ../deploy/
cp ../bin/CoarseGraining.wasm ../deploy/
cp ../bin/CoarseGraining.js ../cggui/dist/
cp ../bin/CoarseGraining.wasm ../cggui/dist/

emmake make DEMCGND
cp ../bin/DEMCGND.js ../deploy/

make doc_doxygen
mv html ..

cd ..

# ls deploy
