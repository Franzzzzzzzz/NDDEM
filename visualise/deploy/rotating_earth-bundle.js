/*
 * ATTENTION: The "eval" devtool has been used (maybe by default in mode: "development").
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./visualise/examples/js/rotating_earth.js":
/*!*************************************************!*\
  !*** ./visualise/examples/js/rotating_earth.js ***!
  \*************************************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

eval("__webpack_require__.r(__webpack_exports__);\n/* harmony import */ var three__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! three */ \"./node_modules/three/build/three.module.js\");\n\n// import { GUI } from 'three/examples/jsm/libs/lil-gui.module.min.js';\n\nvar root_dir = window.location.origin + '/';\nif ( window.location.hostname.includes('benjymarks') ) { root_dir = 'http://www.benjymarks.com/nddem/'}\nelse if ( window.location.hostname.includes('github') ) { root_dir = 'https://franzzzzzzzz.github.io/NDDEM/'; var cache=true; }\n\nfunction update_texture(t) {\n    if ( sphere !== undefined ) {\n        sphere.material.map.offset.x = t/10.;/// console.log(t);\n    }\n};\n\nvar fname = \"visualise/resources/earthmap.jpg\";\n\nconst urlParams = new URLSearchParams(window.location.search);\nconst recorder = new CCapture({\n    verbose: true,\n    display: true,\n    framerate: 30,\n    quality: 100,\n    format: 'png',\n    timeLimit: 100,\n    frameLimit: 0,\n    autoSaveTime: 0\n});\nvar record = false;\n\nif ( urlParams.has('fname') ) { fname = urlParams.get('fname'); };\n\nlet clock = new three__WEBPACK_IMPORTED_MODULE_0__.Clock();\nvar scene = new three__WEBPACK_IMPORTED_MODULE_0__.Scene();\nscene.background = new three__WEBPACK_IMPORTED_MODULE_0__.Color( 0x111111 );\nvar camera = new three__WEBPACK_IMPORTED_MODULE_0__.PerspectiveCamera( 50, window.innerWidth/window.innerHeight, 0.1, 1000 );\ncamera.position.z = 6;\ncamera.position.x = 1.5;\n\nvar renderer = new three__WEBPACK_IMPORTED_MODULE_0__.WebGLRenderer();\n// var controls = new THREE.TrackballControls( camera, renderer.domElement );\nrenderer.setSize( window.innerWidth, window.innerHeight );\ndocument.body.appendChild( renderer.domElement );\n\nvar background_light = new three__WEBPACK_IMPORTED_MODULE_0__.AmbientLight( 0xffffff );\nscene.add( background_light );\nvar light = new three__WEBPACK_IMPORTED_MODULE_0__.DirectionalLight(0x505050);\nlight.position.x = -2\nlight.position.z = 2\nscene.add( light );\n\nvar sphere;\nvar rect;\n\nvar loader = new three__WEBPACK_IMPORTED_MODULE_0__.TextureLoader()\n    .load( root_dir + fname, function( texture ) {\n        texture.wrapS = three__WEBPACK_IMPORTED_MODULE_0__.RepeatWrapping;\n        texture.wrapT = three__WEBPACK_IMPORTED_MODULE_0__.RepeatWrapping;\n        var sphere_geometry = new three__WEBPACK_IMPORTED_MODULE_0__.SphereGeometry( 1, 32, 32 );\n        var rect_geometry = new three__WEBPACK_IMPORTED_MODULE_0__.PlaneBufferGeometry( 1, 1 );\n\n        var material = new three__WEBPACK_IMPORTED_MODULE_0__.MeshStandardMaterial( { map: texture } );\n        sphere = new three__WEBPACK_IMPORTED_MODULE_0__.Mesh( sphere_geometry, material );\n        rect = new three__WEBPACK_IMPORTED_MODULE_0__.Mesh( rect_geometry, material );\n        sphere.position.x = 4\n        rect.position.x = 0\n        rect.scale.set(4,2,1);\n        sphere.rotation.y = Math.PI/2.;\n\n        scene.add( sphere );\n        scene.add( rect );\n    } );\n\n// var gui = new GUI();\n// gui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;\n// gui.open();\n\nif ( urlParams.has('record') ) { addRecordOnKeypress() };\n\nfunction addRecordOnKeypress() {\n    document.addEventListener(\"keydown\", function(event) {\n        if (event.code == 'Space') {\n            if (record) {\n                recorder.stop();\n                recorder.save();\n            }\n            else {\n                recorder.start();\n            }\n            record = !record;\n        }\n    }, false);\n}\n\nvar animate = function () {\n    // controls.update();\n    update_texture( clock.getElapsedTime() );\n    requestAnimationFrame( animate );\n    renderer.render( scene, camera );\n    if ( record ) {\n        recorder.capture(renderer.domElement);\n    }\n\n};\nwindow.addEventListener( 'resize', onWindowResize, false );\nanimate();\n\nfunction onWindowResize() {\n    camera.aspect = window.innerWidth / window.innerHeight;\n    camera.updateProjectionMatrix();\n    renderer.setSize( window.innerWidth, window.innerHeight );\n    if ( controls !== undefined ) { controls.handleResize(); }\n};\n\n\n//# sourceURL=webpack://nddem/./visualise/examples/js/rotating_earth.js?");

/***/ }),

/***/ "./node_modules/three/build/three.module.js":
/*!**************************************************!*\
  !*** ./node_modules/three/build/three.module.js ***!
  \**************************************************/
/***/ ((__unused_webpack___webpack_module__, __webpack_exports__, __webpack_require__) => {


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/define property getters */
/******/ 	(() => {
/******/ 		// define getter functions for harmony exports
/******/ 		__webpack_require__.d = (exports, definition) => {
/******/ 			for(var key in definition) {
/******/ 				if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/ 					Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	(() => {
/******/ 		__webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/make namespace object */
/******/ 	(() => {
/******/ 		// define __esModule on exports
/******/ 		__webpack_require__.r = (exports) => {
/******/ 			if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 				Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 			}
/******/ 			Object.defineProperty(exports, '__esModule', { value: true });
/******/ 		};
/******/ 	})();
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module can't be inlined because the eval devtool is used.
/******/ 	var __webpack_exports__ = __webpack_require__("./visualise/examples/js/rotating_earth.js");
/******/ 	
/******/ })()
;