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

/***/ "./visualise/examples/js/multiple_rotating_earths.js":
/*!***********************************************************!*\
  !*** ./visualise/examples/js/multiple_rotating_earths.js ***!
  \***********************************************************/
/***/ (() => {

eval("var root_dir = window.location.origin + '/';\nif ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/' }\nelse if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/' }\n\nfunction update_texture(t) {\n    if ( sphere1 !== undefined ) {\n        sphere1.rotation.z = -t/5.;/// console.log(t);\n    }\n    if ( sphere2 !== undefined ) {\n        sphere2.rotation.y = -t/5.;/// console.log(t);\n    }\n    if ( sphere3 !== undefined ) {\n        sphere3.rotation.x = t/5.;/// console.log(t);\n    }\n    if ( sphere4 !== undefined ) {\n        sphere4.rotation.z = t/5.;/// console.log(t);\n    }\n    if ( sphere5 !== undefined ) {\n        sphere5.rotation.y = t/5.;/// console.log(t);\n    }\n    if ( sphere6 !== undefined ) {\n        sphere6.rotation.x = -t/5.;/// console.log(t);\n    }\n};\n\nvar fname = \"visualise/resources/earthmap.jpg\";\n\nconst urlParams = new URLSearchParams(window.location.search);\nif ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };\n\nlet clock = new THREE.Clock();\nvar scene = new THREE.Scene();\nscene.background = new THREE.Color( 0x111111 );\nvar camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );\ncamera.position.z = 10;\n// camera.position.x = 1.5;\n// camera.position.y = -1.5;\n\nvar renderer = new THREE.WebGLRenderer();\n// var controls = new THREE.TrackballControls( camera, renderer.domElement );\nrenderer.setSize( window.innerWidth, window.innerHeight );\ndocument.body.appendChild( renderer.domElement );\n\nvar background_light = new THREE.AmbientLight( 0xffffff );\nscene.add( background_light );\nvar light = new THREE.DirectionalLight(0xaaaaaa);\nlight.position.x = -2\nlight.position.z = 2\nscene.add( light );\n\nvar sphere1, sphere2, sphere3,sphere4, sphere5, sphere6;\n\nvar loader1 = new THREE.TextureLoader()\n    .load( root_dir + fname, function( texture ) {\n        texture.wrapS = THREE.RepeatWrapping;\n        texture.wrapT = THREE.RepeatWrapping;\n        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );\n\n        var material = new THREE.MeshStandardMaterial( { map: texture } );\n        sphere1 = new THREE.Mesh( sphere_geometry, material );\n        sphere1.position.x = 2.5\n        sphere1.position.y = 0\n        // sphere1.position.y = 2.5\n        //sphere1.rotation.y = Math.PI/2.;\n\n        scene.add( sphere1 );\n\n        sphere2 = new THREE.Mesh( sphere_geometry, material );\n        sphere2.position.x = 2.5\n        sphere2.position.y = 2.5\n        //sphere2.rotation.y = Math.PI/2.;\n\n        scene.add( sphere2 );\n\n        sphere3 = new THREE.Mesh( sphere_geometry, material );\n        sphere3.position.x = 0\n        sphere3.position.y = 2.5\n        // sphere3.position.y = -2.5\n        //sphere3.rotation.y = Math.PI/2.;\n\n        scene.add( sphere3 );\n\n        sphere4 = new THREE.Mesh( sphere_geometry, material );\n        // sphere4.position.x = 2.5\n        sphere4.position.y = -2.5\n        // sphere1.position.y = 2.5\n        //sphere1.rotation.y = Math.PI/2.;\n\n        scene.add( sphere4 );\n\n        sphere5 = new THREE.Mesh( sphere_geometry, material );\n        sphere5.position.x = -2.5\n        sphere5.position.y = -2.5\n        //sphere2.rotation.y = Math.PI/2.;\n\n        scene.add( sphere5 );\n\n        sphere6 = new THREE.Mesh( sphere_geometry, material );\n        sphere6.position.x = -2.5\n        // sphere6.position.y = 2.5\n        // sphere3.position.y = -2.5\n        //sphere3.rotation.y = Math.PI/2.;\n\n        scene.add( sphere6 );\n    } );\n\n// var gui = new dat.GUI();\n// gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;\n// gui.open();\n\nvar animate = function () {\n    // controls.update();\n    update_texture( clock.getElapsedTime() );\n    requestAnimationFrame( animate );\n    renderer.render( scene, camera );\n\n};\nwindow.addEventListener( 'resize', onWindowResize, false );\nanimate();\n\nfunction onWindowResize() {\n    camera.aspect = window.innerWidth / window.innerHeight;\n    camera.updateProjectionMatrix();\n    renderer.setSize( window.innerWidth, window.innerHeight );\n    controls.handleResize();\n};\n\n\n//# sourceURL=webpack://nddem/./visualise/examples/js/multiple_rotating_earths.js?");

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module can't be inlined because the eval devtool is used.
/******/ 	var __webpack_exports__ = {};
/******/ 	__webpack_modules__["./visualise/examples/js/multiple_rotating_earths.js"]();
/******/ 	
/******/ })()
;