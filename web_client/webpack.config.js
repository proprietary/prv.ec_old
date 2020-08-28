const path = require('path');

module.exports = {
	mode: process.env.NODE_ENV === 'development' ? 'development' : 'production',
	entry: './index.ts',
	module: {
		rules: [
			{
				test: /\.ts$/,
				use: 'ts-loader',
				exclude: /node_modules/,
			},
		],
	},
	resolve: {
		extensions: ['.js', '.ts'],
		modules: [
			path.resolve(__dirname, 'vendor'),
			'node_modules',
		],
	},
	output: {
		filename: 'bundle.js',
		path: path.resolve(__dirname, 'dist'),
	},
};
