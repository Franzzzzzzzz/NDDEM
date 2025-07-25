import * as THREE from "three";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader.js";

const urlParams = new URLSearchParams(window.location.search);
const recorder = new CCapture({
    verbose: true,
    display: true,
    framerate: 30,
    quality: 100,
    format: 'png',
    timeLimit: 100,
    frameLimit: 0,
    autoSaveTime: 0
});

// Graphics variables
var container, stats;
var camera, controls, scene, renderer;
var textureLoader;
var clock = new THREE.Clock();

// Physics variables
var gravityConstant = - 9.8;
var collisionConfiguration;
var dispatcher;
var broadphase;
var solver;
var softBodySolver;
var physicsWorld;
var rigidBodies = [];
var margin = 0.01;
var rope;
var ropes = [];
var ballRadius = 0.1;
var earth;
var stars;
var record = false;

var screenshot = false;
var root_dir = window.location.origin + '/';
if (window.location.hostname.includes('benjymarks')) { root_dir = 'https://www.benjymarks.com/nddem/' }
else if (window.location.hostname.includes('github')) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/' }

var gravity; var scale_vector; var transformAux1;

Ammo().then(function (AmmoLib) {

    Ammo = AmmoLib;

    // var gravity = new Ammo.btVector3(0,0,0);
    // var scale_vector = new Ammo.btVector3(0,0,0);
    transformAux1 = new Ammo.btTransform();

    init();
    animate();

});

function init() {
    initGraphics();
    initPhysics();
    createObjects();
    makeStars();
    if (urlParams.has('record')) { addRecordOnKeypress() };
}

function addRecordOnKeypress() {
    document.addEventListener("keydown", function (event) {
        if (event.code == 'Space') {
            if (record) {
                recorder.stop();
                recorder.save();
            }
            else {
                recorder.start();
            }
            record = !record;
        }
    }, false);
}

function initGraphics() {
    container = document.getElementById('container');
    camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 2000);
    scene = new THREE.Scene();
    // var axesHelper = new THREE.AxesHelper( 5 ); // X - red, Y - green, Z - blue
    // scene.add( axesHelper );
    scene.background = new THREE.Color(0x111111); // revealjs background
    // scene.background = new THREE.Color( 0xe72564 ); // pink background

    camera.position.set(0, -1, 3);
    camera.rotation.z = 3. * Math.PI / 4;
    // camera.up.set(-1,-1,0);
    camera.lookAt(-0, -0, 0);

    // controls = new THREE.OrbitControls( camera );
    // controls.target.set( 0, 0, 0 );
    // controls.update();

    if (screenshot) {
        renderer = new THREE.WebGLRenderer({ preserveDrawingBuffer: true }); // for screenshots
    }
    else { renderer = new THREE.WebGLRenderer(); }

    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.shadowMap.enabled = true;

    textureLoader = new THREE.TextureLoader();

    var ambientLight = new THREE.AmbientLight(0xffffff);
    scene.add(ambientLight);

    var light = new THREE.DirectionalLight(0xffffff, 1);
    light.position.set(- 10, 2, 50);
    light.castShadow = true;
    light.shadow.mapSize.x = 2048;
    light.shadow.mapSize.y = 2048;
    scene.add(light);

    var light2 = new THREE.PointLight(0xffffff, 1);
    light2.position.set(0, 0.2, 0);
    light2.castShadow = true;
    light2.shadow.mapSize.x = 2048;
    light2.shadow.mapSize.y = 2048;
    scene.add(light2);

    var light_ambient = new THREE.AmbientLight(0x404040); // soft white light
    scene.add(light_ambient);

    container.innerHTML = "";

    container.appendChild(renderer.domElement);
    //
    // stats = new Stats();
    // stats.domElement.style.position = 'absolute';
    // stats.domElement.style.top = '0px';
    // container.appendChild( stats.domElement );

    window.addEventListener('resize', onWindowResize, false);

    // new THREE.GLTFLoader()
    //     // .setPath( 'http://localhost:54321/visualise/resources/' )
    //     .load( 'https://rawcdn.githack.com/benjym/CIVL5999/9d29107b940c23142a4f1b7499d6c8d2d9a81bce/Earth.glb', // resource URL
    // 	   function ( gltf ) {
    //             var s = 0.000005;
    //             earth = gltf.scene;
    //             earth.scale.set(s,s,s);
    //             earth.position.x = 0.5
    //             earth.position.y = -0.4
    //             earth.position.z = 2.2
    //             earth.rotation.x = 0.1;
    //             scene.add( earth );
    // 	       },
    //        undefined,
    //        function (err) {console.log(err);});

    new GLTFLoader()
        .setPath(root_dir + 'visualise/resources/')
        .load('1227 Earth.gltf',
            // .load( 'https://rawcdn.githack.com/benjym/CIVL5999/master/resources/1227%20Earth.gltf', // resource URL
            function (gltf) {
                var s = 0.025;
                var mesh = gltf.scene;
                // earth.applyMatrix( new THREE.Matrix4().makeTranslation( 0, -27., 0 ) );
                mesh.scale.set(s, s, s);
                mesh.position.y = - 27. * s; // 54 is original height

                earth = new THREE.Object3D();
                earth.add(mesh); // now located at origin

                earth.position.y = -0.56;
                earth.position.x = 0.37;
                // earth.position.y = - 1.18;
                // earth.rotation.y = -1.0;
                earth.rotation.z = 0.7;
                scene.add(earth);

                if (screenshot) {
                    setTimeout(function () { window.open(renderer.domElement.toDataURL('image/png'), 'screenshot'); }, 2000); // take screenshot after 3s
                }

            }
        );


}

function initPhysics() {

    // Physics configuration

    collisionConfiguration = new Ammo.btSoftBodyRigidBodyCollisionConfiguration();
    dispatcher = new Ammo.btCollisionDispatcher(collisionConfiguration);
    broadphase = new Ammo.btDbvtBroadphase();
    solver = new Ammo.btSequentialImpulseConstraintSolver();
    softBodySolver = new Ammo.btDefaultSoftBodySolver();
    physicsWorld = new Ammo.btSoftRigidDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration, softBodySolver);
    physicsWorld.setGravity(new Ammo.btVector3(0, gravityConstant, 0));
    physicsWorld.getWorldInfo().set_m_gravity(new Ammo.btVector3(0, gravityConstant, 0));

}

function createObjects() {

    var pos = new THREE.Vector3();
    var quat = new THREE.Quaternion();

    // Balls
    var numballs = 50
    var balls = [];

    var ballMass = 0.1;
    var ropeNumSegments = 10;
    var ropeLength = 1.5;
    var ropeMass = 0.1;

    var base_ball = new THREE.Mesh(new THREE.SphereGeometry(ballRadius, 20, 20), new THREE.MeshPhongMaterial({ color: 0xe72564 }));
    base_ball.castShadow = true;
    base_ball.receiveShadow = true;

    for (let n = 0; n < numballs; n++) {
        var ballShape = new Ammo.btSphereShape(ballRadius);
        // ballShape.setMargin( 10000*margin );

        var ball = base_ball.clone()
        var x = -ropeLength / 3. * Math.random();
        var z = ropeLength / 3. * Math.random();
        var y = ropeLength - Math.sqrt(Math.pow(x, 2) + Math.pow(z, 2)) //+ 0.01*(Math.random() - 0.5)
        pos.set(x, y, z);
        quat.set(0, 0, 0, 1);
        createRigidBody(ball, ballShape, ballMass, pos, quat);
        ball.userData.physicsBody.setFriction(0.5);
        ball.phi = Math.random() * Math.PI * 2.;
        balls.push(ball);

        var ropePos = ball.position.clone();
        ropePos.y += ballRadius;

        var segmentLength = ropeLength / ropeNumSegments;
        var ropeGeometry = new THREE.BufferGeometry();
        // var ropeMaterial = new THREE.MeshStandardMaterial( { color: 0xe72564 } );
        var ropeMaterial = new THREE.LineBasicMaterial({ color: 0xD2275D });
        var ropePositions = [];
        var ropeIndices = [];

        for (var i = 0; i < ropeNumSegments + 1; i++) {
            ropePositions.push(i * ropePos.x / ropeNumSegments,
                i * ropePos.y / ropeNumSegments,
                i * ropePos.z / ropeNumSegments
            );
        }
        for (var i = 0; i < ropeNumSegments; i++) { ropeIndices.push(i, i + 1); }

        ropeGeometry.setIndex(new THREE.BufferAttribute(new Uint16Array(ropeIndices), 1));
        ropeGeometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(ropePositions), 3));
        ropeGeometry.computeBoundingSphere();
        rope = new THREE.LineSegments(ropeGeometry, ropeMaterial);
        rope.castShadow = true;
        rope.receiveShadow = true;
        scene.add(rope);


        // Rope physic object
        var softBodyHelpers = new Ammo.btSoftBodyHelpers();
        var ropeStart = new Ammo.btVector3(0., 0., 0.);
        var ropeEnd = new Ammo.btVector3(ropePos.x, ropePos.y, ropePos.z);
        var ropeSoftBody = softBodyHelpers.CreateRope(physicsWorld.getWorldInfo(), ropeStart, ropeEnd, ropeNumSegments - 1, 0);
        var sbConfig = ropeSoftBody.get_m_cfg();
        sbConfig.set_kDF(0.1); // friction
        sbConfig.set_kDP(0.5); // Damping
        // sbConfig.set_kPR( pressure ); // pressure
        sbConfig.set_viterations(10);
        sbConfig.set_piterations(10);

        ropeSoftBody.setTotalMass(ropeMass, false);
        Ammo.castObject(ropeSoftBody, Ammo.btCollisionObject).getCollisionShape().setMargin(margin * 3);
        physicsWorld.addSoftBody(ropeSoftBody, 1, - 1);
        rope.userData.physicsBody = ropeSoftBody;
        // Disable deactivation
        ropeSoftBody.setActivationState(4);

        // The base
        var armMass = 0;
        var baseMaterial = new THREE.MeshPhongMaterial({ color: 0x606060 });
        quat.set(0, 0, 0, 1);

        pos.set(-0.05, -0.05, -0.05);
        var arm = createParalellepiped(0.1, 0.1, 0.1, armMass, pos, quat, baseMaterial);
        arm.visible = false;
        // arm.castShadow = true;
        // arm.receiveShadow = true;

        // Glue the rope extremes to the ball and the arm
        var influence = 1;
        ropeSoftBody.appendAnchor(0, arm.userData.physicsBody, true, influence);
        ropeSoftBody.appendAnchor(ropeNumSegments, ball.userData.physicsBody, true, influence);

        ropes.push(rope);
    }
}

function createParalellepiped(sx, sy, sz, mass, pos, quat, material) {

    var threeObject = new THREE.Mesh(new THREE.BoxGeometry(sx, sy, sz, 1, 1, 1), material);
    var shape = new Ammo.btBoxShape(new Ammo.btVector3(sx * 0.5, sy * 0.5, sz * 0.5));
    shape.setMargin(margin);

    createRigidBody(threeObject, shape, mass, pos, quat);

    return threeObject;

}

function createRigidBody(threeObject, physicsShape, mass, pos, quat) {

    threeObject.position.copy(pos);
    threeObject.quaternion.copy(quat);

    var transform = new Ammo.btTransform();
    transform.setIdentity();
    transform.setOrigin(new Ammo.btVector3(pos.x, pos.y, pos.z));
    transform.setRotation(new Ammo.btQuaternion(quat.x, quat.y, quat.z, quat.w));
    var motionState = new Ammo.btDefaultMotionState(transform);

    var localInertia = new Ammo.btVector3(0, 0, 0);
    physicsShape.calculateLocalInertia(mass, localInertia);

    var rbInfo = new Ammo.btRigidBodyConstructionInfo(mass, motionState, physicsShape, localInertia);
    var body = new Ammo.btRigidBody(rbInfo);

    threeObject.userData.physicsBody = body;

    scene.add(threeObject);

    if (mass > 0) {

        rigidBodies.push(threeObject);

        // Disable deactivation
        body.setActivationState(4);

    }

    physicsWorld.addRigidBody(body);

}

function createRandomColor() {

    return Math.floor(Math.random() * (1 << 24));

}

function createMaterial() {

    return new THREE.MeshPhongMaterial({ color: createRandomColor() });

}

function onWindowResize() {

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.lookAt(0, 0, 0);
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);

}

function animate() {

    requestAnimationFrame(animate);

    render();
    // stats.update();

}

function render() {

    var deltaTime = clock.getDelta();
    updateSizes(clock.elapsedTime, deltaTime);
    moveStars(clock.elapsedTime);
    updatePhysics(deltaTime);

    renderer.render(scene, camera);

    if (record) {
        recorder.capture(renderer.domElement);
    }

}

function makeStars() {
    var numParticles = 1000000; // added 00 to last longer for talk
    var positions = new Float32Array(numParticles * 3);
    var scales = new Float32Array(numParticles);

    for (var i = 0; i < numParticles; i++) {
        var starVertex = new THREE.Vector3();
        positions[i * 3] = (Math.random() - 0.5) * 10000; // multiplied by 10 for talk
        positions[i * 3 + 1] = (Math.random() - 0.5) * 10000; // multiplied by 10 for talk
        positions[i * 3 + 2] = - (Math.random() + 0.2) / 1.2 * 500;
        scales[i] = 2; //3 for poster
    }

    let geometry = new THREE.BufferGeometry();
    geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    geometry.setAttribute('scale', new THREE.BufferAttribute(scales, 1));

    var material = new THREE.ShaderMaterial({
        uniforms: {
            color: { value: new THREE.Color(0xffffff) },
        },
        vertexShader: document.getElementById('vertexshader').textContent,
        fragmentShader: document.getElementById('fragmentshader').textContent
    });
    //
    stars = new THREE.Points(geometry, material);
    scene.add(stars);
}

function updateSizes(t, dt) {
    if (earth !== undefined) {
        earth.rotateOnAxis(new THREE.Vector3(0, 1, 0), dt / 10.); // axis, angle
    };
    // cycle gravity direction
    const theta = Math.cos(t / 2.) / 2.;
    gravity = new Ammo.btVector3((1 - Math.sin(theta)) * gravityConstant, -Math.cos(theta) * gravityConstant, 0);
    physicsWorld.setGravity(gravity);
    physicsWorld.getWorldInfo().set_m_gravity(gravity);

    for (var i = 0, il = rigidBodies.length; i < il; i++) {
        var objThree = rigidBodies[i];
        var objPhys = objThree.userData.physicsBody;
        var scale = 1. + Math.sin(t / 2. + objThree.phi) / 2.;
        scale_vector = new Ammo.btVector3(scale, scale, scale);
        objPhys.getCollisionShape().setLocalScaling(scale_vector); // WORKS FOR....
        objThree.scale.set(scale, scale, scale);

    }
}

function moveStars(t) {
    // console.log(stars);
    stars.position.x = t / 1.;
    stars.position.y = -t / 1.;
}

function updatePhysics(deltaTime) {


    // Step world
    physicsWorld.stepSimulation(deltaTime, 10);

    // Update ropes
    for (var j = 0, il = rigidBodies.length; j < il; j++) {

        var rope = ropes[j]
        var softBody = rope.userData.physicsBody;
        var ropePositions = rope.geometry.attributes.position.array;
        var numVerts = ropePositions.length / 3;
        var nodes = softBody.get_m_nodes();
        var indexFloat = 0;
        for (var i = 0; i < numVerts; i++) {

            var node = nodes.at(i);
            var nodePos = node.get_m_x();
            ropePositions[indexFloat++] = nodePos.x();
            ropePositions[indexFloat++] = nodePos.y();
            ropePositions[indexFloat++] = nodePos.z();

        }
        rope.geometry.attributes.position.needsUpdate = true;

        var objThree = rigidBodies[j];
        var objPhys = objThree.userData.physicsBody;
        var ms = objPhys.getMotionState();
        if (ms) {

            ms.getWorldTransform(transformAux1);
            var p = transformAux1.getOrigin();
            var q = transformAux1.getRotation();
            objThree.position.set(p.x(), p.y(), p.z());
            objThree.quaternion.set(q.x(), q.y(), q.z(), q.w());
        }

    }

}
