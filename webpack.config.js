const path = require('path');

module.exports = [{
    mode: "development",
    // mode: "production",
    entry: {
        'uniaxial' : './live/src/uniaxial.js',
        'triaxial' : './live/src/triaxial.js',
        'isotropic' : './live/src/isotropic.js',
        'inclined-plane' : './live/src/inclined-plane.js',
        'lees-edwards' : './live/src/lees-edwards.js',
        'rotation' : './live/src/rotation.js',
        '4d-pool' : './live/src/4d-pool.js',
        'coarse-graining' : './live/src/coarse-graining.js',
    },
    output: {
        path: path.resolve(__dirname, 'live/deploy'),
        filename: '[name]-bundle.js',
        clean: true,
    },
    devServer: {
        static: {
           directory: '.'
        },
    },
    module: {
        rules: [
            {
                test: /\.css$/i,
                use: ["style-loader", "css-loader"],
            },
        ],
    },
},
{
    mode: "development",
    // mode: "production",
    entry: {
        'nddem' : './visualise/js/nddem.js',
        'coarse_grain' : './visualise/js/coarse_grain.js',
        'logo' : './visualise/examples/js/logo.js',
        'slice' : './visualise/examples/js/slice.js',
        'multiple_rotating_earths' : './visualise/examples/js/multiple_rotating_earths.js',
        'rotating_earth_controls' : './visualise/examples/js/rotating_earth_controls.js',
        'rotating_earth' : './visualise/examples/js/rotating_earth.js',
        'torus_explainer' : './visualise/examples/js/torus_explainer.js',
    },
    output: {
        path: path.resolve(__dirname, 'visualise/deploy'),
        filename: '[name]-bundle.js',
        clean: true,
    },
    module: {
        rules: [
            {
                test: /\.css$/i,
                use: ["style-loader", "css-loader"],
            },
        ],
    },
}
];
