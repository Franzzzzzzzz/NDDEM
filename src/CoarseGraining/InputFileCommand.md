# List of input file commands for coarse-graining (CG)
### Description
Input files are in json format.
In the following: 

- double quotes are omitted when obvious
- the syntax `[ xxx | yyy | zzz]` in the following indicates a field that contain eiher `xxx`, `yyy`, or `zzz`.

### Commands
#### Input file description
- `file`: contains an array of input files to read. The input files objects contain the following fields:
    - `filename`: name of the input file
    - `format: [liggghts | NDDEM | mercury_legacy | mercury_vtu | interactive]` -- format of the input file. `interactive` should not be used outside an API call from NDDEM.
    - `content: [particles | contacts | both]` -- content type of the input file .
    - `number` -- Number of files (1 for single file dumped).
    - `mapping: { ["variable": "column",]+ }` -- Each field map a physical variable understood by CG to a column/field in the input file.
- `density`: default density. Used if the particle mass is not available in the input file.
- `radius`: default radius. Used if the particle diameter is not available in the input file.
- `diameter`: default diameter. Used if the particle diameter is not available in the input file.

#### Defining the Coarse Graining
- `dimension`: number of spatial dimensions
- `fields`: array of fields to process. Recognised fields are the following: `RHO, I, VAVG, TC, TK, ROT, MC, MK, mC, EKT, eKT, EKR, eKR, qTC, qTK, qRC, qRK, zT, zR, RADIUS, TotalStress, Pressure, KineticPressure, ShearStress, StrainRate,VolumetricStrainRate, ShearStrainRate, RotationalVelocity, RotationalVelocityMag`.
- `boxes`: array. Number of boxes in each dimension. 
- `boundaries`: volume that is coarse-grained. Do not specify in order to process the whole simulation (if the simulation bounds are saved in the input file).
- `window: [Rect3D | Sphere3DIntersect | SphereNDIntersect | RectND | Hann3D | Lucy3D | Lucy3DFancyInt | LucyND | LucyND_Periodic]`: shape of the averaging window.
- `window size`: size of the averaging window.
- `extra fields`: array of extrafield objects, to allow for computation of arbitrary fields. 
    - `name`: name of the field to compute. Should be unique.
    - `type: [Particle | Contact | Fluctuation]` -- Type of field. `Particle` will be calculated during Pass 1, `Contact` and `Fluctuation` are not implemented as of 2022-09-26.
    - `tensor order: [0|1|2]` -- 0 is scalar, 1 is vector, 2 is second order tensor. 
    


#### Time handling
- `skip`: number of timestep to skip
- `max time`: number of timestep to process (total, including skip)
- `time average: [None | Intermediate | Final | Intermediate and Final]`: when to time average. `Intermediate` averages before pass 2 to better calculate the fluctuations. `Final` averages the final fields. 

- `savefile`: name of the final file (no extension)
- `saveformat: [mat | vtk | numpy | netCDF]`: output file format. Can be an array for multiple output format at once. 
