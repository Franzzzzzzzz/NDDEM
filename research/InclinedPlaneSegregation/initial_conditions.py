import numpy as np
from scipy.special import gamma

r_min = 0.25
r_max = 0.5
H_target = 10
nu_target = 0.5
phi_0 = [1,0.81,0.6,0.4,0.22,0.18]
delta_phi = [0,0.48,0.22,0.1,0.05,0.02]
I_target = 1e-2
large_fraction = 0.5
for i,N in enumerate([1,2,3,4,5,6]):
    phi = phi_0[i] - delta_phi[i]*I_target
    if N ==1:
        V_target = H_target
    elif N == 2:
        V_target = H_target*5
    else:
        V_target = H_target*5*3.4**(N-2)
    V_solid = phi*V_target
    V_small = np.pi**(N/2)/gamma(N/2+1)*r_min**N
    V_large = np.pi**(N/2)/gamma(N/2+1)*r_max**N

    N_small = int(np.ceil((1-large_fraction)*V_solid/V_small))
    N_large = int(np.ceil(large_fraction*V_solid/V_large))

    print(f'N={N}, N_small={N_small}, N_large={N_large}, N_total={N_small+N_large}')
    