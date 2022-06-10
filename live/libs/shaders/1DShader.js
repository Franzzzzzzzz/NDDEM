import { ShaderMaterial } from "three";

var N = 2;
var uniforms = {
  N: { value: N },
  // N_lines: { value: 5.0 },
  //A: { value: new THREE.Matrix4() },
  A: { value: [] }, // Size N*N
  xview: { value: [] }, //Size N-3
  xpart: { value: [] }, //Size N-3
  // x4: { value: 0 },
  // x4p: { value: 0 },
  R: { value: 0.5 },
  ambient: { value: 1.0 },
  opacity: { value: 1.0 },
};

for (var ij = 0; ij < N - 3; ij++) {
  uniforms.xview.value[ij] = 0.0;
  uniforms.xpart.value[ij] = 0.0;
}
if (N > 3) {
  uniforms.x4.value = 0.0;
}
for (var ij = 0; ij < N * N; ij++) {
  if (ij % N == Math.floor(ij / N)) uniforms.A.value[ij] = 1;
  else uniforms.A.value[ij] = 0;
}

var NDDEMShader = new ShaderMaterial({
  uniforms: uniforms,
  // lights: true,

  vertexShader: [
    "uniform int N;", // number of dimensions in simulation
    "uniform float N_lines;", // number of lines to render across particle
    "uniform float A[3*3];", // orientation matrix for this particle
    "uniform float R;", // particle radius

    "varying vec3 vColor;", // colour at vertex (output)
    "varying vec3 vNormal;", // normal at vertex (output)

    "bool isnan( float val ) { return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true; }",

    "void main() {",
    "vNormal = normal;", // for directional lighting
    "const float pi = 3.14159265359;",
    "float R_draw;", // radius particle will be drawn at
    // "float R_draw_squared = pow(R,2.0) ;",
    // "if ( R_draw_squared > 0.0 ) {", // only if visible
    // "R_draw = sqrt(R_draw_squared);",
    "vec4 x;",
    "vec4 x_rotated;",
    // "float phi2;",
    // get 3d locations in x,y,z,w in coord system where center of sphere is at 0,0,0,0
    "x.y = R*cos((uv.y-0.5)*pi)*cos((uv.x-0.5)*2.0*pi);",
    "x.z = - R*cos((uv.y-0.5)*pi)*sin((uv.x-0.5)*2.0*pi);",
    "x.x = - R*sin((uv.y-0.5)*pi);",
    //x.w = x4 - x4p;

    // compute the rotated location by doing transpose(A) * x, with A the orientation matrix from the dumps
    "x_rotated.x = A[0]*x.x + A[3]*x.y + A[6]*x.z ;",
    "x_rotated.y = A[1]*x.x + A[4]*x.y + A[7]*x.z ;",
    "x_rotated.z = A[2]*x.x + A[5]*x.y + A[8]*x.z ;",
    "x_rotated.w=0. ;",

    // convert that new vector in hyperspherical coordinates (you can have a look at the hyperspherical_xtophi function in Tools.h)
    // "float rsqr = pow(length(x_rotated),2.0);",
    // "float phi0 = acos(x_rotated.x/sqrt(rsqr));",
    // "rsqr = rsqr - x_rotated.x*x_rotated.x;",
    // "float phi1 = acos(x_rotated.y/sqrt(rsqr));",
    // "rsqr = rsqr - x_rotated.y*x_rotated.y;",
    // "if ( x_rotated.w == 0.0 ) {",
    // "if ( x_rotated.z < 0.0 ) { phi2 = pi; }",
    // "else if ( x_rotated.z == 0.0 ) {",
    // "phi1 = 0.0;",
    // "phi2 = 0.0;",
    // "}",
    // "else { phi2 = 0.0; }",
    // "}",
    // "else {",
    // "phi2 = acos(x_rotated.z/sqrt(rsqr));",
    // "}",

    // BENJY VERSION JUST FOR 3D --- removes annoying flickering from nans i couldnt get rid of
    "float phi0 = acos(x_rotated.x/R);",
    "float phi1;",
    "if ( x_rotated.z > 0. ) { phi1 = atan(x_rotated.y/x_rotated.z); }",
    "else if ( x_rotated.z < 0. ) { phi1 = atan(x_rotated.y/x_rotated.z) + 3.14159265359; }",
    "else { phi1 = 1.57079632679; }",

    // "if ( isnan(phi0) ) { phi0 = acos(sign(x_rotated.x)*x_rotated.x); }", // added by benjy, total guess
    // "if ( isnan(phi1) ) { phi1 = acos(sign(x_rotated.y)*x_rotated.y); }",
    // "if ( x_rotated.w < 0.0 ) { phi2 = 2.0*pi - phi2; }",

    "vColor.r = abs(sin(phi0*3.0));",
    "vColor.g = abs(sin(phi1*2.0));",
    "vColor = vColor * abs(sin(phi0));",
    // "}",
    // "else { vColor.r = 0.0; }",
    "gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );",

    "}",
  ].join("\n"),

  fragmentShader: [
    "uniform float ambient;", // brightness of particle

    "varying vec3 vNormal;",
    "varying vec3 vColor;",

    "void main() {",

    // add directional lighting
    "vec3 light = vec3( 0, 0, -1 );", // bit of trial and error here
    "light = normalize( light );",
    "float directional = max( dot( vNormal, light ), 0.0 );",
    "gl_FragColor = vec4( 0.6*( ambient + directional ) * vColor, opacity );", // colours by vertex colour

    // no directional lighting
    // "const float ambient = 1.0;",
    // "gl_FragColor = vec4( ( ambient ) * vColor, 1.0 );", // colours by vertex colour

    "}",
  ].join("\n"),
});

export { NDDEMShader };
