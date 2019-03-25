var container; // main div element
var camera, scene, controls, renderer; // UI elements
var controller1, controller2; // VR controllers

var raycaster, intersected = []; // catching grains
var tempMatrix = new THREE.Matrix4(); // catching grains

var particles, wristband, wristband1, axesHelper; // groups of objects
var R,r; // parameters of torus
var N; // number of dimensions
var world = []; // properties that describe the domain
var ref_dim = {'c': 0} //, 'x': 00, 'y': 1, 'z': 2}; // reference dimensions
var time = {'cur': 0, 'prev': 0, 'min':0, 'max': 99, 'play': false, 'rate': 0.1} // temporal properties
var axeslength, fontsize; // axis properties
var VR_scale = 5.; // mapping from DEM units to VR units

init();

function init() {
    var request = new XMLHttpRequest();
    // request.open('GET', "http://localhost:8000/" + window.fname + window.inname + "?_="+ (new Date).getTime(), true);
    request.open('GET', "http://localhost:8000/" + window.fname + window.inname, true);
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
                build_world();
                animate();
            }
        }
    }
}

function build_world() {
    container = document.createElement( 'div' );
    document.body.appendChild( container );
    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x808080 );
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
    if (N == 2) {
        camera = new THREE.OrthographicCamera();
        camera.position.set(0.5,0.5,1.5);
    }
    else {
        camera = new THREE.PerspectiveCamera(70, window.innerWidth / window.innerHeight, 0.1, 1000 ); // fov, aspect, near, far
        camera.position.set(0.5*world[0].max,
                            1.*world[1].max,
                            -2.*world[0].max
                        );
        camera.scale.set(0.5,0.5,0.5);
    }
}
function add_renderer() {
    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
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
        //dat.GUIVR.enableMouse( camera, renderer );
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
        // controller1.addEventListener( 'selectstart', onSelectStart );
        // controller1.addEventListener( 'selectend', onSelectEnd );
        scene.add( controller1 );
        //
        controller2 = renderer.vr.getController( 1 );
        // controller2.addEventListener( 'selectstart', onSelectStart );
        // controller2.addEventListener( 'selectend', onSelectEnd );
        scene.add( controller2 );
        //
        // controls = new THREE.TrackballControls( camera );
        console.log("VR mode loaded");

        // var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );
        //
        // var line = new THREE.Line( geometry );
        // line.name = 'line';
        // line.scale.z = 5;
        //
        // if (window.viewerType == "VR") {
        //     controller1.add( line.clone() );
        //     controller2.add( line.clone() );
        //     };
        // raycaster = new THREE.Raycaster();

    } else if (window.viewerType == 'keyboard') {
        controls = new THREE.TrackballControls( camera, renderer.domElement  );
        aim_camera()
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
    controls.target0.set(
        (world[0].min + world[0].max)/2.,
        (world[1].min + world[1].max)/2.,
        (world[2].min + world[2].max)/2.,
    );
    controls.up0.set( 1, 0, 0 ); // set z as up
    controls.reset();
}

function make_axes() {
    if (typeof axesHelper == 'undefined') {
        axeslength = 5 ; // length of axes vectors
        fontsize = 0.5; // font size
        axesHelper = new THREE.AxesHelper( axeslength ); // X - red, Y - green, Z - blue
        // axesHelper.position.set(world[ref_dim.N_0])

        scene.add( axesHelper );
    }

    if (ref_dim.c != ref_dim.x) {
        ref_dim.x = ref_dim.c;
        if (ref_dim.c < N - 1) { ref_dim.y = ref_dim.c + 1; }
        else { ref_dim.y = ref_dim.c + 1 - N; }
        if (ref_dim.c < N - 2) { ref_dim.z = ref_dim.c + 2; }
        else { ref_dim.z = ref_dim.c + 2 - N; }

        if (axesHelper.children.length > 1 ) {
            for( var i = axesHelper.children.length - 1; i >= 0; i--) {
                obj = axesHelper.children[i];
                axesHelper.remove(obj);
            }
        }
        var loader = new THREE.FontLoader();
    	loader.load( 'http://localhost:8000/visualise/node_modules/three/examples/fonts/helvetiker_bold.typeface.json', function ( font ) {
    		var textGeo_x = new THREE.TextBufferGeometry( "x" + ref_dim.x, { font: font, size: fontsize, height: fontsize/2., } );
    		var textMaterial_x = new THREE.MeshPhongMaterial( { color: 0xff0000 } );
    		var mesh_x = new THREE.Mesh( textGeo_x, textMaterial_x );
    		mesh_x.position.x = axeslength - 1.5*fontsize;
            mesh_x.rotation.z = Math.PI;
            mesh_x.rotation.y = Math.PI;
            // mesh_x.position.y = -fontsize;
    		axesHelper.add( mesh_x );

            var textGeo_y = new THREE.TextGeometry( "x" + ref_dim.y, { font: font, size: fontsize, height: fontsize/2., } );
    		var textMaterial_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
    		var mesh_y = new THREE.Mesh( textGeo_y, textMaterial_y );
    		mesh_y.position.y = axeslength-fontsize*1.5;
            mesh_y.rotation.z = -Math.PI/2.;
            mesh_y.rotation.x = Math.PI;
    		axesHelper.add( mesh_y );

            var textGeo_z = new THREE.TextGeometry( "x" + ref_dim.z, { font: font, size: fontsize, height: fontsize/2., } );
    		var textMaterial_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
    		var mesh_z = new THREE.Mesh( textGeo_z, textMaterial_z );
    		mesh_z.position.z = axeslength;//-fontsize;
            mesh_z.rotation.z = -Math.PI/2.;
            mesh_z.rotation.x = Math.PI/2.;
    		axesHelper.add( mesh_z );
        });

    }
}

function make_lights() {
    scene.add( new THREE.HemisphereLight( 0x808080, 0x606060 ) );

    var light = new THREE.DirectionalLight( 0xffffff );
    light.position.set( 2*world[0].max, -6, 0 );

    // var light1 = new THREE.DirectionalLight( 0xffffff );
    // light1.position.set( 0, 0, -12 );
    // light.castShadow = true;
    // light.shadow.camera.top = 2;
    // light.shadow.camera.bottom = - 2;
    // light.shadow.camera.right = 2;
    // light.shadow.camera.left = - 2;
    // light.shadow.mapSize.set( 4096, 4096 );
    scene.add( light );
    // scene.add( light1 );

    var helper = new THREE.DirectionalLightHelper( light, 5 );
    // var helper1 = new THREE.DirectionalLightHelper( light1, 5 );
    scene.add( helper );
    // scene.add( helper1 );
}

function make_walls() {
    var geometry = new THREE.PlaneBufferGeometry( 1, 1 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0xeeeeee,
        roughness: 1.0,
        metalness: 0.0,
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
    var geometry = new THREE.TorusBufferGeometry( R, r, 64, 32 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0xffffff,
        roughness: 0.7,
        metalness: 0.5
    } );

    wristband = new THREE.Mesh( geometry, material );
    wristband.castShadow = true;
    wristband.receiveShadow = true;

    var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., 64, 32 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0x000000,
        roughness: 0.7,
    } );
    wristband_phi = new THREE.Mesh( geometry, material );

    var geometry = new THREE.TorusBufferGeometry( r, r/10., 64, 32 );
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
        var geometry = new THREE.TorusBufferGeometry( R, r, 64, 32 );
        var material = new THREE.MeshStandardMaterial( {
            color: 0xffffff,
            roughness: 0.7,
            metalness: 0.5
        } );

        wristband1 = new THREE.Mesh( geometry, material );
        wristband1.castShadow = true;
        wristband1.receiveShadow = true;

        var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., 64, 32 );
        var material = new THREE.MeshStandardMaterial( {
            color: 0x000000,
            roughness: 0.7,
        } );
        wristband1_phi = new THREE.Mesh( geometry, material );

        var geometry = new THREE.TorusBufferGeometry( r, r/10., 64, 32 );
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

function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load("http://localhost:8000/visualise/data/vtk//dump-00000.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};

function make_initial_spheres_CSV() {
    Papa.parse("http://localhost:8000/" + window.fname + "dump-00000.csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        complete: function(results) {
            particles = new THREE.Group();
            scene.add( particles );

            spheres = results.data;
            var numSpheres = spheres.length;
            var geometry = new THREE.SphereGeometry( 1, 32, 32 );
            var pointsGeometry = new THREE.SphereGeometry( 1, 6, 6 );
            var scale = 20.;
            for (var i = 0; i<numSpheres; i++) {
                // if (i == 0) {
                //     var material = new THREE.MeshStandardMaterial( {
                //         color: 0xFF00FF,
                //         roughness: 0.0,
                //         metalness: 0.0
                //     } );
                // }
                // else {
                    var color = Math.random() * 0xffffff;
                    var material = new THREE.MeshStandardMaterial( {
                        // color: 0xffffff,
                        color: color,
                        roughness: 0.7,
                        metalness: 0.0
                    } );
                // }
                var object = new THREE.Mesh( geometry, material );
                object.castShadow = true;
                object.receiveShadow = true;
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
    // Papa.parse("http://localhost:8000/" + window.fname + "dump-"+t+"0000.csv"+"?_="+ (new Date).getTime(), { // definitely no caching
    Papa.parse("http://localhost:8000/" + window.fname + "dump-"+t+"0000.csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: true,
        complete: function(results) {
            spheres = results.data;
            var numSpheres = spheres.length;
            for (i = 0; i<numSpheres; i++) {
                var object = particles.children[i];
                object.position.set(spheres[i].x0,spheres[i].x1,spheres[i].x2);
                if (N == 2) {
                    var R_draw = spheres[i].R;
                             }
                if (N == 3) {
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
    camera.aspect = window.innerWidth / window.innerHeight;
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
    if (window.viewerType == "keyboard" || window.viewerType == "anaglyph") { controls.update(); }
    renderer.setAnimationLoop( render );
};

function render() {
    // if (window.viewerType == "VR") {
    //     cleanIntersected();
    //     intersectObjects( controller1 );
    //     intersectObjects( controller2 );
    // }
    if (window.viewerType == "anaglyph") { effect.render( scene, camera ); }
    else { renderer.render( scene, camera ); }
};
