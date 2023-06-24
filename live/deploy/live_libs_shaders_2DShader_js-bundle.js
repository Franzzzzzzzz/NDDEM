"use strict";
/*
 * ATTENTION: The "eval" devtool has been used (maybe by default in mode: "development").
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
(self["webpackChunknddem"] = self["webpackChunknddem"] || []).push([["live_libs_shaders_2DShader_js"],{

/***/ "./live/libs/shaders/2DShader.js":
/*!***************************************!*\
  !*** ./live/libs/shaders/2DShader.js ***!
  \***************************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

eval("__webpack_require__.r(__webpack_exports__);\n/* harmony export */ __webpack_require__.d(__webpack_exports__, {\n/* harmony export */   NDDEMShader: () => (/* binding */ NDDEMShader)\n/* harmony export */ });\n/* harmony import */ var three__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! three */ \"./node_modules/three/build/three.module.js\");\n\n\nvar N = 2;\nvar uniforms = {\n  N: { value: N },\n  A: { value: [] }, // Size N*N\n  R: { value: 0.5 },\n  ambient: { value: 1.0 },\n  opacity: { value: 1.0 },\n};\n\nfor (var ij = 0; ij < N * N; ij++) {\n  if (ij % N == Math.floor(ij / N)) uniforms.A.value[ij] = 1;\n  else uniforms.A.value[ij] = 0;\n}\n\nvar NDDEMShader = new three__WEBPACK_IMPORTED_MODULE_0__.ShaderMaterial({\n  uniforms: uniforms,\n  // lights: true,\n\n  vertexShader: [\n    \"uniform int N;\", // number of dimensions in simulation\n    \"uniform float A[2*2];\", // orientation matrix for this particle\n    \"uniform float R;\", // particle radius\n\n    \"varying vec3 vColor;\", // colour at vertex (output)\n    \"varying vec3 vNormal;\", // normal at vertex (output)\n\n    // \"bool isnan( float val ) { return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true; }\",\n\n    \"void main() {\",\n    \"vNormal = normal;\", // for directional lighting\n    \"const float pi = 3.14159265359;\",\n    \n    \"vec2 x;\",\n    \"vec2 x_rotated;\",\n    // \"float phi2;\",\n    // get 2d locations in x,y in coord system where center of sphere is at 0,0\n    // \"x.x = - R*sin((uv.x - 0.5)*pi);\",//*cos((uv.y - 0.5)*pi);\",\n    // \"x.y = - R*sin((uv.y - 0.5)*pi);\",\n    \"x.x = -2.0*R*(uv.x - 0.5);\",\n    \"x.y = -2.0*R*(uv.y - 0.5);\",\n    \n\n    // compute the rotated location by doing transpose(A) * x, with A the orientation matrix from the dumps\n    \"x_rotated.x = A[0]*x.x + A[2]*x.y;\",\n    \"x_rotated.y = A[1]*x.x + A[3]*x.y;\",\n\n    // \"float phi0 = atan(x_rotated.y/x_rotated.x);\",\n\n    \"vColor.r = pow(x_rotated.x/R,2.0);\",\n    \"vColor.b = pow(x_rotated.y/R,2.0);\",\n    // \"vColor.r = phi0;\",\n    // \"vColor.r = abs(sin(phi0*4.0));\",\n    // \"vColor.b = abs(cos(phi0*4.0));\",\n    // \"vColor.g = vColor * abs(sin(phi0));\",\n    // \"vColor = vColor * abs(sin(phi0));\",\n\n    \"gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );\",\n\n    \"}\",\n  ].join(\"\\n\"),\n\n  fragmentShader: [\n    \"uniform float ambient;\", // brightness of particle\n    \"uniform float opacity;\",\n    // \"varying vec3 vNormal;\",\n    \"varying vec3 vColor;\",\n\n    \"void main() {\",\n\n    // add directional lighting\n    // \"vec3 light = vec3( 0, 0, -1 );\", // bit of trial and error here\n    // \"light = normalize( light );\",\n    // \"float directional = max( dot( vNormal, light ), 0.0 );\",\n    // \"gl_FragColor = vec4( 0.6*( ambient + directional ) * vColor, opacity );\", // colours by vertex colour\n\n    // no directional lighting\n    // \"const float ambient = 1.0;\",\n    \"gl_FragColor = vec4( ( ambient ) * vColor, opacity );\", // colours by vertex colour\n\n    \"}\",\n  ].join(\"\\n\"),\n});\n\n\n\n\n//# sourceURL=webpack://nddem/./live/libs/shaders/2DShader.js?");

/***/ })

}]);