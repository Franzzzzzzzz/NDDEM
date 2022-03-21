AFRAME.registerComponent("rotate-d4", {
  schema: {
    gripped: { type: "boolean", default: false },
  },

  init: function () {
    previous_torus_rotation_z = 0;
    previous_torus_rotation_y = 0;
    new_orientation = 0;
    previous_direction = new THREE.Quaternion();
    current_direction = new THREE.Quaternion();
    diff = new THREE.Quaternion();
    diff_angle = new THREE.Euler();

    leftHand = document.querySelector("#leftHand").object3D;
    world = document.querySelector("a-entity[infile]").components.infile.data
      .world;

    var self = this;
    var el = this.el;
    data = this.data;
    el.addEventListener("gripdown", function (evt) {
      document.querySelector(
        "#particles"
      ).components.particles.data.redraw = true;
      leftHand.getWorldQuaternion(previous_direction);
      previous_torus_rotation_z = document.querySelector("#leftTorus").object3D
        .rotation.z;
      previous_torus_rotation_y = world[4].cur;
      //console.log(previous_torus_rotation)
      self.data.gripped = true;
    });
    el.addEventListener("gripup", function (evt) {
      document.querySelector(
        "#particles"
      ).components.particles.data.redraw = false;
      self.data.gripped = false;
    });
  },

  tick: function () {
    if (this.data.gripped) {
      leftHand.getWorldQuaternion(current_direction);
      diff = current_direction.inverse().multiply(previous_direction);
      diff_angle.setFromQuaternion(diff); // + Math.PI;// between 0 and 2 Pi

      // move in D4 by rotations in z
      new_orientation = previous_torus_rotation_z + diff_angle.z;
      if (new_orientation < 0) {
        new_orientation += 2 * Math.PI;
      } else if (new_orientation > 2 * Math.PI) {
        new_orientation -= 2 * Math.PI;
      }
      world[3].cur =
        (new_orientation * (world[3].max - world[3].min)) / Math.PI / 2;
      document.querySelector(
        "#leftTorus"
      ).object3D.rotation.z = new_orientation;

      // move in D4 by rotations in z
      new_orientation =
        previous_torus_rotation_y +
        (2 * diff_angle.y * (world[4].max - world[4].min)) / Math.PI;
      if (new_orientation < world[4].min) {
        new_orientation += world[4].max - world[4].min;
      } else if (new_orientation > world[4].max) {
        new_orientation -= world[4].max - world[4].min;
      }
      world[4].cur = new_orientation;
      //document.querySelector("#leftTorus").object3D.rotation.z = new_orientation;
    }
  },
});
