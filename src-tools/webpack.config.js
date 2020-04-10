'use strict';

const path = require('path');
const nodeExternals = require('webpack-node-externals');

module.exports = {
    entry: './index.js',
    mode: 'development',
    output: {
        path: path.resolve(__dirname),
        filename: 'duktool.js'
    },
    target: 'node',
    externals: [ nodeExternals() ],
    module: {
        rules: [
            {
                test: /\.m?js$/,
                exclude: /(node_modules|bower_components)/,
                use: {
                    loader: "babel-loader",
                    options: {
                        presets: [ '@babel/preset-env' ]
                    }
                }
            }
        ]
    },
    /*
    node: {
        fs: 'empty'
    }
    */
};
