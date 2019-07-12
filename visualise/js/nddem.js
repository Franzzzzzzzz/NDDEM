// var THREE = require('three');

var container; // main div element
var camera, scene, controls, renderer; // UI elements
var controller1, controller2; // VR controllers
var raycaster, intersected = []; // catching grains
var tempMatrix = new THREE.Matrix4(); // catching grains
var particles, wristband1, wristband2, axesHelper, axesLabels; // groups of objects
var R,r; // parameters of torus
var N; // number of dimensions
var world = []; // properties that describe the domain
var ref_dim = {'c': 1} //, 'x': 00, 'y': 1, 'z': 2}; // reference dimensions
var time = {'cur': 0, 'prev': 0, 'min':0, 'max': 99, 'play': false, 'rate': 0.5} // temporal properties
if ( typeof window.autoplay !== 'undefined' ) { time.play = window.autoplay === 'true' };
if ( typeof window.rate !== 'undefined' ) { time.rate = parseFloat(window.rate) };
var axeslength, fontsize; // axis properties
var vr_scale = 0.5; // mapping from DEM units to VR units
var human_height = 1.8; // height of the human in m
var view_mode = window.view_mode; // options are: undefined (normal), catch_particle, rotations, velocity, rotation_rate
var quality, shadows, timestep;
var velocity = {'vmax': 1, 'omegamax': 1} // default GUI options
var roof, bg;
var redraw_left = false; // force redrawing of particles
var redraw_right = false;
var left_hand, right_hand;
var hard_mode;
if ( typeof window.zoom !== 'undefined' ) { var zoom = parseFloat(window.zoom); }
else { var zoom = 20; } // default zoom level
if ( typeof window.shadows !== 'undefined' ) { shadows = window.shadows == 'true' }
else { shadows = true; };
if ( typeof window.quality !== 'undefined' ) { quality = parseInt(window.quality) }
else { quality = 5}; // quality flag - 5 is default, 8 is ridiculous
if ( typeof window.pinky !== 'undefined' ) { pinky = parseInt(window.pinky) }
else { pinky = 100}; // which particle to catch

var fname = window.fname;
if (fname.substr(-1) != '/') { fname += '/' }; // add trailing slash if required


var lut = new THREE.Lut( "blackbody", 512 ); // options are rainbow, cooltowarm and blackbody
var arrow_material;
if ( typeof window.cache !== 'undefined' ) { cache = window.cache == 'true' }
else { cache = false; };
if ( typeof window.hard_mode !== 'undefined' ) { hard_mode = window.hard_mode == 'true'; }
else { hard_mode = false; }

init();

function init() {
    var request = new XMLHttpRequest();
    //request.open('GET', "http://localhost:54321/Samples/" + fname + "in?_="+ (new Date).getTime(), true);
    request.open('GET', "http://localhost:54321/Samples/" + fname + "in", true);
    request.send(null);
    request.onreadystatechange = function () {
        if (request.readyState === 4 && request.status === 200) {
            var type = request.getResponseHeader('Content-Type');
            if (type.indexOf("text") !== 1) {
                lines = request.responseText.split('\n');
                for (i=0;i<lines.length;i++) {
                    // console.log(lines[i])
                    line = lines[i].replace(/ {1,}/g," "); // remove multiple spaces
                    l = line.split(' ')
                    if (l[0] == 'dimensions') {
                        N = parseInt(l[1]);
                        for (j=0;j<N;j++) {
                            world.push({});
                            world[j].min = 0.;
                            world[j].max = 1.;
                            world[j].cur = 0.5;
                            world[j].prev = 0.5;
                            world[j].wall = false;
                        }
                    }
                    else if (l[0] == 'boundary') {
                        if (l[2] == 'WALL' || l[2] == 'PBC') {
                            world[l[1]].min = parseFloat(l[3]);
                            world[l[1]].max = parseFloat(l[4]);
                            world[l[1]].cur = (world[l[1]].min + world[l[1]].max)/2.;
                            world[l[1]].prev = world[l[1]].cur;
                        }
                        if ( l[2] == 'WALL' ) { world[l[1]].wall = true; }
                    }
                    else if (l[0] == 'set') {
                        if (l[1] == 'T') { time.max = parseInt(l[2]) - 1; }
                        else if (l[1] === 'tdump') { timestep = parseInt(l[2]) }
                    }
                    else if (l[0] == 'freeze') { pinky = parseInt(l[1]); }
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

    N_tag = document.createElement( 'div' );
    N_tag.setAttribute("id", "N_tag");
    N_tag.innerHTML = N + "D";
    container.appendChild(N_tag);

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x111111 ); // revealjs background colour
    // scene.background = new THREE.Color( 0xFFFFFF ); // white
    if ( window.display_type === 'VR' ) {
        var geometry = new THREE.SphereBufferGeometry( 500, 60, 40 );
        // invert the geometry on the x-axis so that all of the faces point inward
        geometry.scale( - 1, 1, 1 );
        var texture = new THREE.TextureLoader().load( 'http://localhost:54321/visualise/resources/eso0932a.jpg' );
        var material = new THREE.MeshBasicMaterial( { map: texture } );
        bg = new THREE.Mesh( geometry, material );
        bg.rotation.z = Math.PI/2; // TODO: CHECK THIS!
        scene.add(bg);

        controller1 = new THREE.Object3D;
        controller2 = new THREE.Object3D;
    }


    make_camera();
    make_walls();
    if ( !fname.includes('Spinner') ) { make_axes(); }
    make_lights();
    add_renderer();
    if ( !fname.includes('Submarine') ) { add_controllers(); }
    // add_controllers();
    if ( N > 3 && !fname.includes('Spinner') && !hard_mode) { add_torus(); }
    // load_hyperspheres_VTK();
    if ( view_mode === 'rotations' ) { make_initial_sphere_texturing(); }
    else { make_initial_spheres_CSV(); update_spheres_CSV(0,false);}
    //update_spheres_CSV(0,false);
    add_gui();
    window.addEventListener( 'resize', onWindowResize, false );
    if ( window.display_type === 'VR' ) { add_vive_models(); }

    window.addEventListener( 'vr controller connected', function( event ){
    	//  Here it is, your VR controller instance.
    	//  It’s really a THREE.Object3D so you can just add it to your scene:
    	var controller = event.detail
        if ( controller.gamepad.hand === 'left' ) {
            controller.add(controller1);
            left_hand = new THREE.Object3D;
            left_hand.previous_torus_rotation_z = 0.;
            left_hand.previous_torus_rotation_y = 0.;
            left_hand.new_orientation = 0.;
            left_hand.previous_direction = new THREE.Quaternion();
            left_hand.current_direction  = new THREE.Quaternion();
            left_hand.diff               = new THREE.Quaternion();
            left_hand.diff_angle = new THREE.Euler();
            console.log('Added left hand'); }
        else if ( controller.gamepad.hand === 'right' ) {
            controller.add(controller2);
            right_hand = new THREE.Object3D;
            right_hand.previous_torus_rotation_z = 0.;
            right_hand.previous_torus_rotation_y = 0.;
            right_hand.new_orientation = 0.;
            right_hand.previous_direction = new THREE.Quaternion();
            right_hand.current_direction  = new THREE.Quaternion();
            right_hand.diff               = new THREE.Quaternion();
            right_hand.diff_angle = new THREE.Euler();
            console.log('Added right hand'); }
    	scene.add( controller )
    	controller.standingMatrix = renderer.vr.getStandingMatrix()
    	controller.head = window.camera

    	//  Allow this controller to interact with DAT GUI.
    	//var guiInputHelper = dat.GUIVR.addInputObject( controller )
    	//scene.add( guiInputHelper )
    	//  Button events. How easy is this?!
    	//  We’ll just use the “primary” button -- whatever that might be ;)
    	//  Check out the THREE.VRController.supported{} object to see
    	//  all the named buttons we’ve already mapped for you!



    	controller.addEventListener( 'primary press began', function( event ){
            if ( controller.gamepad.hand === 'left' ) {
                if ( N > 3 ) {
                    redraw_left = true;
                    controller1.getWorldQuaternion(left_hand.previous_direction);
                    left_hand.previous_torus_rotation_z = wristband1.rotation.z;
                }
                if ( N > 4 ) { left_hand.previous_torus_rotation_x = (world[4].cur - world[4].min)/(world[4].max - world[4].min)*2*Math.PI; }
            }
            else {
                if ( N > 5 ) {
                    redraw_right = true;
                    controller2.getWorldQuaternion(right_hand.previous_direction);
                    right_hand.previous_torus_rotation_z = wristband2.rotation.z;
                }
                if ( N > 6 ) { right_hand.previous_torus_rotation_x = (world[6].cur - world[6].min)/(world[6].max - world[6].min)*2*Math.PI; }
            }
    		//guiInputHelper.pressed( true )
    	})
    	controller.addEventListener( 'primary press ended', function( event ){
            if ( controller.gamepad.hand === 'left' ) {
                redraw_left = false;
            }
            else {
                redraw_right = false;
            }
    		//guiInputHelper.pressed( false )
    	})
        controller.addEventListener( 'thumbpad press began', function( event ){
            time.play = !time.play;
            //guiInputHelper.pressed( true )
        })

    	controller.addEventListener( 'disconnected', function( event ){
    		controller.parent.remove( controller )
    	})
    })

}

function update_higher_dims_left() {
    controller1.getWorldQuaternion(left_hand.current_direction);
    left_hand.diff = left_hand.current_direction.inverse().multiply(left_hand.previous_direction);
    left_hand.diff_angle.setFromQuaternion(left_hand.diff);// + Math.PI;// between 0 and 2 Pi

    // move in D4 by rotations in z
    if ( N > 3 ) {
        left_hand.new_orientation = left_hand.previous_torus_rotation_z + left_hand.diff_angle.z;
        if      ( left_hand.new_orientation < 0.         )  { left_hand.new_orientation += 2.*Math.PI; }
        else if ( left_hand.new_orientation > 2.*Math.PI )  { left_hand.new_orientation -= 2.*Math.PI; }
        world[3].cur = left_hand.new_orientation*(world[3].max - world[3].min)/Math.PI/2.;
        wristband1.rotation.z = left_hand.new_orientation;
    }
    // move in D5 by rotations in y
    if ( N > 4 ) {
        // left_hand.new_orientation = left_hand.previous_torus_rotation_y + 2.*left_hand.diff_angle.y*(world[4].max - world[4].min)/Math.PI;
        // if      ( left_hand.new_orientation < world[4].min )  { left_hand.new_orientation += (world[4].max - world[4].min); }
        // else if ( left_hand.new_orientation > world[4].max )  { left_hand.new_orientation -= (world[4].max - world[4].min); }
        // world[4].cur = left_hand.new_orientation;
        left_hand.new_orientation = left_hand.previous_torus_rotation_x + 2.*left_hand.diff_angle.x; // double rotation in reality
        if      ( left_hand.new_orientation < 0.         )  { left_hand.new_orientation +=  2.*Math.PI; }
        else if ( left_hand.new_orientation > 2.*Math.PI )  { left_hand.new_orientation -=  2.*Math.PI; }
        world[4].cur = left_hand.new_orientation*(world[4].max - world[4].min)/Math.PI/2.;

    }
}

function update_higher_dims_right() {
    controller2.getWorldQuaternion(right_hand.current_direction);
    right_hand.diff = right_hand.current_direction.inverse().multiply(right_hand.previous_direction);
    right_hand.diff_angle.setFromQuaternion(right_hand.diff);// + Math.PI;// between 0 and 2 Pi

    // move in D4 by rotations in z
    if ( N > 5 ) {
        right_hand.new_orientation = right_hand.previous_torus_rotation_z + right_hand.diff_angle.z;
        if      ( right_hand.new_orientation < 0.         )  { right_hand.new_orientation += 2.*Math.PI; }
        else if ( right_hand.new_orientation > 2.*Math.PI )  { right_hand.new_orientation -= 2.*Math.PI; }
        world[5].cur = right_hand.new_orientation*(world[5].max - world[5].min)/Math.PI/2.;
        wristband2.rotation.z = right_hand.new_orientation;
    }
    // move in D4 by rotations in y
    if ( N > 6 ) {
        right_hand.new_orientation = right_hand.previous_torus_rotation_x + 2.*right_hand.diff_angle.x;
        if      ( right_hand.new_orientation < 0.         )  { right_hand.new_orientation +=  2.*Math.PI; }
        else if ( right_hand.new_orientation > 2.*Math.PI )  { right_hand.new_orientation -=  2.*Math.PI; }
        world[6].cur = right_hand.new_orientation*(world[6].max - world[6].min)/Math.PI/2.;
    }
}

function add_vive_models() {
    var loader = new THREE.OBJLoader();
		loader.setPath( 'http://localhost:54321/visualise/resources/vive/' );
		loader.load( 'vr_controller_vive_1_5.obj', function ( object ) {
			var loader = new THREE.TextureLoader();
			loader.setPath( 'http://localhost:54321/visualise/resources/vive/' );
			var controller = object.children[ 0 ];
			controller.material.map = loader.load( 'onepointfive_texture.png' );
			controller.material.specularMap = loader.load( 'onepointfive_spec.png' );
			controller.castShadow = true;
			controller.receiveShadow = true;

            // Pause label
            var font_loader = new THREE.FontLoader();
            font_loader.load( 'http://localhost:54321/visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json', function ( font ) {
                console.log('making pause');
                var fontsize = 0.005;
                var geometry = new THREE.TextBufferGeometry( "  Play \nPause", { font: font, size: fontsize, height: fontsize/5. } );
                var textMaterial = new THREE.MeshPhongMaterial( { color: 0xffffff } );
                var pause_label = new THREE.Mesh( geometry, textMaterial );
                pause_label.rotation.x = -Math.PI/2.;
                pause_label.position.y = fontsize;
                pause_label.position.x = -0.01;
                pause_label.position.z = 0.05;
                controller.add(pause_label);

                controller1.add( controller.clone() );
                controller2.add( controller.clone() );

                // Move label
                geometry = new THREE.TextBufferGeometry( "Move", { font: font, size: fontsize, height: fontsize/5. } );
                var move_label = new THREE.Mesh( geometry, textMaterial );
                move_label.rotation.x = -Math.PI/2.;
                move_label.rotation.y = Math.PI;
                move_label.position.y = -0.03 -fontsize;
                move_label.position.x = 0.01;
                move_label.position.z = 0.045;
                if ( N > 3 ) { controller1.add(move_label); }
                if ( N > 5 ) { controller2.add(move_label); }

                console.log('Added vive models to both controllers');
            });
		} );

}

function make_camera() {
    var aspect = window.innerWidth / window.innerHeight;
    if ( N < 3 ) {
        camera = new THREE.OrthographicCamera(-100.*aspect/zoom,100.*aspect/zoom,100./zoom,-100./zoom,-1000,1000);
        camera.position.set((world[0].min + world[0].max)/2./2.,(world[1].min + world[1].max)/2.,-1.0);
    }
    else {
        if ( fname.includes('Spinner') ) {
            camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
            camera.position.set(0,0,N);
        }
        else if ( fname.includes('Lonely') || fname.includes('Drops') ) {
            camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
            camera.position.set((world[0].min + world[0].max)/2.,(world[1].min + world[1].max)/2.,-world[0].max/zoom*25.);
        }
        else {
            if ( window.display_type == 'anaglyph' ) {
                camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
                camera.position.set(0.5*world[0].max/zoom,
                                       -world[0].max/zoom,
                                       -world[0].max/zoom
                                );
                camera.focalLength = 3;
            }
            else {
                camera = new THREE.PerspectiveCamera(70, aspect, 0.1, 1000 ); // fov, aspect, near, far
                camera.position.set(0.5*world[0].max/zoom*5,
                                       -world[0].max/zoom*5,
                                       -world[0].max/zoom*5
                                );
            }
        }
    }
}
function add_renderer() {
    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    // renderer.setSize( container.offsetWidth, container.offsetHeight );
    if ( shadows ) { renderer.shadowMap.enabled = true; }
    if (window.display_type == "VR") { renderer.vr.enabled = true; };
    container.appendChild( renderer.domElement );

    if (window.display_type == "VR") {
        container.appendChild( WEBVR.createButton( renderer ) );
        window.addEventListener('vrdisplayactivate', () => {
                renderer.vr.getDevice().requestPresent( [ { source: renderer.domElement } ] );
            }); // TODO - TOTALLY UNTESTED BUT SHOULD DROP YOU INTO VR AUTOMATICALLY. FROM HERE: https://github.com/mrdoob/three.js/issues/13105#issuecomment-373246458
    };
}

function add_gui() {
    if ( window.display_type == 'anaglyph' || window.display_type == 'keyboard' ) {
        var gui = new dat.GUI();
        //gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension').onChange( function( val ) { make_axes(); }) ;
        if (N > 3) {
            for (i=3;i<N;i++) {
                if ( view_mode === 'rotations' ) { gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).step(0.1).name('x'+(i+1)) ; }
                else { gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).step(0.01).name('x'+(i+1)) ; }
            }
        }
        gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
        gui.add( time, 'rate').min(0).max(1.0).name('Rate') ;
        gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
        if ( view_mode === 'velocity' ) {
            gui.add( velocity, 'vmax').name('Max vel').min(0).max(2).listen().onChange ( function() { update_spheres_CSV(Math.floor(time.cur),false); });
        }
        if ( view_mode === 'rotation_rate' ) {
            gui.add( velocity, 'omegamax').name('Max rot vel').min(0).max(20).listen().onChange ( function() { update_spheres_CSV(Math.floor(time.cur),false); });
        }
        gui.open();
    }
    // else {
    //     var gui = dat.GUIVR.create('MuDEM');
    //     dat.GUIVR.enableMouse( camera, renderer );
    //     gui.add( ref_dim, 'c').min(0).max(N-1).step(1).listen().name('Reference dimension') ;
    //     if (N > 3) {
    //         for (i=3;i<N;i++) {
    //             gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
    //         }
    //     }
    //     gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
    //     gui.add( time, 'rate').min(0).max(1.0).name('Autoplay rate') ;
    //     gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
    //
    //     gui.position.set(0,0,0.)
    //     gui.rotation.x = -Math.PI/3.;
    //     gui.scale.set(0.5,0.5,0.5);
    //     controller2.add( gui );
    //     var input1 = dat.GUIVR.addInputObject( controller1, renderer );
    //     document.addEventListener( 'mousedown', function(){ input1.pressed( true ); } ); // TODO: CAN I SOMEHOW USE THIS TO FAKE THE .pressed() IF I CAN MANUALLY PIPE THE A REAL PRESS EVENT??
    //     // see here: https://github.com/dataarts/dat.guiVR/wiki/Input-Support-(Vive-Controllers,-Mouse,-etc)
    //
    //     //var input2 = dat.GUIVR.addInputObject( controller2 , renderer);
    //     scene.add( input1 );
    //     //scene.add( input2 );
    // }
}

function add_controllers() {
    if (window.display_type == "VR") {
        // built in THREEjs
        //controller1 = renderer.vr.getController( 0 ); // JUST HAS ONE BUTTON MAPPED! - SEE WebVRManager
        //controller2 = renderer.vr.getController( 1 );
        // THREEJS example file
        //controller1 = new THREE.ViveController(0);
        //controller2 = new THREE.ViveController(1);
        // Stewdio version from https://github.com/stewdio/THREE.VRController
        //controller1 =
        // var geometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
        // var material = new THREE.MeshPhongMaterial( { color: 0xdddddd } );
        // var sphere = new THREE.Mesh( geometry, material );
        //controller1.add( sphere );
        //scene.add( controller1 );
        //scene.add( controller2 );

        //if ( view_mode === 'catch_particle' ) {
            //controller1.addEventListener( 'selectstart', onSelectStart ); // left hand
            //controller1.addEventListener( 'selectend', onSelectEnd );
            // controller1.addEventListener( 'gripsdown', leftTorusGripDown );
            // controller1.addEventListener( 'gripsup', leftTorusGripUp );
            // controller2.addEventListener( 'triggerdown', pauseOnTrigger ); // right hand

            //controller2.addEventListener( 'selectend', onSelectEnd );
        //}
        //
        controls = new THREE.TrackballControls( camera, renderer.domElement );
        aim_camera()
        console.log("VR mode loaded");

        if ( view_mode === 'catch_particle' ) {
            var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );

            var line = new THREE.Line( geometry );
            line.name = 'line';
            line.scale.z = 5;

            if (window.display_type == "VR") {
                controller1.add( line.clone() );
                controller2.add( line.clone() );
                };
            raycaster = new THREE.Raycaster();
        }

    } else if (window.display_type == 'keyboard') {
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
    } else if (window.display_type == 'anaglyph') {
        controls = new THREE.TrackballControls( camera, renderer.domElement );
        aim_camera()
        effect = new THREE.AnaglyphEffect( renderer );
        effect.setSize(window.innerWidth, window.innerHeight);
        console.log('Anaglyph mode loaded');
    };
}
function aim_camera() {
    if ( fname.includes('Lonely') || fname.includes('Drops')) {
        controls.target0.set(
            (world[0].min + world[0].max)/2., // NOTE: HARDCODED a FACTOR OF 2 BECAUSE OF CONSOLIDATION
            (world[1].min + world[1].max)/2.,
            (world[2].min + world[2].max)/2.,
        );
    }
    else if ( N > 2 ) {
        controls.target0.set(
            (world[0].min + world[0].max)/2./2., // NOTE: HARDCODED a FACTOR OF 2 BECAUSE OF CONSOLIDATION
            (world[1].min + world[1].max)/2.,
            (world[2].min + world[2].max)/2.,
        );
    }
    if ( fname.includes('Spinner') ) { controls.up0.set( 0, 1, 0 ); } // set x as up
    else { controls.up0.set( 1, 0, 0 ); }
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
        if ( window.display_type === 'VR' ) {
            axesHelper.position.set(0,-human_height,0);
            axesLabels.position.set(0,-human_height,0);
            axesHelper.scale.set(vr_scale,vr_scale,vr_scale);
            axesLabels.scale.set(vr_scale,vr_scale,vr_scale);
            axesLabels.rotation.z = Math.PI/2;
            axesLabels.rotation.y = Math.PI/2;
            thickness = 0.05; // line thickness
        }

        scene.add( axesHelper );
        scene.add( axesLabels );

        var arrow_body = new THREE.CylinderGeometry( thickness, thickness, axeslength, Math.pow(2,quality), Math.pow(2,quality) );
        var arrow_head = new THREE.CylinderGeometry( 0., 2.*thickness, 4*thickness, Math.pow(2,quality), Math.pow(2,quality) );
        arrow_material = new THREE.MeshPhongMaterial( { color: 0xdddddd } );
        // var arrow_material_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
        // var arrow_material_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
        var arrow_x = new THREE.Mesh( arrow_body, arrow_material );
        var arrow_y = new THREE.Mesh( arrow_body, arrow_material );
        var arrow_z = new THREE.Mesh( arrow_body, arrow_material );
        var arrow_head_x = new THREE.Mesh( arrow_head, arrow_material );
        var arrow_head_y = new THREE.Mesh( arrow_head, arrow_material );
        var arrow_head_z = new THREE.Mesh( arrow_head, arrow_material );

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
    	loader.load( 'http://localhost:54321/visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json', function ( font ) {
    		var textGeo_x = new THREE.TextBufferGeometry( "x" + ref_dim.x, { font: font, size: fontsize, height: fontsize/5., } );
    		var textMaterial_x = new THREE.MeshPhongMaterial( { color: 0xff0000 } );
    		var mesh_x = new THREE.Mesh( textGeo_x, arrow_material );
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
                var textGeo_y = new THREE.TextGeometry( "x" + ref_dim.y, { font: font, size: fontsize, height: fontsize/5., } );
        		var textMaterial_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
        		var mesh_y = new THREE.Mesh( textGeo_y, arrow_material );
                mesh_y.position.x = 0.3;
                mesh_y.position.y = axeslength;//-fontsize*1.5;
                mesh_y.position.z = fontsize/4.;
                mesh_y.rotation.z = -Math.PI/2.;
                mesh_y.rotation.x = Math.PI;
        		axesLabels.add( mesh_y );
            };
            if ( N > 2 ) {
                var textGeo_z = new THREE.TextGeometry( "x" + ref_dim.z, { font: font, size: fontsize, height: fontsize/5., } );
        		var textMaterial_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
        		var mesh_z = new THREE.Mesh( textGeo_z, arrow_material );
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
    var material = new THREE.MeshStandardMaterial( {
        color: 0xaaaaaa,
        // roughness: 1.0,
        // metalness: 1.0,
    } );
    material.transparent = true;
    material.opacity = 0.5;
    // material.side = THREE.DoubleSide;

    // for ( var i=0;i<N;i++ ) {
        // if ( world[i].wall ) {
                // do it nicely
        // }
    // }

    if ( world[0].wall ) {
        var floor = new THREE.Mesh( geometry, material );
        floor.scale.set(world[2].max - world[2].min,world[1].max - world[1].min,1)
        floor.rotation.y = + Math.PI / 2;
        floor.position.set(world[0].min,(world[1].max - world[1].min)/2.,(world[2].max - world[2].min)/2.)
        scene.add( floor );

        roof = new THREE.Mesh( geometry, material );
        roof.scale.set(world[2].max - world[2].min,world[1].max - world[1].min,1)
        roof.rotation.y = - Math.PI / 2;
        roof.position.set(world[0].max,(world[1].max - world[1].min)/2.,(world[2].max - world[2].min)/2.)
        if ( fname.includes('Uniaxial') ) {
            roof.material.side = THREE.DoubleSide;
            roof.material.opacity = 0.9;
            floor.material.side = THREE.DoubleSide;
            floor.material.opacity = 0.9;
        }
        scene.add( roof );
    }

    if ( world[1].wall ) {
        var left_wall = new THREE.Mesh( geometry, material );
        left_wall.scale.set(world[2].max - world[2].min,world[0].max - world[0].min,1)
        left_wall.rotation.x = - Math.PI / 2;
        left_wall.position.set((world[0].max - world[0].min)/2.,world[1].min,(world[2].max - world[2].min)/2.)
        scene.add( left_wall );

        var right_wall = new THREE.Mesh( geometry, material );
        right_wall.scale.set(world[2].max - world[2].min,world[0].max - world[0].min,1)
        right_wall.rotation.x = Math.PI / 2;
        right_wall.position.set((world[0].max - world[0].min)/2.,world[1].max,(world[2].max - world[2].min)/2.)
        scene.add( right_wall );
    }

    if (N > 2) {
        if ( world[2].wall ) {
            var front_wall = new THREE.Mesh( geometry, material );
            front_wall.scale.set(world[0].max - world[0].min,world[1].max - world[1].min,1)
            front_wall.position.set((world[0].max - world[0].min)/2.,(world[1].max - world[1].min)/2.,world[2].min)
            scene.add( front_wall );

            var back_wall = new THREE.Mesh( geometry, material );
            back_wall.scale.set(world[0].max - world[0].min,world[1].max - world[1].min,1)
            back_wall.rotation.y = Math.PI;
            back_wall.position.set((world[0].max - world[0].min)/2.,(world[1].max - world[1].min)/2.,world[2].max)
            scene.add( back_wall );
        }
    }

}

function add_torus() {
    if (window.display_type == "VR") { R = 0.1; }
    else { R = 0.5; }
    r = R/2.;
    var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality+1)*2, Math.pow(2,quality+1) );
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

    var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality+1)*2, Math.pow(2,quality+1) );
    var material = new THREE.MeshPhongMaterial( {
        color: 0x000000,
        // roughness: 0.7,
    } );
    wristband1_phi = new THREE.Mesh( geometry, material );

    var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality+1)*2, Math.pow(2,quality+1) );
    wristband1_theta = new THREE.Mesh( geometry, material );
    wristband1_theta.rotation.y = Math.PI/2;


    if (window.display_type == "VR") {
        wristband1.position.set(0.,0.,0.1);
        wristband1.rotation.set(0.,0.,Math.PI);
        wristband1_phi.position.set(0.,0.,0.1);
        wristband1_theta.position.set(0.,R,0.1);
        controller1.add( wristband1 );
        controller1.add( wristband1_phi );
        controller1.add( wristband1_theta );
    }
    else {
        wristband1.position.set(      2.5,-3*R,  0.5);
        wristband1.rotation.set(0.,0.,Math.PI);
        wristband1_phi.position.set(  2.5,-3*R,  0.5);
        wristband1_theta.position.set(2.5,-3*R+R,0.5);
        scene.add( wristband1 );
        scene.add( wristband1_phi );
        scene.add( wristband1_theta );
    }

    if ( N > 5 ) {
        var geometry = new THREE.TorusBufferGeometry( R, r, Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0xffffff,
            // roughness: 0.7,
            // metalness: 0.5
        } );

        wristband2 = new THREE.Mesh( geometry, material );
        if ( shadows ) {
            wristband2.castShadow = true;
            wristband2.receiveShadow = true;
        }

        var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., Math.pow(2,quality)*2, Math.pow(2,quality) );
        var material = new THREE.MeshPhongMaterial( {
            color: 0x000000,
            // roughness: 0.7,
        } );
        wristband2_phi = new THREE.Mesh( geometry, material );

        var geometry = new THREE.TorusBufferGeometry( r, r/10., Math.pow(2,quality)*2, Math.pow(2,quality) );
        wristband2_theta = new THREE.Mesh( geometry, material );
        wristband2_theta.rotation.y = Math.PI/2;


        if (window.display_type == "VR") {
            wristband2.position.set(0.,0.,0.1);
            wristband2.rotation.set(0.,0.,Math.PI);
            wristband2_phi.position.set(0.,0.,0.1);
            wristband2_theta.position.set(0.,R,0.1);
            controller2.add( wristband2 );
            controller2.add( wristband2_phi );
            controller2.add( wristband2_theta );
        }
        else {
            wristband1.position.x -= 1.5
            wristband1_phi.position.x -= 1.5
            wristband1_theta.position.x -= 1.5
            wristband2.position.set(      4,-3*R,  0.5);
            wristband2.rotation.set(0.,0.,Math.PI);
            wristband2_phi.position.set(  4,-3*R,  0.5);
            wristband2_theta.position.set(4,-3*R+R,0.5);
            scene.add( wristband2 );
            scene.add( wristband2_phi );
            scene.add( wristband2_theta );
        }
    }

}

function remove_everything() {
    window.addEventListener("message", receiveMessage, false);
    function receiveMessage(event) {
        console.log('Closing renderer')
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
            for (i = wristband1.children.length; i = 0; i--) {
                var object = controller1.children[i];
                object.geometry.dispose();
                object.material.dispose();
            }
        };
        if ( N > 5 ) {
            for (i = wristband2.children.length; i = 0; i--) {
                var object = controller1.children[i];
                object.geometry.dispose();
                object.material.dispose();
            }
        }
        renderer.dispose();
    }
}

/*function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load("http://localhost:54321/visualise/data/vtk//dump-00000.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};*/ // FG: Removed as I don't think we use that anymore

function make_initial_sphere_texturing() {
    var commandstring = "" ;
    for ( i=3 ; i<N ; i++)
    {
        commandstring = commandstring + ('x' + (i+1) +'='+ world[i].cur.toFixed(1)) ;
        if (i<N-1) commandstring += "&" ;
    };
    request = new XMLHttpRequest();
    /*request.open('POST', "http://localhost:54321/make_textures?" +
                 "arr=" + JSON.stringify(arr) +
                 "&N=" + N +
                 "&t=" + "00000" +
                 "&quality=" + quality +
                 "&fname=" + fname,
                 true);*/
    request.open('GET', 'http://localhost:54321/load?ND=' + N + '&path='+ fname + '&texturepath=../../Textures&resolution=' + quality, true)
    request.send(null)

    request.onload = function() {
        request.open('GET', 'http://localhost:54321/render?ts=00000&' + commandstring, true) ;
        request.send(null)
        request.onload = function() { make_initial_spheres_CSV(); update_spheres_CSV(0,false);}
    }
    // Let's do the first rendering as well
    //request.open('GET', 'http://localhost:54321/render?ts=00000&' + commandstring, true) ;
    //request.send(null);

    // request.onreadystatechange = function () {}
};

function make_initial_spheres_CSV() {
    if ( cache ) { var filename = "http://localhost:54321/Samples/" + fname + "dump-00000.csv" }
    else { var filename = "http://localhost:54321/Samples/" + fname + "dump-00000.csv" + "?_="+ (new Date).getTime(); }
    Papa.parse(filename, {
        download: true,
        dynamicTyping: true,
        header: true,
        complete: function(results) {
            particles = new THREE.Group();
            scene.add( particles );
            spheres = results.data;
            if ( N == 1 ) {
                var geometry = new THREE.CylinderGeometry( 1, 1, 2, Math.pow(2,quality), Math.pow(2,quality) );
            }
            else {
                var geometry = new THREE.SphereGeometry( 1, Math.pow(2,quality), Math.pow(2,quality) );
            }
            var pointsGeometry = new THREE.SphereGeometry( 1, Math.max(Math.pow(2,quality-2),4), Math.max(Math.pow(2,quality-2),4) );
            var scale = 20.; // size of particles on tori
            for (var i = 0; i<spheres.length; i++) {
                if ( N === 2 ) {
                    var color = (( Math.random() + 0.25) / 1.5) * 0xffffff;
                    var material = new THREE.PointsMaterial( {
                        color: color,
                    } );
                }
                else {
                    if ( view_mode === 'catch_particle' ) {
                        if ( i == pinky ) { var color = 0xe72564; }
                        else              { var color = 0xaaaaaa; }
                        var material = new THREE.MeshPhongMaterial( { color: color } );
                    }
                    else {
                        if ( view_mode === 'rotations' ) {
                            texture_path = "http://localhost:54321/Textures/Texture-"+i+"-00000"
                            for ( var i=3;i<N;i++) { texture_path += "-0.0"; }
                            var texture = new THREE.TextureLoader().load(texture_path + ".png"); //TODO
                            var material = new THREE.MeshBasicMaterial( { map: texture } );
                        }
                        else {
                            var color = (( Math.random() + 0.25) / 1.5) * 0xffffff;
                            var material = new THREE.MeshPhongMaterial( { color: color } );
                        }
                    };
                }
                var object = new THREE.Mesh( geometry, material );
                object.position.set(spheres[i].x0,spheres[i].x1,spheres[i].x2);
                object.rotation.z = Math.PI/2.;
                if ( shadows ) {
                    object.castShadow = true;
                    object.receiveShadow = true;
                }
                particles.add( object );
                if ( N > 3 && !fname.includes('Spinner') && !hard_mode) {
                    pointsMaterial = new THREE.PointsMaterial( { color: color } );
                    object2 =  new THREE.Mesh( pointsGeometry, pointsMaterial );
                    if ( fname.includes('Lonely') ) { object2.scale.set(2.*R/scale,2.*R/scale,2.*R/scale); }
                    else { object2.scale.set(R/scale,R/scale,R/scale); }
                    object2.position.set(0.,0.,0.);
                    wristband1.add(object2);
                    if ( N > 5 ) {
                        object3 = object2.clone();
                        wristband2.add(object3);
                    }
                }
            }
            if ( fname.includes("Submarine") ) { camera.position.set(particles.children[pinky].position.x,particles.children[pinky].position.y,particles.children[pinky].position.z); console.log(camera.position) }
        }
    });
};

function load_textures(t, Viewpoint) {
    if ( particles !== undefined) {
        var loader = new THREE.TextureLoader();
        for ( ii = 0; ii < particles.children.length - 1; ii++ ) {
            if ( cache ) { var filename = "http://localhost:54321/Textures/" + "Texture-" + ii + "-" + Viewpoint+".png" }
            else { var filename = "http://localhost:54321/Textures/" + "Texture-" + ii + "-"+Viewpoint + ".png" + "?_="+ (new Date).getTime() }
            loader.load(filename,
                        function( texture ) { //TODO not sure why not working ... ...
                            //var myRe = /-[0-9]+.png/g
                            //var res=myRe.exec(texture.image.currentSrc)
                            //var myRe2 = /[0-9]+/
                            //var iii = myRe2.exec(texture.image.currentSrc)[0]
                            var iii = texture.image.currentSrc.split('-')[1]
                            console.log(texture.image.currentSrc); console.log(iii);
                            var o = particles.children[iii];
                            o.material.map = texture;
                            o.material.map.needsUpdate = true;
                        });
        }
    }
}

function update_spheres_texturing (t) {
      if  ( true ) { //TODO Do something better ...
          var commandstring = "" ; var Viewpoint = String(t*timestep).padStart(5,'0')  ;

          for ( i=3 ; i<N ; i++)
          {
              commandstring = commandstring + "&" + ('x' + (i+1) +'='+ world[i].cur.toFixed(1)) ;
              Viewpoint = Viewpoint + "-" + world[i].cur.toFixed(1) ;
          }

          var request = new XMLHttpRequest();
          /*request.open('POST',
                       "http://localhost:54321/make_textures?" +
                       "arr=" + JSON.stringify(arr) +
                       "&N=" + N +
                       "&t=" + t + "0000" +
                       "&quality=" + quality +
                       "&fname=" + fname,
                       true);*/
          var runvalue = 0 ;
          if (time.play) runvalue = 1 ;
          request.open('GET', 'http://localhost:54321/render?ts='+String(t*timestep).padStart(5,'0') + commandstring + '&running=' + runvalue, true) ;

          request.onload = function() {
              load_textures(t, Viewpoint);
          }
          request.send('');
      }
      else {
          load_textures(t, Viewpoint);
      }
}

function update_spheres_CSV(t,changed_higher_dim_view) {

    if ( cache ) { var filename = "http://localhost:54321/Samples/" + fname + "dump-"+String(t*timestep).padStart(5,'0') +".csv" }
    else { var filename = "http://localhost:54321/Samples/" + fname + "dump-"+String(t*timestep).padStart(5,'0') +".csv"+"?_="+ (new Date).getTime() }
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
                if (N < 4) {
                    var R_draw = spheres[i].R;
                             }
                else if (N == 4) {
                    var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                            Math.pow( (world[3].cur - spheres[i].x3), 2)
                                          );

                    //if ( (world[3].cur >  world[3].max-spheres[i].R ) // NOTE: IMPLEMENT THIS!!
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
                 else if (N == 10) {
                     var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                             Math.pow( (world[3].cur - spheres[i].x3), 2) -
                                             Math.pow( (world[4].cur - spheres[i].x4), 2) -
                                             Math.pow( (world[5].cur - spheres[i].x5), 2) -
                                             Math.pow( (world[6].cur - spheres[i].x6), 2) -
                                             Math.pow( (world[7].cur - spheres[i].x7), 2) -
                                             Math.pow( (world[8].cur - spheres[i].x8), 2) -
                                             Math.pow( (world[9].cur - spheres[i].x9), 2)
                                         ); // FIXME - IS THIS RIGHT?
                 };
                if (isNaN(R_draw)) {
                    object.visible = false;
                }
                if ( fname.includes('Submarine') && i==pinky ) { object.visible = false; }
                else {
                    if ( window.display_type === 'VR') {
                        R_draw = R_draw*vr_scale;
                        object.position.set(spheres[i].x1*vr_scale,spheres[i].x0*vr_scale - human_height,spheres[i].x2*vr_scale);
                    }
                    else {
                        object.position.set(spheres[i].x0,spheres[i].x1,spheres[i].x2);
                    }
                    object.scale.set(R_draw,R_draw,R_draw);
                    object.visible = true;

                    if ( view_mode === 'velocity' ) {
                        lut.setMin(0);
                        lut.setMax(velocity.vmax);
                        object.material.color = lut.getColor(spheres[i].Vmag);
                    }
                    else if ( view_mode === 'rotation_rate' ) {
                        lut.setMin(0);
                        lut.setMax(velocity.omegamax);
                        object.material.color = lut.getColor(spheres[i].Omegamag);
                    }
                };

                if ( N == 4 && !fname.includes('Spinner')) {
                    var object2 = wristband1.children[i];
                    phi = 2.*Math.PI*( world[3].cur - spheres[i].x3 )/(world[3].max - world[3].min) - Math.PI/2.;
                    x = (R + r)*Math.cos(phi);
                    y = (R + r)*Math.sin(phi);
                    z = 0.;
                    object2.position.set(x,y,z);
                };

                if ( N > 4 && !fname.includes('Spinner') && !hard_mode ) {
                    var object2 = wristband1.children[i];
                    phi   = 2.*Math.PI*(world[3].cur - spheres[i].x3)/(world[3].max - world[3].min) - Math.PI/2.;
                    theta = 2.*Math.PI*(world[4].cur - spheres[i].x4)/(world[4].max - world[4].min) ;
                    x = (R + r*Math.cos(theta))*Math.cos(phi);
                    y = (R + r*Math.cos(theta))*Math.sin(phi);
                    z = r*Math.sin(theta);
                    object2.position.set(x,y,z);
                };

                if ( N == 6 && !fname.includes('Spinner') ) {
                    var object3 = wristband2.children[i];
                    phi = 2.*Math.PI*( world[5].cur - spheres[i].x5 )/(world[5].max - world[5].min) - Math.PI/2.;
                    x = (R + r)*Math.cos(phi);
                    y = (R + r)*Math.sin(phi);
                    z = 0.;
                    object3.position.set(x,y,z);
                };

                if ( N >= 7 && !fname.includes('Spinner') ) {
                    var object3 = wristband2.children[i];
                    phi   = 2.*Math.PI*(world[5].cur - spheres[i].x5)/(world[5].max - world[5].min) - Math.PI/2.;
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
        // var zoom = 10.
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
    if ( controls !== undefined) { controls.handleResize(); }
    if (window.display_type == 'anaglyph') { effect.setSize( window.innerWidth, window.innerHeight ); };
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
}

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

function animate() {
    THREE.VRController.update();
    if ( redraw_left ) { update_higher_dims_left(); }
    if ( redraw_right ) { update_higher_dims_right(); }
    if ( fname.includes('Uniaxial') ) {
        roof.position.x = world[0].max - 5. - time.cur/10.;
    }
    if (N > 3) {
        for (iii=3;iii<N;iii++) {
            if (world[iii].cur != world[iii].prev) {
                update_spheres_CSV(Math.floor(time.cur),true);
                if (view_mode === 'rotations') {update_spheres_texturing(Math.floor(time.cur),) ;}
                world[iii].prev = world[iii].cur;
            }
        }
    }
    if (time.play) { time.cur += time.rate*timestep/10000.; };
    //if ( Math.floor(time.cur) != time.prev ) {
    if ( ( Math.floor(time.cur) !== time.prev ) ){//|| redraw ){
        update_spheres_CSV(Math.floor(time.cur),false);
        if (view_mode === 'rotations') {update_spheres_texturing(Math.floor(time.cur),) ;}
        time.prev = Math.floor(time.cur);
    }
    if (time.cur > time.max) { time.cur -= time.max; }
    requestAnimationFrame( animate );
    if ( controls !== undefined ) { controls.update(); }
    renderer.setAnimationLoop( render );
};

function render() {
    if ( renderer.vr.isPresenting() ) {
        //scene.scale.set( vr_scale, vr_scale, vr_scale );
        //scene.rotation.z = -Math.PI/2.;
    }// TODO: SET VR SCALING TO LOOK GOOD
    if ( view_mode === 'catch_particle' && window.display_type == "VR" ) {
        cleanIntersected();
        intersectObjects( controller1 );
        intersectObjects( controller2 );
    }
    if (window.display_type == "anaglyph") { effect.render( scene, camera ); }
    else { renderer.render( scene, camera ); }
};
