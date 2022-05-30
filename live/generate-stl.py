import numpy as np

class d4cube():
    def __init__(self,zero_dim):
        # corresponding to https://www.researchgate.net/profile/Frits-Post/publication/228953628/figure/fig1/AS:655135839617025@1533207964411/Decomposition-of-a-hexahedron-into-6-tetrahedra.png

        # define the vertices of the 3D cube
        A = [ 1, 1,-1]
        B = [-1, 1,-1]
        C = [-1,-1,-1]
        D = [ 1,-1,-1]
        E = [ 1, 1, 1]
        F = [-1, 1, 1]
        G = [-1,-1, 1]
        H = [ 1,-1, 1]

        # add an empty dimension at the required location
        for vertex in [A,B,C,D,E,F,G,H]:
            vertex.insert(zero_dim,0)

        # define all the tetrahedra
        tetrahedron_1 = [A,E,F,G]
        tetrahedron_2 = [A,B,F,G]
        tetrahedron_3 = [A,B,C,G]
        tetrahedron_4 = [A,E,H,G]
        tetrahedron_5 = [A,D,H,G]
        tetrahedron_6 = [A,C,D,G]

        self.data = np.array([tetrahedron_1,tetrahedron_2,tetrahedron_3,tetrahedron_4,tetrahedron_5,tetrahedron_6],
                             dtype=np.float32)
        # self.data += 1e-4*(np.random.rand(6,4,4)-0.5) # fix convex hull issues
        self.N = 4

    def translate(self,vec):
        for i, solid in enumerate(self.data):
            for j, facet in enumerate(solid):
                self.data[i][j] += vec

    def scale(self,vec):
        for i, solid in enumerate(self.data):
            for j, facet in enumerate(solid):
                self.data[i][j] *= vec

def normal_vector(points):
    p0, p1, p2 = points
    x0, y0, z0 = p0
    x1, y1, z1 = p1
    x2, y2, z2 = p2

    ux, uy, uz = u = [x1-x0, y1-y0, z1-z0] #first vector
    vx, vy, vz = v = [x2-x0, y2-y0, z2-z0] #sec vector

    u_cross_v = [uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx] #cross product
    return u_cross_v/np.linalg.norm(u_cross_v)

def write_stl(filename,N,solids):
    with open(filename, 'w') as f:
        for solid in solids:
            f.write(f'solid {N}\n')
            for facet in solid:
                if N == 3:
                    normal = ' '.join(str(v) for v in normal_vector(facet))
                else:
                    normal = '0 '*N
                f.write(f'    facet normal {normal}\n')
                f.write(f'        outer loop\n')
                for vertex in facet:
                    f.write(f"            vertex {' '.join(str(v) for v in vertex)}\n")
                f.write(f'        endloop\n')
                f.write('    endfacet\n')
            f.write('endsolid\n')

def test_4D_simplex():
    N = 4 # dimension

    facet = [
        [1,0,0,0],
        [0,1,0,0],
        [0,0,1,0],
        [0,0,0,1]
    ]

    solid = [facet]

    solids = [solid]
    return N,solids

def test_4D_simplices():
    N = 4 # dimension

    facet1 = [
        [1,0,0,0],
        [0,1,0,0],
        [0,0,1,0],
        [0,0,0,1]
    ]
    facet2 = [
        [1,0,1,0],
        [0,1,0,1],
        [-1,0,1,0],
        [0,1,0,1]
    ]

    solid1 = [facet1,facet2]

    solids = [solid1]
    return N,solids

def make_3D_pool_table():
    N = 3 # dimension
    L1 = 12
    L2 = 6
    L3 = 1

    bottom_wall_a = [
        [-L1,L2,0],
        [L1,-L2,0],
        [L1,L2,0],
    ]
    bottom_wall_b = [
        [-L1,L2,0],
        [L1,-L2,0],
        [-L1,-L2,0],
    ]
    left_wall_a = [
        [L1,-L2,0],
        [-L1,-L2,L3],
        [L1,-L2,L3],
    ]
    left_wall_b = [
        [-L1,-L2,L3],
        [L1,-L2,0],
        [-L1,-L2,0],
    ]

    right_wall_a = [
        [-L1,L2,L3],
        [L1,L2,0],
        [L1,L2,L3],
    ]
    right_wall_b = [
        [L1,L2,0],
        [-L1,L2,L3],
        [-L1,L2,0],
    ]

    front_wall_a = [
        [L1,L2,0],
        [L1,-L2,0],
        [L1,L2,L3],
    ]
    front_wall_b = [
        [L1,-L2,L3],
        [L1,L2,L3],
        [L1,-L2,0],
    ]
    back_wall_a = [
        [-L1,-L2,0],
        [-L1,L2,0],
        [-L1,L2,L3],
    ]
    back_wall_b = [
        [-L1,L2,L3],
        [-L1,-L2,L3],
        [-L1,-L2,0],
    ]

    bottom_wall = [bottom_wall_a,bottom_wall_b]
    left_wall   = [left_wall_a,  left_wall_b]
    right_wall  = [right_wall_a, right_wall_b]
    front_wall  = [front_wall_a, front_wall_b]
    back_wall   = [back_wall_a,  back_wall_b]

    solids = [bottom_wall,left_wall,right_wall,front_wall,back_wall]
    return N,solids

def make_4D_pool_table_no_holes():
    N = 4 # dimension
    L1 = 2
    L2 = 0.1 # this is the direction of gravity
    L3 = 1
    L4 = 0.5


    bottom_wall = d4cube(zero_dim=1)
    bottom_wall.scale([L1,1,L3,L4])
    bottom_wall.translate([0,-L2,0,0])

    left_wall = d4cube(zero_dim=2)
    left_wall.scale([L1,L2,1,L4])
    left_wall.translate([0,0,-L3,0])

    right_wall = d4cube(zero_dim=2)
    right_wall.scale([L1,L2,1,L4])
    right_wall.translate([0,0,L3,0])

    front_wall = d4cube(zero_dim=0)
    front_wall.scale([1,L2,L3,L4])
    front_wall.translate([L1,0,0,0])

    back_wall = d4cube(zero_dim=0)
    back_wall.scale([1,L2,L3,L4])
    back_wall.translate([-L1,0,0,0])

    d4wall_a = d4cube(zero_dim=3)
    d4wall_a.scale([L1,L2,L3,1])
    d4wall_a.translate([0,0,0,-L4])

    d4wall_b = d4cube(zero_dim=3)
    d4wall_b.scale([L1,L2,L3,1])
    d4wall_b.translate([0,0,0, L4])

    solids = [bottom_wall.data,left_wall.data,right_wall.data,front_wall.data,back_wall.data,d4wall_a.data,d4wall_b.data]
    return N,solids

def make_4D_pool_table():
    N = 4 # dimension
    L1 = 2
    L2 = 0.1 # this is the direction of gravity
    L3 = 1
    L4 = 0.5
    pocket_size = 0.15
    n_bottom_walls = 20
    bottom_walls = []
    for i in range(n_bottom_walls):
        theta = i*np.pi/2/(n_bottom_walls-1) # angle along arc of pocket
        L1_cur = L1/2. - pocket_size*np.sin(theta)
        L3_cur = L3 - pocket_size*np.cos(theta)
        bottom_wall = d4cube(zero_dim=1)
        bottom_wall.scale([L1_cur,1,L3_cur,L4])
        bottom_wall.translate([L1/2.,-L2,0,0])
        bottom_walls.append(bottom_wall.data.copy())

        bottom_wall.translate([-L1,0,0,0])
        bottom_walls.append(bottom_wall.data)


    # bottom_wall_b = d4cube(zero_dim=2)
    # bottom_wall_b.scale([L1-pocket_size,1,L3,L4])
    # bottom_wall_b.translate([0,-L2,0,0])

    left_wall = d4cube(zero_dim=2)
    left_wall.scale([L1,L2,1,L4])
    left_wall.translate([0,0,-L3,0])

    right_wall = d4cube(zero_dim=2)
    right_wall.scale([L1,L2,1,L4])
    right_wall.translate([0,0,L3,0])

    front_wall = d4cube(zero_dim=0)
    front_wall.scale([1,L2,L3,L4])
    front_wall.translate([L1,0,0,0])

    back_wall = d4cube(zero_dim=0)
    back_wall.scale([1,L2,L3,L4])
    back_wall.translate([-L1,0,0,0])

    d4wall_a = d4cube(zero_dim=3)
    d4wall_a.scale([L1,L2,L3,1])
    d4wall_a.translate([0,0,0,-L4])

    d4wall_b = d4cube(zero_dim=3)
    d4wall_b.scale([L1,L2,L3,1])
    d4wall_b.translate([0,0,0, L4])

    solids = bottom_walls + [left_wall.data,right_wall.data,front_wall.data,back_wall.data,d4wall_a.data,d4wall_b.data]
    return N,solids

N, solids = make_4D_pool_table_no_holes()
write_stl('stls/4d-pool-no-holes.stl',N,solids)
N, solids = make_4D_pool_table()
write_stl('stls/4d-pool.stl',N,solids)
N, solids = make_3D_pool_table()
write_stl('stls/3d-pool.stl',N,solids)
N, solids = test_4D_simplex()
write_stl('stls/4d-simplex.stl',N,solids)
N, solids = test_4D_simplices()
write_stl('stls/4d-simplices.stl',N,solids)
