import bpy
import bmesh
import numpy as np

TextureFolder = '/home/franz/Desktop/PostDoc_Local/NDDEM/Textures/' ;
#TextureFolder = '/Users/FGuillard/Simulations/MD/DEM_ND/Textures/'
D=5 ;
#path="/Users/FGuillard/Dropbox/DEM_ND/Samples/SingleTest/"
path = "/home/franz/Dropbox/DEM_ND/Samples/D5/" ;
frames="/home/franz/Test/" ;
tdump=1000 ; 
#frames="/Users/FGuillard/Test/"

def render(output_dir, output_file_format):
  bpy.context.scene.render.filepath = output_dir + output_file_format
  bpy.ops.render.render(write_still = True)

def create_sphere (x,y,z,diam,t):
  bpyscene = bpy.context.scene
  mesh = bpy.data.meshes.new('S'+str(n))
  basic_sphere = bpy.data.objects.new("S"+str(n), mesh)
  bpyscene.objects.link(basic_sphere)
  bpyscene.objects.active = basic_sphere
  bm.to_mesh(mesh) ;
  bpyscene.objects.active.location = (x,y,z) ;
  bpyscene.objects.active.scale = (diam,diam,diam) ;
  bpy.ops.object.modifier_add(type='SUBSURF')
  bpy.ops.object.shade_smooth()


  mat=bpy.data.materials.new('M'+str(n))
  tex=bpy.data.textures.new('CT', type = 'IMAGE')
  if (D==3):
      img=bpy.data.images.load(TextureFolder+"Texture-"+str(n)+"-"+'{:05d}'.format(t*tdump)+".png")
  elif (D==4):
      img=bpy.data.images.load(TextureFolder+"Texture-"+str(n)+"-"+'{:05d}'.format(t*tdump)+"-1.7.png")
  elif (D==5):
      try:
        img=bpy.data.images.load(TextureFolder+"Texture-"+str(n)+"-"+'{:05d}'.format(t*tdump)+"-1.7-1.7.png")
      except:
        pass ; 
      #img=bpy.data.images.load("/Users/FGuillard/Test/TestTexture.png")
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


bm = bmesh.new()
bmesh.ops.create_uvsphere(bm, u_segments=32, v_segments=16, diameter=1)

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
bpy.context.scene.world.light_settings.environment_energy = 2.

for t in range(380, 600):
    X=np.loadtxt(path+'dump-'+'{:05d}'.format(t*tdump)+'.csv', skiprows=1, delimiter=',')
    n=0
    for i in np.atleast_2d(X):
        if (D==3):
            r=i[D] ;
        elif (D==4):
            r = np.sqrt(i[D]*i[D]- (i[3]-1.7)**2) ;
        elif (D==5):
            r = np.sqrt(i[D]*i[D]- (i[3]-1.7)**2 -(i[4]-1.7)**2) ;
            
        if (r>0) :
            create_sphere(i[0],i[1],i[2],r, t) ;
        n=n+1
    name='renderD'+str(D)+'-'+str(t)+'.png'
    render(frames, name);
    clean()
