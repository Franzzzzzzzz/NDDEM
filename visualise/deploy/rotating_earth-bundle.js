/*
 * ATTENTION: The "eval" devtool has been used (maybe by default in mode: "development").
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
/******/ (() => { // webpackBootstrap
/******/ 	var __webpack_modules__ = ({

/***/ "./visualise/examples/js/rotating_earth.js":
/*!*************************************************!*\
  !*** ./visualise/examples/js/rotating_earth.js ***!
  \*************************************************/
/***/ (() => {

eval("var root_dir = window.location.origin + '/';\nif ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/'}\nelse if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/'; cache=true; }\n\nfunction update_texture(t) {\n    if ( sphere !== undefined ) {\n        sphere.material.map.offset.x = t/10.;/// console.log(t);\n    }\n};\n\nvar fname = \"visualise/resources/earthmap.jpg\";\n\nconst urlParams = new URLSearchParams(window.location.search);\nconst recorder = new CCapture({\n    verbose: true,\n    display: true,\n    framerate: 30,\n    quality: 100,\n    format: 'png',\n    timeLimit: 100,\n    frameLimit: 0,\n    autoSaveTime: 0\n});\nvar record = false;\n\nif ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };\n\nlet clock = new THREE.Clock();\nvar scene = new THREE.Scene();\nscene.background = new THREE.Color( 0x111111 );\nvar camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );\ncamera.position.z = 6;\ncamera.position.x = 1.5;\n\nvar renderer = new THREE.WebGLRenderer();\n// var controls = new THREE.TrackballControls( camera, renderer.domElement );\nrenderer.setSize( window.innerWidth, window.innerHeight );\ndocument.body.appendChild( renderer.domElement );\n\nvar background_light = new THREE.AmbientLight( 0xffffff );\nscene.add( background_light );\nvar light = new THREE.DirectionalLight(0x505050);\nlight.position.x = -2\nlight.position.z = 2\nscene.add( light );\n\nvar sphere;\nvar rect;\n\nvar loader = new THREE.TextureLoader()\n    .load( root_dir + fname, function( texture ) {\n        texture.wrapS = THREE.RepeatWrapping;\n        texture.wrapT = THREE.RepeatWrapping;\n        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );\n        var rect_geometry = new THREE.PlaneBufferGeometry( 1, 1 );\n\n        var material = new THREE.MeshStandardMaterial( { map: texture } );\n        sphere = new THREE.Mesh( sphere_geometry, material );\n        rect = new THREE.Mesh( rect_geometry, material );\n        sphere.position.x = 4\n        rect.position.x = 0\n        rect.scale.set(4,2,1);\n        sphere.rotation.y = Math.PI/2.;\n\n        scene.add( sphere );\n        scene.add( rect );\n    } );\n\n// var gui = new dat.GUI();\n// gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;\n// gui.open();\n\nif ( urlParams.has('record') ) { addRecordOnKeypress() };\n\nfunction addRecordOnKeypress() {\n    document.addEventListener(\"keydown\", function(event) {\n        if (event.code == 'Space') {\n            if (record) {\n                recorder.stop();\n                recorder.save();\n            }\n            else {\n                recorder.start();\n            }\n            record = !record;\n        }\n    }, false);\n}\n\nvar animate = function () {\n    // controls.update();\n    update_texture( clock.getElapsedTime() );\n    requestAnimationFrame( animate );\n    renderer.render( scene, camera );\n    if ( record ) {\n        recorder.capture(renderer.domElement);\n    }\n\n};\nwindow.addEventListener( 'resize', onWindowResize, false );\nanimate();\n\nfunction onWindowResize() {\n    camera.aspect = window.innerWidth / window.innerHeight;\n    camera.updateProjectionMatrix();\n    renderer.setSize( window.innerWidth, window.innerHeight );\n    if ( controls !== undefined ) { controls.handleResize(); }\n};\n\n\n//# sourceURL=webpack://nddem/./visualise/examples/js/rotating_earth.js?");

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module can't be inlined because the eval devtool is used.
/******/ 	var __webpack_exports__ = {};
/******/ 	__webpack_modules__["./visualise/examples/js/rotating_earth.js"]();
/******/ 	
/******/ })()
;