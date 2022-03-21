AFRAME.registerComponent("particles", {
  schema: {
    view_mode: { type: "string", default: "default" },
    cache: { type: "boolean", default: false },
    quality: { type: "number", default: 4 },
    shadows: { type: "boolean", default: false },
    time: { type: "number", default: 0 },
    prev: { type: "number", default: 0 },
    rate: { type: "number", default: 0.1 },
    tmax: { type: "number", default: 50 },
    paused: { type: "number", default: 1 },
    redraw: { type: "boolean", default: false },
  },
  update: function () {
    data = this.data;
    console.log;
    data.fname = document.querySelector(
      "a-entity[infile]"
    ).components.infile.data.fname;
    data.N = document.querySelector(
      "a-entity[infile]"
    ).components.infile.data.N;
    data.world = document.querySelector(
      "a-entity[infile]"
    ).components.infile.data.world;
    el = this.el;
    if (data.view_mode === "rotations") {
      var arr = new Array();
      for (i = 0; i < data.N; i++) {
        if (i < 3) {
          arr.push("NaN");
        } else {
          arr.push(world[i].cur);
        }
      }
      var request = new XMLHttpRequest();
      request.open(
        "POST",
        "http://localhost:8000/make_textures?" +
          "arr=" +
          JSON.stringify(arr) +
          "&N=" +
          N +
          "&t=" +
          "00000" +
          "&quality=" +
          data.quality +
          "&fname=" +
          data.fname,
        true
      );
      request.send(null);
      // request.onreadystatechange = function () {}
    }
    if (this.data.cache) {
      var filename =
        "http://localhost:8000/Samples/" + this.data.fname + "dump-00000.csv";
    } else {
      var filename =
        "http://localhost:8000/Samples/" +
        this.data.fname +
        "dump-00000.csv" +
        "?_=" +
        new Date().getTime();
    }
    Papa.parse(filename, {
      download: true,
      dynamicTyping: true,
      header: true,
      complete: function (results) {
        this.particles = new THREE.Group();
        el.setObject3D("particles", this.particles);
        spheres = results.data;
        var R = 0.1;
        if (data.N > 3) {
          var wristband = document
            .querySelector("#leftTorus")
            .getObject3D("wristband");
        }
        if (data.N > 5) {
          var wristband1 = document
            .querySelector("#rightTorus")
            .getObject3D("wristband");
        }
        if (data.N == 1) {
          var geometry = new THREE.CylinderGeometry(
            1,
            1,
            2,
            Math.pow(2, data.quality),
            Math.pow(2, data.quality)
          );
        } else {
          var geometry = new THREE.SphereGeometry(
            1,
            Math.pow(2, data.quality),
            Math.pow(2, data.quality)
          );
        }
        var pointsGeometry = new THREE.SphereGeometry(
          1,
          Math.max(Math.pow(2, data.quality - 2), 4),
          Math.max(Math.pow(2, data.quality - 2), 4)
        );
        var scale = 20; // size of particles on tori
        for (var i = 0; i < spheres.length; i++) {
          if (data.N === 2) {
            var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
            var material = new THREE.PointsMaterial({
              color: color,
            });
          } else {
            if (data.view_mode === "catch_particle") {
              if (i == pinky) {
                var color = 0xe72564;
              } else {
                var color = 0xaaaaaa;
              }
              var material = new THREE.MeshPhongMaterial({ color: color });
            } else {
              if (data.view_mode === "rotations") {
                var texture = new THREE.TextureLoader().load(
                  "http://localhost:8000/Textures/" +
                    fname +
                    "Texture-00000-0.png"
                );
                var material = new THREE.MeshBasicMaterial({ map: texture });
              } else {
                var color = ((Math.random() + 0.25) / 1.5) * 0xffffff;
                var material = new THREE.MeshPhongMaterial({ color: color });
              }
            }
          }
          var object = new THREE.Mesh(geometry, material);
          object.position.set(spheres[i].x0, spheres[i].x1, spheres[i].x2);
          object.rotation.z = Math.PI / 2;
          if (data.shadows) {
            object.castShadow = true;
            object.receiveShadow = true;
          }
          this.particles.add(object);
          if (data.N > 3 && !data.fname.includes("Spinner")) {
            pointsMaterial = new THREE.PointsMaterial({ color: color });
            object2 = new THREE.Mesh(pointsGeometry, pointsMaterial);
            object2.scale.set(R / scale, R / scale, R / scale);
            object2.position.set(0, 0, 0);
            wristband.add(object2);
            if (data.N > 5) {
              object3 = object2.clone();
              wristband1.add(object3);
            }
          }
        }
        if (data.fname.includes("Submarine")) {
          camera.position.set(
            particles.children[pinky].position.x,
            particles.children[pinky].position.y,
            particles.children[pinky].position.z
          );
          console.log(camera.position);
        }
      },
    });
  },

  tick: function (t, changed_higher_dim_view) {
    data = this.data;
    data.fname = document.querySelector(
      "a-entity[infile]"
    ).components.infile.data.fname;
    data.N = document.querySelector(
      "a-entity[infile]"
    ).components.infile.data.N;
    world = document.querySelector("a-entity[infile]").components.infile.data
      .world;
    el = this.el;
    var R = 0.1;
    var r = R / 2;
    if (data.N > 3) {
      var wristband = document
        .querySelector("#leftTorus")
        .getObject3D("wristband");
    }
    if (data.N > 5) {
      var wristband1 = document
        .querySelector("#rightTorus")
        .getObject3D("wristband");
    }
    //console.log(data.alpha_3);
    data.time += data.rate * data.paused;
    if (Math.floor(data.time) !== data.prev || data.redraw) {
      //data.redraw = false;
      data.prev = Math.floor(data.time);
      if (data.time > data.tmax) {
        data.time -= data.tmax;
      }
      if (data.view_mode === "rotations") {
        if (changed_higher_dim_view) {
          var arr = new Array();
          for (var i = 0; i < N; i++) {
            if (i < 3) {
              arr.push("NaN");
            } else {
              arr.push(world[i].cur);
            }
          }
          var request = new XMLHttpRequest();
          request.open(
            "POST",
            "http://localhost:8000/make_textures?" +
              "arr=" +
              JSON.stringify(arr) +
              "&N=" +
              N +
              "&t=" +
              t +
              "0000" +
              "&quality=" +
              data.quality +
              "&fname=" +
              data.fname,
            true
          );
          request.onload = function () {
            load_textures(t);
          };
          request.send("");
        } else {
          load_textures(t);
        }
      }

      if (data.cache) {
        var filename =
          "http://localhost:8000/Samples/" +
          data.fname +
          "dump-" +
          Math.floor(data.time) +
          "0000.csv";
      } else {
        var filename =
          "http://localhost:8000/Samples/" +
          data.fname +
          "dump-" +
          Math.floor(data.time) +
          "0000.csv" +
          "?_=" +
          new Date().getTime();
      }
      Papa.parse(filename, {
        download: true,
        dynamicTyping: true,
        header: true,
        cache: data.cache,
        complete: function (results) {
          spheres = results.data;
          particles = el.getObject3D("particles");
          for (i = 0; i < spheres.length; i++) {
            var object = particles.children[i];
            if (data.N == 1) {
              spheres[i].x1 = 0;
            }
            if (data.N < 3) {
              spheres[i].x2 = 0;
            }
            object.position.set(spheres[i].x0, spheres[i].x1, spheres[i].x2);
            if (data.N < 4) {
              var R_draw = spheres[i].R;
            } else if (data.N == 4) {
              var R_draw = Math.sqrt(
                Math.pow(spheres[i].R, 2) -
                  Math.pow(world[3].cur - spheres[i].x3, 2)
              );
            } else if (data.N == 5) {
              var R_draw = Math.sqrt(
                Math.pow(spheres[i].R, 2) -
                  Math.pow(world[3].cur - spheres[i].x3, 2) -
                  Math.pow(world[4].cur - spheres[i].x4, 2)
              );
            } else if (data.N == 6) {
              var R_draw = Math.sqrt(
                Math.pow(spheres[i].R, 2) -
                  Math.pow(world[3].cur - spheres[i].x3, 2) -
                  Math.pow(world[4].cur - spheres[i].x4, 2) -
                  Math.pow(world[5].cur - spheres[i].x5, 2)
              );
            } else if (data.N == 7) {
              var R_draw = Math.sqrt(
                Math.pow(spheres[i].R, 2) -
                  Math.pow(world[3].cur - spheres[i].x3, 2) -
                  Math.pow(world[4].cur - spheres[i].x4, 2) -
                  Math.pow(world[5].cur - spheres[i].x5, 2) -
                  Math.pow(world[6].cur - spheres[i].x6, 2)
              );
            }
            if (isNaN(R_draw)) {
              object.visible = false;
            }
            if (data.fname.includes("Submarine") && i == pinky) {
              object.visible = false;
            } else {
              object.visible = true;
              object.scale.set(R_draw, R_draw, R_draw);
              if (data.view_mode === "velocity") {
                lut.setMin(0);
                lut.setMax(velocity.vmax);
                object.material.color = lut.getColor(spheres[i].Vmag);
              } else if (data.view_mode === "rotation_rate") {
                lut.setMin(0);
                lut.setMax(velocity.omegamax);
                object.material.color = lut.getColor(spheres[i].Omegamag);
              }
            }

            if (data.N == 4 && !data.fname.includes("Spinner")) {
              var object2 = wristband.children[i];
              //console.log(spheres[i].x3);
              phi =
                (2 * Math.PI * (world[3].cur - spheres[i].x3)) /
                  (world[3].max - world[3].min) +
                Math.PI / 2;
              x = (R + r) * Math.cos(phi);
              y = (R + r) * Math.sin(phi);
              z = 0;
              object2.position.set(x, y, z);
            }
            // console.log(data.N);
            if (data.N > 4 && !data.fname.includes("Spinner")) {
              var object2 = wristband.children[i];
              phi =
                (2 * Math.PI * (world[3].cur - spheres[i].x3)) /
                  (world[3].max - world[3].min) +
                Math.PI / 2;
              theta =
                (2 * Math.PI * (world[4].cur - spheres[i].x4)) /
                (world[4].max - world[4].min);
              x = (R + r * Math.cos(theta)) * Math.cos(phi);
              y = (R + r * Math.cos(theta)) * Math.sin(phi);
              z = r * Math.sin(theta);
              object2.position.set(x, y, z);
            }

            if (data.N == 6 && !data.fname.includes("Spinner")) {
              var object3 = wristband1.children[i];
              phi =
                (2 * Math.PI * (world[5].cur - spheres[i].x5)) /
                  (world[5].max - world[5].min) +
                Math.PI / 2;
              x = (R + r) * Math.cos(phi);
              y = (R + r) * Math.sin(phi);
              z = 0;
              object3.position.set(x, y, z);
            }

            if (data.N == 7 && !data.fname.includes("Spinner")) {
              var object3 = wristband1.children[i];
              phi =
                (2 * Math.PI * (world[5].cur - spheres[i].x5)) /
                  (world[5].max - world[5].min) +
                Math.PI / 2;
              theta =
                (2 * Math.PI * (world[6].cur - spheres[i].x6)) /
                (world[6].max - world[6].min);
              x = (R + r * Math.cos(theta)) * Math.cos(phi);
              y = (R + r * Math.cos(theta)) * Math.sin(phi);
              z = r * Math.sin(theta);
              object3.position.set(x, y, z);
            }
          }
        },
      });
    }
  },
});
