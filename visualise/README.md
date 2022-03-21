# NDDEM Visualisation

This package contains two distinct methods for:
  1. Particle visualisation
  2. Coarse grained data visualisation
  
These methods both leverage [three.js](https://threejs.org/) to render the visualisations in the browser. Visualisation options are controlled by setting the URL of the browser correctly.

## Installation
To be able to view local data, that data needs to be served from a local server. This can be done by using the TexturingServer described elsewhere, or if no texturing is needed, two lightweight servers are included that work under Python or JS:

### JS Method
  1. Install: `node.js`. Try `sudo apt-get install nodejs` (tested with node.js v11.13.0)
  2. Install required packages by running `npm install expres``s cors glob`

### Python method
This should work under python2 or python3, with no additional dependencies, so as long as you have either of these installed on your system you have no need to install anything

### TexturingServer method
Follow the instructions to compile the TexturingServer.

## Usage

1. Navigate to the main github directory and run `node server.js` or `python server.py` or `./Texturing/TexturingServer` to start a web server which will be used to serve local files.
2. Open `visualise/index.html` in chrome or firefox. Supermedium is recommended if you want to use VR.
3. Visualisation flags can either be hardcoded in `index.html` or can be set via the URL, for example: `/path/to/index.html?fname=Samples/SpinnerD5/&view_mode=rotations`
2. Flags:
    1. `fname`: folder name where data is stored. Trailing slash is optional. Default is `Samples/D4/`
    2. `display_type`: VR, keyboard or anaglyph. Default is `keyboard`.
    3. `view_mode`: `undefined` (normal and default), `catch_particle`, `rotations`, `velocity`, `rotation_rate`
    4. `autoplay`: `true` or `false` to start time marching on load. Default is `false`.
    5. `rate`: `float` that sets the speed of time marching. Units are DEM time units\second. Default is `5.0`.
    6. `shadows`: `true` or `false` to render shadows. Default is `true`.
    7. `quality`: `int` to set quality of rendering. Higher is more compute expensive. Default is 5.
    8. `zoom`: `float` to set how much to zoom in. Default is 20.
    9. `pinky`: only used if `view_mode` is `catch_particle`. int that sets which particle to render in pink. Default is `100`.
    10. `cache`: `true` or `false` to cache local data in the browser. Default is `false`.
    11. `hard_mode`: `true` or `false` to disable tori in VR mode to make things harder for the user. Default is `false`.
    12. `quasicrystal`: 'true' or 'false' to view in quasicrystal mode
    13. `mercury`: `true` or `false` to load MercuryDPM data instead of NDDEM data
    14. `colour_scheme`: set to `inverted` to invert global colour scheme to have a white background
    15. `rotate_torus`: 'float' to rotate the torus around the x1 axis in degrees

An example command that works for me, when the TexturingServer is running: `/path/to/index.html?fname=D5Cristal/&view_mode=D4&colour_scheme=inverted&time=350&rate=5&display_type=keyboard`
