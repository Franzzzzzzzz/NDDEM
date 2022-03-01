import { ShaderMaterial } from "three";

var uniforms = {
  ambient: { value: 1.0 },
};

var PoolTableShader = new ShaderMaterial({
  uniforms: uniforms,

  vertexShader: [

    "varying vec3 vColor;", // colour at vertex (output)
    "varying vec3 vNormal;", // normal at vertex (output)

    // "float rand(float n){return fract(sin(n) * 43758.5453123);}",

    "void main() {",
        "vNormal = normal;", // for directional lighting

        "vColor.r = 0.0;",
        "vColor.g = 0.9;",
    // "}",

        "gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );",

    "}",
  ].join("\n"),

  fragmentShader: [
    "uniform float ambient;", // brightness of particle

    "varying vec3 vNormal;",
    "varying vec3 vColor;",

    "float rand(float co) { return fract(sin(co*(91.3458)) * 47453.5453); }",
    "float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }",
    "float rand(vec3 co){ return rand(co.xy+rand(co.z)); }",

    "void main() {",

    // add directional lighting
    // "const float ambient = 1.0;",
    "vec3 light = vec3( ambient*rand(gl_FragCoord.xy) );",
    "light = normalize( light );",
    "float directional = max( dot( vNormal, light ), 0.0 );",
    "gl_FragColor = vec4( 0.6*( ambient + directional ) * vColor, 1.0 );", // colours by vertex colour

    "}",
  ].join("\n"),
});

export { PoolTableShader };
