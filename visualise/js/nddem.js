var container;
var camera, scene, controls, renderer;
var controller1, controller2;

var raycaster, intersected = [];
var tempMatrix = new THREE.Matrix4();

var group, wristband;

var R,r; // parameters of torus
var N; // number of dimensions
var world = [];
var ref_dim = {'N_0': 0}; // reference dimension
var time = {'cur': 0, 'prev': 0, 'min':0, 'max': 99, 'play': false}

init();

function init() {
    var request = new XMLHttpRequest();
    request.open('GET', "http://localhost:8000/CppCode/"+window.fname+"?_="+ (new Date).getTime(), true);
    request.send(null);
    N = request.onreadystatechange = function () {
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
                        if (l[2] == 'WALL') {
                            world[l[1]].min = parseFloat(l[3]);
                            world[l[1]].max = parseFloat(l[4]);
                            world[l[1]].cur = (world[l[1]].min + world[l[1]].max)/2.;
                            world[l[1]].prev = world[l[1]].cur;
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

    if (N == 2) {
        camera = new THREE.OrthographicCamera();// window.innerWidth / - 2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / - 2, 1, 1000 );
        // camera.enableRotate = false
        camera.position.set(0.5,0.5,1.5);
        // camera.lookAt(0.5,0.5,0.5);
    }
    else {
        camera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 0.1, 10 );
        camera.position.set(0,0.5,3);
    }

    // make_walls();

    make_lights();

    group = new THREE.Group();
    scene.add( group );

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.gammaInput = true;
    renderer.gammaOutput = true;
    renderer.shadowMap.enabled = true;
    // if (window.viewerType == "VR") { renderer.vr.enabled = true; };
    container.appendChild( renderer.domElement );

    if (window.viewerType == "VR") { container.appendChild( WEBVR.createButton( renderer ) ); };

    // controllers

    if (window.viewerType == "VR") {
        controller1 = renderer.vr.getController( 0 );
        controller1.addEventListener( 'selectstart', onSelectStart );
        controller1.addEventListener( 'selectend', onSelectEnd );
        scene.add( controller1 );

        controller2 = renderer.vr.getController( 1 );
        controller2.addEventListener( 'selectstart', onSelectStart );
        controller2.addEventListener( 'selectend', onSelectEnd );
        scene.add( controller2 );

        controls = new THREE.TrackballControls( camera );
        console.log("VR mode loaded");
    } else if (window.viewerType == 'keyboard') {
        controls = new THREE.TrackballControls( camera );
        // var gui = new dat.GUI();
        console.log('Keyboard mode loaded');
    } else if (window.viewerType == 'anaglyph') {
        controls = new THREE.TrackballControls( camera, renderer.domElement );
        effect = new THREE.AnaglyphEffect( renderer );
        console.log('Anaglyph mode loaded');
    };

    // load_hyperspheres_VTK();
    if ( N == 5 ) { add_torus(); }
    make_initial_spheres_CSV();
    update_spheres_CSV(0);

    if ( window.viewerType == 'anaglyph' ) {
        var gui = new dat.GUI();
        gui.add( ref_dim, 'N_0').min(0).max(N).step(1).listen().name('Reference dimension') ;
        if (N > 3) {
            for (i=3;i<N;i++) {
                gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
            }
        }
        gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
        gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
        gui.open();
    }
    else {
        var gui = dat.GUIVR.create('MuDEM');
        dat.GUIVR.enableMouse( camera, renderer );
        gui.add( ref_dim, 'N_0').min(0).max(N).step(1).listen().name('Reference dimension') ;
        if (N > 3) {
            for (i=3;i<N;i++) {
                gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).name('X'+i) ;
            }
        }
        gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time') ;
        gui.add( time, 'play').name('Autoplay').onChange( function(flag) { time.play = flag; })
        gui.position.set(0,1.5,0.5)

        if (window.viewerType == "VR") {
            controller2.add( gui );
            var input1 = dat.GUIVR.addInputObject( controller1 );
            var input2 = dat.GUIVR.addInputObject( controller2 );
            scene.add( input1 );
            scene.add( input2 );
        }
        else {
            scene.add( gui );
        }
    }

    var geometry = new THREE.BufferGeometry().setFromPoints( [ new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, - 1 ) ] );

    var line = new THREE.Line( geometry );
    line.name = 'line';
    line.scale.z = 5;

    if (window.viewerType == "VR") {
        controller1.add( line.clone() );
        controller2.add( line.clone() );
        };
    raycaster = new THREE.Raycaster();
    window.addEventListener( 'resize', onWindowResize, false );

}

function make_lights() {
    // var axesHelper = new THREE.AxesHelper( 1 );
    // scene.add( axesHelper );

    scene.add( new THREE.HemisphereLight( 0x808080, 0x606060 ) );

    var light = new THREE.DirectionalLight( 0xffffff );
    light.position.set( 0, 6, 0 );
    light.castShadow = true;
    light.shadow.camera.top = 2;
    light.shadow.camera.bottom = - 2;
    light.shadow.camera.right = 2;
    light.shadow.camera.left = - 2;
    light.shadow.mapSize.set( 4096, 4096 );
    scene.add( light );
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
    R = 0.2;
    r = R/2.;
    var geometry = new THREE.TorusBufferGeometry( R, r, 64, 32 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0xffffff,
        roughness: 0.7,
        metalness: 0.5
    } );

    wristband = new THREE.Mesh( geometry, material );
    wristband.position.set(0.,0.,0.);
    wristband.castShadow = true;
    wristband.receiveShadow = true;

    var geometry = new THREE.TorusBufferGeometry( r+R-r/6., r/5., 64, 32 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0x000000,
        roughness: 0.7,
    } );
    wristband_phi = new THREE.Mesh( geometry, material );
    wristband_phi.position.set(0.,0.,0.);

    var geometry = new THREE.TorusBufferGeometry( r, r/10., 64, 32 );
    var material = new THREE.MeshStandardMaterial( {
        color: 0x000000,
        roughness: 0.7,
    } );
    wristband_theta = new THREE.Mesh( geometry, material );
    wristband_theta.position.set(0.,R,0.);
    wristband_theta.rotation.y = Math.PI/2;


    if (window.viewerType == "VR") {
        controller1.add( wristband );
        controller1.add( wristband_phi );
        controller1.add( wristband_theta );
    }
    else {
        scene.add( wristband );
        scene.add( wristband_phi );
        scene.add( wristband_theta );
    }

}

function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load("http://localhost:8000/visualise/data/vtk//dump-0.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};

function make_initial_spheres_CSV() {
    Papa.parse("http://localhost:8000/visualise/data/csv/dump-"+0+".csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        complete: function(results) {
            spheres = results.data;
            var numSpheres = spheres.length;
            // var c = new THREE.Color(1,1,1);
            var geometry = new THREE.SphereGeometry( 1, 32, 32 );
            for (var i = 0; i<numSpheres; i++) {
                if (i == 0) {
                    var material = new THREE.MeshStandardMaterial( {
                        color: 0xFF00FF,
                        roughness: 0.0,
                        metalness: 0.0
                    } );
                }
                else {
                    var material = new THREE.MeshStandardMaterial( {
                        color: 0xffffff,//Math.random() * 0xffffff,
                        roughness: 0.7,
                        metalness: 0.0
                    } );
                }
                var object = new THREE.Mesh( geometry, material );
                object.castShadow = true;
                object.receiveShadow = true;
                group.add( object );
                if (N == 5) {
                    object2 = object.clone();
                    object2.scale.set(0.01,0.01,0.01);
                    object2.position.set(0.,0.,0.);
                    wristband.add(object2);
                }
            }
        }
    });
};

function update_spheres_CSV(t) {
    Papa.parse("http://localhost:8000/visualise/data/csv/dump-"+t+".csv"+"?_="+ (new Date).getTime(), { // definitely no caching
    // Papa.parse("http://localhost:8000/visualise/data/csv/dump-"+t+".csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: false,
        complete: function(results) {
            spheres = results.data;
            var numSpheres = spheres.length;
            for (i = 0; i<numSpheres; i++) {
                var object = group.children[i];
                object.position.set(spheres[i].X0,spheres[i].X1,spheres[i].X2);
                if (N == 2) {
                    var R_draw = spheres[i].R;
                             }
                if (N == 3) {
                    var R_draw = spheres[i].R;
                             }
                else if (N == 4) {
                    var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                            Math.pow( (world[3].cur - spheres[i].X3), 2)
                                          );
                             }
                 else if (N == 5) {
                     var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                             Math.pow( (world[3].cur - spheres[i].X3), 2) //-
                                             // Math.pow( (world[4].cur - spheres[i].X4), 2)
                                         ); // FIXME - IS THIS RIGHT?
                 }
                if (isNaN(R_draw)) {
                    object.visible = false;
                }
                else {
                    object.visible = true;
                    object.scale.set(R_draw,R_draw,R_draw);
                    // console.log(object.material.color);
                }

                if ( N==5 ) {
                    var object2 = wristband.children[i];
                    phi = 2.*Math.PI*(world[3].cur - spheres[i].X3)/(world[3].max - world[3].min) + Math.PI/2.;
                    theta = 2.*Math.PI*(world[4].cur - 0)/(world[4].max - world[4].min) - Math.PI;// FIXME
                    x = (R + r*Math.cos(theta))*Math.cos(phi);
                    y = (R + r*Math.cos(theta))*Math.sin(phi);
                    z = r*Math.sin(theta);
                    object2.position.set(x,y,z);
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
        group.add( object );
        controller.userData.selected = undefined;
    }
};

function getIntersections( controller ) {

    tempMatrix.identity().extractRotation( controller.matrixWorld );

    raycaster.ray.origin.setFromMatrixPosition( controller.matrixWorld );
    raycaster.ray.direction.set( 0, 0, - 1 ).applyMatrix4( tempMatrix );

    return raycaster.intersectObjects( group.children );

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
    if (time.play) { time.cur += 0.5; };
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
    if (window.viewerType == "VR") {
        cleanIntersected();
        intersectObjects( controller1 );
        intersectObjects( controller2 );
    }
    if (window.viewerType == "anaglyph") { effect.render( scene, camera ); }
    else { renderer.render( scene, camera ); }
};
