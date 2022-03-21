Below is the default output of the visualisation software for a 3D simulation of spheres flowing down an inclined plane. You can manually move through time, or enable auto playing in the top right. The speed it is playing can be controlled with the "Rate" parameter.

<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/index.html?fname=D3"></iframe>

### Getting around in ND

But what about dimensions higher than 3? For this, we need some way to project particles from higher dimensions down to 3D. In this example, we will project a 3D sphere down to a 2D circle by slicing it. Move the pink plane on the left around with the slider and see the resulting size of the circle on the right. The circle is largest when the sphere is sliced through its centre.

<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/examples/slice.html"></iframe>

Now, lets try that same thing, but we are going to slice a 5D hypersphere with a 3D volume. For this, there are two coordinates for where we are slicing, which you can control with the sliders. The torus on the right hand side represents where we are in these two dimensions. Notice how as you move around, the small pink ball moves. This small pink ball represents the location of the hypersphere relative to these two coordinates. The sphere on the left is again largest when we slice through its centre &mdash; when the ball is at the centre of the black cross

<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/examples/torus_explainer.html"></iframe>

Let's now go and look at inclined plane flow in 5D

<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/index.html?fname=D5&rotate_torus=-150"></iframe>

### Particle rotation

To be able to see a particle rotate, we need to attach a texture to it. Here is how we visualise the earth rotating.
<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/examples/rotating_earth.html"></iframe>

In 3D, there are three directions that an object can rotate in.
<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/examples/multiple_rotating_earths.html"></iframe>

<!-- <span style="color:red">In higher dimensions, there are more!... Information needed. See paper. Maybe just include the full movie we are making for the supplementary information?</span> -->

As we can't render the textures needed to do this without a local installation, here is a video explaining how rotations work

<video style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" controls><source src="https://www.benjymarks.com/nddem/videos/hypersphere-rotations.mp4"></video>


### Trying out VR (this will look blank if you don't have a VR headset attached)
<iframe style="width:90%;height:500px;display:block;margin-left:auto;margin-right:auto;" src="../../visualise/examples/vr-menu.html"></iframe>
