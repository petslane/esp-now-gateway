module.exports = {
    productionSourceMap: false,
    configureWebpack: {
        optimization: {
            splitChunks: false
        },
        output: {
            filename: '[name].js'
        },
    },
    css: {
        extract: false,
    }
};
