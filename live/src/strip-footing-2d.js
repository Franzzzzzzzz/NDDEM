import css from "../css/main.css";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';

var urlParams = new URLSearchParams(window.location.search);

if (!urlParams.has('lut')) { urlParams.set('lut', 'Size') }

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
let show_stats = false;
let mesh_added = false;

var params = {
    dimension: 2,
    boundary0: {
        type: 'WALL',
        min: -0.5,
        max: 0.5,
        max1: 0.08,
        max2: 200
    },
    boundary1: {
        type: 'WALL',
        min: -0.2,
        max: 0.2,
    },
    boundary2: {
        type: 'None',
        min: 0,
        max: 0,
    },

    strip : {
        width: 0.1,
        height: 0.1,
        x : 0,
        y : 0.25,
        vy : -0.1,
        vx : 0,
    },
    // N: 400,
    gravity: true,
    theta: 0,
    paused: false,
    r_min: 0.01,
    r_max: 0.011,
    omega: 5, // rotation rate
    lut: 'Size',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    wallremoved: false,
    particle_density: 2700,
    target_nu: 0.8,
    show_gui: true,
    period: 5000,
    damping: 0.01,
    time: 0,
    particle_opacity: 0.8,
}


function setup() {
    let V_solid = params.target_nu * (params.boundary0.max - params.boundary0.min) * (params.boundary1.max - params.boundary1.min);
    params.average_radius = (params.r_min + params.r_max) / 2.;
    params.N = Math.floor(V_solid / (Math.PI * params.average_radius * params.average_radius));

    params.particle_volume = Math.PI * Math.pow(params.average_radius, 2);
    params.particle_mass = params.particle_volume * params.particle_density;

    params.F_mag_max = 50 * params.density_ratio;
}

if (urlParams.has('no_stats')) {
    show_stats = false;
}
if (urlParams.has('quality')) { params.quality = parseInt(urlParams.get('quality')); }
if (urlParams.has('theta')) { params.theta = -parseFloat(urlParams.get('theta')); }
if (urlParams.has('period')) { params.period = parseFloat(urlParams.get('period')); }


if (urlParams.has('auto')) {
    setInterval(() => {
        params.wallremoved = !params.wallremoved;
        update_wall();
    }, params.period);
}
if (urlParams.has('nogui')) { params.show_gui = false; }
if (urlParams.has('target_nu')) { params.target_nu = parseFloat(urlParams.get('target_nu')); }

SPHERES.createNDParticleShader(params).then(init);

async function init() {
    setup();
    // console.log(r)
    await NDDEMPhysics();


    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    let aspect = window.innerWidth / window.innerHeight;
    let zoom = 1.5;
    let left = params.boundary0.min;
    let right = params.boundary0.max;
    let top = params.boundary1.max;
    let bottom = params.boundary1.min;
    if ((top - bottom) > (right - left)) {
        left = zoom * params.boundary0.min;
        right = zoom * params.boundary0.max;
        top = zoom * params.boundary1.max/ aspect;
        bottom = zoom * params.boundary1.min/ aspect;
    } else {
        left = zoom * params.boundary0.min/ aspect;
        right = zoom * params.boundary0.max/ aspect;
        top = zoom * params.boundary1.max;
        bottom = zoom * params.boundary1.min;
    }
    camera = new THREE.OrthographicCamera(left, right, top, bottom, -100, 1000);
    camera.up.set(0, 1, 0);

    camera.position.x = (params.boundary0.max + params.boundary0.min) / 2.;
    camera.position.y = (params.boundary1.max + params.boundary1.min) / 2.;
    camera.position.z = (params.boundary0.max - params.boundary0.min) * 2.;

    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x000000);

    const hemiLight = new THREE.AmbientLight();
    // hemiLight.intensity = 0.35;
    scene.add(hemiLight);


    // let axesHelper = new THREE.AxesHelper();
    // scene.add(axesHelper);

    SPHERES.add_spheres(S, params, scene);
    WALLS.createWalls(scene);

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;
    var container = document.getElementById('canvas');
    container.appendChild(renderer.domElement);

    // gui
    if (params.show_gui) {
        gui = new GUI();

        gui.width = 300;
        gui.add(params, 'damping', 0, 1, 0.001).name('Damping').listen().onChange(() => {
            S.simu_interpret_command("set damping " + String(params.damping));
        });
        gui.add(params, 'particle_opacity', 0, 1, 0.01).name('Particle opacity').listen();
    }


    controls = new OrbitControls(camera, renderer.domElement);
    controls.enableRotate = false;
    controls.target.x = params.boundary0.min + (params.boundary0.max - params.boundary0.min) / 2;
    controls.target.y = params.boundary1.min + (params.boundary1.max - params.boundary1.min) / 2;
    controls.target.z = 0;

    controls.update();

    window.addEventListener('resize', onWindowResize, false);

    if (show_stats) { make_graph(); }

    animate();
}

function update_wall() {
    let theta;
    if (params.wallremoved) {
        theta = params.theta * Math.PI / 180;
        params.boundary0.max = params.boundary0.max2;
    } else {
        theta = 0;
        params.boundary0.max = params.boundary0.max1;
    }
    S.simu_interpret_command("boundary 0 " + String(params.boundary0.type) + " " + String(params.boundary0.min) + " " + String(params.boundary0.max));
    S.simu_interpret_command("gravity " + String(-10 * Math.sin(theta)) + " " + String(-10 * Math.cos(theta)));
    if (!params.wallremoved) {
        S.simu_interpret_command("auto location randomdrop");
        crazy_damp();
    }
}


function onWindowResize() {

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize(window.innerWidth, window.innerHeight);

}

function animate() {
    requestAnimationFrame(animate);
    SPHERES.move_spheres(S, params);

    S.simu_step_forward(15);
    params.time += 15 * params.dt;
    update_strip_footing();
    controls.update();
    WALLS.update(params);

    renderer.render(scene, camera);
}

async function NDDEMPhysics() {

    if ('DEMCGND' in window === false) {

        console.error('NDDEMPhysics: Couldn\'t find DEMCGND.js');
        return;

    }

    await DEMCGND().then((NDDEMCGLib) => {
        S = eval('new NDDEMCGLib.DEMCG' + String(params.dimension) + 'D (params.N)');
        finish_setup();
    });

    function finish_setup() {
        S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
        let m = Math.PI*0.5*0.5*params.particle_density;
        let min_mass = Math.PI * params.r_min * params.r_min * params.particle_density;
        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform "+params.r_min+" "+params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");

        S.simu_interpret_command("boundary 0 " + String(params.boundary0.type) + " " + String(params.boundary0.min + params.r_max) + " " + String(params.boundary0.max - params.r_max));
        S.simu_interpret_command("boundary 1 " + String(params.boundary1.type) + " " + String(params.boundary1.min + params.r_max) + " " + String(params.boundary1.max - params.r_max));

        // S.simu_interpret_command("auto location randomsquare");
        S.simu_interpret_command("auto location randomdrop");
        // S.simu_interpret_command("gravity 0 -10");

        S.simu_interpret_command("boundary 0 " + String(params.boundary0.type) + " " + String(params.boundary0.min) + " " + String(params.boundary0.max));
        S.simu_interpret_command("boundary 1 " + String(params.boundary1.type) + " " + String(params.boundary1.min) + " " + String(params.boundary1.max));

        S.simu_interpret_command("gravity " + String(-10 * Math.sin(0)) + " " + String(-10 * Math.cos(0)));

        add_strip_footing();
        // console.log("Strip footing added");
        mesh_added = true;

        let tc = 2e-3;
        params.dt = tc / 10;
        let rest = 0.5;
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient(tc, rest, min_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8 * vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 1");
        S.simu_interpret_command("set Mu_wall 1");
        S.simu_interpret_command("set damping " + String(params.damping));
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(params.dt));
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init();

        crazy_damp();

    }
}

function crazy_damp() {
    for (let t = 0; t < 10; t++) {
        S.simu_step_forward(5);

        for (let i = 0; i < params.N; i++) {
            S.simu_setVelocity(i, [0, 0]);
            // S.simu_setOmega(i,[0,0]);
        }
    }
}

function add_strip_footing() {
    if (mesh_added) { S.simu_interpret_command('mesh remove 0'); }

    // Create a box mesh for the strip using params.strip
    const x = params.strip.x;
    const y = params.strip.y;
    const w = params.strip.width;
    const h = params.strip.height;

    // Define the four corners of the box (counter-clockwise)
    const v0 = [x - w / 2, y - h / 2, 0];
    const v1 = [x + w / 2, y - h / 2, 0];
    const v2 = [x + w / 2, y + h / 2, 0];
    const v3 = [x - w / 2, y + h / 2, 0];

    // Mesh object with one closed polygon (the box)
    const meshObj = {
        dimension: 2,
        objects: [
            {
                dimensionality: 1,
                vertices: [v0, v1]
            },
            {
                dimensionality: 1,
                vertices: [v1, v2]
            },
            {
                dimensionality: 1,
                vertices: [v2, v3]
            },
            {
                dimensionality: 1,
                vertices: [v3, v0]
            }
        ]
    };

    // console.log(meshObj)

    S.simu_interpret_command('mesh string ' + JSON.stringify(meshObj));
}

function update_strip_footing() {
    if (mesh_added) { S.simu_interpret_command('mesh remove 0'); }

    // Create a box mesh for the strip using params.strip
    const x = params.strip.x + params.strip.vx * params.time;
    const y = params.strip.y + params.strip.vy * params.time;
    const w = params.strip.width;
    const h = params.strip.height;

    // Define the four corners of the box (counter-clockwise)
    const v0 = [x - w / 2, y - h / 2, 0];
    const v1 = [x + w / 2, y - h / 2, 0];
    const v2 = [x + w / 2, y + h / 2, 0];
    const v3 = [x - w / 2, y + h / 2, 0];

    // Mesh object with one closed polygon (the box)
    const meshObj = {
        dimension: 2,
        objects: [
            {
                dimensionality: 1,
                vertices: [v0, v1]
            },
            {
                dimensionality: 1,
                vertices: [v1, v2]
            },
            {
                dimensionality: 1,
                vertices: [v2, v3]
            },
            {
                dimensionality: 1,
                vertices: [v3, v0]
            }
        ]
    };

    // console.log(meshObj)

    S.simu_interpret_command('mesh string ' + JSON.stringify(meshObj));
}
