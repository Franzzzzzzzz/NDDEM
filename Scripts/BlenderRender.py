import bpy
import bmesh
import numpy as np

bm = bmesh.new()
bmesh.ops.create_uvsphere(bm, u_segments=32, v_segments=16, diameter=1)



def render(output_dir, output_file_format):
  bpy.context.scene.render.filepath = output_dir + output_file_format
  bpy.ops.render.render(write_still = True)

def create_sphere (x,y,z,r,t):
  bpyscene = bpy.context.scene
  mesh = bpy.data.meshes.new('S'+str(n))
  basic_sphere = bpy.data.objects.new("S"+str(n), mesh)
  bpyscene.objects.link(basic_sphere)
  bpyscene.objects.active = basic_sphere
  bm.to_mesh(mesh) ;
  bpyscene.objects.active.location = (x,y,z) ;
  bpyscene.objects.active.scale = (r,r,r) ;
  bpy.ops.object.modifier_add(type='SUBSURF')
  bpy.ops.object.shade_smooth()


  mat=bpy.data.materials.new('M'+str(n))
  tex=bpy.data.textures.new('CT', type = 'IMAGE')
  img=bpy.data.images.load("/Users/FGuillard/Test/Textures/Texture-"+str(n)+"-"+str(t)+"000-1.7.png")
  tex.image=img ;
  mtex = mat.texture_slots.add()
  mtex.texture_coords = "OBJECT"
  mtex.texture = tex
  mtex.mapping = "SPHERE"
  mtex.object=bpy.data.objects["S"+str(n)]
  bpy.context.scene.objects.active.data.materials.append(mat)

def clean() :
    n=0 ;
    for i in bpy.data.meshes:
        bpy.data.meshes.remove(i) ;
    for i in bpy.data.materials:
        bpy.data.materials.remove(i) ;
    for i in bpy.data.textures:
        bpy.data.textures.remove(i) ;
    for i in bpy.data.images:
        bpy.data.images.remove(i) ;

n=0 ;
bpy.data.meshes.remove(bpy.data.meshes["Cube"])
bpy.data.lamps.remove(bpy.data.lamps["Lamp"])

scene = bpy.data.scenes["Scene"]
scene.camera.rotation_mode = 'XYZ'
scene.camera.rotation_euler[0] = 0*(np.pi/180.0)
scene.camera.rotation_euler[1] = 0*(np.pi/180.0)
scene.camera.rotation_euler[2] = 0*(np.pi/180.0)

# Set camera translation
scene.camera.location.x = 5
scene.camera.location.y = 2.5
scene.camera.location.z = 15

bpy.context.scene.world.light_settings.use_environment_light = True

D=4 ;
path="/Users/FGuillard/Dropbox/DEM_ND/Samples/D4/"

for t in range(10, 1000):
    X=np.loadtxt(path+'dump-'+str(t)+'000.csv', skiprows=1, delimiter=',')
    n=0
    for i in X:
        r = i[D]*i[D]- (i[3]-1.7)**2 ;
        if (r>0) :
            create_sphere(i[0],i[1],i[2],np.sqrt(r), t) ;
        n=n+1
    name='render'+str(t)+'.png'
    render('/Users/FGuillard/Test/', name);
    clean()
