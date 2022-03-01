import { ShaderMaterial } from "three";

var N = 6;
var uniforms = {
  N: { value: N },
  N_lines: { value: 5.0 },
  //A: { value: new THREE.Matrix4() },
  A: { value: [] }, // Size N*N
  xview: { value: [] }, //Size N-3
  xpart: { value: [] }, //Size N-3
  // x4: { value: 0 },
  // x4p: { value: 0 },
  R: { value: 0.5 },
  ambient: { value: 1.0 },
};

for (var ij = 0; ij < N - 3; ij++) {
  uniforms.xview.value[ij] = 0.0;
  uniforms.xpart.value[ij] = 0.0;
}
// if (N > 3) {
  // uniforms.x4.value = 0.0;
// }
for (var ij = 0; ij < N * N; ij++) {
  if (ij % N == Math.floor(ij / N)) uniforms.A.value[ij] = 1;
  else uniforms.A.value[ij] = 0;
}

var NDDEMShader = new ShaderMaterial({
  uniforms: uniforms,

  vertexShader: [
    "#define N 6",
    "uniform float N_lines;", // number of lines to render across particle
    "uniform float A[N*N];", // orientation matrix for this particle
    "uniform float R;", // particle radius
    "uniform float xview[N-3] ;",
    "uniform float xpart[N-3] ;",
    "varying vec3 vColor;", // colour at vertex (output)
    "varying vec3 vNormal;", // normal at vertex (output)
    "vec3 colors[N-1] ; //{0,1,1},{1,0,1}}",
    //colors[0] = vec3(1.,0.,0.) ;
    "vec3 colorscale = vec3(2.,3.,2.) ;",
    "vec3 tmp ;",
    "float x[N];",
    "float x_rotated[N];",
    "float phi[N-1];",

    "bool isnan(float val)",
    "{",
    "return (val <= 0.0 || 0.0 <= val) ? false : true;",
    "}",

    "void main() {",
    "colors[0]=vec3(1.,0.,0.) ;",
    "colors[1]=vec3(0.,1.,0.) ;",
    "colors[2]=vec3(0.,0.,1.) ;",
    "colors[3]=vec3(1.,1.,0.) ;",
    "colors[4]=vec3(0.,1.,1.) ;",

    "vNormal = normal;", // for directional lighting
    "const float pi = 3.14159265359;",
    "float R_draw;", // radius particle will be drawn at
    "float R_draw_squared = R*R ;",
    "for (int i=0 ; i<N-3 ; i++)",
    "R_draw_squared -= (xview[i] - xpart[i])*(xview[i] - xpart[i]);",
    "if ( R_draw_squared > 0.0 ) {", // only if visible
    "R_draw = sqrt(R_draw_squared);",
    // get 3d locations in x,y,z,w in coord system where center of sphere is at 0,0,0,0
    "x[1] = R_draw*cos((uv.y-0.5)*pi)*cos((uv.x-0.5)*2.0*pi);",
    "x[2] = - R_draw*cos((uv.y-0.5)*pi)*sin((uv.x-0.5)*2.0*pi);",
    "x[0] = - R_draw*sin((uv.y-0.5)*pi);",

    "for (int i=0 ; i<N-3 ; i++)",
    "x[i+3] = xview[i] - xpart[i];",

    // compute the rotated location by doing transpose(A) * x, with A the orientation matrix from the dumps
    "float rsqr = 0. ;",
    "for (int i=0 ; i<N ; i++)",
    "{",
    "x_rotated[i] = 0. ;",
    "for (int j=0 ; j<N ; j++)",
    "x_rotated[i] += A[j*N+i]*x[j] ;",
    "rsqr += x_rotated[i]*x_rotated[i] ;",
    "}",

    // convert that new vector in hyperspherical coordinates (you can have a look at the hyperspherical_xtophi function in Tools.h)
    "tmp[0] = x_rotated[0] ;",
    "tmp[1] = x_rotated[1] ;",
    "tmp[2] = x_rotated[2] ;",
    "for (int i=0 ; i<N-3 ; i++)",
    "{",
    "x_rotated[i] = x_rotated[i+3] ;",
    "}",
    "x_rotated[N-3] = tmp[0];",
    "x_rotated[N-2] = tmp[1];",
    "x_rotated[N-1] = tmp[2];",

    "int lastnonzero = 0 ;",
    "for (int j=N-1 ; j>=0 ; j--)",
    "{",
    "if (abs(x_rotated[j])>1e-6)",
    "break ;",
    "lastnonzero = j ;",
    "}",
    /*for (int j=N-1 ; j>=0 && abs(x_rotated[j])<1e-6 ; j--)
                    {
                        lastnonzero = j ;
                    }*/
    "lastnonzero-- ;",

    "for (int i=0 ; i<N-1 ; i++)",
    "{",
    "if (i>=lastnonzero)",
    "{",
    "if (x_rotated[i]<0.) phi[i] = pi ;",
    "else phi[i] = 0. ;",
    "}",

    "phi[i] = acos(x_rotated[i]/sqrt(rsqr)) ;",
    "if (isnan(phi[i])) {phi[i]=pi ;}",
    "rsqr -= x_rotated[i]*x_rotated[i] ;",
    "}",
    "if (x_rotated[N-1]<0.) phi[N-2] = 2.*pi - phi[N-2] ;",

    // Coloring
    "vColor.r = 0.0;",
    "vColor.g = 0.0;",
    "vColor.b = 0.0;",

    "for (int i=0 ; i<N-2 ; i++)",
    "{",
    "vColor.r += (colors[i][0] * abs(sin(3.*phi[i])))/colorscale[0] ;",
    "vColor.g += (colors[i][1] * abs(sin(3.*phi[i])))/colorscale[1] ;",
    "vColor.b += (colors[i][2] * abs(sin(3.*phi[i])))/colorscale[2] ;",
    "}",
    "vColor.r += (colors[N-2][0] * abs(sin(4.*phi[N-2]/2.)))/colorscale[0] ;",
    "vColor.g += (colors[N-2][1] * abs(sin(4.*phi[N-2]/2.)))/colorscale[1] ;",
    "vColor.b += (colors[N-2][2] * abs(sin(4.*phi[N-2]/2.)))/colorscale[2] ;",
    "for (int i=0 ; i<N-2 ; i++)",
    "{",
    "vColor.r *= abs(sin(phi[i])) ;",
    "vColor.g *= abs(sin(phi[i])) ;",
    "vColor.b *= abs(sin(phi[i])) ;",
    "}",
    "}",
    "else { vColor.r = 0.0; }",
    "gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );",
    "}",
  ].join("\n"),

  fragmentShader: [
    "uniform float ambient;", // brightness of particle

    "varying vec3 vNormal;",
    "varying vec3 vColor;",

    "void main() {",

    // add directional lighting
    // "const float ambient = 1.0;",
    "vec3 light = vec3( 1.0 );",
    "light = normalize( light );",
    "float directional = max( dot( vNormal, light ), 0.0 );",
    "gl_FragColor = vec4( 0.6*( ambient + directional ) * vColor, 1.0 );", // colours by vertex colour

    // no directional lighting
    // const float ambient = 1.0;
    // gl_FragColor = vec4( ( ambient ) * vColor, 1.0 ); // colours by vertex colour

    "}",
  ].join("\n"),
});

export { NDDEMShader };
