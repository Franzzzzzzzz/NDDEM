let minLabel = document.createElement('span');
let maxLabel = document.createElement('span');
let colorbar = document.createElement('span');
let unitsLabel = document.createElement('div');

let cb = document.createElement('div');
cb.id = 'colorbar';
document.body.appendChild(cb);

function intToRGB(i) {
    return '#' + (i + 0x1000000).toString(16).slice(-6);
}

export function hideColorbar() {
    cb.style.display = 'none';
}

export function renderColorbar(lut) {
    cb.style.display = 'flex';
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

    unitsLabel.innerHTML = lut.units;
    // unitsLabel.style.marginBottom = '20px';
    unitsLabel.style.width = '100%';
    unitsLabel.style.position = 'absolute';
    unitsLabel.style.top = '20px';
    unitsLabel.style.left = '0';
    unitsLabel.style.right = '0';
    // unitsLabel.style.bottom = '20px';
    unitsLabel.style.textAlign = 'center';

    // Create the colorbar
    // colorbar.style.display = 'inline-block';
    colorbar.style.flexGrow = 1;
    colorbar.style.width = `calc(100% - ${minLabel.offsetWidth + maxLabel.offsetWidth + 50}px)`;
    colorbar.style.height = '20px';
    colorbar.style.background = gradient;
    colorbar.style.verticalAlign = 'middle';
    colorbar.style.opacity = lut.opacity;

    // Append to container
    cb.appendChild(unitsLabel);
    cb.appendChild(minLabel);
    cb.appendChild(colorbar);
    cb.appendChild(maxLabel);
    
}
