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

/***/ "./visualise/examples/js/slice.js":
/*!****************************************!*\
  !*** ./visualise/examples/js/slice.js ***!
  \****************************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

eval("__webpack_require__.r(__webpack_exports__);\n/* harmony import */ var three__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! three */ \"./node_modules/three/build/three.module.js\");\n\n\nconst urlParams = new URLSearchParams(window.location.search);\nconst recorder = new CCapture({\n    verbose: true,\n    display: true,\n    framerate: 30,\n    quality: 100,\n    format: 'png',\n    timeLimit: 100,\n    frameLimit: 0,\n    autoSaveTime: 0\n});\nlet container = document.createElement(\"div\");\ndocument.body.appendChild(container);\nlet N_tag = document.createElement( 'div' );\nN_tag.setAttribute(\"id\", \"N_tag\");\nN_tag.innerHTML = \"3D\";\ncontainer.appendChild(N_tag);\n\nfunction update_spheres(x) {\n    var R_draw = Math.sqrt(1 - Math.abs(x));\n    if ( R_draw == 0 ) { circle.visible = false; }\n    else {\n        circle.visible = true;\n        circle.scale.set(R_draw,R_draw,R_draw);\n    };\n\n    wall.position.x = x;\n};\nvar record = false; var sign = 1;\nvar controls;\nvar slice = {'loc':-1};\nvar scene = new three__WEBPACK_IMPORTED_MODULE_0__.Scene();\nscene.background = new three__WEBPACK_IMPORTED_MODULE_0__.Color( 0x111111 );\nvar camera = new three__WEBPACK_IMPORTED_MODULE_0__.PerspectiveCamera( 75, window.innerWidth/window.innerHeight, 0.1, 1000 );\ncamera.position.z = 4;\ncamera.position.x = 1.5;\n\nif ( urlParams.has('record') ) { addRecordOnKeypress() };\nfunction addRecordOnKeypress() {\n    document.addEventListener(\"keydown\", function(event) {\n        if (event.code == 'Space') {\n            if (record) {\n                recorder.stop();\n                recorder.save();\n            }\n            else {\n                recorder.start();\n            }\n            record = !record;\n        }\n    }, false);\n}\n\nvar renderer = new three__WEBPACK_IMPORTED_MODULE_0__.WebGLRenderer();\n// var controls = new THREE.TrackballControls( camera, renderer.domElement );\nrenderer.setPixelRatio( window.devicePixelRatio );\nrenderer.setSize( window.innerWidth, window.innerHeight );\nrenderer.shadowMap.enabled = true;\ncontainer.appendChild( renderer.domElement );\n\nvar background_light = new three__WEBPACK_IMPORTED_MODULE_0__.AmbientLight( 0x777777 );\nscene.add( background_light );\nvar light = new three__WEBPACK_IMPORTED_MODULE_0__.PointLight(0x999999);\nlight.position.z = 8\nlight.position.x = 5\nscene.add( light );\n\n\nvar sphere_geometry = new three__WEBPACK_IMPORTED_MODULE_0__.SphereGeometry( 1, 256, 256 );\nvar circle_geometry = new three__WEBPACK_IMPORTED_MODULE_0__.CircleGeometry( 1, 256 );\nvar wall_geometry = new three__WEBPACK_IMPORTED_MODULE_0__.PlaneBufferGeometry( 1, 1 );\nvar material = new three__WEBPACK_IMPORTED_MODULE_0__.MeshStandardMaterial( { color: 0xeeeeee } );\nvar wall_material = new three__WEBPACK_IMPORTED_MODULE_0__.MeshStandardMaterial( { color: 0xe72564 } );\n\nvar sphere  = new three__WEBPACK_IMPORTED_MODULE_0__.Mesh( sphere_geometry, material );\nvar circle  = new three__WEBPACK_IMPORTED_MODULE_0__.Mesh( circle_geometry, material );\nvar wall  = new three__WEBPACK_IMPORTED_MODULE_0__.Mesh( wall_geometry, wall_material );\nsphere.position.x = 0\ncircle.position.x = 3\ncircle.visible = false;\nwall.rotation.y = Math.PI/2.;\nwall.position.x = slice.loc;\nwall.scale.set(4,4,4);\n\nscene.add( sphere );\nscene.add( circle );\nscene.add( wall );\n\nvar gui = new dat.GUI();\ngui.add( slice, 'loc').min(-1).max(1).step(0.01).listen().name('Slice').onChange( function( val ) { update_spheres(val); }) ;\ngui.open();\n\nvar animate = function () {\n    if ( controls !== undefined) { controls.update(); }\n    requestAnimationFrame( animate );\n    renderer.render( scene, camera );\n    if ( record ) {\n        recorder.capture(renderer.domElement);\n        slice.loc += sign*0.05; update_spheres(slice.loc);\n        if ( slice.loc > 1) {sign = -1;}\n        else if( slice.loc < -1) { sign = 1;}\n    }\n};\nwindow.addEventListener( 'resize', onWindowResize, false );\nanimate();\n\nfunction onWindowResize() {\n    camera.aspect = window.innerWidth / window.innerHeight;\n    camera.updateProjectionMatrix();\n    renderer.setSize( window.innerWidth, window.innerHeight );\n    if ( controls !== undefined) { controls.handleResize(); }\n};\n\n\n//# sourceURL=webpack://nddem/./visualise/examples/js/slice.js?");

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
/******/ 	var __webpack_exports__ = __webpack_require__("./visualise/examples/js/slice.js");
/******/ 	
/******/ })()
;