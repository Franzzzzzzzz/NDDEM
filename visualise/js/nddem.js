var container;
var camera, scene, controls, renderer;
var controller1, controller2;

var raycaster, intersected = [];
var tempMatrix = new THREE.Matrix4();

var group;
var W_old; // keep past value of W
var t_old;
var N = 4; // number of dimensions
var world = [];
for (i=0;i<N;i++) {
    world.push({});
    world[i].min = 0;
    world[i].max = 1;
    world[i].cur = 0.5;
    world[i].prev = 0.5;
}


var time = {'cur': 0, 'prev': 0, 'min':0, 'max': 99}


init();
animate();

function init() {

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
    floor.receiveShadow = false;
    scene.add( floor );
    // floor.position.set(1.5,0,0.5)
    // scene.add( floor );

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

    group = new THREE.Group();
    scene.add( group );

    // make_random_objects();
    // load_hyperspheres_VTK();
    make_initial_spheres_CSV();
    update_spheres_CSV(0);

    renderer = new THREE.WebGLRenderer( { antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.gammaInput = true;
    renderer.gammaOutput = true;
    renderer.shadowMap.enabled = true;
    if (window.viewerType == "VR") { renderer.vr.enabled = true; };
    container.appendChild( renderer.domElement );

    if (window.viewerType == "VR") { document.body.appendChild( WEBVR.createButton( renderer ) ); };

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

        console.log("VR mode loaded");
    } else if (window.viewerType == 'keyboard') {
        controls = new THREE.TrackballControls( camera );
        // var gui = new dat.GUI();
        console.log('Keyboard mode loaded');
    } else if (window.viewerType == 'anaglyph') {
        controls = new THREE.TrackballControls( camera );
        effect = new THREE.AnaglyphEffect( renderer );
        console.log('Anaglyph mode loaded');
    };

    var gui = dat.GUIVR.create('MuDEM');
    dat.GUIVR.enableMouse( camera, renderer );
    if (N > 3) {
        for (i=3;i<N;i++) {
            gui.add( world[i], 'cur').min(world[i].min).max(world[i].max).listen().name('X'+i).step(0.01) ;
        }
    }
    if (N == 4) {  }
    gui.add( time, 'cur').min(time.min).max(time.max).step(1).listen().name('Time (Up/Down)') ;
    gui.position.set(0,1.5,0.5)
    scene.add( gui );


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

function make_random_objects() {
    var geometries = [
        new THREE.BoxBufferGeometry( 0.2, 0.2, 0.2 ),
        new THREE.ConeBufferGeometry( 0.2, 0.2, 64 ),
        new THREE.CylinderBufferGeometry( 0.2, 0.2, 0.2, 64 ),
        new THREE.IcosahedronBufferGeometry( 0.2, 3 ),
        new THREE.TorusBufferGeometry( 0.2, 0.04, 64, 32 )
    ];

    for ( var i = 0; i < 50; i ++ ) {

        var geometry = geometries[ Math.floor( Math.random() * geometries.length ) ];
        var material = new THREE.MeshStandardMaterial( {
            color: Math.random() * 0xffffff,
            roughness: 0.7,
            metalness: 0.0
        } );

        var object = new THREE.Mesh( geometry, material );

        object.position.x = Math.random() * 4 - 2;
        object.position.y = Math.random() * 2;
        object.position.z = Math.random() * 4 - 2;

        object.rotation.x = Math.random() * 2 * Math.PI;
        object.rotation.y = Math.random() * 2 * Math.PI;
        object.rotation.z = Math.random() * 2 * Math.PI;

        object.scale.setScalar( Math.random() + 0.5 );

        object.castShadow = true;
        object.receiveShadow = true;

        group.add( object );

    }
};

function load_hyperspheres_VTK() {
    var loader = new THREE.VTKLoader();
    loader.load("http://localhost:8000/data/vtk//dump-0.vtu", function ( geometry ) {
        console.log(geometry);
    } );
};

function make_initial_spheres_CSV() {
    Papa.parse("http://localhost:8000/data/csv/dump-"+0+".csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        complete: function(results) {
            spheres = results.data;
            var numSpheres = spheres.length;
            // var c = new THREE.Color(1,1,1);
            var geometry = new THREE.SphereGeometry( 1, 32, 32 );
            for (var i = 0; i<numSpheres; i++) {
                var material = new THREE.MeshStandardMaterial( {
                    color: Math.random() * 0xffffff,
                    roughness: 0.7,
                    metalness: 0.0
                } );
                var object = new THREE.Mesh( geometry, material );
                object.castShadow = true;
                object.receiveShadow = true;
                group.add( object );
            }
        }
    });
};

function update_spheres_CSV(t) {
    Papa.parse("http://localhost:8000/data/csv/dump-"+t+".csv"+"?_="+ (new Date).getTime(), { // definitely no caching
    // Papa.parse("http://localhost:8000/data/csv/dump-"+t+".csv", {
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
                                             Math.pow( (world[3].cur - spheres[i].X3), 2) -
                                             Math.pow( (world[4].cur - spheres[i].X4), 2)
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
        if (world[3].cur != world[3].prev) {
            update_spheres_CSV(time.cur);
            world[3].prev = world[3].cur;
        }
    }
    if (time.cur != time.prev) {
        update_spheres_CSV(time.cur);
        time.prev = time.cur;
    }
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
