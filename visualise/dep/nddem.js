var fname = "dump-";
var camera, scene, renderer;
var effect, controls;

var W_old; // keep past value of W
var t_old;
var origin = new THREE.Vector3(0,0,0);

var params = {
    W_min: -3,
    W_max: 3,
    W: 0.5,
    nW: 32, //number of increments in W direction
    radius: 5,
    theta: 0.12,
    time: 0,
    t_max: 1,
    phi_min: -2,
    phi_max: 2,
    phi: 0.12,

};

var mouse = {x:0,y:0};
var cameraMoves = {x:0,y:0,z:-0.1,move:false,speed:0.5};
init();
animate();



function init() {
    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 0.01, 1000 );
    scene.add( camera );

    window.addEventListener('mousemove', mouseMove);

    set_bounds()
    spheres = make_initial_spheres()
    update_spheres(0)

    var light_1 = new THREE.DirectionalLight( 0x9DB5B2 );
    light_1.position.set( - 5, -5, -5 );
    // light_1.castShadow = false;
    // light_1.shadow.camera.zoom = 1;
    scene.add( light_1 );
    light_1.target.position.set( 0, 0, - 2 );
    scene.add( light_1.target );

    var light_2 = new THREE.DirectionalLight( 0xDAF0EE );
    light_2.position.set(  5, -5, -5 );
    // light_2.castShadow = false;
    // light_2.shadow.camera.zoom = 1;
    scene.add( light_2 );
    light_2.target.position.set( 0, 0, - 2 );
    scene.add( light_2.target );

    var light_3 = new THREE.DirectionalLight( 0x94D1BE );
    light_3.position.set( 5, 5, 5 );
    // light_3.castShadow = false;
    // light_3.shadow.camera.zoom = 4;
    scene.add( light_3 );
    light_3.target.position.set( 0, 0, - 2 );
    scene.add( light_3.target );

    var cam_helper = new THREE.CameraHelper( light_1.shadow.camera );
    var light_1_helper = new THREE.DirectionalLightHelper( light_1, 15 );
    var light_2_helper = new THREE.DirectionalLightHelper( light_2, 15 );
    var light_3_helper = new THREE.DirectionalLightHelper( light_3, 15 );
    var axisHelper = new THREE.AxesHelper( 3 );
    // scene.add( cam_helper );
    // scene.add( light_1_helper );
    // scene.add( light_2_helper );
    // scene.add( light_3_helper );
    scene.add( axisHelper );

    renderer = new THREE.WebGLRenderer( { antialias: true, alpha: true } );
    renderer.autoClear = false;
    renderer.setClearColor(0x000000, 0.0);
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.shadowMap.enabled = true;
    document.body.appendChild( renderer.domElement );

    if (window.viewerType == 'VR') {
        renderer.vr.enabled = true;
        WEBVR.getVRDisplay( function ( display ) {
            renderer.vr.setDevice( display );
            document.body.appendChild( WEBVR.getButton( display, renderer.domElement ) );
        } );

        // controls = new THREE.VRControls( camera );
        // effect = new THREE.VREffect( renderer );
    } else if (window.viewerType == 'keyboard') {
        controls = new THREE.TrackballControls( camera );
    } else if (window.viewerType == 'anaglyph'); {
        controls = new THREE.TrackballControls( camera );
        effect = new THREE.AnaglyphEffect( renderer );
    };
    // controls = new THREE.OrbitControls( camera );



    // dat.GUIVR.enableMouse( camera ); // https://workshop.chromeexperiments.com/examples/guiVR/#1--Basic-Usage
    var gui = new dat.GUI();
    gui.add( params, 'W').min(params.W_min).max(params.W_max).listen().name('W (Left/Right)').step(0.01) ;
    gui.add( params, 'time').min(0).step(1).listen().name('Time (Up/Down)') ;
    gui.add( params, 'radius').min(0).max(10).step(0.01).listen().name('Cam Dist (R/F)') ;
    gui.add( params, 'theta').step(0.01).listen().name('Cam Incl (W/S)') ;
    gui.add( params, 'phi').step(0.01).listen().name('Cam Azim (A/D)') ;
    gui.open();

    window.addEventListener( 'resize', onWindowResize, false );

}

function onWindowResize() {

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    if (window.viewerType == 'VR' || window.viewerType == 'anaglyph') { effect.setSize( window.innerWidth, window.innerHeight ); }

}
async function set_bounds() {
    Papa.parse("http://localhost:8000/data/Boundaries.csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: false,
        complete: function(results) {
            bounds = results.data;
            var nD = bounds.length;
            // console.log(nD);
            origin.x = ( bounds[0].Low + bounds[0].High)/2. ;
            origin.y = ( bounds[1].Low + bounds[1].High)/2. ;
            origin.z = ( bounds[2].Low + bounds[2].High)/2. ;

            params.radius = 3*Math.max( (bounds[0].High - bounds[0].Low),
                                         Math.max((bounds[1].High - bounds[1].Low),
                                                  (bounds[2].High - bounds[2].Low)) );
            reposition_camera();
        }
    });}

function make_initial_spheres() {
Papa.parse("http://localhost:8000/data/"+fname+0+".csv", {
    download: true,
    dynamicTyping: true,
    header: true,
    complete: function(results) {
        var container = new THREE.Object3D;
        container.name = 'container';
        scene.add( container );
        spheres = results.data;
        var numSpheres = spheres.length;
        var c = new THREE.Color(1,1,1);
        var geometry = new THREE.SphereGeometry( 1, 32, 32 );
        var material = new THREE.MeshPhongMaterial({ color: 0xffffff, });
        for (i = 0; i<numSpheres; i++) {
            var mesh = new THREE.Mesh( geometry, material );
            container.add( mesh );
        }
    }
});}
function update_spheres(t) {
    Papa.parse("http://localhost:8000/data/"+fname+t+".csv", {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: false,
        complete: function(results) {
            spheres = results.data;
            var numSpheres = spheres.length;
            var c = scene.getObjectByName('container');
            for (i = 0; i<numSpheres; i++) {
                var mesh = c.children[i];
                mesh.position.x = spheres[i].X;
                mesh.position.y = spheres[i].Y;
                mesh.position.z = spheres[i].Z;
                var R_draw = Math.sqrt( Math.pow(spheres[i].R,2.) -
                                        Math.pow( (params.W - spheres[i].W), 2) );
                if (isNaN(R_draw)) {
                    mesh.visible = false;
                }
                else {
                    mesh.visible = true;
                    mesh.scale.x = R_draw;
                    mesh.scale.y = R_draw;
                    mesh.scale.z = R_draw;
                    // console.log(i, R_draw);
                }
            }
        }
    });}

function mouseMove(e){

    // camera.position.x += Math.max(Math.min((e.clientX - mouse.x) * 0.01, cameraMoves.speed), -cameraMoves.speed);
    // camera.position.y += Math.max(Math.min((mouse.y - e.clientY) * 0.01, cameraMoves.speed), -cameraMoves.speed);
    params.theta += Math.max(Math.min((e.clientY - mouse.y) * 0.001, cameraMoves.speed), -cameraMoves.speed);
    params.phi   += Math.max(Math.min((e.clientX - mouse.x) * 0.001, cameraMoves.speed), -cameraMoves.speed);

        mouse.x = e.clientX;
        mouse.y = e.clientY;
}

function animate() {
    if (params.W != W_old) {
        update_spheres(params.time);
        W_old = params.W;
    }
    if (params.time != t_old) {
        update_spheres(params.time);
        t_old = params.time;
    }
    if (window.viewerType == 'VR' || window.viewerType == 'anaglyph') { renderer.setAnimationLoop( render ); }//{ effect.requestAnimationFrame( animate ); }
    else { window.requestAnimationFrame( animate ); };
    render();

};

function reposition_camera() {
    // if (params.theta > 1) {params.theta -= 1;};
    // if (params.theta < 0) {params.theta += 1;};

    // if (params.theta > 0.5) {params.theta -= 1;};
    // if (params.theta < -0.5) {params.theta += 1;};

    camera.position.x = origin.x + params.radius * Math.sin( params.theta*3.1416*2 ) * Math.cos( params.phi*3.1416*2 );
    camera.position.z = origin.z + params.radius * Math.sin( params.theta*3.1416*2 ) * Math.sin( params.phi*3.1416*2 );
    camera.position.y = origin.y + params.radius * Math.cos( params.theta*3.1416*2 );

    camera.lookAt( origin );
};

function render() {

    // if (window.viewerType == 'keyboard') {
        document.onkeydown = function(e) {
            switch (e.keyCode) {
                case 37: // left key
                    if (params.W >= params.W_min) {params.W -= 0.01}
                    break;
                case 39: // right key
                    if (params.W <= params.W_max) {params.W += 0.01}
                    break;
                case 40: // down key
                    if (params.time >= 1) {params.time -= 1}
                    break;
                case 38: // up key
                    params.time += 1;
                    break;
                case 65: // a key
                    params.phi -= 0.01;
                    break;
                case 68: // d key
                    params.phi += 0.01;
                    break;
                case 70: // f key
                    if (params.radius >= 0) {params.radius += 0.05}
                    break;
                case 82: // r key
                    if (params.radius >= 0) {params.radius -= 0.05}
                    break;
                case 87: // w key
                    params.theta += 0.01
                    break;
                case 83: // s key
                    params.theta -= 0.01
                    break;
            }
        }
    reposition_camera();
    // };


    controls.update();
    // if (window.viewerType == 'VR' || window.viewerType == 'anaglyph') { effect.render( scene, camera ); }
    // else { renderer.render( scene, camera ); };
    renderer.render(scene, camera);

};
