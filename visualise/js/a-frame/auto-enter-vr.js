AFRAME.registerComponent("auto-enter-vr", {
  init: function () {
    this.el.sceneEl.enterVR();
  },
});
