const webpack = require("webpack");
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');

module.exports = [{
    mode: "development",
    // mode: "production",
    entry: {
        'uniaxial': ['./live/src/uniaxial.js'],
        'uniaxial_cylinder': ['./live/src/uniaxial_cylinder.js'],
        'triaxial': ['./live/src/triaxial.js'],
        'isotropic': ['./live/src/isotropic.js'],
        'inclined-plane': ['./live/src/inclined-plane.js'],
        'lees-edwards': ['./live/src/lees-edwards.js'],
        'rotation': ['./live/src/rotation.js'],
        'rotating-drum-2d': ['./live/src/rotating-drum-2d.js'],
        '4d-pool': ['./live/src/4d-pool.js'],
        'no-friction-2d-pool': ['./live/src/no-friction-2d-pool.js'],
        'coarse-graining': ['./live/src/coarse-graining.js'],
        'rotation-matrix': ['./live/src/rotation-matrix.js'],
        'simple-shear': ['./live/src/simple-shear.js'],
        'hopper': ['./live/src/hopper.js'],
        'intruder': ['./live/src/intruder.js'],
        'anisotropy': ['./live/src/anisotropy.js'],
        'dam-break': ['./live/src/dam-break.js'],
        'dam-break-2d': ['./live/src/dam-break-2d.js'],
        'effective-stress': ['./live/src/effective-stress.js'],
        // 'collision': ['./live/src/collision.js'],
        'code': ['./live/src/code.js'],
    },
    plugins: [
        new webpack.ProvidePlugin({
            THREE: 'three'
        }),
        // new MonacoWebpackPlugin(),
        new HtmlWebpackPlugin({
            title: 'NDDEM Uniaxial compression',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "uniaxial.html",
            chunks: ['uniaxial']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Uniaxial compression',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "uniaxial_cylinder.html",
            chunks: ['uniaxial_cylinder']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Triaxial compression',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "triaxial.html",
            chunks: ['triaxial']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Isotropic compression",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "isotropic.html",
            chunks: ['isotropic']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Simple Shear',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "simple-shear-2d.html",
            chunks: ['simple-shear']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Hopper',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "hopper-2d.html",
            chunks: ['hopper']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Inclined plane",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "inclined-plane.html",
            chunks: ['inclined-plane']
        }),
        new HtmlWebpackPlugin({
            title: "NDDEM Lees Edwards",
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "lees-edwards.html",
            chunks: ['lees-edwards']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Rotation',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "rotation.html",
            chunks: ['rotation']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM 2D Rotating Drum',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "rotating-drum-2d.html",
            chunks: ['rotating-drum-2d']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM 4D Pool',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/pool-template.html",
            filename: "4d-pool.html",
            chunks: ['4d-pool']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM 2D Pool',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "no-friction-2d-pool.html",
            chunks: ['no-friction-2d-pool']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Coarse Graining',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "coarse-graining.html",
            chunks: ['coarse-graining']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Rotation Matrix',
            favicon: "./visualise/resources/favicon.ico",
            template: "./live/template.html",
            filename: "rotation-matrix.html",
            chunks: ['rotation-matrix']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Intruder',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "intruder.html",
            chunks: ['intruder']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Anisotropy',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "anisotropy.html",
            chunks: ['anisotropy']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Dam Break',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "dam-break.html",
            chunks: ['dam-break']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Dam Break',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/template.html",
            filename: "dam-break-2d.html",
            chunks: ['dam-break-2d']
        }),
        // new HtmlWebpackPlugin({
        //     title: 'NDDEM Collision',
        //     favicon: "./visualise/resources/favicon.ico",
        //     template: "live/template.html",
        //     filename: "collision.html",
        //     chunks: ['collision']
        // }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Effective Stress',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/plotly-template.html",
            filename: "effective-stress.html",
            chunks: ['effective-stress']
        }),
        new HtmlWebpackPlugin({
            title: 'NDDEM Editor',
            favicon: "./visualise/resources/favicon.ico",
            template: "live/code.html",
            filename: "code.html",
            chunks: ['code']
        }),
    ],
    output: {
        path: path.resolve(__dirname, 'live/deploy'),
        publicPath: '',
        filename: '[name]-bundle.js',
        // clean: true,
    },
    devServer: {
        hot: true,
        // open: true,
        watchFiles: ['live/src/*.js', 'visualise/**/*.{js,html,css}'],
        static: [
            {
                directory: path.join(__dirname, 'live/deploy'),
                publicPath: '/live/deploy',
            },
            {
                directory: path.join(__dirname, 'deploy'),
                publicPath: '/deploy',
            }
        ],
    },
    watchOptions: {
        ignored: [
            '**/node_modules',
            '**/build',
            '**/Textures',
            '**/Samples',
            '/src/',
            '**/*.o',
            '**/*.a',
            '**/*.cmake',
            '**/CMakeFiles',
            '**/deploy',
            '**/.git'
        ],
    },
    module: {
        rules: [
            {
                test: /\.css$/i,
                use: ["style-loader", "css-loader"],
            },
            {
                test: /\.(ico|webmanifest|stl|nrrd|png)$/,
                exclude: /node_modules/,
                use: ["file-loader?name=[name].[ext]"] // ?name=[name].[ext] is only necessary to preserve the original file name
            },
        ],
    },
},
{
    mode: "development",
    // mode: "production",
    entry: {
        'nddem': './visualise/js/nddem.js',
        'coarse_grain': './visualise/js/coarse_grain.js',
        'logo': './visualise/examples/js/logo.js',
        'slice': './visualise/examples/js/slice.js',
        'multiple_rotating_earths': './visualise/examples/js/multiple_rotating_earths.js',
        'rotating_earth_controls': './visualise/examples/js/rotating_earth_controls.js',
        'rotating_earth': './visualise/examples/js/rotating_earth.js',
        'torus_explainer': './visualise/examples/js/torus_explainer.js',
    },
    output: {
        path: path.resolve(__dirname, 'visualise/deploy'),
        filename: '[name]-bundle.js',
        // clean: true,
    },
    watchOptions: {
        ignored: [
            '**/node_modules',
            '**/build',
            '**/Textures',
            '**/Samples',
            '**/*.o',
            '**/*.a',
            '**/*.cmake',
            '**/CMakeFiles',
            '**/deploy',
            '**/.git'
        ],
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
