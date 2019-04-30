// var THREE = require('three');

var container; // main div element
var camera, scene, controls, renderer; // UI elements
var controller1, controller2; // VR controllers

var raycaster, intersected = []; // catching grains
var tempMatrix = new THREE.Matrix4(); // catching grains

var particles, wristband, wristband1, axesHelper, axesLabels; // groups of objects
var R,r; // parameters of torus
var N; // number of dimensions
var world = []; // properties that describe the domain
var ref_dim = {'c': 0} //, 'x': 00, 'y': 1, 'z': 2}; // reference dimensions
var time = {'cur': 0, 'prev': 0, 'min':0, 'max': 99, 'play': false, 'rate': 0.5} // temporal properties
if ( typeof window.autoplay !== 'undefined' ) { time.play = window.autoplay === 'true' };
if ( typeof window.rate !== 'undefined' ) { time.rate = parseFloat(window.rate) };
var axeslength, fontsize; // axis properties
var VR_scale = 5.; // mapping from DEM units to VR units
var find_particle = window.find_particle === 'true'; // show just the first particle in a unique colour
var view_mode = window.view_mode; // options are: undefined (normal), catch_particle, rotations, velocity, rotation_rate
// var catch_particle = window.catch_particle === 'true'; // enable ability to catch particles
var quality, shadows;
var cache = true;

if ( typeof window.shadows !== 'undefined' ) { shadows = window.shadows == 'true' }
else { shadows = true; };
if ( typeof window.quality !== 'undefined' ) { quality = parseInt(window.quality) }
else { quality = 5}; // quality flag - 5 is default, 8 is ridiculous

var lut = new THREE.Lut( "cooltowarm", 512 ); // options are rainbow, cooltowarm and blackbody

init();

function init() {
    var request = new XMLHttpRequest();
    if ( cache ) {
        request.open('POST', "http://localhost:8000/in?fname=" + window.fname, true);
    }
    else {
        request.open('POST', "http://localhost:8000/in?fname=" + window.fname + "&_="+ (new Date).getTime(), true);
    };
    request.send(null);
    request.onreadystatechange = function () {
        if (request.readyState === 4 && request.status === 200) {
            var type = request.getResponseHeader('Content-Type');
            if (type.indexOf("text") !== 1) {
                lines = request.responseText.split('\n');
                for (i=0;i<lines.length;i++) {
                    l = lines[i].split(' ')
                    if (l[0] == 'dimensions') {
                        N = parseInt(l[1]);
                        for (j=0;j<N;j++) {
                            world.push({});
                            world[j].min = 0.;
                            world[j].max = 1.;
                            world[j].cur = 0.5;
                            world[j].prev = 0.5;
                        }

                    }
                    else if (l[0] == 'boundary') {
                        if (l[2] == 'WALL' || l[2] == 'PBC') {
                            world[l[1]].min = parseFloat(l[3]);
                            world[l[1]].max = parseFloat(l[4]);
                            world[l[1]].cur = (world[l[1]].min + world[l[1]].max)/2.;
                            world[l[1]].prev = world[l[1]].cur;
                        }
                    }
                    else if (l[0] == 'set') {
                        if (l[1] == 'T') {
                            time.max = parseInt(l[2]) - 1;
                        }
                    }
                }
                if ( N == 1 ) { // just used for setting up cameras etc
                    world.push({});
                    world[1].min = 0.;
                    world[1].max = 0.;
                    world[1].cur = 0.5;
                    world[1].prev = 0.5;
                };
                if ( N < 3 ) { // just used for setting up cameras etc
                    world.push({});
                    world[2].min = 0.;
                    world[2].max = 0.;
                    world[2].cur = 0.5;
                    world[2].prev = 0.5;
                }
                build_world();
                remove_everything(); // only runs on postMessage receive
                animate();
            }
        }
    }
}

function build_world() {
    container = document.createElement( 'div' );
    document.body.appendChild( container );
    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111111 ); // revealjs background colour
    // scene.background = new THREE.Color( 0xFFFFFF ); // white
    make_camera();
    // make_walls();
    make_axes();
    make_lights();
    add_renderer();
    add_controllers();
    if ( N > 3 ) { add_torus(); }
    // load_hyperspheres_VTK();
    make_initial_spheres_CSV();
    update_spheres_CSV(0);
    add_gui();
    window.addEventListener( 'resize', onWindowResize, false );

}

function make_camera() {
    var aspect = window.innerWidth / window.innerHeight;
    if ( N < 3 ) {
        zoom = 8.0;
        camera = new THREE.OrthographicCamera(-zoom*aspect,zoom*aspect,zoom,-zoom,-1000,1000);
        camera.position.set((world[0].min + world[0].max)/2./2.,(world[1].min + world[1].max)/2.,-1.0);
    }
    else {
        if ( window.viewerType == 'anaglyph' ) {
            camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
            camera.position.set(0.5*world[0].max/8.,
                                   -world[0].max/8.,
                                   -world[0].max/8.
                            );
            camera.focalLength = 3;
        }
        else {
            camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
            camera.position.set(0.25*world[0].max,
                                -0.5*world[0].max,
                                -0.5*world[0].max
                            );
        }
    }
}
function add_renderer() {
    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    if ( shadows ) { renderer.shadowMap.enabled = true; }
    if (window.viewerType == "VR") { renderer.vr.enabled = true; };
    container.appendChild( renderer.domElement );

    if (window.viewerType == "VR") { container.appendChild( WEBVR.createButton( renderer ) ); };
}

function add_gui() {
    if ( window.viewerType == 'anaglyph' || window.viewerType == 'keyboard' ) {
        var gui = new dat.GUI();
        gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension').onChange( function( val ) { make_axes(); }) ;
        if (N > 3) {
            for (i=3;i<N;i++) {
                gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
            }
        }
        gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
        gui.add( time, 'rate').min(0).max(1.0).name('Autoplay rate') ;
        gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
        gui.open();
    }
    else {
        var gui = dat.GUIVR.create('MuDEM');
        dat.GUIVR.enableMouse( camera, renderer );
        gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension') ;
        if (N > 3) {
            for (i=3;i<N;i++) {
                gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
            }
        }
        gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
        gui.add( time, 'rate').min(0).max(1.0).name('Autoplay rate') ;
        gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })

        gui.position.set(0,0,0.)
        gui.rotation.x = -Math.PI/3.;
        gui.scale.set(0.5,0.5,0.5);
        controller2.add( gui );
        var input1 = dat.GUIVR.addInputObject( controller1, renderer );
        //var input2 = dat.GUIVR.addInputObject( controller2 , renderer);
        scene.add( input1 );
        //scene.add( input2 );
    }
}

function add_controllers() {
    if (window.viewerType == "VR") {
        controller1 = renderer.vr.getController( 0 );
        controller2 = renderer.vr.getController( 1 );
        scene.add( controller1 );
        scene.add( controller2 );

        if ( view_mode === 'catch_particle' ) {
            controller1.addEventListener( 'selectstart', onSelectStart );
            controller1.addEventListener( 'selectend', onSelectEnd );
            controller2.addEventListener( 'selectstart', onSelectStart );
            controller2.addEventListener( 'selectend', onSelectEnd );
        }
        //
        controls = new THREE.TrackballControls( camera, renderer.domElement );
        aim_camera()
        console.log("VR mode loaded");

        if ( view_mode === 'catch_particle' ) {
            var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );

            var line = new THREE.Line( geometry );
            line.name = 'line';
            line.scale.z = 5;

            if (window.viewerType == "VR") {
                controller1.add( line.clone() );
                controller2.add( line.clone() );
                };
            raycaster = new THREE.Raycaster();
        }

    } else if (window.viewerType == 'keyboard') {
        if ( N < 3 ) {
            controls = new THREE.OrbitControls( camera, renderer.domElement );
            controls.target.set( (world[0].min + world[0].max)/2./2., (world[1].min + world[1].max)/2., 0 ); // view direction perpendicular to XY-plane. NOTE: VALUE OF 5 IS HARDCODED IN OTHER PLACES
            controls.enableRotate = false;
            camera.up.set(1,0,0);
        }
        else {
            controls = new THREE.TrackballControls( camera, renderer.domElement );
            aim_camera()
        }

        // var gui = new dat.GUI();
        console.log('Keyboard mode loaded');
    } else if (window.viewerType == 'anaglyph') {
        controls = new THREE.TrackballControls( camera, renderer.domElement );
        aim_camera()
        effect = new THREE.AnaglyphEffect( renderer );
        effect.setSize(window.innerWidth, window.innerHeight);
        console.log('Anaglyph mode loaded');
    };
}
function aim_camera() {
    if ( N > 2 ) {
        controls.target0.set(
            (world[0].min + world[0].max)/2./2., // NOTE: HARDCODED a FACTOR OF 2 BECAUSE OF CONSOLIDATION
            (world[1].min + world[1].max)/2.,
            (world[2].min + world[2].max)/2.,
        );
    }
    controls.up0.set( 1, 0, 0 ); // set x as up
    controls.reset();
}

function make_axes() {
    if (typeof axesLabels == 'undefined') {
        axeslength = 5 ; // length of axes vectors
        fontsize = 0.5; // font size
        thickness = 0.1; // line thickness
        axesHelper = new THREE.Group();
        axesLabels = new THREE.Group();
        // axesHelper = new THREE.AxesHelper( axeslength ); // X - red, Y - green, Z - blue
        // axesHelper.position.set(world[ref_dim.N_0])
        scene.add( axesHelper );
        scene.add( axesLabels );

        var arrow_body = new THREE.CylinderGeometry( thickness, thickness, axeslength, Math.pow(2,quality), Math.pow(2,quality) );
        var arrow_head = new THREE.CylinderGeometry( 0., 2.*thickness, 4*thickness, Math.pow(2,quality), Math.pow(2,quality) );
        var arrow_material_x = new THREE.MeshPhongMaterial( { color: 0xff0000 } );
        var arrow_material_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
        var arrow_material_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
        var arrow_x = new THREE.Mesh( arrow_body, arrow_material_x );
        var arrow_y = new THREE.Mesh( arrow_body, arrow_material_y );
        var arrow_z = new THREE.Mesh( arrow_body, arrow_material_z );
        var arrow_head_x = new THREE.Mesh( arrow_head, arrow_material_x );
        var arrow_head_y = new THREE.Mesh( arrow_head, arrow_material_y );
        var arrow_head_z = new THREE.Mesh( arrow_head, arrow_material_z );

        arrow_x.position.x = axeslength/2.;
        arrow_x.rotation.z = -Math.PI/2.;
        arrow_head_x.position.x = axeslength+thickness;
        arrow_head_x.rotation.z = -Math.PI/2.;

        arrow_y.position.y = axeslength/2.;
        arrow_head_y.position.y = axeslength+thickness;

        arrow_z.position.z = axeslength/2.;
        arrow_z.rotation.x = -Math.PI/2.;
        arrow_head_z.position.z = axeslength+thickness;
        arrow_head_z.rotation.x = Math.PI/2.;

        if ( N == 1 ) {
            arrow_x.position.y = -1.5;
            arrow_head_x.position.y = -1.5;
        }

        axesHelper.add( arrow_x );
        axesHelper.add( arrow_head_x );

        if ( N > 1 ) {
            axesHelper.add( arrow_y );
            axesHelper.add( arrow_head_y );
        };
        if ( N > 2 ) {
            axesHelper.add( arrow_z );
            axesHelper.add( arrow_head_z );
        };


    }

    if (ref_dim.c != ref_dim.x) {
        ref_dim.x = ref_dim.c;
        if (ref_dim.c < N - 1) { ref_dim.y = ref_dim.c + 1; }
        else { ref_dim.y = ref_dim.c + 1 - N; }
        if (ref_dim.c < N - 2) { ref_dim.z = ref_dim.c + 2; }
        else { ref_dim.z = ref_dim.c + 2 - N; }

        if (axesLabels.children.length > 0 ) {
            for( var i = axesLabels.children.length - 1; i >= 0; i--) {
                obj = axesLabels.children[i];
                axesLabels.remove(obj);
            }
        }
        var loader = new THREE.FontLoader();
    	loader.load( 'http://localhost:8000/visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json', function ( font ) {
    		var textGeo_x = new THREE.TextBufferGeometry( "x" + ref_dim.x, { font: font, size: fontsize, height: fontsize/2., } );
    		var textMaterial_x = new THREE.MeshPhongMaterial( { color: 0xff0000 } );
    		var mesh_x = new THREE.Mesh( textGeo_x, textMaterial_x );
    		mesh_x.position.x = axeslength;// - 1.5*fontsize;
            mesh_x.position.y = -0.3;
            mesh_x.position.z = fontsize/4.;
            mesh_x.rotation.z = Math.PI;
            mesh_x.rotation.y = Math.PI;
            if ( N == 1 ) {
                mesh_x.position.y = -1.8;
            }
    		axesLabels.add( mesh_x );

            if ( N > 1 ) {
                var textGeo_y = new THREE.TextGeometry( "x" + ref_dim.y, { font: font, size: fontsize, height: fontsize/2., } );
        		var textMaterial_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
        		var mesh_y = new THREE.Mesh( textGeo_y, textMaterial_y );
                mesh_y.position.x = 0.3;
                mesh_y.position.y = axeslength;//-fontsize*1.5;
                mesh_y.position.z = fontsize/4.;
                mesh_y.rotation.z = -Math.PI/2.;
                mesh_y.rotation.x = Math.PI;
        		axesLabels.add( mesh_y );
            };
            if ( N > 2 ) {
                var textGeo_z = new THREE.TextGeometry( "x" + ref_dim.z, { font: font, size: fontsize, height: fontsize/2., } );
        		var textMaterial_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
        		var mesh_z = new THREE.Mesh( textGeo_z, textMaterial_z );
                mesh_z.position.x = 0.3;
                mesh_z.position.y = fontsize/4.;
                mesh_z.position.z = axeslength + 1.5*fontsize;
                mesh_z.rotation.z = -Math.PI/2.;
                mesh_z.rotation.x = Math.PI/2.;
        		axesLabels.add( mesh_z );
            }
        });

    }
}

function make_lights() {
    var background_light = new THREE.AmbientLight( 0x909090 );
    scene.add( background_light );

    var light = new THREE.DirectionalLight( 0xffffff );
    light.position.set( world[0].max, -world[0].max, ( world[2].min + world[2].max)/2. );
    light.lookAt(( world[2].min + world[2].max)/2.,( world[2].min + world[2].max)/2.,( world[2].min + world[2].max)/2.)

    var light1 = new THREE.DirectionalLight( 0xffffff );
    light1.position.set( world[0].max, world[0].max, ( world[2].min + world[2].max)/2. );
    light1.lookAt(( world[2].min + world[2].max)/2.,( world[2].min + world[2].max)/2.,( world[2].min + world[2].max)/2.)


    if ( shadows ) {
        light.castShadow = true;
        light.shadow.mapSize.set( 4096, 4096 );
    };

    scene.add( light );
    scene.add( light1 );
}

function make_walls() {
    var geometry = new THREE.PlaneBufferGeometry( 1, 1 );
    var material = new THREE.MeshPhongMaterial( {
        color: 0xeeeeee,
        // roughness: 1.0,
        // metalness: 0.0,
    } );
    material.transparent = true;
    material.opacity = 0.5;
    material.side = THREE.DoubleSide;

    var floor = new THREE.Mesh( geometry, material );
    floor.rotation.x = - Math.PI / 2;
    floor.position.set(0.5,0,0.5)
    scene.add( floor );

    var roof = new THREE.Mesh( geometry, material );
    roof.rotation.x = - Math.PI / 2;
    roof.position.set(0.5,1,0.5)
    scene.add( roof );

    var left_wall = new THREE.Mesh( geometry, material );
    left_wall.rotation.y = - Math.PI / 2;
    left_wall.position.set(0.5,1,0.5)
    scene.add( left_wall );

    var right_wall = new THREE.Mesh( geometry, material );
    right_wall.rotation.y = - Math.PI / 2;
    right_wall.position.set(0.5,1,0.5)
    scene.add( right_wall );

    if (N > 2) {
        var front_wall = new THREE.Mesh( geometry, material );
        front_wall.position.set(0.5,1,0.5)
        scene.add( front_wall );

        var back_wall = new THREE.Mesh( geometry, material );
        front_wall.position.set(0.5,1,0.5)
        scene.add( back_wall );
    }

}

function add_torus() {
    if (window.viewerType == "VR") { R = 0.1; }
    else { R = 0.5; }
    r = R/2.;
    var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality)*2, Math.pow(2,quality) );
    var material = new THREE.MeshPhongMaterial( {
        color: 0xffffff,
        // roughness: 0.7,
        // metalness: 0.5
    } );

    wristband = new THREE.Mesh( geometry, material );
    if ( shadows ) {
        wristband.castShadow = true;
        wristband.receiveShadow = true;
    }

    var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality)*2, Math.pow(2,quality) );
    var material = new THREE.MeshPhongMaterial( {
        color: 0x000000,
        // roughness: 0.7,
    } );
    wristband_phi = new THREE.Mesh( geometry, material );

    var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality)*2, Math.pow(2,quality) );
    wristband_theta = new THREE.Mesh( geometry, material );
    wristband_theta.rotation.y = Math.PI/2;


    if (window.viewerType == "VR") {
        wristband.position.set(0.,0.,0.);
        wristband_phi.position.set(0.,0.,0.);
        wristband_theta.position.set(0.,R,0.);
        controller1.add( wristband );
        controller1.add( wristband_phi );
        controller1.add( wristband_theta );
    }
    else {
        wristband.position.set(      -3*R,0.5,  0.5);
        wristband_phi.position.set(  -3*R,0.5,  0.5);
        wristband_theta.position.set(-3*R,R+0.5,0.5);
        scene.add( wristband );
        scene.add( wristband_phi );
        scene.add( wristband_theta );
    }

    if ( N > 5 ) {
        var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0xffffff,
            // roughness: 0.7,
            // metalness: 0.5
        } );

        wristband1 = new THREE.Mesh( geometry, material );
        if ( shadows ) {
            wristband1.castShadow = true;
            wristband1.receiveShadow = true;
        }

        var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0x000000,
            // roughness: 0.7,
        } );
        wristband1_phi = new THREE.Mesh( geometry, material );

        var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality)*2, Math.pow(2,quality) );
        wristband1_theta = new THREE.Mesh( geometry, material );
        wristband1_theta.rotation.y = Math.PI/2;


        if (window.viewerType == "VR") {
            wristband1.position.set(0.,0.,0.);
            wristband1_phi.position.set(0.,0.,0.);
            wristband1_theta.position.set(0.,R,0.);
            controller2.add( wristband1 );
            controller2.add( wristband1_phi );
            controller2.add( wristband1_theta );
        }
        else {
            wristband1.position.set(      -3*R,2.5,  0.5);
            wristband1_phi.position.set(  -3*R,2.5,  0.5);
            wristband1_theta.position.set(-3*R,R+2.5,0.5);
            scene.add( wristband1 );
            scene.add( wristband1_phi );
            scene.add( wristband1_theta );
        }
    }

}

function remove_everything() {
    window.addEventListener("message", receiveMessage, false);
    function receiveMessage(event) {
        // console.log(particles);
        // console.log(wristband);
        // console.log(wristband1);
        // remove all particles
        for (i = particles.children.length; i = 0; i--) {
            var object = particles.children[i];
            object.geometry.dispose();
            object.material.dispose();
            if ( view_mode === 'rotations' ) { object.texture.dispose(); }
        }
        if ( N > 3) {
            for (i = wristband.children.length; i = 0; i--) {
                var object = controller1.children[i];
                object.geometry.dispose();
                object.material.dispose();
            }
        };
        if ( N > 5 ) {
            for (i = wristband1.children.length; i = 0; i--) {
                var object = controller1.children[i];
                object.geometry.dispose();
                object.material.dispose();
            }
        }
        renderer.dispose();
    }
}

function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load("http://localhost:8000/visualise/data/vtk//dump-00000.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};

function make_initial_spheres_CSV() {
    if ( view_mode === 'rotations' ) {
        var arr = new Array();
        for ( i=0; i<N; i++ ) {
            if ( i < 3 ) { arr.push('NaN') }
            else { arr.push(world[i].cur) };
        };
        var request = new XMLHttpRequest();
        request.open('POST', "http://localhost:8000/make_textures?" +
                     "arr=" + JSON.stringify(arr) +
                     "&N=" + N +
                     "&t=" + "0" +
                     "&fname=" + fname,
                     true);
        request.send(null);
        // request.onreadystatechange = function () {}
    };
    if ( cache ) {
        var filename = "http://localhost:8000/" + window.fname + "dump-00000.csv";
    }
    else {
        var filename = "http://localhost:8000/" + window.fname + "dump-00000.csv" + "?_="+ (new Date).getTime();
    }
    Papa.parse(filename, {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: cache,
        complete: function(results) {
            particles = new THREE.Group();
            scene.add( particles );
            spheres = results.data;
            if ( N == 1) {
                var geometry = new THREE.CylinderGeometry( 1, 1, 2, Math.pow(2,quality), Math.pow(2,quality) );
            }
            else {
                var geometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
            }
            var pointsGeometry = new THREE.SphereGeometry( 1, Math.max(Math.pow(2,quality-2),4), Math.max(Math.pow(2,quality-2),4) );
            var scale = 20.; // size of particles on tori
            for (var i = 0; i<spheres.length; i++) {
                if ( N === 2 ) {
                    var color = Math.random() * 0xffffff;
                    var material = new THREE.PointsMaterial( {
                        color: color,
                    } );
                }
                else {
                    if ( find_particle ) {
                        if (i == 0) {
                            var material = new THREE.MeshPhongMaterial( { color: 0xFF00FF,
                                                                             // roughness: 0.7,
                                                                             // metalness: 0.0
                                                                         } ); }
                        else { var material = new THREE.MeshPhongMaterial( { color: 0xffffff,
                                                                                // roughness: 0.7,
                                                                                // metalness: 0.0
                                                                            } ); }
                    }
                    else {
                        var color = Math.random() * 0xffffff;
                        if ( view_mode === 'rotations' ) {
                            var texture = new THREE.TextureLoader().load("http://localhost:8000/" + window.fname + "Texture-0.png");
                            var material = new THREE.MeshBasicMaterial( { map: texture } );
                        }
                        else {
                            var material = new THREE.MeshPhongMaterial( {
                                color: color,
                                // roughness: 0.2,
                                // metalness: 0.5
                            } );
                        }
                    };
                }
                var object = new THREE.Mesh( geometry, material );
                object.rotation.z = Math.PI/2.;
                if ( shadows ) {
                    object.castShadow = true;
                    object.receiveShadow = true;
                }
                particles.add( object );
                if ( N > 3 ) {
                    pointsMaterial = new THREE.PointsMaterial( { color: color } );
                    object2 =  new THREE.Mesh( pointsGeometry, pointsMaterial );
                    object2.scale.set(R/scale,R/scale,R/scale);
                    object2.position.set(0.,0.,0.);
                    wristband.add(object2);
                    if ( N > 5 ) {
                        object3 = object2.clone();
                        wristband1.add(object3);
                    }
                }
            }
        }
    });
};

function update_spheres_CSV(t) {
    if ( view_mode === 'rotations' ) {
        var arr = new Array();
        for ( var i=0; i<N; i++ ) {
            if ( i < 3 ) { arr.push('NaN') }
            else { arr.push(world[i].cur) };
        };
        var request = new XMLHttpRequest();
        request.open('POST',
                     "http://localhost:8000/make_textures?" +
                     "arr=" + JSON.stringify(arr) +
                     "&N=" + N +
                     "&t=" + t + "0000" +
                     "&fname=" + fname,
                     true);
        request.onload = function() {
            var loader = new THREE.TextureLoader();
            for ( ii = 0; ii < particles.children.length; ii++ ) {
                if ( cache ) {
                    var filename = "http://localhost:8000/" + window.fname + "Texture-" + ii + ".png";
                }
                else {
                    var filename = "http://localhost:8000/" + window.fname + "Texture-" + ii + ".png" + "?_="+ (new Date).getTime();
                }
                loader.load(filename,
                            function( texture ) {
                                var myRe = /-[0-9]+.png/g
                                var res=myRe.exec(texture.image.currentSrc)
                                var myRe2 = /[0-9]+/
                                var iii = myRe2.exec(res[0])[0]
                                var o = particles.children[iii];
                                o.material.map = texture;
                                o.material.map.needsUpdate = true;
                            });
            }
        }
        request.send('');
    };
    if ( cache ) {
        var filename = "http://localhost:8000/" + window.fname + "dump-"+t+"0000.csv";
    }
    else {
        var filename = "http://localhost:8000/" + window.fname + "dump-"+t+"0000.csv"+"?_="+ (new Date).getTime();
    }
    Papa.parse(filename, {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: cache,
        complete: function(results) {
            spheres = results.data;
            for (i = 0; i<spheres.length; i++) {
                var object = particles.children[i];
                if ( N == 1 ) { spheres[i].x1 = 0; };
                if ( N < 3 ) { spheres[i].x2 = 0; };
                object.position.set(spheres[i].x0,spheres[i].x1,spheres[i].x2);
                if (N < 4) {
                    var R_draw = spheres[i].R;
                             }
                else if (N == 4) {
                    var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                            Math.pow( (world[3].cur - spheres[i].x3), 2)
                                          );
                             }
                 else if (N == 5) {
                     var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                             Math.pow( (world[3].cur - spheres[i].x3), 2) -
                                             Math.pow( (world[4].cur - spheres[i].x4), 2)
                                         ); // FIXME - IS THIS RIGHT?
                 }
                 else if (N == 6) {
                     var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                             Math.pow( (world[3].cur - spheres[i].x3), 2) -
                                             Math.pow( (world[4].cur - spheres[i].x4), 2) -
                                             Math.pow( (world[5].cur - spheres[i].x5), 2)
                                         ); // FIXME - IS THIS RIGHT?
                 }
                 else if (N == 7) {
                     var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                             Math.pow( (world[3].cur - spheres[i].x3), 2) -
                                             Math.pow( (world[4].cur - spheres[i].x4), 2) -
                                             Math.pow( (world[5].cur - spheres[i].x5), 2) -
                                             Math.pow( (world[6].cur - spheres[i].x6), 2)
                                         ); // FIXME - IS THIS RIGHT?
                 }
                if (isNaN(R_draw)) {
                    object.visible = false;
                }
                else {
                    object.visible = true;
                    object.scale.set(R_draw,R_draw,R_draw);
                    if ( view_mode === 'velocity' ) {
                        lut.setMax(1.); // TODO: MOVE TO GUI
                        object.material.color = lut.getColor(spheres[i].Vmag);
                    }
                    else if ( view_mode === 'rotation_rate' ) {
                        lut.setMax(1.); // TODO: MOVE TO GUI
                        object.material.color = lut.getColor(spheres[i].Omegamag);
                    }
                }

                if ( N == 4 ) {
                    var object2 = wristband.children[i];
                    phi = 2.*Math.PI*( world[3].cur - spheres[i].x3 )/(world[3].max - world[3].min) + Math.PI/2.;
                    x = (R + r)*Math.cos(phi);
                    y = (R + r)*Math.sin(phi);
                    z = 0.;
                    object2.position.set(x,y,z);
                };

                if ( N > 4 ) {
                    var object2 = wristband.children[i];
                    phi   = 2.*Math.PI*(world[3].cur - spheres[i].x3)/(world[3].max - world[3].min) + Math.PI/2.;
                    theta = 2.*Math.PI*(world[4].cur - spheres[i].x4)/(world[4].max - world[4].min) ;
                    x = (R + r*Math.cos(theta))*Math.cos(phi);
                    y = (R + r*Math.cos(theta))*Math.sin(phi);
                    z = r*Math.sin(theta);
                    object2.position.set(x,y,z);
                };

                if ( N == 6 ) {
                    var object3 = wristband1.children[i];
                    phi = 2.*Math.PI*( world[5].cur - spheres[i].x5 )/(world[5].max - world[5].min) + Math.PI/2.;
                    x = (R + r)*Math.cos(phi);
                    y = (R + r)*Math.sin(phi);
                    z = 0.;
                    object3.position.set(x,y,z);
                };

                if ( N == 7 ) {
                    var object3 = wristband1.children[i];
                    phi   = 2.*Math.PI*(world[5].cur - spheres[i].x5)/(world[5].max - world[5].min) + Math.PI/2.;
                    theta = 2.*Math.PI*(world[6].cur - spheres[i].x6)/(world[6].max - world[6].min) ;
                    x = (R + r*Math.cos(theta))*Math.cos(phi);
                    y = (R + r*Math.cos(theta))*Math.sin(phi);
                    z = r*Math.sin(theta);
                    object3.position.set(x,y,z);
                };
            }
        }
    });
};

function onWindowResize() {
    if ( N < 3 ) {
        var aspect = window.innerWidth / window.innerHeight;
        var zoom = 10.
        camera.left   = -zoom*aspect;
        camera.right  =  zoom*aspect;
        camera.bottom = -zoom;
        camera.top    =  zoom;
    }
    else {
        camera.aspect = window.innerWidth / window.innerHeight;
    }

    camera.updateProjectionMatrix();
    renderer.setSize( window.innerWidth, window.innerHeight );
    controls.handleResize();
    if (window.viewerType == 'anaglyph') { effect.setSize( window.innerWidth, window.innerHeight ); };
    render();
};

function onSelectStart( event ) {
    var controller = event.target;
    var intersections = getIntersections( controller );
    if ( intersections.length > 0 ) {
        var intersection = intersections[ 0 ];
        tempMatrix.getInverse( controller.matrixWorld );
        var object = intersection.object;
        object.matrix.premultiply( tempMatrix );
        object.matrix.decompose( object.position, object.quaternion, object.scale );
        object.material.emissive.b = 1;
        controller.add( object );
        controller.userData.selected = object;
    }
};

function onSelectEnd( event ) {
    var controller = event.target;
    if ( controller.userData.selected !== undefined ) {
        var object = controller.userData.selected;
        object.matrix.premultiply( controller.matrixWorld );
        object.matrix.decompose( object.position, object.quaternion, object.scale );
        object.material.emissive.b = 0;
        particles.add( object );
        controller.userData.selected = undefined;
    }
};

function getIntersections( controller ) {

    tempMatrix.identity().extractRotation( controller.matrixWorld );

    raycaster.ray.origin.setFromMatrixPosition( controller.matrixWorld );
    raycaster.ray.direction.set( 0, 0, - 1 ).applyMatrix4( tempMatrix );

    return raycaster.intersectObjects( particles.children );

}

function intersectObjects( controller ) {

    // Do not highlight when already selected

    if ( controller.userData.selected !== undefined ) return;

    var line = controller.getObjectByName( 'line' );
    var intersections = getIntersections( controller );

    if ( intersections.length > 0 ) {

        var intersection = intersections[ 0 ];

        var object = intersection.object;
        object.material.emissive.r = 1;
        intersected.push( object );

        line.scale.z = intersection.distance;

    } else {

        line.scale.z = 5;

    }

}

function cleanIntersected() {
    while ( intersected.length ) {
        var object = intersected.pop();
        object.material.emissive.r = 0;
    }
};

//

function animate() {
    if (N > 3) {
        for (i=3;i<N;i++) {
            if (world[i].cur != world[i].prev) {
                update_spheres_CSV(Math.floor(time.cur));
                world[i].prev = world[i].cur;
            }
        }
    }
    if (time.play) { time.cur += time.rate; };
    if (Math.floor(time.cur) != time.prev) {
        update_spheres_CSV(Math.floor(time.cur));
        time.prev = time.cur;
    }
    if (time.cur > time.max) { time.cur -= time.max; }
    requestAnimationFrame( animate );
    if ( controls !== 'undefined' ) { controls.update(); }
    renderer.setAnimationLoop( render );
};

function render() {
    if ( view_mode === 'catch_particle' ) {
        cleanIntersected();
        intersectObjects( controller1 );
        intersectObjects( controller2 );
    }
    if (window.viewerType == "anaglyph") { effect.render( scene, camera ); }
    else { renderer.render( scene, camera ); }
};
