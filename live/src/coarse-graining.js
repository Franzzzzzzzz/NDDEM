import css from "../css/main.css";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
import * as RAYCAST from '../libs/RaycastHandler.js';
import * as CGHANDLER from '../libs/CGHandler.js';

let info_div = document.createElement("div")
info_div.id = 'info_div';
info_div.innerHTML = "Click on a particle to grab it"
document.body.appendChild(info_div);

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let gui;
let S;

var params = {
    dimension: 2,
    L: 0.06, //system size
    N: 150,
    zoom: 1000,
    // packing_fraction: 0.5,
    constant_volume: true,
    axial_strain: 0,
    volumetric_strain: 0,
    paused: false,
    g_mag: 1e3,
    theta: 0, // slope angle in DEGREES
    d4: { cur: 0 },
    r_max: 0.0045,
    r_min: 0.0035,
    particle_density: 1,
    freq: 0.05,
    new_line: false,
    shear_rate: 10,
    lut: 'White',
    cg_field: 'Density',
    quality: 5,
    cg_width: 50,
    cg_height: 50,
    cg_opacity: 0.8,
    cg_window_size: 3,
    particle_opacity: 0.5,
    F_mag_max: 0.005,
    aspect_ratio: 1,
}


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
    var aspect = window.innerWidth / window.innerHeight;
    camera = new THREE.OrthographicCamera(
        (-100 * aspect) / params.zoom,
        (100 * aspect) / params.zoom,
        100 / params.zoom,
        -100 / params.zoom,
        -1000,
        1000
    );
    camera.position.set(0, 0, -5 * params.L);
    camera.up.set(0, 0, 1);
    camera.lookAt(0, 0, 0);
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
        mesh.scale.x = 2 * params.L + 2 * params.thickness;
        mesh.scale.z = 2 * params.L + 2 * params.thickness;
    });

    CGHANDLER.add_cg_mesh(2 * params.L, 2 * params.L, scene);

    // geometry = new THREE.PlaneGeometry( 2*params.L, 0.1*params.L );
    // material = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
    // colorbar_mesh = new THREE.Mesh( geometry, material );
    // colorbar_mesh.position.y = -1.1*params.L;
    // scene.add( colorbar_mesh );


    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

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
    // gui.add ( params, 'cg_width', 1, 100,1).name('Resolution').listen().onChange( () => {
    // params.cg_height = params.cg_width;
    // update_cg_params(S, params);
    // });
    // gui.add ( params, 'paused').name('Paused').listen();

    // const controls = new OrbitControls( camera, renderer.domElement );
    // controls.update();

    // let colorbar = container.getE('div');
    // colorbar.id = 'ttt';

    window.addEventListener('resize', onWindowResize, false);
    RAYCAST.update_world(S, camera, params);


    animate();
}

function onWindowResize() {

    // camera.aspect = window.innerWidth / window.innerHeight;
    // camera.updateProjectionMatrix();
    var aspect = window.innerWidth / window.innerHeight;
    // camera.left = -params.zoom * aspect;
    // camera.right = params.zoom * aspect;
    // camera.bottom = -params.zoom;
    // camera.top = params.zoom;

    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() {
    requestAnimationFrame(animate);
    SPHERES.move_spheres(S, params);
    // RAYCAST.animate_locked_particle(S, camera, SPHERES.spheres, params);
    if (!params.paused) {
        S.simu_step_forward(15);
        CGHANDLER.update_2d_cg_field(S, params);
    }
    SPHERES.draw_force_network(S, params, scene);
    renderer.render(scene, camera);
}
function update_cg_params(S, params) {
    var cgparam = {};
    cgparam["file"] = [{ "filename": "none", "content": "particles", "format": "interactive", "number": 1 }];
    cgparam["boxes"] = [params.cg_width, params.cg_height];
    // cgparam["boundaries"]=[[-params.L,-params.L,-params.L],[params.L,params.L,params.L]] ;
    cgparam["boundaries"] = [
        [-params.L, -params.L],
        [params.L, params.L]];
    cgparam["window size"] = params.cg_window_size * params.average_radius;
    cgparam["skip"] = 0;
    cgparam["max time"] = 1;
    cgparam["time average"] = "None";
    cgparam["fields"] = ["RHO", "VAVG", "TC", "Pressure", "Shear stress"];
    cgparam["periodicity"] = [false, false];
    cgparam["window"] = "Lucy2D";
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
            m = Math.PI * 0.5 * 0.5 * params.particle_density;
        } else {
            m = 4. / 3. * Math.PI * 0.5 * 0.5 * 0.5 * params.particle_density;
        }

        S.simu_interpret_command("mass -1 " + String(m));
        S.simu_interpret_command("auto rho");
        S.simu_interpret_command("auto radius uniform " + params.r_min + " " + params.r_max);
        S.simu_interpret_command("auto mass");
        S.simu_interpret_command("auto inertia");
        S.simu_interpret_command("auto skin");

        S.simu_interpret_command("boundary 0 WALL -" + String(params.L) + " " + String(params.L));
        S.simu_interpret_command("boundary 1 WALL -" + String(params.L) + " " + String(params.L));
        if (params.dimension > 2) {
            S.simu_interpret_command("boundary 2 WALL -" + String(params.r_max) + " " + String(params.r_max));
        }
        if (params.dimension > 3) {
            S.simu_interpret_command("boundary 3 WALL -" + String(params.L) + " " + String(params.L));
        }
        S.simu_interpret_command("gravity -1 " + "0 ".repeat(params.dimension - 2))

        S.simu_interpret_command("auto location randomdrop");

        // S.simu_interpret_command("set Kn 2e5");
        // S.simu_interpret_command("set Kt 8e4");
        // S.simu_interpret_command("set GammaN 75");
        // S.simu_interpret_command("set GammaT 75");
        // S.simu_interpret_command("set Mu 1");
        // S.simu_interpret_command("set Mu_wall 0");
        // S.simu_interpret_command("set T 150");
        // S.simu_interpret_command("set dt 0.0002");
        // S.simu_interpret_command("set tdump 10"); // how often to calculate wall forces
        // S.simu_interpret_command("auto skin");
        // S.simu_finalise_init () ;

        let tc = 1e-2;
        let rest = 0.5; // super low restitution coeff to dampen out quickly
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
