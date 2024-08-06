import css from "../css/main.css";

import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
import { damp } from "three/src/math/MathUtils.js";
import { max } from "three/examples/jsm/nodes/ShaderNode.js";
// import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';

var urlParams = new URLSearchParams(window.location.search);

var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel, controls;
let physics, position;
let gui;
let floor, roof, left, right, front, back;
let S;
let NDDEMCGLib;
let particle_volume;
let show_stats = false;
// const thickness = radius;
const material_density = 2700;
let thickness;

var params = {
    dimension: 3,
    H: 10, //system size
    L: 5,
    D: 1, //system size
    // N: 600,
    packing_fraction: 0.55,
    gravity: true,
    paused: false,
    theta: 0,
    H_cur: 0,
    d4: { cur: 0 },
    d5: { cur: 0 },
    size_ratio: 2,
    r_max: 0.5,
    omega: 5, // rotation rate
    lut: 'Size',
    quality: 5,
    vmax: 20, // max velocity to colour by
    omegamax: 20, // max rotation rate to colour by
    wallremoved: false,
    damping: 0.01,
}

function update_params() {
    // params.L = params.H/2; //system size
    params.LL = params.H*10; //system size
    params.r_min = params.r_max / params.size_ratio;
    let min_particle_volume = 4. / 3. * Math.PI * Math.pow(params.r_min, 3);
    let max_particle_volume = 4. / 3. * Math.PI * Math.pow(params.r_max, 3);
    params.min_particle_mass = material_density * min_particle_volume;

    params.average_radius = (params.r_min + params.r_max) / 2.;
    thickness = params.average_radius;

    
    params.N_small = Math.floor(params.L * params.H * params.D * params.packing_fraction / min_particle_volume);
    params.N_large = Math.floor(params.L * params.H * params.D * params.packing_fraction / max_particle_volume);
    params.N = params.N_small + params.N_large;
}

update_params();

if (urlParams.has('quality')) { params.quality = parseInt(urlParams.get('quality')); }
if (urlParams.has('lut')) { params.lut = urlParams.get('lut'); }

// params.back = -params.D;
// params.front = params.D;
// params.left = 0.;
// params.right = params.H;
// params.floor = 0.;
// params.roof = params.L;

// update_L();
//
// function update_L() {
//     var L = params.N*4./3.*Math.PI*Math.pow(radius,3);
//     L = Math.pow(solid_volume/params.packing_fraction,1./3.)
// }

SPHERES.createNDParticleShader(params).then(init);

async function init() {

    physics = await NDDEMPhysics();
    // physics.main(params.dimensions, params.N, inputfile)
    position = new THREE.Vector3();

    //

    camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.set(0, 3 * params.L, -3 * params.L);
    camera.up.set(1, 0, 0);

    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x666666);

    // const axesHelper = new THREE.AxesHelper( 50 );
    // scene.add( axesHelper );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add(hemiLight);

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set(5, 5, 5);
    dirLight.castShadow = true;
    dirLight.shadow.camera.zoom = 2;
    scene.add(dirLight);

    //const wall_geometry = new THREE.BoxGeometry( params.L*2 + thickness*2, thickness, params.L*2 + thickness*2 );
    //const wall_geometry = new THREE.SphereGeometry( params.L, 32, 32 );
    const wall_material = new THREE.MeshLambertMaterial();
    wall_material.wireframe = true;

    const wall_geometry = new THREE.BoxGeometry(1, 1, 1);
    left = new THREE.Mesh(wall_geometry, wall_material);
    left.scale.y = 2 * thickness;
    left.scale.z = 2 * params.D;
    left.scale.x = params.H;
    left.position.x = params.H / 2.;

    //floor.rotation.x = Math.PI / 2.;
    //floor.position.z = - params.L * params.aspect_ratio - params.thickness / 2.;
    scene.add(left);

    right = new THREE.Mesh(wall_geometry, wall_material);
    right.scale.y = 2 * thickness;
    right.scale.z = 2 * params.D;
    right.scale.x = params.H;
    right.position.x = params.H / 2.;
    right.position.y = params.L;
    scene.add(right);

    floor = new THREE.Mesh(wall_geometry, wall_material);
    floor.scale.x = 2 * thickness;
    floor.scale.y = params.LL;
    floor.scale.z = 2 * params.D;
    floor.position.y = params.LL / 2.;
    // left.receiveShadow = true;
    scene.add(floor);

    /*floor = new THREE.Mesh(wall_geometry, wall_material);
    floor.scale.y = params.thickness;
    floor.rotation.x = Math.PI / 2.;
    floor.position.z = - params.L * params.aspect_ratio - params.thickness / 2.;
    // left.receiveShadow = true;
    scene.add(floor);*/


    SPHERES.add_spheres(S, params, scene);
    //

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.shadowMap.enabled = true;
    renderer.outputEncoding = THREE.sRGBEncoding;

    var container = document.getElementById('canvas');
    container.appendChild(renderer.domElement);

    // stats = new Stats();
    // panel = stats.addPanel( new Stats.Panel( 'Pressure', 'white', 'black' ) );
    // stats.showPanel( 3 ); // 0: fps, 1: ms, 2: mb, 3+: custom
    // // document.body.appendChild( stats.dom );
    // var thisParent = document.getElementById("stats");
    // thisParent.appendChild( stats.domElement );
    //
    // var statsALL = document.getElementById("stats").querySelectorAll("canvas");
    //
    // for(var i=0; i<statsALL.length; i++){
    //     statsALL[i].style.width = "240px";
    //     statsALL[i].style.height = "160px";
    // }

    // gui
    gui = new GUI();

    gui.width = 300;

    gui.add(params, 'H', 5, 30, 1).name('Height').listen().onChange(reset_domain_and_particles);
    gui.add(params, 'size_ratio', 1, 3, 0.1).name('Size ratio').listen().onChange(reset_domain_and_particles);
    gui.add(params, 'damping', 0, 0.02, 0.001).name('Damping').listen().onChange(() => {
        S.simu_interpret_command("set damping " + String(params.damping));
    });
    gui.add(params, 'wallremoved').name('Remove the dam wall').listen().onChange(move_dam_wall);

    controls = new OrbitControls(camera, renderer.domElement);
    controls.target.y = params.L / 2.;
    controls.target.x = params.H / 2.;
    controls.update();

    window.addEventListener('resize', onWindowResize, false);

    animate();
}

function move_dam_wall() {
    let theta;
    if (params.wallremoved) {
        theta = params.theta * Math.PI / 180;
        right.position.y = params.LL;
        S.simu_interpret_command("boundary 1 WALL 0 " + String(params.LL));
    } else {
        theta = 0;
        right.position.y = params.L;
        S.simu_interpret_command("boundary 1 WALL 0 " + String(params.L));
        reset_domain_and_particles();
    }
    S.simu_interpret_command("gravity " + String(-100 * Math.cos(theta)) + " 0 " + String(-100 * Math.sin(theta)));
    S.simu_interpret_command("boundary 2 WALL -" + String(params.D) + " " + String(params.D));
}

function reset_domain_and_particles() {
    params.wallremoved = false;
    update_params();
    if ( S !== undefined ) {
        S.simu_finalise();
    }
    
    if ( params.dimension == 2 ) {
        S = new NDDEMCGLib.DEMCG2D (params.N);
    }
    else if ( params.dimension == 3 ) {
        S = new NDDEMCGLib.DEMCG3D (params.N);
    }
    else if ( params.dimension == 4 ) {
        S = new NDDEMCGLib.DEMCG4D (params.N);
    }
    else if ( params.dimension == 5 ) {
        S = new NDDEMCGLib.DEMCG5D (params.N);
    }
    finish_setup();


    let theta;
    if (params.wallremoved) {
        theta = params.theta * Math.PI / 180;
        right.position.y = params.LL;
        S.simu_interpret_command("boundary 1 WALL 0 " + String(params.LL));
    } else {
        theta = 0;
        right.position.y = params.L;
        S.simu_interpret_command("boundary 1 WALL 0 " + String(params.L));
    }
    S.simu_interpret_command("gravity " + String(-10 * Math.cos(theta)) + " 0 " + String(-10 * Math.sin(theta)));

    S.simu_interpret_command("boundary 0 WALL 0 " + String(params.H));
    S.simu_interpret_command("boundary 2 WALL -" + String(params.D) + " " + String(params.D));
    
    if (!params.wallremoved) {
        S.simu_interpret_command("auto location randomdrop");
    }

    SPHERES.update_radii(S, params);
    SPHERES.add_spheres(S, params, scene);
}

function onWindowResize() {

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize(window.innerWidth, window.innerHeight);

}

function animate() {
    requestAnimationFrame(animate);
    SPHERES.move_spheres(S, params);

    S.simu_step_forward(5);
    controls.update();

    renderer.render(scene, camera);
}

async function NDDEMPhysics() {

    if ('DEMCGND' in window === false) {

        console.error('NDDEMPhysics: Couldn\'t find DEMCGND.js');
        return;

    }

    await DEMCGND().then((lib) => {
        NDDEMCGLib = lib;
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
}
function finish_setup() {
    let tc = 1e-2;
    let rest = 0.5; // super low restitution coeff to dampen out quickly

    let vals = SPHERES.setCollisionTimeAndRestitutionCoefficient(tc, rest, params.min_particle_mass);
    // console.log(vals);

    S.simu_interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
    S.simu_interpret_command("radius -1 0.5");
    let m = 4. / 3. * Math.PI * 0.5 * 0.5 * 0.5 * material_density;
    S.simu_interpret_command("mass -1 " + String(m));
    // S.simu_interpret_command("mass -1 1");
    S.simu_interpret_command("auto rho");
    S.simu_interpret_command("auto radius bidisperse " + params.r_min + " " + params.r_max + " 0.5");
    S.simu_interpret_command("auto mass");
    S.simu_interpret_command("auto inertia");

    S.simu_interpret_command("boundary 0 WALL 0 " + String(params.H));
    S.simu_interpret_command("boundary 1 WALL 0 " + String(params.L));
    S.simu_interpret_command("boundary 2 PBC -" + String(params.D) + " " + String(params.D));

    // S.simu_interpret_command("auto location randomsquare");
    S.simu_interpret_command("auto location randomdrop");
    S.simu_interpret_command("gravity -100 0 0 "); // intensity, omega, rotdim0, rotdim1

    S.simu_interpret_command("set Kn " + String(vals.stiffness));
    S.simu_interpret_command("set Kt " + String(0.8 * vals.stiffness));
    S.simu_interpret_command("set GammaN " + String(vals.dissipation));
    S.simu_interpret_command("set GammaT " + String(vals.dissipation));
    S.simu_interpret_command("set Mu 0.5");
    S.simu_interpret_command("set Mu_wall 1");
    S.simu_interpret_command("set damping " + String(params.damping));
    S.simu_interpret_command("set T 150");
    S.simu_interpret_command("set dt " + String(tc / 20));
    S.simu_interpret_command("auto skin");
    S.simu_finalise_init();
}
