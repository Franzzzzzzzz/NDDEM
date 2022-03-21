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
        'nddem' : './visualise/js/nddem.js',
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
}];
