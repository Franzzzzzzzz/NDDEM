import numpy as np
from scipy.special import gamma

radius = 0.5
L = 5
phi_0 = [1,0.8,0.6,0.4,0.25,0.18]
with open('force_chain_params.txt','w') as f:
    f.write('dimension,radius,particle_volume,target_density, system_length,num_particles\n')
    for i,N in enumerate(range(1,7)):
        particle_volume = np.pi**(N/2)*radius**N/gamma(N/2 + 1)
        num_particles = int(np.ceil(phi_0[i]*L**N/particle_volume))
        f.write(f'{N},{radius},{particle_volume},{phi_0[i]},{L},{num_particles}\n')
        
