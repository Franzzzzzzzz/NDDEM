import * as THREE from "three";
import { TextGeometry } from "three/examples/jsm/geometries/TextGeometry.js";
import { FontLoader } from "three/examples/jsm/loaders/FontLoader.js";

var walls;
/**
 * Make the axes, including labels and arrows
 */
function make_axes(scene, params, world) {
  if (typeof axesLabels == "undefined") {
    // if you haven't already made the axes
    if (params.colour_scheme === "inverted") {
      var arrow_colour = 0x333333;
    } else {
      var arrow_colour = 0xdddddd;
    }
    var ref_length = Math.min(
      world[0].max - world[0].min,
      world[1].max - world[1].min,
      world[2].max - world[2].min
    );
    var axeslength = 0.5 * ref_length; // length of axes vectors
    var fontsize = 0.05 * ref_length; // font size
    var thickness = 0.02 * ref_length; // line thickness
    var axesHelper = new THREE.Group();
    var axesLabels = new THREE.Group();
    // axesHelper = new THREE.AxesHelper( axeslength ); // X - red, Y - green, Z - blue
    if (params.display_type === "VR") {
      axesHelper.position.set(0, -params.human_height, 0);
      axesLabels.position.set(0, -params.human_height, 0);
      axesHelper.scale.set(params.vr_scale, params.vr_scale, params.vr_scale);
      axesLabels.scale.set(params.vr_scale, params.vr_scale, params.vr_scale);
      axesLabels.rotation.z = Math.PI / 2;
      axesLabels.rotation.y = Math.PI / 2;
      thickness = 0.01 * ref_length; // line thickness
    }

    scene.add(axesHelper);
    scene.add(axesLabels);

    var arrow_body = new THREE.CylinderGeometry(
      thickness,
      thickness,
      axeslength,
      Math.pow(2, params.quality),
      Math.pow(2, params.quality)
    );
    var arrow_head = new THREE.CylinderGeometry(
      0,
      2 * thickness,
      4 * thickness,
      Math.pow(2, params.quality),
      Math.pow(2, params.quality)
    );
    if (params.N < 3) {
      var arrow_material = new THREE.PointsMaterial({ color: arrow_colour });
    } else {
      var arrow_material = new THREE.MeshPhongMaterial({ color: arrow_colour });
    }
    // var arrow_material_y = new THREE.MeshPhongMaterial( { color: 0x00ff00 } );
    // var arrow_material_z = new THREE.MeshPhongMaterial( { color: 0x0000ff } );
    var arrow_x = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_y = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_z = new THREE.Mesh(arrow_body, arrow_material);
    var arrow_head_x = new THREE.Mesh(arrow_head, arrow_material);
    var arrow_head_y = new THREE.Mesh(arrow_head, arrow_material);
    var arrow_head_z = new THREE.Mesh(arrow_head, arrow_material);

    arrow_x.position.x = axeslength / 2;
    arrow_x.rotation.z = -Math.PI / 2;
    arrow_head_x.position.x = axeslength + thickness;
    arrow_head_x.rotation.z = -Math.PI / 2;

    arrow_y.position.y = axeslength / 2;
    arrow_head_y.position.y = axeslength + thickness;

    arrow_z.position.z = axeslength / 2;
    arrow_z.rotation.x = -Math.PI / 2;
    arrow_head_z.position.z = axeslength + thickness;
    arrow_head_z.rotation.x = Math.PI / 2;

    if (params.N == 1) {
      arrow_x.position.y = -1.5;
      arrow_head_x.position.y = -1.5;
    }

    axesHelper.add(arrow_x);
    axesHelper.add(arrow_head_x);

    if (params.N > 1) {
      axesHelper.add(arrow_y);
      axesHelper.add(arrow_head_y);
    }
    if (params.N > 2) {
      axesHelper.add(arrow_z);
      axesHelper.add(arrow_head_z);
    }
  }

  if (world.ref_dim.c != world.ref_dim.x) {
    world.ref_dim.x = world.ref_dim.c;
    if (params.N > 3) {
      if (world.ref_dim.c < params.N - 1) {
        world.ref_dim.y = world.ref_dim.c + 1;
      } else {
        world.ref_dim.y = world.ref_dim.c + 1 - params.N;
      }
      if (world.ref_dim.c < params.N - 2) {
        world.ref_dim.z = world.ref_dim.c + 2;
      } else {
        world.ref_dim.z = world.ref_dim.c + 2 - params.N;
      }
    } else {
      world.ref_dim.y = world.ref_dim.c + 1;
      world.ref_dim.z = world.ref_dim.c + 2;
    }
    if (axesLabels.children.length > 0) {
      for (var i = axesLabels.children.length - 1; i >= 0; i--) {
        obj = axesLabels.children[i];
        axesLabels.remove(obj);
      }
    }
    // console.log(ref_dim)
    var loader = new FontLoader();
    loader.load(
      params.root_dir +
        "../node_modules/three/examples/fonts/helvetiker_bold.typeface.json",
      function (font) {
        var textGeo_x = new TextGeometry("x" + world.ref_dim.x, {
          font: font,
          size: fontsize,
          height: fontsize / 5,
        });
        var textMaterial_x = new THREE.MeshPhongMaterial({ color: 0xff0000 });
        var mesh_x = new THREE.Mesh(textGeo_x, arrow_material);
        mesh_x.position.x = axeslength; // - 1.5*fontsize;
        mesh_x.position.y = -0.05 * ref_length;
        mesh_x.position.z = fontsize / 4;
        mesh_x.rotation.z = Math.PI;
        mesh_x.rotation.y = Math.PI;
        if (params.N == 1) {
          mesh_x.position.y = -1.8;
        }
        axesLabels.add(mesh_x);

        if (params.N > 1) {
          var textGeo_y = new TextGeometry("x" + world.ref_dim.y, {
            font: font,
            size: fontsize,
            height: fontsize / 5,
          });
          var textMaterial_y = new THREE.MeshPhongMaterial({ color: 0x00ff00 });
          var mesh_y = new THREE.Mesh(textGeo_y, arrow_material);
          mesh_y.position.x = 0.05 * ref_length;
          mesh_y.position.y = axeslength; //-fontsize*1.5;
          mesh_y.position.z = fontsize / 4;
          mesh_y.rotation.z = -Math.PI / 2;
          mesh_y.rotation.x = Math.PI;
          axesLabels.add(mesh_y);
        }
        if (params.N > 2) {
          var textGeo_z = new TextGeometry("x" + world.ref_dim.z, {
            font: font,
            size: fontsize,
            height: fontsize / 5,
          });
          var textMaterial_z = new THREE.MeshPhongMaterial({ color: 0x0000ff });
          var mesh_z = new THREE.Mesh(textGeo_z, arrow_material);
          mesh_z.position.x = 0.05 * ref_length;
          mesh_z.position.y = fontsize / 4;
          mesh_z.position.z = axeslength + 1.5 * fontsize;
          mesh_z.rotation.z = -Math.PI / 2;
          mesh_z.rotation.x = Math.PI / 2;
          axesLabels.add(mesh_z);
        }
      }
    );
  }
  axesHelper.position.set(world[0].min, world[1].min, world[2].min); // move to bottom left hand corner
  axesLabels.position.set(world[0].min, world[1].min, world[2].min); // move to bottom left hand corner
}

/**
 * Make any necessary walls
 */
function make_walls(scene, params, world) {
  walls = new THREE.Group();
  if (params.display_type === "VR") {
    var base_plane_geometry = new THREE.PlaneBufferGeometry(1, 1);
    var base_plane_material = new THREE.MeshStandardMaterial({
      color: 0x000000,
    });
    var base_plane = new THREE.Mesh(base_plane_geometry, base_plane_material);
    base_plane.rotation.x = -Math.PI / 2;
    base_plane.scale.set(10, 10, 10);
    base_plane.position.y = -0.1 - params.human_height;
    scene.add(base_plane);
  }

  var geometry = new THREE.PlaneBufferGeometry(1, 1);
  var material = new THREE.MeshStandardMaterial({
    color: 0xaaaaaa,
    // roughness: 1.0,
    // metalness: 1.0,
  });
  material.transparent = true;
  material.opacity = 0.5;
  // material.side = THREE.DoubleSide;

  // for ( var i=0;i<N;i++ ) {
  // if ( world[i].wall ) {
  // do it nicely
  // }
  // }

  if ( params.data_type === 'mercury-with-vtk-walls' ) {
      // scene.add(world.vtk_walls);
  }

  if (world[0].wall) {
    var floor = new THREE.Mesh(geometry, material);
    if (params.display_type === "VR") {
      floor.scale.set(
        (world[2].max - world[2].min) * params.vr_scale,
        (world[1].max - world[1].min) * params.vr_scale,
        1
      );
      floor.rotation.x = +Math.PI / 2;
      floor.position.set(
        ((world[1].max - world[1].min) / 2) * params.vr_scale,
        world[0].min * params.vr_scale - params.human_height,
        ((world[2].max - world[2].min) * params.vr_scale) / 2
      );
      floor.material.side = THREE.DoubleSide;
    } else {
      floor.scale.set(
        world[2].max - world[2].min,
        world[1].max - world[1].min,
        1
      );
      floor.position.set(
        world[0].min,
        (world[1].max - world[1].min) / 2,
        (world[2].max - world[2].min) / 2
      );
      floor.rotation.y = +Math.PI / 2;
    }
    walls.add(floor);

    var roof = new THREE.Mesh(geometry, material);
    if (params.display_type === "VR") {
      roof.scale.set(
        (world[2].max - world[2].min) * params.vr_scale,
        (world[1].max - world[1].min) * params.vr_scale,
        1
      );
      roof.rotation.x = -Math.PI / 2;
      roof.position.set(
        ((world[1].max - world[1].min) / 2) * params.vr_scale,
        world[0].max * params.vr_scale - params.human_height,
        ((world[2].max - world[2].min) / 2) * params.vr_scale
      );
      roof.material.side = THREE.DoubleSide;
    } else {
      roof.scale.set(
        world[2].max - world[2].min,
        world[1].max - world[1].min,
        1
      );
      roof.rotation.y = -Math.PI / 2;
      roof.position.set(
        world[0].max,
        (world[1].max - world[1].min) / 2,
        (world[2].max - world[2].min) / 2
      );
    }

    if (params.fname.includes("Uniaxial")) {
      roof.material.side = THREE.DoubleSide;
      roof.material.opacity = 0.9;
      floor.material.side = THREE.DoubleSide;
      floor.material.opacity = 0.9;
    }
    walls.add(roof);
  }

  if (world[1].wall) {
    var left_wall = new THREE.Mesh(geometry, material);
    if (params.display_type === "VR") {
      left_wall.scale.set(
        (world[2].max - world[2].min) * params.vr_scale,
        (world[0].max - world[0].min) * params.vr_scale,
        1
      );
      left_wall.rotation.y = -Math.PI / 2;
      left_wall.position.set(
        world[1].min * params.vr_scale,
        ((world[0].max - world[0].min) / 2) * params.vr_scale -
          params.human_height,
        ((world[2].max - world[2].min) / 2) * params.vr_scale
      );
      left_wall.material.side = THREE.DoubleSide;
    } else {
      left_wall.scale.set(
        world[2].max - world[2].min,
        world[0].max - world[0].min,
        1
      );
      left_wall.rotation.x = -Math.PI / 2;
      left_wall.position.set(
        (world[0].max - world[0].min) / 2,
        world[1].min,
        (world[2].max - world[2].min) / 2
      );
    }
    walls.add(left_wall);

    var right_wall = new THREE.Mesh(geometry, material);
    if (params.display_type === "VR") {
      right_wall.scale.set(
        (world[2].max - world[2].min) * params.vr_scale,
        (world[0].max - world[0].min) * params.vr_scale,
        1
      );
      right_wall.rotation.y = Math.PI / 2;
      right_wall.position.set(
        world[1].max * params.vr_scale,
        ((world[0].max - world[0].min) / 2) * params.vr_scale -
          params.human_height,
        ((world[2].max - world[2].min) / 2) * params.vr_scale
      );
      right_wall.material.side = THREE.DoubleSide;
    } else {
      right_wall.scale.set(
        world[2].max - world[2].min,
        world[0].max - world[0].min,
        1
      );
      right_wall.rotation.x = Math.PI / 2;
      right_wall.position.set(
        (world[0].max - world[0].min) / 2,
        world[1].max,
        (world[2].max - world[2].min) / 2
      );
    }
    walls.add(right_wall);
  }

  if (params.N > 2) {
    if (world[2].wall) {
      var front_wall = new THREE.Mesh(geometry, material);
      if (params.display_type === "VR") {
        front_wall.scale.set(
          (world[1].max - world[1].min) * params.vr_scale,
          (world[0].max - world[0].min) * params.vr_scale,
          1
        );
        front_wall.position.set(
          ((world[1].max - world[1].min) / 2) * params.vr_scale,
          ((world[0].max - world[0].min) / 2) * params.vr_scale -
            params.human_height,
          world[2].min * params.vr_scale
        );
        front_wall.material.side = THREE.DoubleSide;
      } else {
        front_wall.scale.set(
          world[0].max - world[0].min,
          world[1].max - world[1].min,
          1
        );
        front_wall.position.set(
          (world[0].max - world[0].min) / 2,
          (world[1].max - world[1].min) / 2,
          world[2].min
        );
      }
      scene.add(front_wall);

      var back_wall = new THREE.Mesh(geometry, material);
      if (params.display_type === "VR") {
        back_wall.scale.set(
          (world[1].max - world[1].min) * params.vr_scale,
          (world[0].max - world[0].min) * params.vr_scale,
          1
        );
        back_wall.rotation.x = Math.PI;
        back_wall.position.set(
          ((world[1].max - world[1].min) / 2) * params.vr_scale,
          ((world[0].max - world[0].min) / 2) * params.vr_scale -
            params.human_height,
          world[2].max * params.vr_scale
        );
        back_wall.material.side = THREE.DoubleSide;
      } else {
        back_wall.scale.set(
          world[0].max - world[0].min,
          world[1].max - world[1].min,
          1
        );
        back_wall.rotation.y = Math.PI;
        back_wall.position.set(
          (world[0].max - world[0].min) / 2,
          (world[1].max - world[1].min) / 2,
          world[2].max
        );
      }
      walls.add(back_wall);
    }
  }
  scene.add(walls);
}

export { make_axes, make_walls };
