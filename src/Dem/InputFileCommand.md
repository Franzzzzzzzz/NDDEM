# List of input file commands for NDDEM
### Description
Input files are in plain text.
In the following: 

- double quotes are omitted when obvious
- the syntax `[ xxx | yyy | zzz]` in the following indicates a field that contain eiher `xxx`, `yyy`, or `zzz`.
- words fully capitalise should be replaced by numbers. 
- numbers between braces, eg. `{IDS}`, can have a stepping syntax, similar to matlab, eg. they can be a single number `42`, two numbers separated by column `42:51` (equivalent to `42:1:51`), three numbers separated by columns `42:0.1:51`. The syntax is `beginning:step:end`, the end is inclusive. 

Some command start with an initial keyword, follow by another keyword changing their behaviour.

### Special initial keyword:
- `#`: any line starting with a hash is a comment and is discarded.
- `event TT command`: a line starting with the `event` keyword will be processed only when the time `TT` is elapsed. Basically any other command working by itself can be used as a later time event. 

### Multi-keyword commands
First indentation level keyword must be followed by one of the second level indentation one.

- `set`: set the value of a parameter. Expect a number after the keyword, unless otherwise noted
  - `T`: Total time
  - `dt`: duration of a timestep
  - `tdump`: number of timestep between each dump of result
  - `tinfo`: number of timestep between each line of information in the terminal
  - `rho`: density of the grains
  - `Kn`: normal stiffness
  - `Kt`: tangential stiffness
  - `GammaN`: normal dissipation
  - `GammaT`: tangential dissipation
  - `Mu`: friction coefficient between grains
  - `Mu_wall`: friction coefficient between wall and grains
  - `damping`: global damping coefficient
  - `orientationtracking`: calculate grain orientation or not (expect a true or false value)
  - `skin`: skin size [DO NOT USE]
  - `gradientdescent_gamma`: decay rate when used for gradient descent algorithm (only for wall ELLIPSE currently)
  - `gradientdescent_tol`: tolerance for stopping the gradient descent algo. 

- `auto`: perform one of the following operations:
  - `mass`: set the mass of particles from density
  - `rho`: setting the density from the mass of particle #0. 
  - `inertia`: setting the moment of inertia for spheres.
  - `location [square|randomsquare ...]`: set the particle location
    - `square`: square lattice
    - `randomsquare`: square lattice with some randomness in particle location
    - `randomdrop`: random coordinates of the particles in the simulation region
    - `insphere`: randomly place the particles in a sphere, assuming that the wall number d is a sphere. 
    - `roughinclineplane`: stick grain along the plane of normal [1,0,0 ...] to create roughness.
    - `rhoughinclineplane2`: ????? not quite sure what this does anymore. 
    - `quasicristal`: ????? not quite sure what this does anymore. 
  - `radius [uniform|bidisperse] SMALLRADIUS LARGERADIUS [RATIO]`: set the particle radius from a distribution. Bidisperse is now defined by volume. For bidisperse, $ratio=V_{large}/(V_{small}+V_{large})$.
  - `skin`: set the skin size. Should be used after the particle radius has been set. 
  
- `rigid N`: handles rigid bodies number N
  - `set NPARTICLES ID1 ID2 ... IDN`: particles belonging to the rigid body. 
  - `gravity [on|off]`: use gravity on the particles of the rigid body or not. 
  - `addforce FX0 FX1 ...`: addforce to the whole rigid body. 
  - `velocity [VX0|nan] [VX1|nan] ... `: set velocity of the rigid body, or not if `nan` is used. 
  
### Singular commands
These command have a single behaviour, described here.

- `directory`: where the data dump are saved. The directory is created if it does not exist. 
- `dimensions D NUMBERPARTICLES`: number of spatial dimensions and particles to use. Need to match the command line arguments.
- `location {IDS} {X0} {X1} ...`: set the location of the indicated particles. 
- `velocity ID X0 X1 ...`: set the velocity of the indicated particles. 
- `omega ID X00 X10 ...`: set the angular velocity of the indicated particles.
- `freeze ID`: set zero acceleration of the indicated particles. 
- `radius ID RAD`: set the particle radius
- `mass ID M`: set the particle mass
- `gravity G0 G1 ... Gd`: set the gravity vector
- `gravityangle INTENSITY ANGLE`: set the gravity angled from first dimension
- `gravityrotate INTENSITY OMEGA DIM1 DIM2`: make the gravity rotate at an angular velocity OMEGA between DIM1 and 2.
- `ContactModel [Hooke|Hertz]`: set contact model. 
- `boundary DIM [PBC|WALL|MOVINGWALL|SPHERE|ROTATINGSPHERE|PBCLE|ELLIPSE] LOCMIN LOCMAX extrainfo`: wall type along dimension DIM (the types are strings, not values). All walls require extra arguments:
  - `MOVINGWALL LOCMIN LOCMAX VELMINX VELMINY`
  - `SPHERE RADIUS X1 ... XD`: sphere of radius RADIUS, center at the X location
  - `ROTATINGSPHERE RADIUS X1 ... XD R12 R13 ... R1D R23 ... R_D-1_D`: Rotating sphere of radius RADIUS, center at the X location, rotation matrix given by the R components (upper right corner of the skew-symetric rotation matrix). 
  - `PBCLE LOCMIN LOCMAX VELOCITY`: Lees-Edward boundary condition. Should be in dimension 0. 
  - `ELLIPSE RX RY CX CY`: Ellipse wall (dim 2 only)
  - `allother LOCMIN LOCMAX`
  - Special command: `boundary DIM REMOVE` Delete the boundary in the indicated dimension. Usefull to remove default created boundaries, but use with caution. 
- `rigid`: TODO (in development) ...
- `mesh [file|translate|rotate|export] ...`
  - `file filename.json`: load a mesh file in json format. (in development)
  - `translate X0 X1 ... XN`: translate all the meshes by the given vector
  - `rotate X00 X01 ... X0N X10 ... XNN`: rotate all the meshes by the given rotation vector. Center of rotation is the mesh origin (first point)
  - `erase ID`: erase the mesh with the given ID
  - `export filename.json`: write all the current meshes in a json file
  
- `dumps filename [VTK|NETCDF|XML|XMLbase64|CSV|CSVA|CONTACTFORCES|WALLFORCE] with N [cf. below ...]`: select dump format and output fiels, the last 4 formats are plain text. `N` is how many fields output follow, which will be in the output dumped file. Note that not all fields are available for all formats. Field can include:
  - Particle data field: `[Position|Velocity|Omega|OmegaMag|Orientation|Coordination|Radius|Ids]*`
  - Contact data fields: `[Ids|Fn|Ft|Torque|Branch|Fn_el|Fn_visc|Ft_el|Ft_visc|Ft_fric|Ft_frictype|Ghost_mask|Ghost_direction]*`. NB: if Ghost_mask>0; then the contact crosses a periodic boundary condition. 






