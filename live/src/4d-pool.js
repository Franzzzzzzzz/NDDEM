import * as THREE from "three";
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';

import * as SPHERES from "../libs/SphereHandler.js"
import * as WALLS from "../libs/WallHandler.js"
import * as LAYOUT from '../libs/Layout.js'
import { NDSTLLoader, renderSTL } from '../libs/NDSTLLoader.js';
// import { PoolTableShader } from '../libs/PoolTableShader.js';

var urlParams = new URLSearchParams(window.location.search);
var clock = new THREE.Clock();

let camera, scene, renderer, stats, panel;
let physics, position;
let gui;
let boxes, spheres;
let floor, roof, left, right, front, back;
let S;
let x;
let NDDEMLib;
let pointer;
let frameRate = 60;
let v;
let pressure = 0;
let shear = 0;
let density = 0;
let pressure_time = [];
let shear_time = [];
let density_time = [];
let particle_volume;
let started = false;
let show_stats = true;
const material_density = 2700;
let old_time = 0;
let new_time = 0;
let counter = 0;
let p_controller, q_controller;
let NDsolids, material, STLFilename;
let meshes = new THREE.Group();
var direction = new THREE.Vector3();
var raycaster = new THREE.Raycaster();
const mouse = new THREE.Vector2();
let intersection_plane = new THREE.Plane();
let camera_direction = new THREE.Vector3();
let ray;
let isMobile;

let INTERSECTED = null;
let last_intersection = null;
let locked_particle = null;
let ref_location;

var params = {
    radius: 0.05,
    dimension: 4,
    L1: 2,
    L2: 1,
    L3: 0.1, // this is the direction of gravity
    L4: 0.5,
    pocket_size: 0.05,
    pyramid_size: 5,
    // packing_fraction: 0.5,
    axial_strain: 0,
    volumetric_strain: 0,
    gravity: true,
    paused: false,
    H_cur: 0,
    pressure_set_pt: 1e4,
    deviatoric_set_pt: 0,
    d4_cur:0,
    dt: 0.0005,
    track_white_ball: true,
    strength: 1,
}

params.N = get_num_particles(params.pyramid_size);

var quality = 7;
let sunk_balls = [];

var NDParticleShader;
import("../libs/shaders/" + params.dimension + "DShader.js").then((module) => {
    NDParticleShader = module.NDDEMShader;
    init();
});
// SPHERES.createNDParticleShader(params).then( init() );

async function init() {

    physics = await NDDEMPhysics();
    position = new THREE.Vector3();

    camera = new THREE.PerspectiveCamera( 15, window.innerWidth / window.innerHeight, 0.1, 1000 );
    camera.position.set( 2*params.L1, 0, 2*params.L1 );
    camera.up.set(0, 0, 1);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x666666 );

    const hemiLight = new THREE.HemisphereLight();
    hemiLight.intensity = 0.35;
    scene.add( hemiLight );

    const dirLight = new THREE.DirectionalLight();
    dirLight.position.set( 0, 0, 5 );
    dirLight.castShadow = true;
    dirLight.shadow.mapSize.width = 2*1024;
    dirLight.shadow.mapSize.height = 2*1024;
    scene.add( dirLight );

    ray = new THREE.Line(
        new THREE.BufferGeometry().setFromPoints([
            new THREE.Vector3(0,-3,0),
            new THREE.Vector3(0,0,0),
        ]),
        new THREE.LineBasicMaterial( { color: 0xffffff })
    );

    add_spheres();

    // STLFilename = './stls/4d-pool.stl'; // this one has crap pockets
    STLFilename = './stls/4d-pool-no-holes.stl';
    // const texture = new THREE.TextureLoader().load( 'textures/golfball.jpg', function(t) {
        // t.encoding = THREE.sRGBEncoding;
        // t.mapping = THREE.EquirectangularReflectionMapping;
    // } );
    material = new THREE.MeshStandardMaterial( {
        color: 0x00aa00,
        metalness: 1,
        roughness: 1,
        // map: texture,
    } );

    loadSTL();

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    renderer.outputEncoding = THREE.sRGBEncoding;
    document.body.appendChild( renderer.domElement );

    // gui
    gui = new GUI();
    gui.width = 400;

    gui.add( params, 'd4_cur', -params.L4,params.L4, 0.001).name( 'D4 location (w/s)').listen();
    gui.add( params, 'strength', 0,2, 0.001).name( 'Strength (a/d)').listen().onChange(() => {
        ray.scale.y = 2*params.strength;
    });
    gui.add ( params, 'track_white_ball').name('Track white ball (Space)').listen();
    const controls = new OrbitControls( camera, renderer.domElement );
    controls.target.y = -params.L3;
    controls.update();

    isMobile = navigator.userAgentData.mobile; //resolves true/false
    if ( isMobile ) {
        document.getElementById("hit_me").hidden = false;
        document.getElementById("web_instructions").hidden = true;
        document.getElementById("hit_me").addEventListener ("click", hit_white_ball, false);
        gui.width = window.innerWidth - 30;
    }
    else {
        document.getElementById('hit_me').hidden = true;
        document.getElementById("web_instructions").hidden = false;
        // document.getElementById ("hit_me").addEventListener ("click", hit_white_ball, false);
        // gui.width = window.innerWidth - 30;

    }

    window.addEventListener( 'resize', onWindowResize, false );
    window.addEventListener( 'mousemove', onMouseMove, false );
    window.addEventListener( 'keypress', onSelectParticle, false );

    animate();
}

function onMouseMove( event ) {

    // calculate mouse position in normalized device coordinates
    // (-1 to +1) for both components

    mouse.x = ( event.clientX / window.innerWidth ) * 2 - 1;
    mouse.y = - ( event.clientY / window.innerHeight ) * 2 + 1;

}
function hit_white_ball() {
    var work = params.strength;
    var force = work/params.dt;
    S.setExternalForce(0, 1, [force*direction.x,force*direction.y,0.,force*direction.d4]);
}

function onSelectParticle( event ) {
    // console.log(camera.getWorldDirection() )
    if ( event.key === 'Enter' ) {
        // console.log('hit!')
        hit_white_ball();

    }
    if ( event.code === 'Space' ) {
        event.preventDefault(); // stop page from skipping downwards
        params.track_white_ball = !params.track_white_ball;
        // if ( locked_particle === null ) {
        //     locked_particle = INTERSECTED;
        //     // console.log(locked_particle);
        //     ref_location = locked_particle.position;
        //
        //     camera.getWorldDirection( camera_direction ); // update camera direction
        //     // set the plane for the particle to move along to be orthogonal to the camera
        //     intersection_plane.setFromNormalAndCoplanarPoint( camera_direction,
        //                                                       locked_particle.position );
        // }
        // else {
        //     locked_particle = null;
        // }
    }
    else if ( event.key === "w" ) {
        params.track_white_ball = false;
        params.d4_cur += 0.1*params.radius;
        meshes = renderSTL( meshes, NDsolids, scene, material, params.d4_cur );
    }
    else if ( event.key === "s" ) {
        params.track_white_ball = false;
        params.d4_cur -= 0.1*params.radius;
        meshes = renderSTL( meshes, NDsolids, scene, material, params.d4_cur );
    }
    else if ( event.key === "a" ) {
        params.strength -= 0.01;
    }
    else if ( event.key === "d" ) {
        params.strength += 0.01;
    }
}


function onWindowResize(){

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

    if ( isMobile ) {
        gui.width = window.innerWidth - 30;
    }

}

function animate() {
    if ( clock.getElapsedTime() > 1 ) { started = true; }
    requestAnimationFrame( animate );
    move_spheres();

    direction.copy(spheres.children[0].position);
    direction.sub(camera.position);
    let d4offset = params.d4_cur - x[0][3];
    var l = Math.sqrt(direction.x*direction.x + direction.y*direction.y + d4offset*d4offset);
    direction.x /= l;
    direction.y /= l;
    direction.z = 0;
    direction.d4 = d4offset/l;

    ray.rotation.z = Math.atan2(direction.y,direction.x);

    if ( locked_particle !== null ) {
        // console.log(locked_particle)
        // if ( raycaster.ray.intersectsPlane( intersection_plane ) ) { // if the mouse is over the plane - why would i need to check this?
            // console.log('success!')
            // ref_location.position.copy( _intersection.sub( _offset ).applyMatrix4( _inverseMatrix ) );
        // }
        // ref_location.x = mouse.x;
        raycaster.ray.intersectPlane( intersection_plane, ref_location);
        ref_location.clamp( new THREE.Vector3(-params.L1, -params.L2,0),
                            new THREE.Vector3( params.L1,  params.L2,0) );
        // if ( ref_location.x > back.position.x + radius && ref_location.x < front.position.x - radius ) {
            // console.log(ref_location.x)
            // console.log(back.position.x)
        S.fixParticle(locked_particle.NDDEM_ID,[ref_location.x, ref_location.y, ref_location.z]);
        // }
    }

    if ( !params.paused ) {
        S.step_forward(20);
    }

    if ( params.track_white_ball ) {
        if ( NDsolids !== undefined ) {
            x = S.getX();
            params.d4_cur = x[0][3];
            meshes = renderSTL( meshes, NDsolids, scene, material, params.d4_cur );
        }
    }

    // ray.position.set

    // const intersects = raycaster.intersectObjects( spheres.children );
    // console.log(intersects)
    // if ( intersects.length > 0 ) { // if found something
        // console.log(intersects)
    // 	if ( INTERSECTED != intersects[ 0 ].object ) { // if not the same as last time
    //             if ( INTERSECTED !== null ) { INTERSECTED.material.uniforms.ambient.value = 1.0; }
    // 			INTERSECTED = intersects[ 0 ].object;
    //             INTERSECTED.material.uniforms.ambient.value = 5.0;
    // 	}
    // }
    // else { // didn't find anything
    // 	if ( INTERSECTED !== null ) { // there was something before
    //         INTERSECTED.material.uniforms.ambient.value = 1.0;
    //         INTERSECTED = null;
    //     }
    // }


    renderer.render( scene, camera );
    old_time = new_time;

}

function add_spheres() {
    spheres = new THREE.Group();
    scene.add(spheres);

    const geometrySphere = new THREE.SphereGeometry( 0.5, Math.pow(2,quality), Math.pow(2,quality) );

    for ( let i = 0; i < params.N; i ++ ) {
        if ( i == 0 ) {
            var material = new THREE.MeshStandardMaterial( {
                color: 0x0aaaaaa });
        }
        else {
            var material = NDParticleShader.clone();
            material.uniforms.R.value = params.radius;
            material.uniforms.banding.value = 1 + 2*(i%3);
        }
        var object = new THREE.Mesh(geometrySphere, material);
        object.position.set(0,0,0);
        object.rotation.z = Math.PI / 2;
        object.NDDEM_ID = i;
        object.castShadow = true;
        object.receiveShadow = true;
        spheres.add(object);
    }

    // display white ball
    // spheres.children[0].material.uniforms.banding.value = 1.;
    // spheres.children[0].material.uniforms.ambient.value = 5.;

    // display black ball
    spheres.children[11].material.uniforms.banding.value = 0.;
    spheres.children[11].material.uniforms.ambient.value = 1.;

    spheres.children[0].add(ray); // add line

}

function in_pocket(x) {
    if ( x[2] < -params.L3+params.radius-params.pocket_size ) {
        console.log('fallen off table (hopefully out of a hole)')
        return true;
        // return false;
    }
    else {
        return false
    }
}

function move_spheres() {
    x = S.getX();
    var orientation = S.getOrientation();
    if ( urlParams.has('lut') ) {
        if ( urlParams.get('lut') === 'velocity' ) {
            v = S.getVelocity();
        }
        // spheres.instanceColor.needsUpdate = true;

    }
    for ( let i = 0; i < params.N; i ++ ) {
        var object = spheres.children[i];
        // if ( object.visible ) {
        //     if ( in_pocket(x[i]) ) {
        //         object.visible = false;
        //         sunk_balls.push(i)
        //         S.fixParticle(i, [1.1*params.L, sunk_balls.length, 0, 0])
        //         S.setFrozen(i);
        //     }
        // }

        // const matrix = new THREE.Matrix4();
        // matrix.setPosition( x[i][0], x[i][1], x[i][2] );
        if ( params.dimension == 4 ) {
            var D_draw = 2*Math.sqrt(
              Math.pow(params.radius, 2) - Math.pow(params.d4_cur - x[i][3], 2)
            );
            object.scale.set(D_draw, D_draw, D_draw);
            // matrix.scale( new THREE.Vector3(D_draw,D_draw,D_draw) );
        }
        // spheres.setMatrixAt( i, matrix );
        object.position.set( x[i][0], x[i][1], x[i][2] );
        if ( urlParams.has('lut') ) {
            if ( urlParams.get('lut') === 'velocity' ) {
                spheres.setColorAt( i, lut.getColor( 1e-4*( Math.pow(v[i][0],2) + Math.pow(v[i][1],2) + Math.pow(v[i][2],2) ) ) );
            }
        }
        if ( i > 0 ) {
            // console.log(params.d4_cur)
            for (var j = 0; j < params.dimension - 3; j++) {

              object.material.uniforms.xview.value[j] =
                params.d4_cur;
              object.material.uniforms.xpart.value[j] =
                x[i][j + 3];
            }
            object.material.uniforms.A.value = orientation[i];
        }
        // if (params.dimension > 3) {
        //   object.material.uniforms.x4p.value = x[i][j + 3];
        //   object.material.uniforms.x4.value = params.d4_cur;
        // } else {
        //   object.material.uniforms.x4p.value = 0.0;
        // }
    }
    // spheres.instanceMatrix.needsUpdate = true;
    // console.log(orientation[0])
}

async function NDDEMPhysics() {

    if ( 'DEMND' in window === false ) {

        console.error( 'NDDEMPhysics: Couldn\'t find DEMND.js' );
        return;

    }

    NDDEMLib = await DEMND(); // eslint-disable-line no-undef

    if ( params.dimension == 3 ) {
        S = await new NDDEMLib.Simulation3 (params.N);
        finish_setup();
    }
    else if ( params.dimension == 4 ) {
        S = await new NDDEMLib.Simulation4 (params.N);
        finish_setup();
    }



    function finish_setup() {
        S.interpret_command("dimensions " + String(params.dimension) + " " + String(params.N));
        S.interpret_command("radius -1 " + String(params.radius));
        S.interpret_command("mass -1 0.1");
        S.interpret_command("auto rho");
        S.interpret_command("auto inertia");

        S.interpret_command("boundary 0 WALL -"+String(params.L1)+" "+String(params.L1));
        S.interpret_command("boundary 1 WALL -"+String(params.L2)+" "+String(params.L2));
        S.interpret_command("boundary 2 WALL -"+String(params.L3)+" "+String(params.L3));
        S.interpret_command("boundary 3 WALL -"+String(params.L4)+" "+String(params.L4));
        // S.interpret_command("body " + STLFilename);
        S.interpret_command("gravity 0 0 -10 0");
        // S.interpret_command("auto location randomdrop");

        let n = 1;
        let offset = params.L1/2;

        S.interpret_command("location " + String(0) + " " + String(offset) + " " + String(0) + " " + String(-params.L3+params.radius) + " " + String(0.001*(Math.random()-0.5))); // first ball is the white ball

        for ( var k=0; k<params.pyramid_size; k++ ) {
            let cur_pyramid_length = params.pyramid_size - k;
            let w = k*1.825*params.radius;
            for ( var i=0; i<cur_pyramid_length; i++ ) {
                for ( var j=0; j<cur_pyramid_length - i; j++) {
                    let x = i*1.82*params.radius - cur_pyramid_length*params.radius + params.radius - offset;
                    let y = j*2.01*params.radius - (cur_pyramid_length-i)*params.radius + params.radius;// - i%2*radius;
                    // console.log(x,y);
                    S.interpret_command("location " + String(n) + " " + String(x) + " " + String(y) + " " + String(-params.L3+params.radius) + " " + String(w));
                    n++;
                    if ( k > 0 ) { S.interpret_command("location " + String(n) + " " + String(x) + " " + String(y) + " " + String(-params.L3+params.radius) + " " + String(-w)); n++;}

                }
            }
        }
        S.interpret_command("set Kn 2e5");
        S.interpret_command("set Kt 8e4");
        S.interpret_command("set GammaN 75");
        S.interpret_command("set GammaT 75");
        S.interpret_command("set Mu 0.1");
        S.interpret_command("set Mu_wall 0.5");
        S.interpret_command("set damping 5");
        S.interpret_command("set T 150");
        S.interpret_command("set dt " + String(params.dt));
        S.interpret_command("auto skin");
        S.finalise_init () ;
    }
}

function loadSTL( ) {

    const loader = new NDSTLLoader();
    loader.load( [ STLFilename ], function ( solids ) {
        NDsolids = solids;
        meshes = renderSTL(meshes, NDsolids, scene, material, params.d4_cur);
    } )
}

function get_num_particles(L) {
    let N = 0;
    let i = 1;
    for (var n=L; n>0; n--) {
        // console.log(n,i,i*n)
        N += i*n;
        i += 2;
    }
    return N+1; // adding the white ball
}
