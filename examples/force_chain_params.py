import numpy as np
from scipy.special import gamma

radius = 0.5
L_0 = 10
L_1 = L_0/2.
phi_0 = [1,0.81,0.6,0.4,0.25,0.18]
with open('force_chain_params.txt','w') as f:
    f.write('dimension,radius,particle_volume,target_density,system_length_0,system_length_1,num_particles\n')
    for i,N in enumerate(range(1,7)):
        total_volume = L_0*L_1**(N-1)
        particle_volume = np.pi**(N/2)*radius**N/gamma(N/2 + 1)
        num_particles = int(np.ceil(phi_0[i]*total_volume/particle_volume))
        f.write(f'{N},{radius},{particle_volume},{phi_0[i]},{L_0},{L_1},{num_particles}\n')
