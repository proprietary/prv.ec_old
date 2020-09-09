const path = require('path');
// const HtmlWebpackPlugin = require('html-webpack-plugin');
// const InlineChunkHtmlPlugin = require('./vendor/InlineChunkHtmlPlugin.js');

module.exports = {
	// mode: process.env.NODE_ENV === 'development' ? 'development' : 'production',
	mode: 'development',
	entry: path.join(__dirname, 'index.ts'),
	module: {
		rules: [
			{
				test: /\.ts$/,
				use: 'ts-loader',
				exclude: /node_modules/,
			},
			{
				test: /\.(png|svg|jpg|gif)$/,
				use: [
				  'file-loader',
				],
			},
			{
				test: /\.(woff|woff2|eot|ttf|otf)$/,
				use: [
				  'file-loader',
				],
			},		
		],
	},
	resolve: {
		extensions: ['.js', '.ts'],
		modules: [
			'node_modules',
		],
	},
	output: {
		filename: 'bundle.js',
		path: path.resolve(__dirname, 'dist'),
	},
	// plugins: [
	// 	new HtmlWebpackPlugin({
	// 		inject: true,
	// 		template: path.join(__dirname, 'public/index.html'),
	// 	}),
	// 	new InlineChunkHtmlPlugin(HtmlWebpackPlugin, [/\.(js|css)$/]),
	// ],
	devServer: {
		contentBase: path.join(__dirname, 'dist/'),
		publicPath: 'http://127.0.0.1:8000/dist/',
		proxy: {
			'/accept': 'http://127.0.0.1:8000',
		},
		disableHostCheck: true,
		historyApiFallback: {
			index: 'index.html',
		},
	},
};
