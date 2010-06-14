# 
# ALPS Project: Algorithms and Libraries for Physics Simulations
# 
# ALPS Libraries
# 
# Copyright (C) 2010 by Brigitte Surer <surerb@phys.ethz.ch> 
# 
# This software is part of the ALPS libraries, published under the ALPS
# Library License; you can use, redistribute it and/or modify it under
# the terms of the license, either version 1 or (at your option) any later
# version.
#  
# You should have received a copy of the ALPS Library License along with
# the ALPS Libraries; see the file LICENSE.txt. If not, the license is also
# available from http://alps.comp-phys.org/.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
# SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
# FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.
# 
# ****************************************************************************

import pyalps
import numpy as np
import matplotlib.pyplot as plt
import pyalps.pyplot


#prepare the input parameters
parms=[]
for b in [6.,8.,10.,12.,14.,16.]:
    parms.append(
            {
              'ANTIFERROMAGNET'     : 1,
              'CONVERGED'           : 0.03,
              'F'                   : 10,
              'FLAVORS'             : 2,
              'H'                   : 0,
              'H_INIT'              : 0.2,
              'MAX_IT'              : 10,
              'MAX_TIME'            : 60,
              'MU'                  : 0,
              'N'                   : 1000,
              'NMATSUBARA'          : 1000,
              'N_FLIP'              : 0,
              'N_MEAS'              : 10000,
              'N_MOVE'              : 0,
              'N_ORDER'             : 50,
              'N_SHIFT'             : 0,
              'OMEGA_LOOP'          : 1,
              'OVERLAP'             : 0,
              'SEED'                : 0,
              'SITES'               : 1,
              'SOLVER'              : 'Hybridization',
              'SYMMETRIZATION'      : 0,
              'TOLERANCE'           : 0.01,
              'U'                   : 3,
              't'                   : 0.707106781186547,
              'SWEEPS'              : 100000000,
              'THERMALIZATION'      : 1000,
              'BETA'                : b,
              'CHECKPOINT'          : 'solverdump_beta_'+str(b)+'.task1.out.h5',
              'G0TAU_INPUT'         : 'G0_tau_input_beta_'+str(b)
            }
        )
        
#write the input file and run the simulation
for p in parms:
    input_file = pyalps.writeParameterFile('parm_beta_'+str(p['BETA']),p)
    res = pyalps.runDMFT(input_file)

flavors=parms[0]['FLAVORS']
listobs=[]   
for f in range(0,flavors):
    listobs.append('Green_'+str(f))
    
ll=pyalps.load.Hdf5Loader()
data = ll.ReadMeasurementFromFile(pyalps.getResultFiles(pattern='parm_beta_*h5'), respath='/simulation/results/G_tau', measurements=listobs, verbose=True)
for d in data:
    for f in range(0,flavors):
        d[f].x = d[f].x*d[f].props["BETA"]/float(d[f].props["N"])
        plt.figure()
        pyalps.pyplot.plot(d[f])
        plt.xlabel(r'$\tau$')
        plt.ylabel(r'$G(\tau)$')
        plt.title('Hubbard model on the Bethe lattice')
        plt.show()
