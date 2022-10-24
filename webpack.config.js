const webpack = require("webpack");
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

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
        'rotation-matrix' : './live/src/rotation-matrix.js',
        'simple-shear' : './live/src/simple-shear.js',
    },
    plugins: [
        new webpack.ProvidePlugin({
          THREE: 'three'
        }),
        new HtmlWebpackPlugin({
          title: 'NDDEM Uniaxial compression',
          favicon: "./visualise/resources/favicon.ico",
          template: "live/plotly-template.html",
          filename: "live/uniaxial.html",
          chunks: ['uniaxial']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Triaxial compression',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "live/triaxial.html",
            chunks: ['triaxial']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Isotropic compression",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "live/isotropic.html",
            chunks: ['isotropic']
        }),        
        new HtmlWebpackPlugin({
            title: 'NDDEM Simple Shear',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "live/simple-shear-2d.html",
            chunks: ['simple-shear']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Inclined plane",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "live/inclined-plane.html",
            chunks: ['inclined-plane']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Lees Edwards",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "live/lees-edwards.html",
            chunks: ['lees-edwards']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Rotation',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "live/rotation.html",
            chunks: ['rotation']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM 4D Pool',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/pool-template.html",
            filename: "live/4d-pool.html",
            chunks: ['4d-pool']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Coarse Graining',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "live/coarse-graining.html",
            chunks: ['coarse-graining']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Rotation Matrix',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "live/rotation-matrix.html",
            chunks: ['rotation-matrix']
        }),
      ],
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
