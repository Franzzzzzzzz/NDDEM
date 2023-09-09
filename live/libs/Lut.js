import {
	Color,
	MathUtils
} from 'three';

class Lut {

	constructor(colormap, count = 32) {

		this.isLut = true;

		this.lut = [];
		this.map = [];
		this.n = 0;
		this.minV = 0;
		this.maxV = 1;

		this.setColorMap(colormap, count);

	}

	set(value) {

		if (value.isLut === true) {

			this.copy(value);

		}

		return this;

	}

	setMin(min) {

		this.minV = min;

		return this;

	}

	setMax(max) {

		this.maxV = max;

		return this;

	}

	getMin() { return this.minV }
	getMax() { return this.maxV }

	setColorMap(colormap, count = 32) {
		this.map = ColorMapKeywords[colormap];// || ColorMapKeywords.rainbow;
		this.n = count;

		const step = 1.0 / this.n;
		const minColor = new Color();
		const maxColor = new Color();

		this.lut.length = 0;

		// sample at 0

		this.lut.push(new Color(this.map[0][1]));

		// sample at 1/n, ..., (n-1)/n

		for (let i = 1; i < count; i++) {

			const alpha = i * step;

			for (let j = 0; j < this.map.length - 1; j++) {

				if (alpha > this.map[j][0] && alpha <= this.map[j + 1][0]) {

					const min = this.map[j][0];
					const max = this.map[j + 1][0];

					minColor.set(this.map[j][1]);
					maxColor.set(this.map[j + 1][1]);

					const color = new Color().lerpColors(minColor, maxColor, (alpha - min) / (max - min));

					this.lut.push(color);

				}

			}

		}

		// sample at 1

		this.lut.push(new Color(this.map[this.map.length - 1][1]));

		return this;

	}

	copy(lut) {

		this.lut = lut.lut;
		this.map = lut.map;
		this.n = lut.n;
		this.minV = lut.minV;
		this.maxV = lut.maxV;

		return this;

	}

	getColor(alpha) {

		alpha = MathUtils.clamp(alpha, this.minV, this.maxV);

		alpha = (alpha - this.minV) / (this.maxV - this.minV);

		const colorPosition = Math.round(alpha * this.n);

		return this.lut[colorPosition];

	}

	addColorMap(name, arrayOfColors) {

		ColorMapKeywords[name] = arrayOfColors;

		return this;

	}

	createCanvas() {

		const canvas = document.createElement('canvas');
		canvas.width = 1;
		canvas.height = this.n;

		this.updateCanvas(canvas);

		return canvas;

	}

	updateCanvas(canvas) {

		const ctx = canvas.getContext('2d', { alpha: false });

		const imageData = ctx.getImageData(0, 0, 1, this.n);

		const data = imageData.data;

		let k = 0;

		const step = 1.0 / this.n;

		const minColor = new Color();
		const maxColor = new Color();
		const finalColor = new Color();

		for (let i = 1; i >= 0; i -= step) {

			for (let j = this.map.length - 1; j >= 0; j--) {

				if (i < this.map[j][0] && i >= this.map[j - 1][0]) {

					const min = this.map[j - 1][0];
					const max = this.map[j][0];

					minColor.set(this.map[j - 1][1]);
					maxColor.set(this.map[j][1]);

					finalColor.lerpColors(minColor, maxColor, (i - min) / (max - min));

					data[k * 4] = Math.round(finalColor.r * 255);
					data[k * 4 + 1] = Math.round(finalColor.g * 255);
					data[k * 4 + 2] = Math.round(finalColor.b * 255);
					data[k * 4 + 3] = 255;

					k += 1;

				}

			}

		}

		ctx.putImageData(imageData, 0, 0);

		return canvas;

	}

}

const ColorMapKeywords = {

	'rainbow': [[0.0, 0x0000FF], [0.2, 0x00FFFF], [0.5, 0x00FF00], [0.8, 0xFFFF00], [1.0, 0xFF0000]],
	'cooltowarm': [[0.0, 0x3C4EC2], [0.2, 0x9BBCFF], [0.5, 0xDCDCDC], [0.8, 0xF6A385], [1.0, 0xB40426]],
	'blackbody': [[0.0, 0x000000], [0.2, 0x780000], [0.5, 0xE63200], [0.8, 0xFFFF00], [1.0, 0xFFFFFF]],
	'grayscale': [[0.0, 0x000000], [0.2, 0x404040], [0.5, 0x7F7F80], [0.8, 0xBFBFBF], [1.0, 0xFFFFFF]],
	'viridis': [[0.0, 0x440154], [0.05, 0x471265], [0.1, 0x482374], [0.15, 0x45347f], [0.2, 0x404387], [0.25, 0x3a528b], [0.3, 0x345e8d], [0.35, 0x2e6b8e], [0.4, 0x29788e], [0.45, 0x24848d], [0.5, 0x20908c], [0.55, 0x1e9b89], [0.6, 0x22a784], [0.65, 0x2fb37b], [0.7, 0x44be70], [0.75, 0x5ec961], [0.8, 0x79d151], [0.85, 0x9ad83c], [0.9, 0xbdde26], [0.95, 0xdfe318], [1.0, 0xfde724]],
	'inferno': [[0.0, 0x000003], [0.05, 0x06041b], [0.1, 0x160b39], [0.15, 0x2b0a56], [0.2, 0x410967], [0.25, 0x570f6d], [0.3, 0x6a176e], [0.35, 0x7e1e6c], [0.4, 0x932567], [0.45, 0xa72d5f], [0.5, 0xbb3754], [0.55, 0xcc4148], [0.6, 0xdc5039], [0.65, 0xe9622a], [0.7, 0xf37719], [0.75, 0xf98e08], [0.8, 0xfba40a], [0.85, 0xfabd23], [0.9, 0xf5d745], [0.95, 0xf1ee74], [1.0, 0xfcfea4]],
	'plasma': [[0.0, 0x0c0786], [0.05, 0x290593], [0.1, 0x40039c], [0.15, 0x5601a3], [0.2, 0x6a00a7], [0.25, 0x7e03a7], [0.3, 0x8f0da3], [0.35, 0xa01b9b], [0.4, 0xb02a8f], [0.45, 0xbe3883], [0.5, 0xcb4777], [0.55, 0xd6556d], [0.6, 0xe06461], [0.65, 0xea7356], [0.7, 0xf2844b], [0.75, 0xf89540], [0.8, 0xfca635], [0.85, 0xfdb92b], [0.9, 0xfcce25], [0.95, 0xf6e425], [1.0, 0xeff821]],
	'magma': [[0.0, 0x000003], [0.05, 0x060519], [0.1, 0x140d35], [0.15, 0x251155], [0.2, 0x3b0f6f], [0.25, 0x50127b], [0.3, 0x63197f], [0.35, 0x772181], [0.4, 0x8c2980], [0.45, 0xa12f7e], [0.5, 0xb63679], [0.55, 0xca3e72], [0.6, 0xdd4968], [0.65, 0xed595f], [0.7, 0xf6705b], [0.75, 0xfb8861], [0.8, 0xfd9f6c], [0.85, 0xfeb77d], [0.9, 0xfdcf92], [0.95, 0xfce6a8], [1.0, 0xfbfcbf]],
	'bkr': [[0.0, 0x0000FF], [0.5, 0x000000], [1.0, 0xFF0000]],
	'bwr': [[0.0, 0x0000FF], [0.5, 0xFFFFFF], [1.0, 0xFF0000]],
	'spectral': [[0.0, 0x9e0142], [0.05, 0xb71d48], [0.1, 0xd33c4e], [0.15000000000000002, 0xe45549], [0.2, 0xf46d43], [0.25, 0xf88e52], [0.3, 0xfcac60], [0.35, 0xfdc675], [0.4, 0xfee08b], [0.45, 0xfeefa5], [0.5, 0xfefebe], [0.55, 0xf2faab], [0.6, 0xe6f598], [0.65, 0xc7e89e], [0.7, 0xa9dca4], [0.75, 0x86cea4], [0.8, 0x66c2a5], [0.85, 0x4ba4b1], [0.9, 0x3286bc], [0.95, 0x4969ae], [1.0, 0x5e4fa2]],
	'grainsize': [[0.0, 0xFFB566], [0.25, 0xFF4D5B], [0.5, 0xFF3D9B], [0.75, 0xE624FF], [1.0, 0x0000FF]],
	'virino': [[0.0, 0xfcfe9f], [0.05, 0xf9e621], [0.1, 0xb5de2b], [0.15, 0x73d055], [0.2, 0x3dbc74], [0.25, 0x20a486], [0.3, 0x218e8d], [0.35, 0x2b768e], [0.4, 0x365c8d], [0.45, 0x482374], [0.5, 0x010005], [0.55, 0x4c0c6b], [0.6, 0x89226a], [0.65, 0xa82e5f], [0.7, 0xc73e4c], [0.75, 0xe05536], [0.8, 0xf1711e], [0.85, 0xfa9407], [0.9, 0xfbbc21], [0.95, 0xf3e35a], [1.0, 0xfcffa4]],
};

export { Lut, ColorMapKeywords };

