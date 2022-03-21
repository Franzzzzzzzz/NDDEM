import * as THREE from "three";

var particles; // groups of objects

/**
 * Make the initial texturing if showing rotations
 */
function make_initial_sphere_texturing() {
  var commandstring = "";
  for (i = 3; i < params.N; i++) {
    commandstring =
      commandstring + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
    if (i < params.N - 1) commandstring += "&";
  }
  request = new XMLHttpRequest();
  /*request.open('POST', params.root_dir + "make_textures?" +
                 "arr=" + JSON.stringify(arr) +
                 "&N=" + params.N +
                 "&t=" + "00000" +
                 "&quality=" + params.quality +
                 "&fname=" + params.fname,
                 true);*/
  request.open(
    "GET",
    params.root_dir +
      "load?ND=" +
      params.N +
      "&path=" +
      params.fname +
      "&texturepath=../../" +
      params.texture_dir +
      "&resolution=" +
      params.quality,
    true
  );
  request.send(null);

  request.onload = function () {
    request.open(
      "GET",
      params.root_dir + "render?ts=00000&" + commandstring,
      true
    );
    request.send(null);
    request.onload = function () {
      LOADER.load_initial_spheres(params, time).then(() => {
        make_initial_spheres(LOADER.spheres).then(() => {
          update_spheres(LOADER.spheres);
        });
      });
    };
  };
  // Let's do the first rendering as well
  //request.open('GET', params.root_dir + 'render?ts=00000&' + commandstring, true) ;
  //request.send(null);

  // request.onreadystatechange = function () {}
}

/**
 * Load particles from MercuryDPM file format - NOTE: THIS IS NOT WORKING YET
 */

function make_initial_spheres(spheres, params, world, scene, TORUS) {
  particles = new THREE.Group();
  scene.add(particles);
  if (params.N == 1) {
    var geometry = new THREE.CylinderGeometry(
      1,
      1,
      2,
      Math.pow(2, params.quality),
      Math.pow(2, params.quality)
    );
  } else {
    // var geometry = new THREE.SphereGeometry( 1, Math.pow(2,params.quality), Math.pow(2,params.quality) );
    var geometry = new THREE.BufferGeometry().fromGeometry(
      new THREE.SphereGeometry(
        1,
        Math.pow(2, params.quality),
        Math.pow(2, params.quality)
      )
    );
  }
  var pointsGeometry = new THREE.SphereGeometry(
    1,
    Math.max(Math.pow(2, params.quality - 2), 4),
    Math.max(Math.pow(2, params.quality - 2), 4)
  );
  var scale = 20; // size of particles on tori
  if (params.view_mode === "rotations2") {
    var uniforms = {
      N: { value: params.N },
      N_lines: { value: 5.0 },
      //A: { value: new THREE.Matrix4() },
      A: { value: [] }, // Size N*N
      xview: { value: [] }, //Size N-3
      xpart: { value: [] }, //Size N-3
      x4: { value: 0 },
      x4p: { value: 0 },
      R: { value: 1 },
    };
    for (var ij = 0; ij < params.N - 3; ij++) {
      uniforms.xview.value[ij] = world[ij].cur;
      uniforms.xpart.value[ij] = 0;
    }
    if (params.N > 3) {
      uniforms.x4.value = world[3].cur;
    }
    for (var ij = 0; ij < params.N * params.N; ij++) {
      if (ij % params.N == Math.floor(ij / params.N)) uniforms.A.value[ij] = 1;
      else uniforms.A.value[ij] = 0;
    }
    var shaderMaterial = new THREE.ShaderMaterial({
      uniforms: uniforms,
      vertexShader: document.getElementById(
        "vertexshader-" + String(uniforms.N.value) + "D"
      ).textContent,
      fragmentShader: document.getElementById("fragmentshader").textContent,
    });
  }
  for (var i = 0; i < spheres.length; i++) {
    if (params.N < 3) {
      var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
      var material = new THREE.PointsMaterial({
        color: color,
      });
    } else {
      if (
        params.view_mode === "catch_particle" ||
        params.fname.includes("Lonely")
      ) {
        if (i == params.pinky) {
          var color = 0xe72564;
        } else {
          var color = 0xaaaaaa;
        }
        var material = new THREE.MeshPhongMaterial({ color: color });
      } else if (params.view_mode === "rotations2") {
        var material = shaderMaterial.clone();
      } else {
        if (params.view_mode === "rotations") {
          texture_path =
            params.data_dir + params.texture_dir + "/Texture-" + i + "-00000";
          for (var iiii = 3; iiii < params.N; iiii++) {
            texture_path += "-0.0";
          }
          var texture = new THREE.TextureLoader().load(texture_path + ".png"); //TODO
          var material = new THREE.MeshBasicMaterial({ map: texture });
        } else {
          var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
          var material = new THREE.MeshPhongMaterial({ color: color });
        }
      }
    }
    var object = new THREE.Mesh(geometry, material);
    object.position.set(spheres[i][0], spheres[i][1], spheres[i][2]);
    object.rotation.z = Math.PI / 2;
    // if ( params.fname.includes('Coll') || params.fname.includes('Roll') ) { object.rotation.x = Math.PI/2.; }
    if (params.shadows) {
      object.castShadow = true;
      object.receiveShadow = true;
    }
    particles.add(object);
    if (params.N > 3 && !params.fname.includes("Spinner") && !params.no_tori) {
      var pointsMaterial = new THREE.PointsMaterial({ color: color });
      var object2 = new THREE.Mesh(pointsGeometry, pointsMaterial);
      if (params.fname.includes("Lonely")) {
        object2.scale.set(
          (2 * TORUS.R) / scale,
          (2 * TORUS.R) / scale,
          (2 * TORUS.R) / scale
        );
      } else {
        object2.scale.set(TORUS.R / scale, TORUS.R / scale, TORUS.R / scale);
      }
      object2.position.set(0, 0, 0);
      TORUS.wristband1.add(object2);
      if (params.N > 5) {
        var object3 = object2.clone();
        TORUS.wristband2.add(object3);
      }
    }
  }
  if (params.fname.includes("Submarine")) {
    CAMERA.camera.position.set(
      particles.children[params.pinky].position.x,
      particles.children[params.pinky].position.y,
      particles.children[params.pinky].position.z
    );
    console.log(CAMERA.camera.position);
  }
}

function update_orientation(spheres) {
  for (var i = 0; i < spheres.length; i++) {
    // skip header
    var object = particles.children[i];
    object.material.uniforms.A.value = spheres[i];
  }
}

/**
 * Load textures from TexturingServer
 * @param {number} t timestep
 * @param {number} Viewpoint where we are in D>3 space
 */
function load_textures(t, Viewpoint) {
  if (particles !== undefined) {
    var loader = new THREE.TextureLoader();
    for (ii = 0; ii < particles.children.length - 1; ii++) {
      if (params.cache) {
        var filename =
          params.data_dir +
          params.texture_dir +
          "/Texture-" +
          ii +
          "-" +
          Viewpoint +
          ".png";
      } else {
        var filename =
          params.data_dir +
          params.texture_dir +
          "/Texture-" +
          ii +
          "-" +
          Viewpoint +
          ".png" +
          "?_=" +
          new Date().getTime();
      }
      loader.load(filename, function (texture) {
        //TODO not sure why not working ... ...
        //var myRe = /-[0-9]+.png/g
        //var res=myRe.exec(texture.image.currentSrc)
        //var myRe2 = /[0-9]+/
        //var iii = myRe2.exec(texture.image.currentSrc)[0]
        var iii = texture.image.currentSrc.split("-")[1];
        console.log(texture.image.currentSrc);
        console.log(iii);
        var o = particles.children[iii];
        o.material.map = texture;
        o.material.map.needsUpdate = true;
      });
    }
  }
}

/**
 * Update textures from TexturingServer
 * @param {number} t timestep
 */
function update_spheres_texturing(t) {
  if (true) {
    //TODO Do something better ...
    var commandstring = "";
    var Viewpoint = String(t * time.save_rate).padStart(5, "0");

    for (i = 3; i < params.N; i++) {
      commandstring =
        commandstring + "&" + ("x" + (i + 1) + "=" + world[i].cur.toFixed(1));
      Viewpoint = Viewpoint + "-" + world[i].cur.toFixed(1);
    }

    var request = new XMLHttpRequest();
    /*request.open('POST',
                       params.root_dir + "make_textures?" +
                       "arr=" + JSON.stringify(arr) +
                       "&N=" + params.N +
                       "&t=" + t + "0000" +
                       "&quality=" + params.quality +
                       "&fname=" + params.fname,
                       true);*/
    var runvalue = 0;
    if (time.play) runvalue = 1;
    request.open(
      "GET",
      params.data_dir +
        "render?ts=" +
        String(t * time.save_rate).padStart(5, "0") +
        commandstring +
        "&running=" +
        runvalue,
      true
    );

    request.onload = function () {
      load_textures(t, Viewpoint);
    };
    request.send("");
  } else {
    load_textures(t, Viewpoint);
  }
}

function update_spheres(spheres) {
  for (var i = 0; i < spheres.length; i++) {
    var object = particles.children[i];

    if (params.N > 3) {
      var x3_unrotated = spheres[i][3];

      var x0_temp =
        spheres[i][0] * Math.cos(euler.theta_1) -
        spheres[i][3] * Math.sin(euler.theta_1);
      var x3_temp =
        spheres[i][0] * Math.sin(euler.theta_1) +
        spheres[i][3] * Math.cos(euler.theta_1);

      var x1_temp =
        spheres[i][1] * Math.cos(euler.theta_2) -
        x3_temp * Math.sin(euler.theta_2);
      var x3_temp =
        spheres[i][1] * Math.sin(euler.theta_2) +
        x3_temp * Math.cos(euler.theta_2);

      var x2_temp =
        spheres[i][2] * Math.cos(euler.theta_3) -
        x3_temp * Math.sin(euler.theta_3);
      var x3_temp =
        spheres[i][2] * Math.sin(euler.theta_3) +
        x3_temp * Math.cos(euler.theta_3);

      spheres[i][0] = x0_temp;
      spheres[i][1] = x1_temp;
      spheres[i][2] = x2_temp;
      spheres[i][3] = x3_temp;
    }
    if (params.N == 1) {
      spheres[i][1] = 0;
    }
    if (params.N < 3) {
      spheres[i][2] = 0;
    }
    if (params.N < 4) {
      var R_draw = spheres[i][params.N];
    } else if (params.N == 4) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2)
      );

      //if ( (world[3].cur >  world[3].max-spheres[i][params.N] ) // NOTE: IMPLEMENT THIS!!
    } else if (params.N == 5) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2)
      );
    } else if (params.N == 6) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2)
      );
    } else if (params.N == 7) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2)
      );
    } else if (params.N == 8) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2)
      );
    } else if (params.N == 10) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2) -
          Math.pow(world[8].cur - spheres[i][8], 2) -
          Math.pow(world[9].cur - spheres[i][9], 2)
      );
    } else if (params.N == 30) {
      var R_draw = Math.sqrt(
        Math.pow(spheres[i][params.N], 2) -
          Math.pow(world[3].cur - spheres[i][3], 2) -
          Math.pow(world[4].cur - spheres[i][4], 2) -
          Math.pow(world[5].cur - spheres[i][5], 2) -
          Math.pow(world[6].cur - spheres[i][6], 2) -
          Math.pow(world[7].cur - spheres[i][7], 2) -
          Math.pow(world[8].cur - spheres[i][8], 2) -
          Math.pow(world[9].cur - spheres[i][9], 2) -
          Math.pow(world[10].cur - spheres[i][10], 2) -
          Math.pow(world[11].cur - spheres[i][11], 2) -
          Math.pow(world[12].cur - spheres[i][12], 2) -
          Math.pow(world[13].cur - spheres[i][13], 2) -
          Math.pow(world[14].cur - spheres[i][14], 2) -
          Math.pow(world[15].cur - spheres[i][15], 2) -
          Math.pow(world[16].cur - spheres[i][16], 2) -
          Math.pow(world[17].cur - spheres[i][17], 2) -
          Math.pow(world[18].cur - spheres[i][18], 2) -
          Math.pow(world[19].cur - spheres[i][19], 2) -
          Math.pow(world[20].cur - spheres[i][20], 2) -
          Math.pow(world[21].cur - spheres[i][21], 2) -
          Math.pow(world[22].cur - spheres[i][22], 2) -
          Math.pow(world[23].cur - spheres[i][23], 2) -
          Math.pow(world[24].cur - spheres[i][24], 2) -
          Math.pow(world[25].cur - spheres[i][25], 2) -
          Math.pow(world[26].cur - spheres[i][26], 2) -
          Math.pow(world[27].cur - spheres[i][27], 2) -
          Math.pow(world[28].cur - spheres[i][28], 2) -
          Math.pow(world[29].cur - spheres[i][29], 2)
      );
    }
    if (isNaN(R_draw)) {
      object.visible = false;
      if (params.view_mode === "D4" || params.view_mode === "D5") {
        // if ( params.colour_scheme === 'inverted' ) {
        TORUS.wristband1.children[i].material.color = new THREE.Color(0x777777);
        // }
        // else {
        // TORUS.wristband1.children[i].material.color = new THREE.Color( 0x111111 );
        // }
      }
    } else {
      if (params.fname.includes("Submarine") && i == params.pinky) {
        object.visible = false;
      } else {
        if (params.display_type === "VR") {
          R_draw = R_draw * vr_scale;
          object.position.set(
            spheres[i][1] * vr_scale,
            spheres[i][0] * vr_scale - human_height,
            spheres[i][2] * vr_scale
          );
        } else {
          object.position.set(spheres[i][0], spheres[i][1], spheres[i][2]);
        }
        if (params.quasicrystal) {
          scale = 5;
          object.scale.set(
            spheres[i][params.N] / scale,
            spheres[i][params.N] / scale,
            spheres[i][params.N] / scale
          );
        } else {
          object.scale.set(R_draw, R_draw, R_draw);
        }
        object.visible = true;
        if (params.view_mode === "velocity") {
          lut.setMin(0);
          lut.setMax(velocity.vmax);
          object.material.color = lut.getColor(spheres[i].Vmag);
        } else if (params.view_mode === "rotation_rate") {
          lut.setMin(0);
          lut.setMax(velocity.omegamax);
          object.material.color = lut.getColor(spheres[i].Omegamag);
        } else if (params.view_mode === "rotations2") {
          for (var j = 0; j < params.N - 3; j++) {
            particles.children[i].material.uniforms.xview.value[j] =
              world[j + 3].cur;
            particles.children[i].material.uniforms.xpart.value[j] =
              spheres[i][j + 3];
          }
          if (params.N > 3) {
            object.material.uniforms.x4p.value = spheres[i][3];
            object.material.uniforms.x4.value = world[3].cur;
          } else {
            object.material.uniforms.x4p.value = 0.0;
          }
          //if (object.material.uniforms.xpart.value[0] != object.material.uniforms.x4p.value)
          //{console.log(object.material.uniforms.xpart.value[0]) ; console.log(object.material.uniforms.x4p.value) ; }
          // object.material.uniforms.xp.value = new THREE.Vector4(spheres[i][1],spheres[i][2],spheres[i][3],spheres[i][4])
          // object.material.uniforms.R.value = R_draw;
        } else if (params.view_mode === "D4") {
          //lut.setMin(world[3].min);
          //lut.setMax(world[3].max);
          lut.setMin(world[3].cur - 2 * r);
          lut.setMax(world[3].cur + 2 * r);
          object.material.color = lut.getColor(x3_unrotated);
          TORUS.wristband1.children[i].material.color = lut.getColor(
            x3_unrotated
          );
        } else if (params.view_mode === "D5") {
          //lut.setMin(world[4].min);
          //lut.setMax(world[4].max);
          lut.setMin(world[4].cur - 2 * r);
          lut.setMax(world[4].cur + 2 * r);
          object.material.color = lut.getColor(spheres[i][4]);
          TORUS.wristband1.children[i].material.color = lut.getColor(
            spheres[i][4]
          );
        }
      }
    }
    if (!params.no_tori) {
      if (params.N == 4 && !params.fname.includes("Spinner")) {
        var object2 = TORUS.wristband1.children[i];
        var phi =
          (2 * Math.PI * (world[3].cur - spheres[i][3])) /
            (world[3].max - world[3].min) -
          Math.PI / 2;
        var x = (TORUS.R + TORUS.r) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r) * Math.sin(phi);
        var z = 0;
        object2.position.set(x, y, z);
      }

      if (
        params.N > 4 &&
        !params.fname.includes("Spinner") &&
        !params.no_tori
      ) {
        var object2 = TORUS.wristband1.children[i];
        var phi =
          (2 * Math.PI * (world[3].cur - spheres[i][3])) /
            (world[3].max - world[3].min) -
          Math.PI / 2;
        var theta =
          (2 * Math.PI * (world[4].cur - spheres[i][4])) /
          (world[4].max - world[4].min);
        var x = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.sin(phi);
        var z = TORUS.r * Math.sin(theta);
        object2.position.set(x, y, z);
      }

      if (params.N == 6 && !params.fname.includes("Spinner")) {
        var object3 = TORUS.wristband2.children[i];
        var phi =
          (2 * Math.PI * (world[5].cur - spheres[i][5])) /
            (world[5].max - world[5].min) -
          Math.PI / 2;
        var x = (TORUS.R + TORUS.r) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r) * Math.sin(phi);
        var z = 0;
        object3.position.set(x, y, z);
      }

      if (params.N >= 7 && !params.fname.includes("Spinner")) {
        var object3 = TORUS.wristband2.children[i];
        var phi =
          (2 * Math.PI * (world[5].cur - spheres[i][5])) /
            (world[5].max - world[5].min) -
          Math.PI / 2;
        var theta =
          (2 * Math.PI * (world[6].cur - spheres[i][6])) /
          (world[6].max - world[6].min);
        var x = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.cos(phi);
        var y = (TORUS.R + TORUS.r * Math.cos(theta)) * Math.sin(phi);
        var z = TORUS.r * Math.sin(theta);
        object3.position.set(x, y, z);
      }
    }
  }
}

export {
  make_initial_spheres,
  update_spheres,
  update_orientation,
  make_initial_sphere_texturing,
  update_spheres_texturing,
  load_textures,
};
