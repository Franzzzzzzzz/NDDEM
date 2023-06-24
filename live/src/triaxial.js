import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';


var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel;
let S;
let pressure = 0;
let shear = 0;
let density = 0;
// let pressure_time = [];
// let shear_time = [];
// let density_time = [];
// var radius = 0.5;
let started = false;
let stressTcxx, stressTcyy, stressTczz, stressTcxy;
let show_stats = true;
// const thickness = radius;
let old_time = 0;
let new_time = 0;

let NDsolids, material, STLFilename;
let meshes = new THREE.Group();

// const raycaster = new THREE.Raycaster();
// const mouse = new THREE.Vector2();
// let intersection_plane = new THREE.Plane();
// let camera_direction = new THREE.Vector3();

// let INTERSECTED = null;
// let last_intersection = null;
// let locked_particle = null;
// let ref_location;

let graph_fraction = 0.5;
document.getElementById("stats").style.width = String(100 * graph_fraction) + '%';
document.getElementById("canvas").style.width = String(100 * (1 - graph_fraction)) + '%';

var params = {
    dimension: 3,
    L: 0.03, //system size
    aspect_ratio: 1,
    N: 520,
    // packing_fraction: 0.5,
    constant_volume: true,
    consolidate_active: false,
    volumetric_strain: 0,
    axial_strain: 0,
    shear_active: false,
    gravity: false,
    paused: false,
    H_cur: 0,
    W_cur: 0,
    pressure_set_pt: 1000,
    deviatoric_set_pt: 0,
    current_pressure: 0,
    current_shear: 0,
    current_density: 0,
    d4: { cur: 0 },
    r_max: 0.0033,
    r_min: 0.0027,
    freq: 0.05,
    new_line: false,
    clear_line: false,
    loading_rate: 0.001,
    max_volumetric_strain: 0.2,
    max_axial_strain: 0.2,
    lut: 'None',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    particle_density: 2700,
    particle_opacity: 1.0,
}


params.loading_method = 'strain_controlled';
if (urlParams.has('stress_controlled')) {
    params.loading_method = 'stress_controlled';
}

params.average_radius = (params.r_min + params.r_max) / 2.;
params.thickness = params.average_radius;
params.initial_pressure_set_pt = params.pressure_set_pt; // store initial value to reinitialise later
params.particle_volume = 4. / 3. * Math.PI * Math.pow(params.average_radius, 3);
params.particle_mass = params.particle_volume * params.particle_density;
if (urlParams.has('dimension')) {
    params.dimension = parseInt(urlParams.get('dimension'));
}
if (params.dimension === 4) {
    params.L = 2.5;
    params.N = 300
    params.particle_volume = Math.PI * Math.PI * Math.pow(params.average_radius, 4) / 2.;
}
if (urlParams.has('no_stats')) {
    show_stats = false;
}
if (urlParams.has('quality')) { params.quality = parseInt(urlParams.get('quality')); }


params.L_cur = params.L;
params.packing_fraction = params.N * params.particle_volume / Math.pow(2 * params.L, 2) / (params.L * params.aspect_ratio);
params.back = -params.L;
params.front = params.L;
params.left = -params.L;
params.right = params.L;
params.floor = -params.L * params.aspect_ratio;
params.roof = params.L * params.aspect_ratio;

SPHERES.createNDParticleShader(params).then( init );

async function init() {

    await NDDEMPhysics();

    camera = new THREE.PerspectiveCamera(50, window.innerWidth * (1 - graph_fraction) / window.innerHeight, 1e-5, 1000);
    camera.position.set(3 * params.L, 3 * params.L, 1.5 * params.L * params.aspect_ratio);
    camera.up.set(0, 0, 1);
    camera.lookAt(0, 0, 0);

    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x111);

    // const axesHelper = new THREE.AxesHelper( 5 );
    // scene.add( axesHelper );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add(hemiLight);

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set(5, 5, 5);
    dirLight.castShadow = true;
    dirLight.shadow.camera.zoom = 2;
    scene.add(dirLight);

    WALLS.add_cuboid_walls(params, scene);

    SPHERES.add_spheres(S, params, scene);

    if (urlParams.has('stl')) {
        STLFilename = './stls/4d-pool.stl';
        material = new THREE.MeshPhongMaterial({ color: 0x00aa00 });
        material.side = THREE.DoubleSide;
        loadSTL();
    }


    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth * (1 - graph_fraction), window.innerHeight);
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

    var container = document.getElementById('canvas');
    container.appendChild(renderer.domElement);

    let gui = new GUI();
    gui.width = 350;

    gui.add(params, 'loading_rate', 0.0001, 0.01, 0.0001).name('Axial loading rate (mm/s)');
    gui.add(params, 'constant_volume')
        .name('Constant volume shearing').listen()
    // .onChange( function() { WALLS.update_walls_from_PID(params, S); });
    // var lut_folder = gui.addFolder('Colour by');
    gui.add(params, 'particle_opacity', 0, 1).name('Particle opacity').listen().onChange(() => SPHERES.update_particle_material(params,
        // lut_folder
    ));
    gui.add(params, 'lut', ['None', 'Velocity', 'Rotation Rate']).name('Colour by')
        .onChange(() => SPHERES.update_particle_material(params,
            // lut_folder
        ));
    gui.add(params, 'gravity').name('Gravity').listen()
        .onChange(function () {
            if (params.gravity === true) {
                S.simu_interpret_command("gravity 0 0 -10 " + "0 ".repeat(params.dimension - 3))
            }
            else {
                S.simu_interpret_command("gravity 0 0 0 " + "0 ".repeat(params.dimension - 3))
            }
        });
    gui.add(params, 'paused').name('Paused (Enable to rotate graph)').listen();
    gui.add(params, 'clear_line').name('Clear current line').listen()
        .onChange(() => {
            Plotly.deleteTraces('stats', [-1])
            var data = [{
                type: 'scatter3d',
                mode: 'lines',
                x: [], y: [], z: [],
                line: { width: 5 },
                name: 'Load path ' + String(document.getElementById('stats').data.length + 1)
            }]
            Plotly.addTraces('stats', data);
            params.clear_line = false;
        });
    gui.add(params, 'pressure_set_pt', 0, 1e4, 1)
        .name('Consolidation pressure (Pa)').listen();
    let consol_button = gui.add(params, 'consolidate_active').name('Perform consolidation').listen();//.onChange(() => {
    // let shear_button = gui.add( params, 'shear_active').name('Perform shearing').onChange( () => {
    // shear_button.remove()
    // });
    // consol_button.remove();
    // });
    let shear_button = gui.add(params, 'shear_active').name('Perform shearing').listen().onChange(() => {
        params.consolidate_active = false;
        params.V_const = params.aspect_ratio * (params.L_cur - params.W_cur) * (params.L_cur - params.W_cur) * (params.L_cur - params.H_cur);
    });
    gui.add(params, 'new_line').name('New loading path').listen()
        .onChange(() => {
            params.axial_strain = 0;
            params.volumetric_strain = 0;
            // params.pressure_set_pt = params.initial_pressure_set_pt;
            // params.deviatoric_set_pt = 0;
            params.L_cur = params.L;
            params.W_cur = 0;
            params.H_cur = 0;
            params.shear_active = false;
            params.consolidate_active = false;
            // WALLS.update_walls_from_PID(params, S);
            var data = [{
                type: 'scatter3d',
                mode: 'lines',
                x: [], y: [], z: [],
                line: { width: 5 },
                name: 'Load path ' + String(document.getElementById('stats').data.length + 1)
            }]
            Plotly.addTraces('stats', data);
            params.new_line = false;
        });
    const controls = new OrbitControls(camera, container);
    // controls.target.y = 0.5;
    controls.update();

    window.addEventListener('resize', onWindowResize, false);
    // container.addEventListener( 'mousemove', onMouseMove, false );
    // window.addEventListener( 'keypress', onSelectParticle, false );

    if (show_stats) { make_graph(); }

    WALLS.update_triaxial_walls(params, S);
    animate();
}

function onMouseMove(event) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components
    mouse.x = ((event.clientX - window.innerWidth * graph_fraction) / window.innerWidth * (1 - graph_fraction)) * 8 - 1;
    mouse.y = - (event.clientY / window.innerHeight) * 2 + 1;

}

// function onSelectParticle( event ) {
//     if ( event.code === 'KeyW' ) {
//         if ( params.loading_method === 'strain_controlled' && params.volumetric_strain < params.max_volumetric_strain) { params.volumetric_strain += params.loading_rate; }
//         else { params.pressure_set_pt *= 1 + params.loading_rate; }
//         WALLS.update_walls(params, S);
//         }
//     if ( event.code === 'KeyS' ) {
//         if ( params.loading_method === 'strain_controlled' && params.volumetric_strain > 0 ) { params.volumetric_strain -= params.loading_rate; }
//         else { params.pressure_set_pt /= 1 + params.loading_rate; }
//         WALLS.update_walls(params, S);
//         }
//     if ( event.code === 'KeyD' ) {
//         if ( params.loading_method === 'strain_controlled' && params.axial_strain < params.max_axial_strain ) { params.axial_strain += params.loading_rate; }
//         // else if ( params.deviatoric_set_pt === 0 ) { params.deviatoric_set_pt = 1; }
//         else { params.deviatoric_set_pt += params.loading_rate*params.pressure_set_pt; }
//         WALLS.update_walls(params, S);
//         }
//     if ( event.code === 'KeyA' && params.axial_strain > -params.max_axial_strain ) {
//         if ( params.loading_method === 'strain_controlled' ) { params.axial_strain -= params.loading_rate; }
//         // else if ( params.deviatoric_set_pt === 0 ) { params.deviatoric_set_pt = -1; }
//         else { params.deviatoric_set_pt -= params.loading_rate*params.pressure_set_pt; }
//         WALLS.update_walls(params, S);
//         }
// }


function onWindowResize() {

    camera.aspect = window.innerWidth * (1 - graph_fraction) / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize(window.innerWidth * (1 - graph_fraction), window.innerHeight);

    var update = {
        width: window.innerWidth * graph_fraction,
        height: window.innerHeight,
    };
    Plotly.relayout('stats', update);

}

function animate() {
    if (clock.getElapsedTime() > 2) { started = true; }
    requestAnimationFrame(animate);
    SPHERES.move_spheres(S, params);

    if (!params.paused) {
        new_time = clock.getElapsedTime()

        if (show_stats && started) {
            update_graph();
            SPHERES.draw_force_network(S, params, scene);
            WALLS.update_triaxial_walls(params, S, new_time - old_time); // TODO: MAKE THIS WORK BETTER WHEN THE SYSTEM IS PAUSED i.e. DIFFERENCE BETWEEN WALLTIME AND REAL TIME
        }

        S.simu_step_forward(5);
        S.cg_param_read_timestep(0);
        S.cg_process_timestep(0, false);
        var grid = S.cg_get_gridinfo();
        density = S.cg_get_result(0, "RHO", 0)[0];
        // vavg=S.cg_get_result(0, "VAVG", 1)[0] ;
        params.stressTcxx = S.cg_get_result(0, "TC", 0)[0];
        params.stressTcyy = S.cg_get_result(0, "TC", 4)[0];
        params.stressTczz = S.cg_get_result(0, "TC", 8)[0];
        params.stressTcxy = S.cg_get_result(0, "TC", 1)[0];

        params.current_pressure = (params.stressTcxx +
            params.stressTcyy +
            params.stressTczz) / 3.;
        // params.current_shear = -params.stressTcxy;
        params.current_shear = params.stressTczz - params.stressTcxx;
    }

    renderer.render(scene, camera);

    if (stats !== undefined) { stats.update(); }

    old_time = new_time;

}
// function refresh_simulation() {
//     // update_L();
//     // console.log(L)
//     var range = 0.8*(L/2 - radius);
//     for ( let i = 0; i < params.N; i ++ ) {
//         position.set( range*(2*Math.random() - 1), range*(2*Math.random() - 1), range*(2*Math.random() - 1) );
// 		physics.setMeshPosition( spheres, position, i );
//     }
//     position.set( 0, - L/2 - thickness/2., 0 );
//
//     floor.position.y = - L/2 - thickness/2.;
//     roof.position.y = L/2 + thickness/2.;
//     left.position.z = - L/2 - thickness/2.;
//     right.position.z = L/2 + thickness/2.;
//     back.position.x = L/2 + thickness/2.;
//     front.position.x = -L/2 - thickness/2.;
//     camera.position.set( - 3*L, 0, 0 );
//
// }

async function NDDEMPhysics() {

    await DEMCGND().then((NDDEMCGLib) => {
        if (params.dimension == 3) {
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
        // now need to find the mass of a particle with diameter 1
        let m = 4. / 3. * Math.PI * 0.5 * 0.5 * 0.5 * params.particle_density;
        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform " + params.r_min + " " + params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");
        S.simu_interpret_command("auto skin");
        // var wall_type = 'PBC';
        var wall_type = 'WALL';

        S.simu_interpret_command("boundary 0 " + wall_type + " -" + String(params.L) + " " + String(params.L));
        S.simu_interpret_command("boundary 1 " + wall_type + " -" + String(params.L) + " " + String(params.L));
        S.simu_interpret_command("boundary 2 " + wall_type + " -" + String(params.L * params.aspect_ratio) + " " + String(params.L * params.aspect_ratio));
        if (params.dimension == 4) {
            S.simu_interpret_command("boundary 3 " + wall_type + " -" + String(params.L) + " " + String(params.L));
        }
        if (params.dimension == 3) {
            if (params.gravity) {
                S.simu_interpret_command("gravity 0 0 -10");
            } else {
                S.simu_interpret_command("gravity 0 0 0");
            }
        }
        if (params.dimension == 4) {
            S.simu_interpret_command("boundary 3 " + wall_type + " -" + String(params.L) + " " + String(params.L));
            S.simu_interpret_command("gravity 0 0 0 -10")
        }

        // S.simu_interpret_command("auto location randomsquare");
        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-3;
        let rest = 0.2; // super low restitution coeff to dampen out quickly
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient(tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8 * vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 1.0");
        S.simu_interpret_command("set Mu_wall 0");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set damping 0.1"); // NOTE: ARTIFICAL DAMPING!!!
        S.simu_interpret_command("set dt " + String(tc / 20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        S.simu_finalise_init();

        var cgparam = {};
        cgparam["file"] = [{ "filename": "none", "content": "particles", "format": "interactive", "number": 1 }];
        cgparam["boxes"] = Array(params.dimension).fill(1);
        // cgparam["boundaries"]=[
        //     [-params.L+params.r_max,-params.L+params.r_max,-params.L*params.aspect_ratio+params.r_max],
        //     [ params.L-params.r_max, params.L-params.r_max, params.L*params.aspect_ratio-params.r_max]] ;
        cgparam["boundaries"] = [
            Array(params.dimension).fill(-params.L + params.r_max),
            Array(params.dimension).fill(params.L - params.r_max)];
        cgparam["boundaries"][0][2] = -params.L * params.aspect_ratio + params.r_max;
        cgparam["boundaries"][1][2] = params.L * params.aspect_ratio - params.r_max;
        cgparam["window size"] = 2 * params.average_radius;
        cgparam["skip"] = 0;
        cgparam["max time"] = 1;
        cgparam["time average"] = "None";
        cgparam["fields"] = ["RHO", "VAVG", "TC","Pressure","ShearStress"];
        cgparam["periodicity"] = Array(params.dimension).fill(true);
        cgparam["window"] = "Lucy3D";
        cgparam["dimension"] = params.dimension;


        console.log(JSON.stringify(cgparam));
        S.cg_param_from_json_string(JSON.stringify(cgparam));
        S.cg_setup_CG();
    }
}

function update_graph() {
    params.current_density = params.packing_fraction * params.particle_density;

    Plotly.extendTraces('stats', {
        'x': [[params.current_pressure]],
        'y': [[params.current_density]],
        'z': [[params.current_shear]],
    }, [-1])
}

function make_graph() {
    let { data, layout } = LAYOUT.plotly_graph('Pressure (Pa)', 'Density (kg/m<sup>3</sup>)', 'Deviatoric stress (Pa)');
    Plotly.newPlot('stats', data, layout);
}
