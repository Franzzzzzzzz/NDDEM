AFRAME.registerComponent("torus", {
  schema: {
    hand: { type: "string", default: "" },
  },

  // init: function() {
  // console.log('Adding torus to ' + this.data.hand + ' hand');
  // },

  init: function () {
    // PLAY GETS RUN AFTER EVERYTHING ELSE INITIALISED
    data = this.data;
    el = this.el;
    console.log("Adding torus to " + data.hand + " hand");
    var R = 0.1;
    var r = R / 2;
    var N = document.querySelector("#infile").components.infile.data.N;
    var quality = 5; //document.querySelector("#particles").components.particles.data.quality;
    var shadows = false; //document.querySelector("#particles").components.particles.data.shadows;
    var geometry = new THREE.TorusBufferGeometry(
      R,
      r,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    var material = new THREE.MeshPhongMaterial({ color: 0xffffff });

    wristband = new THREE.Mesh(geometry, material);
    if (shadows) {
      wristband.castShadow = true;
      wristband.receiveShadow = true;
    }

    var geometry = new THREE.TorusBufferGeometry(
      r + R - r / 6,
      r / 5,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    var material = new THREE.MeshPhongMaterial({ color: 0x000000 });
    wristband_phi = new THREE.Mesh(geometry, material);

    var geometry = new THREE.TorusBufferGeometry(
      r,
      r / 10,
      Math.pow(2, quality) * 2,
      Math.pow(2, quality)
    );
    wristband_theta = new THREE.Mesh(geometry, material);
    wristband_theta.rotation.y = Math.PI / 2;

    wristband.position.set(0, 0, 0);
    wristband_phi.position.set(0, 0, 0);
    wristband_theta.position.set(0, R, 0);
    el.setObject3D("wristband", wristband);
    el.setObject3D("wristband_phi", wristband_phi);
    el.setObject3D("wristband_theta", wristband_theta);
  },
});
