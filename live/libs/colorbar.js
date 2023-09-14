let minLabel = document.createElement('span');
let maxLabel = document.createElement('span');
let colorbar = document.createElement('span');

let cb = document.createElement('div');
cb.id = 'colorbar';
document.body.appendChild(cb);

function intToRGB(i) {
    return '#' + (i + 0x1000000).toString(16).slice(-6);
}

export function renderColorbar(lut) {
    // remove anything that was there before
    while (cb.firstChild) {
        cb.removeChild(cb.firstChild);
    }

    // Generate the linear gradient for the colorbar based on the lookup table
    let gradientParts = [];
    let range = lut.getMax() - lut.getMin();
    for (let i = 0; i <= 1; i += 0.1) {
        let value = lut.getMin() + range * i;
        let c = lut.getColor(value).getHex();
        let color = intToRGB(c)
        gradientParts.push(`${color} ${i * 100}%`)
    }

    let gradient = `linear-gradient(to right, ${gradientParts.join(', ')})`;

    // Create the labels
    let m, M;
    if (Number.isInteger(lut.getMin())) { m = lut.getMin() }
    else { m = parseFloat(lut.getMin()).toPrecision(2); }
    if (Number.isInteger(lut.getMax())) { M = lut.getMax() }
    else { M = parseFloat(lut.getMax()).toPrecision(2); }
    minLabel.textContent = m;
    minLabel.style.marginRight = '10px';

    maxLabel.textContent = M;
    maxLabel.style.marginLeft = '10px';

    // Create the colorbar
    // colorbar.style.display = 'inline-block';
    colorbar.style.flexGrow = 1;
    colorbar.style.width = `calc(100% - ${minLabel.offsetWidth + maxLabel.offsetWidth + 50}px)`;
    colorbar.style.height = '20px';
    colorbar.style.background = gradient;
    colorbar.style.verticalAlign = 'middle';

    // Append to container
    cb.appendChild(minLabel);
    cb.appendChild(colorbar);
    cb.appendChild(maxLabel);
}
