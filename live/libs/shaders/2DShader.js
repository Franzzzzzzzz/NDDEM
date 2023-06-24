import { ShaderMaterial } from "three";

var N = 2;
var uniforms = {
  N: { value: N },
  A: { value: [] }, // Size N*N
  R: { value: 0.5 },
  ambient: { value: 1.0 },
  opacity: { value: 1.0 },
};

for (var ij = 0; ij < N * N; ij++) {
  if (ij % N == Math.floor(ij / N)) uniforms.A.value[ij] = 1;
  else uniforms.A.value[ij] = 0;
}

var NDDEMShader = new ShaderMaterial({
  uniforms: uniforms,
  // lights: true,

  vertexShader: [
    "uniform int N;", // number of dimensions in simulation
    "uniform float A[2*2];", // orientation matrix for this particle
    "uniform float R;", // particle radius

    "varying vec3 vColor;", // colour at vertex (output)
    "varying vec3 vNormal;", // normal at vertex (output)

    // "bool isnan( float val ) { return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true; }",

    "void main() {",
    "vNormal = normal;", // for directional lighting
    "const float pi = 3.14159265359;",
    
    "vec2 x;",
    "vec2 x_rotated;",
    // "float phi2;",
    // get 2d locations in x,y in coord system where center of sphere is at 0,0
    // "x.x = - R*sin((uv.x - 0.5)*pi);",//*cos((uv.y - 0.5)*pi);",
    // "x.y = - R*sin((uv.y - 0.5)*pi);",
    "x.x = -2.0*R*(uv.x - 0.5);",
    "x.y = -2.0*R*(uv.y - 0.5);",
    

    // compute the rotated location by doing transpose(A) * x, with A the orientation matrix from the dumps
    "x_rotated.x = A[0]*x.x + A[2]*x.y;",
    "x_rotated.y = A[1]*x.x + A[3]*x.y;",

    // "float phi0 = atan(x_rotated.y/x_rotated.x);",

    "vColor.r = pow(x_rotated.x/R,2.0);",
    "vColor.b = pow(x_rotated.y/R,2.0);",
    // "vColor.r = phi0;",
    // "vColor.r = abs(sin(phi0*4.0));",
    // "vColor.b = abs(cos(phi0*4.0));",
    // "vColor.g = vColor * abs(sin(phi0));",
    // "vColor = vColor * abs(sin(phi0));",

    "gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );",

    "}",
  ].join("\n"),

  fragmentShader: [
    "uniform float ambient;", // brightness of particle
    "uniform float opacity;",
    // "varying vec3 vNormal;",
    "varying vec3 vColor;",

    "void main() {",

    // add directional lighting
    // "vec3 light = vec3( 0, 0, -1 );", // bit of trial and error here
    // "light = normalize( light );",
    // "float directional = max( dot( vNormal, light ), 0.0 );",
    // "gl_FragColor = vec4( 0.6*( ambient + directional ) * vColor, opacity );", // colours by vertex colour

    // no directional lighting
    // "const float ambient = 1.0;",
    "gl_FragColor = vec4( ( ambient ) * vColor, opacity );", // colours by vertex colour

    "}",
  ].join("\n"),
});

export { NDDEMShader };
