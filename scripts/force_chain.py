import os, sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.distance import cdist

def unit_vector(vector):
    """ Returns the unit vector of the vector.  """
    return vector / np.linalg.norm(vector)

def angle_between(v1, v2):
    """ Returns the angle in radians between vectors 'v1' and 'v2'::

            >>> angle_between((1, 0, 0), (0, 1, 0))
            1.5707963267948966
            >>> angle_between((1, 0, 0), (1, 0, 0))
            0.0
            >>> angle_between((1, 0, 0), (-1, 0, 0))
            3.141592653589793
    """
    v1_u = unit_vector(v1)
    v2_u = unit_vector(v2)
    return np.arccos(np.clip(np.dot(v1_u, v2_u), -1.0, 1.0))

verbose = True
# verbose = False

timestep = int(sys.argv[1])
N = int(sys.argv[2])
# timestep = 30
radius = 0.5 # MONODISPERSE
# N = 2
data_dir = f'../Samples/D{N}/'
L = 5

if N == 2:
    V = 2.*np.pi*radius
elif N == 3:
    V = 4./3.*np.pi*radius**3


for timestep in range(200):
    filename = f'{data_dir}dumpcontactforce-{timestep*10000:05d}.csv'
    data = np.loadtxt(filename,skiprows=1)


    IDs = data[:,:2].astype(int)
    x1 = data[:,2    :2+  N]
    x2 = data[:,2+  N:2+2*N]
    Fn = data[:,2+2*N:2+3*N]
    Ft = data[:,2+3*N:2+4*N]

    dist = np.sqrt(np.sum(np.square(x1-x2),axis=1))
    # move the ghost particles around
    for i in range(len(dist)):
        if dist[i] > 2*radius:
            for n in range(N):
                if x1[i,n] - x2[i,n] > 2*radius:
                    x2[i,n] += L
                elif x1[i,n] - x2[i,n] < -2*radius:
                    x2[i,n] -= L

    force_loc = 0.5*(x1 + x2)

    # print(len(data))
    # print(np.hstack([x1,x2]))
    # PDF of forces
    if verbose:

        F_mag = np.sqrt((np.sqrt(Fn**2).sum(axis=1)) + (np.sqrt(Ft**2).sum(axis=1)))
        # plt.hist(F_mag,bins=20)
        # plt.show()

        markersize = 35
        plt.clf()
        plt.ion()
        plt.title(f't={timestep}')
        plt.plot(x1[:,0],x1[:,1],'ko',markersize=markersize,alpha=0.2,mec='None')
        plt.plot(x2[:,0],x2[:,1],'ko',markersize=markersize,alpha=0.2,mec='None')
        for i in range(len(data)): # plot branch vectors
            if dist[i] < 2*radius: # using old distance measure
                plt.plot([x1[i,0],x2[i,0]],[x1[i,1],x2[i,1]],'k-',lw=F_mag[i]/F_mag.mean())
            else:
                plt.plot([x1[i,0],x2[i,0]],[x1[i,1],x2[i,1]],'k--',lw=F_mag[i]/F_mag.mean())
        plt.scatter(force_loc[:,0],force_loc[:,1],c=F_mag)
        plt.gca().set_aspect('equal')
        plt.colorbar()
        plt.xlim(-2*radius,L+2*radius)
        plt.ylim(-2*radius,L+2*radius)
        # plt.show()
        plt.pause(1e-5)

# # Force chain calculation
# number_of_particles = IDs.max() + 1 # add one for 0-based counting
# sigma = np.zeros([number_of_particles,N,N])
# xp = np.zeros([number_of_particles,N]) # location of particle with correct ID
#
# for row in range(data.shape[0]):
#     branch_vector = x2[row] - x1[row]
#     if np.sqrt( (branch_vector**2).sum() ) < 2*radius: # ignore periodic boundaries!
#         sigma[IDs[row][0]] += np.outer(Fn[row],branch_vector)
#         sigma[IDs[row][1]] += np.outer(Fn[row],branch_vector) # PROBABLY MISSING A MINUS SIGN?!??
#         sigma[IDs[row][0]] += np.outer(Ft[row],branch_vector)
#         sigma[IDs[row][1]] += np.outer(Ft[row],branch_vector) # PROBABLY MISSING A MINUS SIGN?!??
#         xp[IDs[row][0]] = x1[row]
#         xp[IDs[row][1]] = x2[row]
# sigma /= V
#
# principal_stress = np.zeros([number_of_particles])
# principal_stress_direction = np.zeros([number_of_particles,N]) # keep the whole eigenvector
# for i in range(number_of_particles):
#     eigenvalues, eigenvectors = np.linalg.eig(sigma[i])
#     principal_stress[i] = eigenvalues.max() # CHECK WITH FRANCOIS: IS COMPRESSION POSITIVE WITH THIS DEFINITION??!?? IF SO NEED MAJOR PRINCIPAL STRESS (MAX)!
#     arg = np.argmax(eigenvalues)
#     principal_stress_direction[i] = eigenvectors[:,arg]
#
# if verbose:
#     plt.scatter(xp[:,1],xp[:,0],c=np.abs(principal_stress))
#     plt.colorbar()
#     plt.show()
#
# # Force chain statistics
# mean_abs_principal_stress = np.mean(np.abs(principal_stress))
# highly_stressed = np.abs(principal_stress) > mean_abs_principal_stress
#
# if verbose:
#     plt.scatter(xp[:,1],xp[:,0],c=highly_stressed)
#     plt.colorbar()
#     plt.show()
#
# x_hs = xp[highly_stressed] # just highly_stressed particles
#
# # Now find the number of neighbours within this subset
# distances = cdist(x_hs,x_hs)
# contacts = distances < 2*radius
# num_highly_stressed_neighbours = contacts.sum(axis=0) - 1 # remove self
#
# # Just those that have neighbours
# x_hs_chainable = x_hs[num_highly_stressed_neighbours > 2] # remove singletons
# # x_hs_chainable
