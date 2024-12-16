import numpy as np
from scipy.special import gamma

r_min = 0.5
r_max = 1.0
H_target = 20 # distance in first dimension
nu_target = 0.5
phi_0 = [1,0.81,0.6,0.4,0.22,0.18]
delta_phi = [0,0.48,0.22,0.1,0.05,0.02]
I_target = 1e-2
large_fraction = 0.5
L_0 = 11 # distance in second dimension
L_1 = 6.8 # distance in all remaining dimensions
for i,N in enumerate([2,3,4,5,6]):
    for j,r_max in enumerate([0.5,0.6,0.7,0.8,0.9,1,1.1,1.2,1.3,1.4,1.5]):
        R = r_max/r_min
        phi = phi_0[i] - delta_phi[i]*I_target
        if N ==1:
            V_target = H_target
        elif N == 2:
            V_target = H_target*L_0
        else:
            V_target = H_target*L_0*L_1**(N-2)
        V_solid = phi*V_target
        V_small = np.pi**(N/2)/gamma(N/2+1)*r_min**N
        V_large = np.pi**(N/2)/gamma(N/2+1)*r_max**N

        N_small = int(np.ceil((1-large_fraction)*V_solid/V_small))
        N_large = int(np.ceil(large_fraction*V_solid/V_large))

        N_total = N_small + N_large

        print(f'N={N}, N_small={N_small}, N_large={N_large}, N_total={N_total}')
        
        # Load the template file
        with open(f'in.inclinedD{N}', 'r') as file:
            template_content = file.read()

        # Replace the placeholder with N_total
        modified_content = template_content.replace('__FOLDER__', f'D{N}Ratio{R:.1f}')
        modified_content = modified_content.replace('__NUM_PARTICLES__', str(N_total))
        modified_content = modified_content.replace('__MAX_SIZE__', str(r_max))
        # modified_content = modified_content.replace('__OFFSET__', str(r_max))
        # modified_content = modified_content.replace('__SPEED__', str(2*r_max))
        

        # Save the modified content to a new file
        output_filename = f'upload/in.inclinedD{N}Ratio{R:.1f}'
        with open(output_filename, 'w') as file:
            file.write(modified_content)

        with open(f'D{N}.qsub', 'r') as file:
            template_content = file.read()
        
        qsub_file = template_content.replace('__INFILE__', f'in.inclinedD{N}Ratio{R:.1f}')
        with open(f'upload/D{N}Ratio{R:.1f}.qsub', 'w') as file:
            file.write(qsub_file)