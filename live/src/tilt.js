import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
// import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { Lut } from 'three/examples/jsm/math/Lut.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
import * as AUDIO from '../libs/audio.js';
import * as CGHANDLER from '../libs/CGHandler.js';

// The tilt of the stress, live.
//
// Same simple-shear cell as anisotropy.js (Lees-Edwards, pressure controlled),
// with the graphs replaced by three views of one object. Every contact carries
// its force along a branch; averaging them gives the Love-Weber stress of the
// cell, sigma = (1/A) sum l (x) f. Its size is p and its tilt is an arrow of
// length q/p pointing along sigma_1. With C and S its vertical and sideways
// parts,
//     sigma_v = p (1 + C),  sigma_h = p (1 - C),  tau = p S.
// Friction caps the length of the tilt. Under steady shear it runs out to that
// cap and stays there: the critical state.
//
// The camera is rotated by 90 degrees, so simulation x (the Lees-Edwards
// gradient direction) is vertical on screen and simulation y is horizontal.


// let info_div = document.createElement("div")
// info_div.innerHTML = "Click on a particle to grab it"
// info_div.style.color = "white";
// info_div.style.position = "absolute";
// info_div.style.left = "20px";
// info_div.style.top = "20px";
// document.body.appendChild(info_div);

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100 * graph_fraction) + '%';
document.getElementById("stats").style.height = '100vh';
document.getElementById("canvas").style.width = String(100 * (1 - graph_fraction)) + '%';

let I_div = document.createElement('div');
I_div.style.cssText = 'position:absolute;color:white;bottom:10px;right:10px;font-size:24px;z-index:10000;';
document.body.appendChild(I_div);

var urlParams = new URLSearchParams(window.location.search);

let density, vavg, stressTcxx, stressTcyy, stressTczz, stressTcxy;
let pressure = [], shearstress = [], xloc = [];
let camera, scene, renderer, stats, panel, controls;
let gui;
let S;
// let count;

var params = {
    dimension: 2,
    L: 20 * 0.004, //system size
    initial_density: 0.75,
    zoom: 0.75,
    aspect_ratio: 1,
    // paused: false,
    // g_mag: 1e3,
    // theta: 0, // slope angle in DEGREES
    d4: { cur: 0 },
    r_max: 0.005,
    r_min: 0.003,
    particle_density: 2700,
    friction: 0.5,
    // freq: 0.05,
    // new_line: false,
    shear_rate: 1,
    // lut: 'None',
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 25,
    cg_height: 25,
    cg_opacity: 0,
    cg_window_size: 3,
    particle_opacity: 0.5,
    target_pressure: 1e4,
    F_mag_max: 1e3,
    audio: false,
    audio_sensitivity: 1,
    current_pressure: 0,
}

let rainbow = new Lut("rainbow", 512); // options are rainbow, cooltowarm and blackbody
let cooltowarm = new Lut("cooltowarm", 512); // options are rainbow, cooltowarm and blackbody
let blackbody = new Lut("blackbody", 512); // options are rainbow, cooltowarm and blackbody

params.average_radius = (params.r_min + params.r_max) / 2.;
params.thickness = params.average_radius;


if (urlParams.has('dimension')) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if (params.dimension === 2) {
    params.particle_volume = Math.PI * Math.pow(params.average_radius, 2);
}
else if (params.dimension === 3) {
    params.particle_volume = 4. / 3. * Math.PI * Math.pow(params.average_radius, 3);
}
else if (params.dimension === 4) {

    params.L = 2.5;
    params.N = 300
    params.particle_volume = Math.PI * Math.PI * Math.pow(params.average_radius, 4) / 2.;
}

params.N = Math.ceil(params.initial_density * 4 * params.L * params.L * params.aspect_ratio / params.particle_volume);

params.particle_mass = params.particle_volume * params.particle_density;

if (urlParams.has('cg_width')) { params.cg_width = parseInt(urlParams.get('cg_width')); }
if (urlParams.has('cg_height')) { params.cg_height = parseInt(urlParams.get('cg_height')); }
if (urlParams.has('cg_opacity')) { params.cg_opacity = parseFloat(urlParams.get('cg_opacity')); }
if (urlParams.has('particle_opacity')) { params.particle_opacity = parseFloat(urlParams.get('particle_opacity')); }

if (urlParams.has('quality')) { params.quality = parseInt(urlParams.get('quality')); }

SPHERES.createNDParticleShader(params).then(init);

async function init() {

    await NDDEMCGPhysics();

    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    var aspect = window.innerWidth / window.innerHeight / 2.;
    camera = new THREE.OrthographicCamera(
        -params.L * params.aspect_ratio * aspect / params.zoom,
        params.L * params.aspect_ratio * aspect / params.zoom,
        -params.L * params.aspect_ratio / params.zoom,
        params.L * params.aspect_ratio / params.zoom,
        -10,
        10
    );
    camera.position.set(0, 0, 1);
    camera.rotateZ(Math.PI / 2.);
    // camera.up.set(0, 0, 0);
    // camera.lookAt(0,0,0);
    // console.log(camera)

    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x111);

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add(hemiLight);

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set(5, -5, -5);
    dirLight.castShadow = true;
    scene.add(dirLight);

    SPHERES.add_spheres(S, params, scene);
    SPHERES.update_contact_flags(0x80 | 0x100 | 0x200);

    // WALLS.add_back(params, scene);
    // WALLS.add_left(params, scene);
    // WALLS.add_right(params, scene);
    // WALLS.add_front(params, scene);
    // // WALLS.back.scale.y = params.thickness;//Math.PI/2.;
    // var vert_walls = [WALLS.left,WALLS.right,WALLS.back,WALLS.front];

    // vert_walls.forEach( function(mesh) {
    //     mesh.scale.x = 2*params.L + 2*params.thickness;
    //     mesh.scale.z = 2*params.L + 2*params.thickness;
    // });

    CGHANDLER.add_cg_mesh(2 * params.L, 2 * params.L * params.aspect_ratio, scene);

    // geometry = new THREE.PlaneGeometry( 2*params.L, 0.1*params.L );
    // material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    // colorbar_mesh = new THREE.Mesh( geometry, material );
    // colorbar_mesh.position.y = -1.1*params.L;
    // scene.add( colorbar_mesh );


    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth / 2., window.innerHeight);

    var container = document.getElementById('canvas');
    container.appendChild(renderer.domElement);

    gui = new GUI();
    gui.width = 320;

    if (params.dimension == 4) {
        gui.add(params.d4, 'cur', -params.L, params.L, 0.001)
            .name('D4 location').listen()
            .onChange(function () {
                if (urlParams.has('stl')) {
                    meshes = renderSTL(meshes, NDsolids, scene, material, params.d4.cur);
                }
            });
    }
    // gui.add ( params, 'particle_opacity', 0, 1).name('Particle opacity').listen().onChange( () => SPHERES.update_particle_material(params,
    // lut_folder
    // ));
    gui.add(params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
    gui.add(params, 'cg_field', ['Density', 'Velocity', 'Pressure', 'Shear stress']).name('Field').listen();
    gui.add(params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen().onChange(() => {
        update_cg_params(S, params);
    });
    gui.add(params, 'shear_rate', -1, 1).name('Shear rate').listen().onChange(update_shear_rate);
    // gui.add ( params, 'shear_rate', {Back : -1, Slow: 1e-1, Forward: 1}).name('Shear rate').listen().onChange( update_shear_rate );
    gui.add(params, 'friction', 0, 2).name('Interparticle friction').listen().onChange(() => { S.simu_interpret_command("set Mu " + String(params.friction)) });
    // gui.add ( params, 'target_pressure', 1e4, 1e6, 1e4).name('Target pressure (Pa)').listen().onChange(update_shear_rate);


    gui.add(params, 'audio_sensitivity', 1, 1e3, 1).name('Audio sensitivity');
    gui.add(params, 'audio').name('Audio').listen().onChange(() => {
        if (AUDIO.listener === undefined) {
            AUDIO.make_listener(camera);
            AUDIO.add_fixed_sound_source([0, 0, 0]);
            // SPHERES.add_normal_sound_to_all_spheres();
        } else {
            // AUDIO.remove_listener( camera ); // doesn't do anything at the moment...
            // SPHERES.mute_sounds();
        }
    });

    window.addEventListener('resize', onWindowResize, false);
    // Handle tab visibility changes to prevent timing issues
    document.addEventListener('visibilitychange', function() {
        if (document.hidden) {
            params.paused = true;
            console.log('tab hidden - pausing simulation');
        } else {
            // Tab became visible - resume simulation and reset timing
            params.paused = false;
        }
    });

    make_graph();
    animate();
    update_I();
}

function onWindowResize() {

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / window.innerHeight / 2.;
    camera.left = -params.L * params.aspect_ratio * aspect / params.zoom;
    camera.right = params.L * params.aspect_ratio * aspect / params.zoom;
    camera.bottom = -params.L * params.aspect_ratio / params.zoom;
    camera.top = params.L * params.aspect_ratio / params.zoom;

    renderer.setSize(window.innerWidth / 2., window.innerHeight);
}

function animate() {
    requestAnimationFrame(animate);
    if (!params.paused) {
        if (AUDIO.listener !== undefined) {
            SPHERES.update_fixed_sounds(S, params);
        }
        RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);

        // let v = S.simu_getVelocity();
        // console.log(v);
        SPHERES.move_spheres(S, params);
        S.simu_step_forward(5);
        CGHANDLER.update_2d_cg_field(S, params);
        SPHERES.draw_force_network(S, params, scene);
        update_pressure();
        update_graph();
        // console.log(S.simu_getTime())
        if (S.simu_getTime() > 0.1) {
            WALLS.update_damped_wall(params.current_pressure, params.target_pressure, params, S, 15 * 1e-2 / 20.);
        }

    }

    renderer.render(scene, camera);
}

function update_pressure() {
    S.cg_param_read_timestep(0);
    S.cg_process_timestep(0, false);
    var grid = S.cg_get_gridinfo();
    density = S.cg_get_result(0, "RHO", 0);
    vavg = S.cg_get_result(0, "VAVG", 1);
    let normal_stresses = [];
    for (let i = 0; i < params.dimension; i++) {
        let component = i * (params.dimension + 1);
        let this_stress = S.cg_get_result(0, "TC", component);
        normal_stresses.push(this_stress);
    }
    // stressTcxx=S.cg_get_result(0, "TC", 0) ;
    // stressTcyy=S.cg_get_result(0, "TC", 4) ;
    // stressTczz=S.cg_get_result(0, "TC", 8) ;
    stressTcxy = S.cg_get_result(0, "TC", 1);
    for (var i = 0; i < stressTcxy.length; i++) // for all grid points
    {
        xloc[i] = grid[0] + i * grid[3];
        shearstress[i] = -stressTcxy[i];
        let this_pressure = 0;
        for (let j = 0; j < normal_stresses.length; j++) {
            this_pressure += normal_stresses[j][i];
        }
        pressure[i] = this_pressure / normal_stresses.length; // get isotropic pressure
    }

    // params.viscosity = 1e6;
    // params.inertial_number = 0.1;
    // params.target_pressure = Math.pow(params.shear_rate*params.average_radius/params.inertial_number,2)*params.particle_density;
    params.current_pressure = pressure.reduce((a, b) => a + b, 0) / pressure.length; // average vertical stress
    params.current_shearstress = shearstress.reduce((a, b) => a + b, 0) / shearstress.length; // average shear stress


    // let dt = 1e-3;
}

function update_cg_params(S, params) {
    var cgparam = {};
    cgparam["file"] = [{ "filename": "none", "content": "particles", "format": "interactive", "number": 1 }];
    cgparam["boxes"] = [params.cg_width, params.cg_height];
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"] = [
        [-params.L, -params.L * params.aspect_ratio],
        [params.L, params.L * params.aspect_ratio]];
    cgparam["window size"] = params.cg_window_size * params.average_radius;
    cgparam["skip"] = 0;
    cgparam["max time"] = 1;
    cgparam["time average"] = "None";
    cgparam["fields"] = ["RHO", "VAVG", "TC", "Pressure", "ShearStress"];
    cgparam["periodicity"] = [false, false];
    cgparam["window"] = "LucyND";
    cgparam["dimension"] = 2;


    // console.log(JSON.stringify(cgparam)) ;
    S.cg_param_from_json_string(JSON.stringify(cgparam));
    S.cg_setup_CG();
}

async function NDDEMCGPhysics() {

    if ('DEMCGND' in window === false) {

        console.error('NDDEMPhysics: Couldn\'t find DEMCGND.js');
        return;

    }

    await DEMCGND().then((NDDEMCGLib) => {
        if (params.dimension == 2) {
            S = new NDDEMCGLib.DEMCG2D(params.N);
        }
        else if (params.dimension == 3) {
            S = new NDDEMCGLib.DEMCG3D(params.N);
        }
        else if (params.dimension == 4) {
            S = new NDDEMCGLib.DEMCG4D(params.N);
        }
        else if (params.dimension == 5) {
            S = new NDDEMCGLib.DEMCG5D(params.N);
        }
        finish_setup();
    });


    function finish_setup() {
        S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
        S.simu_interpret_command("radius -1 0.5");
        let m = params.particle_density * SPHERES.get_particle_volume(params.dimension, 0.5);
        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform " + params.r_min + " " + params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");
        S.simu_interpret_command("auto skin");

        S.simu_interpret_command("boundary 0 PBCLE -" + String(params.L) + " " + String(params.L) + " " + String(params.shear_rate));
        S.simu_interpret_command("boundary 1 PBC -" + String(params.L * params.aspect_ratio) + " " + String(params.L * params.aspect_ratio));
        if (params.dimension >= 3) {
            S.simu_interpret_command("boundary 2 PBC -" + String(params.L) + " " + String(params.L));
        }
        if (params.dimension >= 4) {
            S.simu_interpret_command("boundary 3 PBC -" + String(params.L) + " " + String(params.L));
        }
        S.simu_interpret_command("gravity" + " 0".repeat(params.dimension))

        S.simu_interpret_command("auto location randomdrop");

        let tc = 2e-3;
        let rest = 0.5; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient(tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8 * vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu " + String(params.friction));
        // S.simu_interpret_command("set damping 0.001");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(tc / 20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        S.simu_finalise_init();

        // let v = S.simu_getVelocity();
        // console.log(v);
        update_cg_params(S, params);

    }
}

// function update_wall_particle_velocities() {
//     console.log(params.shear_rate)
//     let n_wall = Math.ceil(2*params.aspect_ratio*params.L/(params.average_radius*2));
//         for ( var i=0; i<n_wall; i++ ) {
//             S.simu_setVelocity(i,[params.shear_rate*2*params.L,0]);
//         }
// }

function update_shear_rate() {
    update_I();
    S.simu_setBoundary(0, [-params.L, params.L, params.shear_rate]);
    params.vmax = 1.5 * Math.abs(params.shear_rate) * params.L;
    params.omegamax = 1e3 * Math.abs(params.shear_rate) * params.average_radius;
    SPHERES.update_particle_material(params);
}

function update_I() {
    // I = |gamma_dot| * d / sqrt(P/rho)
    params.I = Math.abs(params.shear_rate) * params.average_radius / Math.sqrt(params.target_pressure / params.particle_density);
    I_div.innerHTML = 'I = ' + params.I.toPrecision(4);
}


// ------------------------------------------------------------------ the tilt

const TRAIL = 400;          // how many past states to show on the disc
const GRAPH_EVERY = 3;      // update the graphs every few frames
const N_BINS = 18;
let frame = 0;
let trail_C = [], trail_S = [];
let history_t = [];
let settled = [];           // recent tilt sizes, for the critical-state rim

function contact_stress() {
    // Love-Weber stress of the whole cell from the contacts drawn on screen,
    // in screen axes: v is simulation x, h is simulation y.
    let Tvv = 0, Thh = 0, Tvh = 0;
    let fan = new Array(N_BINS).fill(0);
    let rmax = 2.2 * params.r_max;
    for (let c = 0; c < SPHERES.F.length; c++) {
        let i = SPHERES.F[c][0];
        let j = SPHERES.F[c][1];
        let lv = SPHERES.x[i][0] - SPHERES.x[j][0];
        let lh = SPHERES.x[i][1] - SPHERES.x[j][1];
        if (Math.hypot(lv, lh) > rmax) continue; // across a periodic boundary
        let fv = SPHERES.F[c][2] + SPHERES.F[c][4];
        let fh = SPHERES.F[c][3] + SPHERES.F[c][5];
        Tvv += lv * fv;
        Thh += lh * fh;
        Tvh += 0.5 * (lv * fh + lh * fv);
        // the fan: force-weighted contact directions, measured from vertical
        let w = Math.abs(lv * SPHERES.F[c][2] + lh * SPHERES.F[c][3]);
        let angle = Math.atan2(lh, lv);           // from vertical, towards horizontal
        angle = ((angle % Math.PI) + Math.PI) % Math.PI;
        fan[Math.min(N_BINS - 1, Math.floor(angle / Math.PI * N_BINS))] += w;
    }
    let sign = (Tvv + Thh) >= 0 ? 1 : -1; // compression positive
    Tvv *= sign; Thh *= sign; Tvh *= sign;
    let p = 0.5 * (Tvv + Thh);
    let C = p > 0 ? 0.5 * (Tvv - Thh) / p : 0;
    let Sx = p > 0 ? Tvh / p : 0;
    let total = fan.reduce((a, b) => a + b, 0);
    fan = fan.map(x => total > 0 ? x / total * N_BINS / Math.PI : 0); // a density on [0, pi)
    return { p, C, S: Sx, fan };
}

async function update_graph() {
    frame += 1;
    if (frame % GRAPH_EVERY !== 0) { return; }
    let st = contact_stress();
    if (!(st.p > 0)) { return; }
    let size = Math.hypot(st.C, st.S);
    let beta = 0.5 * Math.atan2(st.S, st.C); // direction of sigma_1 from vertical

    trail_C.push(st.C); trail_S.push(st.S);
    if (trail_C.length > TRAIL) { trail_C.shift(); trail_S.shift(); }
    settled.push(size);
    if (settled.length > TRAIL) { settled.shift(); }
    let rim = settled.reduce((a, b) => a + b, 0) / settled.length;

    // (a) the fan, doubled round the full circle as a direction has two ends
    let theta = [], r = [];
    for (let k = 0; k < 2 * N_BINS + 1; k++) {
        theta.push((k + 0.5) * 180 / N_BINS);
        r.push(st.fan[k % N_BINS]);
    }
    let rbar = 1 / Math.PI;
    Plotly.update('stats', { 'theta': [theta], 'r': [r] }, {}, [0]);
    let bdeg = beta * 180 / Math.PI;
    Plotly.update('stats', {
        'theta': [[bdeg, bdeg + 180]],
        'r': [[rbar * (1 + 2 * size), rbar * (1 + 2 * size)]],
    }, {}, [1]);

    // (b) the disc
    Plotly.update('stats', { 'x': [trail_S], 'y': [trail_C] }, {}, [2]);
    Plotly.update('stats', { 'x': [[st.S]], 'y': [[st.C]] }, {}, [3]);
    let ring = circle(rim);
    Plotly.update('stats', { 'x': [ring.x], 'y': [ring.y] }, {}, [4]);

    // (c) the size of the tilt against the macroscopic friction
    let t = S.simu_getTime();
    Plotly.extendTraces('stats', {
        'x': [[t], [t]],
        'y': [[size], [Math.abs(params.current_shearstress / params.current_pressure)]],
    }, [6, 7], 2000);

    Plotly.relayout('stats', {
        'annotations[0].text': 'q/p = ' + size.toFixed(3) + ',  β = ' + bdeg.toFixed(0) + '°,  ' +
            'settling at ' + rim.toFixed(3) + ' (φ ≈ ' + (Math.asin(Math.min(rim, 1)) * 180 / Math.PI).toFixed(1) + '°)',
    });
}

function circle(radius) {
    let x = [], y = [];
    for (let k = 0; k <= 120; k++) {
        let a = 2 * Math.PI * k / 120;
        x.push(radius * Math.cos(a));
        y.push(radius * Math.sin(a));
    }
    return { x, y };
}

function make_graph() {
    let ink = '#dddddd';
    let faint = '#666666';
    let tilt = '#ff9800';
    let unit = circle(1);
    let data = [
        // (a) the fan
        { type: 'barpolar', theta: [], r: [], marker: { color: '#4fc3f7', opacity: 0.8 },
          subplot: 'polar', name: 'fan', hoverinfo: 'skip', showlegend: false },
        { type: 'scatterpolar', mode: 'lines', theta: [], r: [], line: { color: tilt, width: 4 },
          subplot: 'polar', name: 'σ₁ direction' },
        // (b) the disc: trail, now, where it settles, and q/p = 1
        { type: 'scatter', mode: 'lines', x: [], y: [], line: { color: faint, width: 1 },
          xaxis: 'x', yaxis: 'y', name: 'recent', hoverinfo: 'skip', showlegend: false },
        { type: 'scatter', mode: 'markers', x: [], y: [], marker: { color: tilt, size: 12 },
          xaxis: 'x', yaxis: 'y', name: 'now' },
        { type: 'scatter', mode: 'lines', x: [], y: [], line: { color: tilt, width: 1, dash: 'dash' },
          xaxis: 'x', yaxis: 'y', name: 'settles at', hoverinfo: 'skip' },
        { type: 'scatter', mode: 'lines', x: unit.x, y: unit.y, line: { color: faint, width: 1, dash: 'dot' },
          xaxis: 'x', yaxis: 'y', name: 'q/p = 1', hoverinfo: 'skip', showlegend: false },
        // (c) time series
        { type: 'scatter', mode: 'lines', x: [], y: [], line: { color: tilt, width: 2 },
          xaxis: 'x2', yaxis: 'y2', name: 'size of the tilt, q/p' },
        { type: 'scatter', mode: 'lines', x: [], y: [], line: { color: '#4fc3f7', width: 1 },
          xaxis: 'x2', yaxis: 'y2', name: 'τ/σ on the shear plane' },
    ];
    let axis = { color: ink, gridcolor: '#333333', zerolinecolor: faint };
    let layout = {
        paper_bgcolor: '#111111',
        plot_bgcolor: '#111111',
        font: { color: ink, family: 'Montserrat' },
        showlegend: true,
        legend: { x: 0.08, y: 0.0, orientation: 'h', bgcolor: 'rgba(0,0,0,0)' },
        margin: { l: 50, r: 20, t: 30, b: 60 },
        polar: {
            domain: { x: [0.02, 0.44], y: [0.5, 0.95] },
            bgcolor: '#111111',
            angularaxis: { rotation: 90, direction: 'clockwise', color: ink, gridcolor: '#333333' },
            radialaxis: { color: faint, gridcolor: '#333333', showticklabels: false },
        },
        xaxis: Object.assign({ domain: [0.56, 0.98], range: [-1.05, 1.05], title: { text: 'S (sideways part)' }, anchor: 'y' }, axis),
        yaxis: Object.assign({ domain: [0.5, 0.95], range: [-1.05, 1.05], title: { text: 'C (vertical part)' },
                               scaleanchor: 'x', anchor: 'x' }, axis),
        xaxis2: Object.assign({ domain: [0.08, 0.98], title: { text: 'time (s)' }, anchor: 'y2' }, axis),
        yaxis2: Object.assign({ domain: [0.12, 0.36], range: [0, 0.8], title: { text: 'q/p' }, anchor: 'x2' }, axis),
        annotations: [{
            text: '', xref: 'paper', yref: 'paper', x: 0.5, y: 0.43, showarrow: false,
            font: { size: 14, color: tilt },
        }, {
            text: 'The fan: contact directions, weighted by force', xref: 'paper', yref: 'paper',
            x: 0.23, y: 1.0, showarrow: false,
        }, {
            text: 'The tilt: (S, C)', xref: 'paper', yref: 'paper', x: 0.77, y: 1.0, showarrow: false,
        }],
    };
    Plotly.newPlot('stats', data, layout, { displayModeBar: false, responsive: true });
}

function linspace(start, stop, num) {
    const step = (stop - start) / (num - 1);
    const result = [];

    for (let i = 0; i < num; i++) {
        const value = start + step * i;
        result.push(value);
    }

    return result
}

function edge_to_center(edge) {
    let result = [];
    for (let j = 0; j < edge.length - 1; j++) {
        const value = (edge[j] + edge[j + 1]) / 2.
        result.push(value);
    }
    return result
}

