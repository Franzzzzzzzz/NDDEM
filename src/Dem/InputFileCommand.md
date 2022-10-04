# List of input file commands for coarse-graining (CG)
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
  - `damping`: ????
  - `orientationtracking`: calculate grain orientation or not (expect a true or false value)
  - `skin`: skin size [DO NOT USE]

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
  - `radius [uniform|bidisperse] SMALLRADIUS LARGERADIUS`: set the particle radius from a distribution.
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
- `boundary DIM [PBC|WALL|MOVINGWALL|SPHERE|ROTATINGSPHERE|PBCLE] LOCMIN LOCMAX extrainfo`: wall type along dimension DIM (the types are strings, not values). All walls require extra arguments:
  - `MOVINGWALL LOCMIN LOCMAX VELMINX VELMINY`
  - `SPHERE ?????`: not sure how the syntax work ??????
  - `ROTATINGSPHERE ?????`: not sure how the syntax work ??????
  - `PBCLE LOCMIN LOCMAX VELOCITY`: Lees-Edward boundary condition. Should be in dimension 0. 
  - `allother LOCMIN LOCMAX`

- `dumps filename [VTK|NETCDF|XML|XMLbase64|CSV|CSVA|CONTACTFORCES|WALLFORCE] with N [Position|Velocity|Omega|OmegaMag|Orientation|Coordination|Radius|Ids|Fn|Ft|Torque|Branch]`: select filename format, the last 4 format are plain text. `N` is how many fields you want out of the indicated list. Note that not all fields are available for all formats. 












