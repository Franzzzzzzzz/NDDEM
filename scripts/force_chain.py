import os, sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.distance import cdist
from scipy.special import gamma

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

N = int(sys.argv[1])
# timestep = 30
# radius = 0.5 # MONODISPERSE
# N = 2
data_dir = f'../Samples/D{N}/'
L = 10
markersize = 38

# Following derivation in
# https://journals.aps.org/pre/abstract/10.1103/PhysRevE.72.041307
num_chainable = []

for timestep in range(500):
# for timestep in [int(sys.argv[2])]:
    filename = f'{data_dir}dumpcontactforce-{timestep*10000:05d}.csv'
    data = np.loadtxt(filename,skiprows=1)
    radii = np.loadtxt(f'{data_dir}dump-{timestep*10000:05d}.csv',skiprows=1,usecols=N,delimiter=',')
    max_radius = radii.max()
    if len(data.shape) < 2:
        print(f'no particles in contact at timestep {timestep:05d}')
        # re-save something
        plt.savefig(f'frame-{timestep:05d}.png',dpi=100)
    else:

        IDs = data[:,:2].astype(int)
        x1 = data[:,2    :2+  N]
        x2 = data[:,2+  N:2+2*N]
        Fn = data[:,2+2*N:2+3*N]
        Ft = data[:,2+3*N:2+4*N]

        dist = np.sqrt(np.sum(np.square(x1-x2),axis=1))
        # move the ghost particles around
        for i in range(len(dist)):
            if dist[i] > max_radius:
                for n in range(N):
                    if x1[i,n] - x2[i,n] > 2*max_radius:
                        x2[i,n] += L
                    elif x1[i,n] - x2[i,n] < -2*max_radius:
                        x2[i,n] -= L

        force_loc = 0.5*(x1 + x2)

        # print(len(data))
        # print(np.hstack([x1,x2]))
        # PDF of forces
        if verbose:

            F_mag = np.sqrt((np.sqrt(Fn**2).sum(axis=1)) + (np.sqrt(Ft**2).sum(axis=1)))
            plt.clf()
            plt.hist(F_mag,bins=20)
            plt.xlabel('Force magnitude')
            plt.ylabel('Count')
            plt.savefig(f'im/D{N}/hist/{timestep:05d}.png')

            plt.clf()
            # plt.ion()
            plt.title(f't={timestep}')
            plt.plot(x1[:,0],x1[:,1],'ko',markersize=markersize*radii.mean(),alpha=0.2,mec='None')
            plt.plot(x2[:,0],x2[:,1],'ko',markersize=markersize*radii.mean(),alpha=0.2,mec='None')
            F_mag_mean = F_mag.mean()
            for i in range(len(data)): # plot branch vectors
                if F_mag[i] < F_mag_mean: c = 'k'
                else: c = 'r'
                if dist[i] < 2*max_radius: # using old distance measure
                    plt.plot([x1[i,0],x2[i,0]],[x1[i,1],x2[i,1]],c=c,ls='-',lw=F_mag[i]/F_mag_mean)
                else:
                    plt.plot([x1[i,0],x2[i,0]],[x1[i,1],x2[i,1]],c=c,ls='--',lw=F_mag[i]/F_mag_mean)
            plt.scatter(force_loc[:,0],force_loc[:,1],c=F_mag)
            plt.gca().set_aspect('equal')
            plt.colorbar()
            plt.xlim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.ylim(-2*max_radius-L/2,2*max_radius+L/2)
            # plt.show()
            # plt.pause(1e-5)
            plt.savefig(f'im/D{N}/frame/{timestep:05d}.png',dpi=100)

        # Force chain calculation
        number_of_particles = IDs.max() + 1 # add one for 0-based counting
        sigma = np.zeros([number_of_particles,N,N])
        xp = np.nan*np.zeros([number_of_particles,N]) # location of particle with correct ID - initialise with nans to remove particles with no contacts from statistics

        for row in range(data.shape[0]):
            branch_vector = x2[row] - x1[row]
            sigma[IDs[row][0]] += np.outer(Fn[row],branch_vector)
            sigma[IDs[row][1]] += np.outer(Fn[row],branch_vector) # PROBABLY MISSING A MINUS SIGN?!??
            sigma[IDs[row][0]] += np.outer(Ft[row],branch_vector)
            sigma[IDs[row][1]] += np.outer(Ft[row],branch_vector) # PROBABLY MISSING A MINUS SIGN?!??
            xp[IDs[row][0]] = x1[row]
            xp[IDs[row][1]] = x2[row]

        for i in range(number_of_particles):
            particle_volume = np.pi**(N/2)*radii[i]**N/gamma(N/2 + 1)
            sigma[i] /= particle_volume

        principal_stress = np.zeros([number_of_particles])
        principal_stress_direction = np.zeros([number_of_particles,N]) # keep the whole eigenvector
        for i in range(number_of_particles):
            eigenvalues, eigenvectors = np.linalg.eig(sigma[i])
            principal_stress[i] = eigenvalues.min() # CHECK WITH FRANCOIS: IS COMPRESSION POSITIVE WITH THIS DEFINITION??!?? IF SO NEED MAJOR PRINCIPAL STRESS (MAX)!
            arg = np.argmax(eigenvalues)
            principal_stress_direction[i] = eigenvectors[:,arg]

        if verbose:
            plt.clf()
            plt.scatter(xp[:,0],xp[:,1],c=np.abs(principal_stress),s=np.square(markersize*radii))
            plt.gca().set_aspect('equal')
            plt.colorbar()
            plt.xlim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.ylim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.savefig(f'im/D{N}/principal_stress/{timestep:05d}.png')

        # Force chain statistics
        mean_abs_principal_stress = np.mean(np.abs(principal_stress))
        highly_stressed = np.abs(principal_stress) > mean_abs_principal_stress

        if verbose:
            plt.clf()
            plt.scatter(xp[:,0],xp[:,1],c=highly_stressed,s=np.square(markersize*radii))
            plt.gca().set_aspect('equal')
            plt.colorbar()
            plt.xlim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.ylim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.savefig(f'im/D{N}/highly_stressed/{timestep:05d}.png')

        x_hs = xp[highly_stressed] # just highly_stressed particles
        r_hs = radii[highly_stressed]

        # Now find the number of neighbours within this subset
        distances = cdist(x_hs,x_hs)
        contacts = distances < 2*max_radius
        num_highly_stressed_neighbours = contacts.sum(axis=0) - 1 # remove self

        # Just those that have neighbours
        x_hs_chainable = x_hs[num_highly_stressed_neighbours > 0] # remove singletons
        r_hs_chainable = r_hs[num_highly_stressed_neighbours > 0] # remove singletons
        num_chainable.append(len(x_hs_chainable))

        if verbose:
            plt.clf()
            plt.scatter(x_hs_chainable[:,0],x_hs_chainable[:,1],c='k',s=np.square(markersize*r_hs_chainable))
            plt.gca().set_aspect('equal')
            plt.colorbar()
            plt.xlim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.ylim(-2*max_radius-L/2,2*max_radius+L/2)
            plt.savefig(f'im/D{N}/chainable/{timestep:05d}.png')

            # print(np.arange(timestep))
            # print(np.array(num_chainable))
            plt.clf()
            # plt.plot(np.arange(timestep),np.array(num_chainable)/number_of_particles,'k.')
            plt.plot(np.array(num_chainable)/number_of_particles,'k.')
            plt.ylim([0,1])
            plt.xlabel('timestep')
            plt.ylabel('fraction of particles that are chainable')
            plt.savefig(f'im/D{N}/chainable_fraction.png')
