function onSelectStart(event) {
  var controller = event.target;
  var intersections = getIntersections(controller);
  if (intersections.length > 0) {
    var intersection = intersections[0];
    //tempMatrix.getInverse( controller.matrixWorld );
    var object = intersection.object;
    //object.matrix.premultiply( tempMatrix );
    //object.matrix.decompose( object.position, object.quaternion, object.scale );
    //object.material.emissive.b = 1;
    //controller.add( object );
    //controller.userData.selected = object;
    console.log(object.name);
    if (object.name === "hitscanVolume") {
      // you got the greyed out area!
      console.log(object);
      object.parent.parent.updateControl(window.input1);
    } else if (object.name === "filledVolume") {
      // you got the blue area!
      console.log(object);
    }
  }
}

function onSelectEnd(event) {
  var controller = event.target;
  if (controller.userData.selected !== undefined) {
    var object = controller.userData.selected;
    //object.matrix.premultiply( controller.matrixWorld );
    //object.matrix.decompose( object.position, object.quaternion, object.scale );
    //object.material.emissive.b = 0;
    //particles.add( object );
    controller.userData.selected = undefined;
  }
}

function getIntersections(controller) {
  tempMatrix.identity().extractRotation(controller.matrixWorld);

  raycaster.ray.origin.setFromMatrixPosition(controller.matrixWorld);
  raycaster.ray.direction.set(0, 0, -1).applyMatrix4(tempMatrix);

  if (view_mode === "catch_particle") {
    return raycaster.intersectObjects(particles.children);
  } else {
    return raycaster.intersectObjects(window.gui.children, true);
  }
}

function intersectObjects(controller) {
  // Do not highlight when already selected

  if (controller.userData.selected !== undefined) return;

  var line = controller.getObjectByName("line");
  var intersections = getIntersections(controller);

  if (intersections.length > 0) {
    var intersection = intersections[0];

    var object = intersection.object;
    object.material.emissive.r = 1;
    intersected.push(object);

    line.scale.z = intersection.distance;
  } else {
    line.scale.z = 5;
  }
}

function cleanIntersected() {
  while (intersected.length) {
    var object = intersected.pop();
    object.material.emissive.r = 0;
  }
}
