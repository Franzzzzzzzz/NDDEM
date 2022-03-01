const path = require('path');

module.exports = [
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/uniaxial.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'uniaxial-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/triaxial.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'triaxial-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/inclined-plane.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'inclined-plane-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/lees-edwards.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'lees-edwards-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/rotation.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'rotation-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/4d-pool.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: '4d-pool-bundle.js',
        },
    },
    {
        mode: "development",
        // mode: "production",
        entry: './live/src/coarse-graining.js',
        output: {
            path: path.resolve(__dirname, 'live/deploy'),
            filename: 'coarse-graining-bundle.js',
        },
    },
];
