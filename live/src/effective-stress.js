import css from "../css/main.css";
import Plotly from "plotly.js-dist";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
import * as CGHANDLER from '../libs/CGHandler.js';

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let water_mesh;
let gui;
let S;

var params = {
    dimension: 2,
    L0: 0.12, //system size
    L1: 0.06,
    N: 450,
    zoom: 800,
    // packing_fraction: 0.5,
    constant_volume: true,
    axial_strain: 0,
    volumetric_strain: 0,
    paused: false,
    g_mag: 9.81,
    theta: 0, // slope angle in DEGREES
    d4: { cur: 0 },
    r_max: 0.0045,
    r_min: 0.0035,
    particle_density: 2000,
    freq: 0.05,
    new_line: false,
    shear_rate: 10,
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.2,
    cg_window_size: 5,
    particle_opacity: 0.5,
    F_mag_max: 150,
    aspect_ratio: 1,
    nogui: false,
    water_density: 1000,
    graph_fraction: 0.5,

}

params.water_table = params.L0; // start with water table in the middle
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

    params.N = 300
    params.particle_volume = Math.PI * Math.PI * Math.pow(params.average_radius, 4) / 2.;
}

params.particle_mass = params.particle_volume * params.particle_density;

if (urlParams.has('cg_width')) { params.cg_width = parseInt(urlParams.get('cg_width')); }
if (urlParams.has('cg_height')) { params.cg_height = parseInt(urlParams.get('cg_height')); }
if (urlParams.has('cg_opacity')) { params.cg_opacity = parseFloat(urlParams.get('cg_opacity')); }
if (urlParams.has('quality')) { params.quality = parseInt(urlParams.get('quality')); }

if (urlParams.has('particle_opacity')) { params.particle_opacity = parseFloat(urlParams.get('particle_opacity')); }
if (urlParams.has('zoom')) {
    params.zoom = parseFloat(urlParams.get('zoom'));
}
if (urlParams.has('L0')) {
    params.L0 = parseFloat(urlParams.get('L0'));
}
if (urlParams.has('L1')) {
    params.L1 = parseFloat(urlParams.get('L1'));
}
if (urlParams.has('N')) { params.N = parseFloat(urlParams.get('N')); }
if (urlParams.has('nogui')) { params.nogui = true; }
if (!urlParams.has('noinfo')) {
    let info_div = document.createElement("div")
    info_div.id = 'info_div';
    info_div.innerHTML = "Click on a particle to grab it"
    document.body.appendChild(info_div);
}

params.boundary = 'OffsetRectangleD0';


SPHERES.createNDParticleShader(params).then(init);

async function init() {

    await NDDEMCGPhysics();

    // camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.1, 1000 );
    var aspect = window.innerWidth / 2. / window.innerHeight;
    camera = new THREE.OrthographicCamera(
        (-100 * aspect) / params.zoom,
        (100 * aspect) / params.zoom,
        100 / params.zoom,
        -100 / params.zoom,
        -10,
        10
    );
    camera.position.set(params.L0, 0, -5 * params.L0);
    camera.up.set(0, 0, 1);
    camera.lookAt(params.L0, 0, 0);
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

    WALLS.add_back(params, scene);
    WALLS.add_left(params, scene);
    WALLS.add_right(params, scene);
    WALLS.add_front(params, scene);
    WALLS.back.scale.y = params.thickness;//Math.PI/2.;
    var vert_walls = [WALLS.left, WALLS.right, WALLS.back, WALLS.front];

    vert_walls.forEach(function (mesh) {
        mesh.scale.x = 2 * params.L0 + 2 * params.thickness;
        mesh.scale.z = 2 * params.L1 + 2 * params.thickness;
    });

    CGHANDLER.add_cg_mesh(2 * params.L0, 2 * params.L1, scene);
    CGHANDLER.cg_mesh.position.x = params.L0;

    let water_geometry = new THREE.PlaneGeometry(1, 1);
    let water_material = new THREE.MeshBasicMaterial({ color: 0x0000ff, opacity: 0.5, transparent: true, side: THREE.DoubleSide });
    water_mesh = new THREE.Mesh(water_geometry, water_material);
    update_water_table();
    scene.add(water_mesh);

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth / 2., window.innerHeight);
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

    var container = document.getElementById('canvas');
    container.appendChild(renderer.domElement);

    if (!params.nogui) {
        gui = new GUI();
        gui.width = 320;

        if (params.dimension == 4) {
            gui.add(params.d4, 'cur', -params.L3, params.L3, 0.001)
                .name('D4 location').listen()
                .onChange(function () {
                    if (urlParams.has('stl')) {
                        meshes = renderSTL(meshes, NDsolids, scene, material, params.d4.cur);
                    }
                });
        }
        gui.add(params, 'particle_opacity', 0, 1).name('Particle opacity').listen().onChange(() => SPHERES.update_particle_material(params,

        ));
        gui.add(params, 'cg_opacity', 0, 1).name('Coarse grain opacity').listen();
        // gui.add(params, 'cg_field', ['Density', 'Velocity', 'Total Pressure', 'Effective Pressure', 'Hydrostatic Pressure', 'DEM Pressure', 'Shear stress']).name('Field').listen();
        gui.add(params, 'cg_field', ['Density', 'Velocity', 'Vertical Stress', 'Horizontal Stress', 'Pressure', 'Shear stress']).name('Field').listen();
        // gui.add(params, 'cg_window_size', 0.5, 6).name('Window size (radii)').listen().onChange(() => {
        //     update_cg_params(S, params);
        // });
        // gui.add ( params, 'cg_width', 1, 100,1).name('Resolution').listen().onChange( () => {
        // params.cg_height = params.cg_width;
        // update_cg_params(S, params);
        // });
        // gui.add ( params, 'paused').name('Paused').listen();

        // const controls = new OrbitControls( camera, renderer.domElement );
        // controls.update();

        // let colorbar = container.getE('div');
        // colorbar.id = 'ttt';
        gui.add(params, 'water_table', 0, 2 * params.L0).name('Water table (m)').listen().onChange(() => {
            update_water_table();
        });
        gui.add(params, 'water_density', 0, 1500).name('Water density (kg/m<sup>3</sup>/m)').listen();
    }

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
    RAYCAST.update_world(S, camera, params);

    make_graph();

    animate();
}

function onWindowResize() {

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / 2. / window.innerHeight;
    // camera.left = -params.zoom * aspect;
    // camera.right = params.zoom * aspect;
    // camera.bottom = -params.zoom;
    // camera.top = params.zoom;

    renderer.setSize(window.innerWidth / 2., window.innerHeight);

    // Plotly.relayout('stats', update_graph);
}

function update_water_table() {
    let height = params.water_table + 2 * params.thickness;
    let width = 2 * params.L1 + 2 * params.thickness;
    water_mesh.scale.set(height, width, 1);
    water_mesh.position.set(height / 2., 0, 1);
}

function update_masses() {
    let radii = S.simu_getRadii();
    let x = S.simu_getX();
    let buoyant_density = params.particle_density - params.water_density;

    for (let i = 0; i < params.N; i++) {
        let r = radii[i];
        let vol;
        if (params.dimension === 2) {
            vol = Math.PI * r * r;
        } else {
            vol = 4 / 3 * Math.PI * r * r * r;
        }
        // console.log(x[i])
        if (x[i][0] < params.water_table) {
            S.simu_setMass(i, vol * buoyant_density);
        }
        else {
            S.simu_setMass(i, vol * params.particle_density);
        }
    }
}

function animate() {
    requestAnimationFrame(animate);
    SPHERES.move_spheres(S, params);
    // RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    if (!params.paused) {
        update_masses();
        S.simu_step_forward(15);
        CGHANDLER.update_2d_cg_field(S, params);
    }
    update_graph();
    SPHERES.draw_force_network(S, params, scene);
    renderer.render(scene, camera);
}


function update_cg_params(S, params) {
    var cgparam = {};
    cgparam["file"] = [{ "filename": "none", "content": "particles", "format": "interactive", "number": 1 }];
    // cgparam["density"] = params.particle_density;
    cgparam["boxes"] = [params.cg_width, params.cg_height];
    cgparam["boundaries"] = [
        [0, -params.L1],
        [2 * params.L0, params.L1]];
    cgparam["window size"] = params.cg_window_size * params.average_radius;
    cgparam["skip"] = 0;
    cgparam["max time"] = 1;
    cgparam["time average"] = "None";
    cgparam["fields"] = ["RHO", "VAVG", "TC", "Pressure", "Shear stress"];
    cgparam["periodicity"] = [false, true];
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
        let m;
        if (params.dimension === 2) {
            m = Math.PI * 0.5 * 0.5 * (params.particle_density - params.water_density);
        } else {
            m = 4. / 3. * Math.PI * 0.5 * 0.5 * 0.5 * params.particle_density;
        }

        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform " + params.r_min + " " + params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");
        // S.simu_interpret_command("auto skin");

        S.simu_interpret_command("boundary 0 WALL 0 " + String(2 * params.L0));
        S.simu_interpret_command("boundary 1 PBC -" + String(params.L1) + " " + String(params.L1));
        if (params.dimension > 2) {
            S.simu_interpret_command("boundary 2 WALL -" + String(params.r_max) + " " + String(params.r_max));
        }
        if (params.dimension > 3) {
            S.simu_interpret_command("boundary 3 WALL -" + String(params.L3) + " " + String(params.L3));
        }
        S.simu_interpret_command("gravity -" + String(params.g_mag) + " " + "0 ".repeat(params.dimension - 2));

        S.simu_interpret_command("auto location randomdrop");

        let tc = 1e-3;
        let rest = 0.5;
        let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient(tc, rest, params.particle_mass)

        S.simu_interpret_command("set Kn " + String(vals.stiffness));
        S.simu_interpret_command("set Kt " + String(0.8 * vals.stiffness));
        S.simu_interpret_command("set GammaN " + String(vals.dissipation));
        S.simu_interpret_command("set GammaT " + String(vals.dissipation));
        S.simu_interpret_command("set Mu 0.5");
        S.simu_interpret_command("set damping 0.001");
        S.simu_interpret_command("set T 150");
        S.simu_interpret_command("set dt " + String(tc / 20));
        S.simu_interpret_command("set tdump 1000000"); // how often to calculate wall forces
        S.simu_interpret_command("auto skin");
        S.simu_finalise_init();

        update_cg_params(S, params);

    }
}

function update_graph() {
    var grid = S.cg_get_gridinfo();
    let xmin = grid[0];
    let dx = grid[3];
    // let dy = grid[4];
    let nx = grid[6];
    let ny = grid[7];

    // pressure = S.cg_get_result(0, "Pressure", 0);
    let DEM_stress = S.cg_get_result(0, "TC", 0);
    let density = S.cg_get_result(0, "RHO", 0);

    // take an average along all same x values
    let avgDensity = new Array(nx).fill(0);
    let DEMPressure = new Array(nx).fill(0);
    let hydraulicPressure = new Array(nx).fill(0);
    let totalPressure = new Array(nx).fill(0);
    let xloc = new Array(nx).fill(0);
    let solid_fraction = new Array(nx).fill(0);
    let porosity = new Array(nx).fill(0);
    let effectivePressure = new Array(nx).fill(0);

    // first pass to get density profile
    for (let i = 0; i < nx; i++) {
        for (let j = 0; j < ny; j++) {
            let index = i + j * nx;
            // console.log(density[index], avgDensity[i])
            if (isNaN(density[index])) {
                avgDensity[i] += 0;
            }
            else {
                avgDensity[i] += density[index];
            }
        }
        avgDensity[i] /= ny;
        solid_fraction[i] = avgDensity[i] / params.particle_density;
        // console.log(solid_fraction[i])
        porosity[i] = 1 - solid_fraction[i];
    }

    // now calculate total stress, water pressure and effective stress from density profile
    let thisWeight;
    for (let i = nx - 1; i >= 0; i--) {
        let x = xmin + (i + 0.5) * dx;

        if (x < params.water_table) { // below water table
            thisWeight = (solid_fraction[i] * params.particle_density + porosity[i] * params.water_density) * params.g_mag * dx;
            hydraulicPressure[i] = params.water_density * params.g_mag * (params.water_table - x);
        }
        else {
            thisWeight = solid_fraction[i] * params.particle_density * params.g_mag * dx;
            hydraulicPressure[i] = 0; // above water table
        }
        totalPressure[i - 1] = totalPressure[i] + thisWeight;
        effectivePressure[i] = totalPressure[i] - hydraulicPressure[i];
    }

    for (let i = 0; i < nx; i++) {
        let x = xmin + (i + 0.5) * dx;
        xloc[i] = x;

        for (let j = 0; j < ny; j++) {
            let index = i + j * nx;

            DEMPressure[i] += DEM_stress[index];
            // console.log(DEM_stress);
        }
        DEMPressure[i] /= ny * 2; // HACK: FACTOR OF TWO FOR MYSTERIOIUS REASONS
    }


    var maxPressure = totalPressure.reduce(function (a, b) { return Math.max(Math.abs(a), Math.abs(b)) }, 0);

    Plotly.update('stats', {
        'y': [xloc],
        'x': [totalPressure],
    }, {}, [0])

    Plotly.update('stats', {
        'y': [xloc],
        'x': [effectivePressure],
    }, {}, [1])

    Plotly.update('stats', {
        'y': [xloc],
        'x': [hydraulicPressure],
    }, {}, [2])

    Plotly.update('stats', {
        'y': [xloc],
        'x': [DEMPressure],
    }, {}, [3])

    var update = {
        xaxis: {
            range: [0, maxPressure],
            title: 'Pressure (Pa)',
        },
    }
    Plotly.relayout('stats', update);
}

function make_graph() {
    let { data, layout } = LAYOUT.plotly_stress_graph('Stress (Pa)', 'Height (m)', ['Total Vertical Stress (Pa)', 'Effective Vertical Stress (Pa)', 'Pore water pressure (Pa)', 'DEM Vertical Stress (Pa)']);
    var maxStress = 4 * params.L0 * params.particle_density * params.g_mag * 0.6;
    // layout.xaxis2 = {
    //     range: [0, maxStress],
    //     title: 'Stress (Pa)',
    //     overlaying: 'x',
    //     side: 'top',
    //     rangemode: 'tozero',
    //     color: 'blue'
    // }
    Plotly.newPlot('stats', data, layout);
}