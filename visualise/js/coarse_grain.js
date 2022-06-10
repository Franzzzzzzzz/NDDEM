import * as THREE from "three";

import { TrackballControls } from "three/examples/jsm/controls/TrackballControls.js";
import { VolumeShader } from "three/examples/jsm/shaders/VolumeShader.js";
import { Volume } from "three/examples/jsm/misc/Volume.js";
import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';
import { VolumeRenderShaderRGB } from "./VolumeShaderRGB.js";
import { NRRDLoader } from "./NRRDLoader.js";

// if ( WEBGL.isWebGL2Available() === false ) {
    // document.body.appendChild( WEBGL.getWebGL2ErrorMessage() );
// }

var root_dir = window.location.origin + '/';
var data_dir = root_dir;
if ( window.location.hostname.includes('benjymarks') ) {
    root_dir = window.location.href;
    data_dir = root_dir;
    let cache = true;
}
else if ( window.location.hostname.includes('github') ) {
    root_dir = 'https://franzzzzzzzz.github.io/NDDEM/';
    data_dir = 'https://www.benjymarks.com/nddem/';
    let cache = true;
}

var container,
    renderer,
    scene,
    camera,
    controls,
    material,
    texture,
    volconfig,
    cmtextures,
    fname,
    world,
    time,
    data,
    rgb,
    gui,
    nx,ny,nz,
    nx_array,
    extra_dims,
    display_data;

const urlParams = new URLSearchParams(window.location.search);
// fname = "Samples/D3/CoarsedRHO.nrrd"
 // fname = "Samples/D3/CoarsedVAVG.nrrd"
// fname = "Samples/ColormapD4_large.nrrd"
// fname = "Samples/Colormap_fake.nrrd"
// fname = "Samples/4D_fake.nrrd"
// fname = "CppCode/CoarseGraining/CoarsedVAVG.nrrd"
// fname = "CppCode/CoarseGraining/CoarsedRHO.nrrd"
fname = "Samples/ColormapD5.nrrd"

if ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };
init();
animate();

/**
* Initialise the threejs scene, adding everything necessary, such as camera, controls, lighting etc.
*/
function init() {
    scene = new THREE.Scene();
    // Create renderer
    var canvas = document.createElement( 'canvas' );
    var context = canvas.getContext( 'webgl2' );
    renderer = new THREE.WebGLRenderer( { canvas: canvas, context: context } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    document.body.appendChild( renderer.domElement );
    // Create camera (The volume renderer does not work very well with perspective yet)
    var h = 4; // frustrum height
    var aspect = window.innerWidth / window.innerHeight;
    // camera = new THREE.OrthographicCamera( - h * aspect / 2, h * aspect / 2, h / 2, - h / 2, 0., 100 );
    camera = new THREE.OrthographicCamera( );
    camera.position.set( 10, 10, 10 );

    controls = new TrackballControls( camera, renderer.domElement );
    // controls.rotateSpeed = 1.0;
    // controls.zoomSpeed = 1.2;
    // controls.panSpeed = 0.8;
    // controls.noZoom = false;
    // controls.noPan = false;
    // controls.staticMoving = true;
    // controls.dynamicDampingFactor = 0.3;
    // controls.keys = [ 65, 83, 68 ];
    controls.addEventListener( 'change', render );

    if ( fname.includes("Colormap") ) { camera.up.set( 1, 0, 0 ); }
    else { camera.up.set( 1, 0, 0 ); };// In our data, x is up


    scene.add( new THREE.AxesHelper( 128 ) );
    // Lighting is baked into the shader a.t.m.
    // var dirLight = new THREE.DirectionalLight( 0xffffff );

    // Load the data ...
    new NRRDLoader()
        .load( data_dir + fname, function ( volume ) {
        time = {'min':0,'max':1,'cur':0,'rate':0.5,'play':false,'c':0}; // c is which component to show
        time.max = volume.time;
        time.nc = volume.dimensions[0]*volume.dimensions[1];
        nx = volume.dimensions[2];
        ny = volume.dimensions[3];
        nz = volume.dimensions[4];

        volconfig = { clim1: volume.windowLow, clim2: volume.windowHigh, renderstyle: 'normal', isothreshold: 0.15, colormap: 'inferno' };

        var start = [0,0,0,0,0,0];
        var end   = [0,0,nx,ny,nz,0];
        data = volume.data;
        rgb = volume.RGB;

        gui = new GUI();
        // if ( rgb ) { gui.close(); }
        if ( !rgb ) {
            var cmin = gui.add( volconfig, 'clim1', volume.windowLow, volume.windowHigh, 0.01 ).onChange( updateUniforms );
            var cmax = gui.add( volconfig, 'clim2', volume.windowLow, volume.windowHigh, 0.01 ).onChange( updateUniforms );
            gui.add( volconfig, 'colormap', { inferno: 'inferno', viridis: 'viridis', gray: 'gray' } ).onChange( updateUniforms ); }
        gui.add( volconfig, 'renderstyle', { normal: 'normal', iso: 'iso' } ).onChange( updateUniforms );
        if ( !rgb ) {
            gui.add( volconfig, 'isothreshold', 0, 1, 0.01 ).onChange( updateUniforms );
            gui.add( time, 'cur').min(time.min).max(time.max-1).step(1).listen().name('Time').onChange( function() { updateUniforms() });
        }

        if ( rgb ) { extra_dims = volume.dimensions.length - 5; }
        else { extra_dims = volume.dimensions.length - 6; }
        console.log('Found ', extra_dims, ' extra dimensions');


        world = [];
        for ( var i=0;i<extra_dims;i++ ) {
            world.push({});
            world[i].min = 0;
            world[i].max = volume.dimensions[i+5]-1;
            world[i].cur = Math.floor(volume.dimensions[i+5]/2);
            gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).step(1).name('x'+String(i+4)).onChange( function() { updateUniforms() }); // TODO: correct scale rather than grid point number
        }
        if ( time.nc > 1 && !rgb ) { gui.add( time, 'c').min(0).max(time.nc-1).step(1).name('component').onChange( function() { updateUniforms() }); }



        if ( volume.RGB ) { nx_array = volume.dimensions.slice(2); }
        else { nx_array = volume.dimensions.slice(2,-1); }

        if ( volume.RGB ) { display_data = get_data_RGB(volume.data,start,end); }
        else if (extra_dims === 1) { display_data = get_data_ND(volume.data,start,end); }
        else { display_data = get_data(volume.data,start,end); }

        // Texture to hold the volume. We have scalars, so we put our data in the red channel.
        // THREEJS will select R32F (33326) based on the RedFormat and FloatType.
        // Also see https://www.khronos.org/registry/webgl/specs/latest/2.0/#TEXTURE_TYPES_FORMATS_FROM_DOM_ELEMENTS_TABLE
        // TODO: look the dtype up in the volume metadata
        texture = new THREE.Data3DTexture( display_data, nx, ny, nz );
        // if ( volume.RGB ) { texture.format = THREE.RGBAFormat; }
        texture.format = THREE.RedFormat;
        texture.type = THREE.FloatType;
        // texture.minFilter = texture.magFilter = THREE.LinearFilter;
        texture.minFilter = texture.magFilter = THREE.NearestFilter;

        texture.unpackAlignment = 1;
        texture.needsUpdate = true;
        // Colormap textures
        cmtextures = {
            inferno: new THREE.TextureLoader().load( root_dir + 'visualise/resources/cm_inferno.png', render ),
            viridis: new THREE.TextureLoader().load( root_dir + 'visualise/resources/cm_viridis.png', render ),
            gray: new THREE.TextureLoader().load( root_dir + 'visualise/resources/cm_gray.png', render )
        };
        // Material
        if ( volume.RGB ) { var shader = VolumeRenderShaderRGB; }
        else { var shader = VolumeRenderShader; } // THERE WAS VolumeRenderShader1 DEFINED BEFORE MOVE TO NEW THREEJS VERSION, SEEM TO HAVE LOST IT SOMEWHERE, NEED TO FIND IT!

        var uniforms = THREE.UniformsUtils.clone( shader.uniforms );
        uniforms[ "u_data" ].value = texture;
        uniforms[ "u_size" ].value.set( nx, ny, nz );
        uniforms[ "u_clim" ].value.set( volconfig.clim1, volconfig.clim2 );
        uniforms[ "u_renderstyle" ].value = volconfig.renderstyle == 'normal' ? 0 : 1; // 0: MIP, 1: ISO
        uniforms[ "u_renderthreshold" ].value = volconfig.isothreshold; // For ISO renderstyle
        uniforms[ "u_cmdata" ].value = cmtextures[ volconfig.colormap ];
        material = new THREE.ShaderMaterial( {
            uniforms: uniforms,
            vertexShader: shader.vertexShader,
            fragmentShader: shader.fragmentShader,
            side: THREE.BackSide // The volume shader uses the backface as its "reference point"
        } );
        // Mesh
        var geometry = new THREE.BoxBufferGeometry( nx, ny, nz );
        geometry.translate( nx / 2 - 0.5, ny / 2 - 0.5, nz / 2 - 0.5 );
        var mesh = new THREE.Mesh( geometry, material );
        let nx_max = Math.max(nx, ny, nz);
        mesh.scale.set(1./nx_max,1./nx_max,1./nx_max);
        scene.add( mesh );
        render();
    } );
    window.addEventListener( 'resize', onWindowResize, false );
}
function updateUniforms() {
    var start = [0,0,0,0,0,time.cur];
    var end   = [0,0,nx,ny,nz,time.cur];
    if ( rgb ) { texture.image.data = get_data_RGB(data,start,end); }
    else if (extra_dims === 1) { texture.image.data = get_data_ND(data,start,end); }
    else { texture.image.data = get_data(data,start,end); }
    texture.needsUpdate = true;
    material.uniforms[ "u_clim" ].value.set( volconfig.clim1, volconfig.clim2 );
    material.uniforms[ "u_renderstyle" ].value = volconfig.renderstyle == 'normal' ? 0 : 1; // 0: MIP, 1: ISO
    material.uniforms[ "u_renderthreshold" ].value = volconfig.isothreshold; // For ISO renderstyle
    material.uniforms[ "u_cmdata" ].value = cmtextures[ volconfig.colormap ];
    render();
}
function onWindowResize() {
    renderer.setSize( window.innerWidth, window.innerHeight );
    var aspect = window.innerWidth / window.innerHeight;
    var frustumHeight = camera.top - camera.bottom;
    camera.left = - frustumHeight * aspect / 2;
    camera.right = frustumHeight * aspect / 2;
    camera.updateProjectionMatrix();
    controls.handleResize();
    render();
}
function animate() {
    requestAnimationFrame( animate );
    controls.update();
}
function render() {

    renderer.render( scene, camera );
}
function get_data(data,start,end) {
    length = nx*ny*nz;
    let output = new Float32Array(length);
    n = [1,time.nc,time.nc*nx,time.nc*nx*ny,time.nc*nx*ny*nz];
    for ( var i=0;i<nx;i++ ) {
        for ( var j=0;j<ny;j++ ) {
            for ( var k=0;k<nz;k++ ) {
                let shaped_pos = [time.c,i,j,k,time.cur]
                var linear_pos = 0;
                for (var l=0; l< n.length; l++) { linear_pos += n[l]*shaped_pos[l]; };
                // console.log(linear_pos > length*time.nc);
                output[i + j*nx + k*nx*ny] = data[linear_pos]
                // output[k + j*nz + i*nz*ny] = data[linear_pos]
            }
        }
    }
    return output;
}

function get_data_ND(data,start,end) {
    length = nx_array.reduce( (a,b) => a * b ); // multiply all components of array
    let output = new Float32Array(length);
    n = [1,time.nc];
    for ( var m=0;m<nx_array.length;m++ ) {
        n.push(time.nc*nx_array.slice(0,m+1).reduce( (a,b) => a * b ))
    };
    // console.log(nx_array)
    console.log(length);
    console.log(data.length);

    // GENERALISED UP TO HERE
    for ( var i=0;i<nx_array[0];i++ ) {
        for ( var j=0;j<nx_array[1];j++ ) {
            for ( var k=0;k<nx_array[2];k++ ) {
                let shaped_pos = [time.c,i,j,k];
                for ( var m=0;m<extra_dims;m++ ) { shaped_pos.push(world[m].cur); }
                shaped_pos.push(time.cur);
                // console.log(shaped_pos)
                var linear_pos = 0;
                for (var l=0; l< n.length; l++) { linear_pos += n[l]*shaped_pos[l]; };
                // console.log(linear_pos);
                // console.log(data[linear_pos]);
                output[i + j*nx_array[0] + k*nx_array[0]*nx_array[1]] = data[linear_pos]
            }
        }
    }

    // console.log(data[linear_pos]);
    // console.log(output[100]);
    return output;
}

function get_data_RGB(data,start,end) {
    length = nx_array.reduce( (a,b) => a * b ); // multiply all components of array
    let output = new Float32Array(length);

    let n = [1,3];
    for ( var m=0;m<nx_array.length;m++ ) {
        n.push(3*nx_array.slice(0,m+1).reduce( (a,b) => a * b ))
    };

    for ( var i=0;i<nx_array[0];i++ ) {
        for ( var j=0;j<nx_array[1];j++ ) {
            for ( var k=0;k<nx_array[2];k++ ) {
                let shaped_pos = [0,i,j,k];
                for ( var m=0;m<extra_dims;m++ ) { shaped_pos.push(world[m].cur); }
                shaped_pos.push(time.cur);

                var linear_pos = 0;
                for (var l=0; l< n.length; l++) { linear_pos += n[l]*shaped_pos[l]; };
                output[i + j*nx + k*nx*ny] = data[linear_pos]*256*256 + data[linear_pos+1]*256 + data[linear_pos+2]
            }
        }
    }
    return output;
}

// function get_data_RGB(data,start,end) { // JUST IN 3D
//     length = nx*ny*nz;
//     output = new Float32Array(length);
//     n = [1,3,3*nx,3*nx*ny,3*nx*ny*nz];
//     for ( var i=0;i<nx;i++ ) {
//         for ( var j=0;j<ny;j++ ) {
//             for ( var k=0;k<nz;k++ ) {
//                 shaped_pos = [0,i,j,k,time.cur]
//                 var linear_pos = 0;
//                 for (var l=0; l< n.length; l++) { linear_pos += n[l]*shaped_pos[l]; };
//                 output[i + j*nx + k*nx*ny] = data[linear_pos]*256*256 + data[linear_pos+1]*256 + data[linear_pos+2]
//             }
//         }
//     }
//     return output;
// }
