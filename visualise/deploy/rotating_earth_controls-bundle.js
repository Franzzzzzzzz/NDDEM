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

/***/ "./visualise/examples/js/rotating_earth_controls.js":
/*!**********************************************************!*\
  !*** ./visualise/examples/js/rotating_earth_controls.js ***!
  \**********************************************************/
/***/ (() => {

eval("var root_dir = window.location.origin + '/';\nif ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/'}\nelse if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/'; cache=true; }\n\nfunction update_texture( rot ) {\n    if ( sphere1 !== undefined ) {\n        sphere1.rotation.x = rot.x;\n        sphere1.rotation.y = rot.y;\n        sphere1.rotation.z = rot.z;/// console.log(t);\n    }\n};\n\nvar fname = \"./visualise/resources/earthmap.jpg\";\nvar rot = {'x':0,'y':0,'z':0};\n\nconst urlParams = new URLSearchParams(window.location.search);\nif ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };\n\nlet clock = new THREE.Clock();\nvar scene = new THREE.Scene();\nscene.background = new THREE.Color( 0x111111 );\nvar camera = new THREE.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );\ncamera.position.z = 5;\n// camera.position.x = 1.5;\n// camera.position.y = -1.5;\n\nvar renderer = new THREE.WebGLRenderer();\nvar controls = new THREE.OrbitControls( camera, renderer.domElement );\nrenderer.setSize( window.innerWidth, window.innerHeight );\ndocument.body.appendChild( renderer.domElement );\n\nvar background_light = new THREE.AmbientLight( 0xffffff );\nscene.add( background_light );\nvar light = new THREE.DirectionalLight(0xaaaaaa);\nlight.position.x = -2\nlight.position.z = 2\nscene.add( light );\n\nvar sphere1, sphere2, sphere3,sphere4, sphere5, sphere6;\nvar axesHelper = new THREE.AxesHelper(2);\nscene.add( axesHelper );\n\nvar loader1 = new THREE.TextureLoader()\n    .load( root_dir + fname, function( texture ) {\n        texture.wrapS = THREE.RepeatWrapping;\n        texture.wrapT = THREE.RepeatWrapping;\n        var sphere_geometry = new THREE.SphereGeometry( 1, 32, 32 );\n\n        var material = new THREE.MeshStandardMaterial( { map: texture } );\n        sphere1 = new THREE.Mesh( sphere_geometry, material );\n        // sphere1.position.x = 2.5\n        // sphere1.position.y = 0\n        // sphere1.position.y = 2.5\n        //sphere1.rotation.y = Math.PI/2.;\n\n        scene.add( sphere1 );\n    } );\n\nvar gui = new dat.GUI();\ngui.add( rot, 'x').min(-Math.PI).max(Math.PI).step(0.01).listen().name('x Rotation').onChange( function( val ) { update_texture(rot); }) ;\ngui.add( rot, 'y').min(-Math.PI).max(Math.PI).step(0.01).listen().name('y Rotation').onChange( function( val ) { update_texture(rot); }) ;\ngui.add( rot, 'z').min(-Math.PI).max(Math.PI).step(0.01).listen().name('z Rotation').onChange( function( val ) { update_texture(rot); }) ;\ngui.open();\n\nvar animate = function () {\n    controls.update();\n    // update_texture( clock.getElapsedTime() );\n    requestAnimationFrame( animate );\n    renderer.render( scene, camera );\n\n};\nwindow.addEventListener( 'resize', onWindowResize, false );\nanimate();\n\nfunction onWindowResize() {\n    camera.aspect = window.innerWidth / window.innerHeight;\n    camera.updateProjectionMatrix();\n    renderer.setSize( window.innerWidth, window.innerHeight );\n    controls.handleResize();\n};\n\n\n//# sourceURL=webpack://nddem/./visualise/examples/js/rotating_earth_controls.js?");

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module can't be inlined because the eval devtool is used.
/******/ 	var __webpack_exports__ = {};
/******/ 	__webpack_modules__["./visualise/examples/js/rotating_earth_controls.js"]();
/******/ 	
/******/ })()
;