// Aspect ratio describes the initial bed width / height, before gravity settles it.
export function createEarthPressurePacking({ particleCount = 2000, aspectRatio = 2,
    width = 0.66, radiusRatio = 0.5 } = {}) {
    if (!Number.isInteger(particleCount) || particleCount < 1 || particleCount > 10000) {
        throw new RangeError('Particle count must be an integer from 1 to 10000.');
    }
    if (!Number.isFinite(aspectRatio) || aspectRatio <= 0 || !Number.isFinite(width) || width <= 0) {
        throw new RangeError('Bed aspect ratio and width must be positive finite numbers.');
    }
    if (!Number.isFinite(radiusRatio) || radiusRatio <= 0 || radiusRatio > 1) {
        throw new RangeError('Minimum / maximum grain radius must be greater than 0 and at most 1.');
    }
    const bedHeight = width / aspectRatio;
    const rowPitch = Math.sqrt(3) / 2;
    let columns = 1, rows = particleCount, spacing = 0;
    // Choose the staggered grid that fits the requested rectangle most efficiently.
    for (let candidate = 1; candidate <= particleCount; candidate++) {
        const candidateRows = Math.ceil(particleCount / candidate);
        const candidateSpacing = Math.min(width / (candidate + (candidateRows > 1 ? 0.5 : 0)),
            bedHeight / (1 + (candidateRows - 1) * rowPitch));
        if (candidateSpacing > spacing) {
            columns = candidate;
            rows = candidateRows;
            spacing = candidateSpacing;
        }
    }
    const rMax = 0.48 * spacing;
    const rMin = radiusRatio * rMax;
    const locations = Array.from({ length: particleCount }, (_, i) => {
        const row = Math.floor(i / columns);
        const particlesInRow = Math.min(columns, particleCount - row * columns);
        // Spread a partially filled last row across the bed rather than piling it on the left.
        const col = Math.floor((i % columns + 0.5) * columns / particlesInRow);
        const jitter = 0.005 * spacing * Math.sin(i * 137.5);
        return [(0.5 + col + (row % 2) * 0.5) * spacing + jitter,
            (0.5 + row * rowPitch) * spacing];
    });
    return { count: particleCount, aspectRatio, width, bedHeight, columns, rows, rMin, rMax,
        wallHeight: Math.max(1.35 * bedHeight, bedHeight + 4 * rMax), locations };
}

// A free-surface, 2D soil bed bounded by a translating left wall.
export class EarthPressureSimulation {
    constructor(library, options = {}) {
        this.library = library;
        Object.assign(this, createEarthPressurePacking(options));
        this.wallSpeed = 0.01;
        this.travel = 0.065;
        this.settlingTime = Math.max(2, 4 * Math.sqrt(2 * this.bedHeight / 9.81));
        this.setup();
    }

    setup() {
        this.S = new this.library.DEMCG2D(this.count);
        const command = text => this.S.simu_interpret_command(text);
        command(`dimensions 2 ${this.count}`);
        command('radius -1 0.5');
        command(`mass -1 ${Math.PI * 0.25 * 2700}`);
        command('auto rho');
        command(`auto radius uniform ${this.rMin} ${this.rMax}`);
        command('auto mass');
        command('auto inertia');
        // Initialise the contact-search grid over the full wall travel envelope.
        command(`boundary 0 WALL ${-this.travel} ${this.width}`);
        // The upper boundary is well above the bed, leaving a free surface.
        command(`boundary 1 WALL 0 ${4 * this.wallHeight}`);
        command('gravity 0 -9.81');
        const mass = Math.PI * this.rMin ** 2 * 2700;
        // Smaller grains and taller beds need stiffer contacts and a smaller timestep.
        const collisionTime = Math.min(0.004, 0.8 * this.rMin / Math.sqrt(9.81 * this.bedHeight));
        this.dt = collisionTime / 20;
        const restitution = 0.25;
        const stiffness = mass * (Math.PI ** 2 + Math.log(restitution) ** 2) / (2 * collisionTime ** 2);
        const damping = -mass * Math.log(restitution) / collisionTime;
        command(`set Kn ${stiffness}`);
        command(`set Kt ${0.8 * stiffness}`);
        command(`set GammaN ${damping}`);
        command(`set GammaT ${damping}`);
        command('set Mu 0.5');
        command('set Mu_wall 0.3');
        command('set damping 0.01');
        command(`set dt ${this.dt}`);
        command('set T 1000000');
        command('set tinfo 1000000');
        command('set tdump 1000000');
        command('ContactStrategy cells');
        command('auto skin');
        // Non-overlapping initial placement; gravity creates the settled bed.
        for (let i = 0; i < this.count; i++) {
            this.S.simu_fixParticle(i, this.locations[i]);
        }
        this.S.simu_finalise_init();
        this.S.simu_setBoundary(0, [0, this.width]);
        this.radii = this.S.simu_getRadii();
        this.time = 0;
        this.wall = 0;
        this.target = 0;
        this.thrust = 0;
        this.samples = [];
        this.sampleTime = 0;
        this.positions = this.S.simu_getX();
        this.S.simu_getWallForce(); // Request the engine's next wall-force calculation.
    }

    get ready() { return this.time >= this.settlingTime; }

    setTarget(value) {
        this.target = Math.max(-this.travel, Math.min(this.travel, value));
    }

    advance(steps = Math.ceil(0.016 / this.dt)) {
        for (let i = 0; i < steps; i++) {
            if (this.ready) {
                // Limit displacement at EVERY DEM step to avoid wall jumps and overlaps.
                const delta = Math.max(-this.wallSpeed * this.dt,
                    Math.min(this.wallSpeed * this.dt, this.target - this.wall));
                if (delta !== 0) {
                    this.wall += delta;
                    this.S.simu_setBoundary(0, [this.wall, this.width]);
                }
            }
            this.S.simu_step_forward(1);
            this.time += this.dt;
        }
        const forces = this.S.simu_getWallForce();
        if (forces.length === 4) {
            // Wall 0 is the lower x boundary; the soil pushes it to the left.
            this.thrust += 0.15 * (Math.abs(forces[0][0]) - this.thrust);
            this.S.simu_getWallForce();
        }
        this.positions = this.S.simu_getX();
        if (this.ready && this.time - this.sampleTime >= 0.08) {
            this.samples.push([this.wall * 1000, this.thrust]);
            if (this.samples.length > 1500) this.samples.shift();
            this.sampleTime = this.time;
        }
    }

    contacts() { return this.S.simu_getContactInfos(0x80 | 0x100); }
}
