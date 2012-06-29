# 
# ALPS Project: Algorithms and Libraries for Physics Simulations
# 
# ALPS Libraries
# 
# Copyright (C) 2010 by Brigitte Surer <surerb@phys.ethz.ch> 
#               2012 by Jakub Imriska  <jimriska@phys.ethz.ch>
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
import pyalps.plot


#prepare the input parameters
parms=[]
for b in [6.,8.,10.,12.,14.,16.]: 
    parms.append(
            {                         
              'ANTIFERROMAGNET'         : 1,
              'CHECKPOINT'              : 'solverdump_beta_'+str(b),
              'CONVERGED'               : 0.005,
              'FLAVORS'                 : 2,
              'H'                       : 0,
              'H_INIT'                  : 0.05,
              'MAX_IT'                  : 16,
              'MAX_TIME'                : 5,
              'MU'                      : 0,
              'N'                       : 500,
              'NMATSUBARA'              : 500, 
              'OMEGA_LOOP'              : 1,
              'SEED'                    : 0, 
              'SITES'                   : 1,
              'SOLVER'                  : 'Interaction Expansion',
              'SYMMETRIZATION'          : 0,
              'U'                       : 3,
              't'                       : 0.707106781186547,
              'SWEEPS'                  : 100000000,
              'THERMALIZATION'          : 1000,
              'ALPHA'                   : -0.01,
              'HISTOGRAM_MEASUREMENT'   : 1,
              'BETA'                    : b
            }
        )

# NOTE: in revision of ALPS older than 6238, the MAX_TIME will effectively be 60 seconds.        
# Comment: the results will highly probably not be well converged.
# For more precise calculations we propose to you to:
#   enhance the MAX_TIME (to 60), 
#   lower the CONVERGED (to 0.003), 
#   increase MAX_IT (to 20)
# ( the runtime of the script with changed parameters will be roughly 2 hours )

## Please run the tutorial3a.py before this one or uncomment the following lines.
## This tutorial relies on the results created there.

# #write the input file and run the simulation
# for p in parms:
#     input_file = pyalps.writeParameterFile('parm_beta_'+str(p['BETA']),p)
#     res = pyalps.runDMFT(input_file)

listobs=['Green_0']   # we look at convergence of a single flavor (=0)
    
ll=pyalps.load.Hdf5Loader()
for p in parms:
    data = ll.ReadDMFTIterations(pyalps.getResultFiles(pattern='parm_beta_'+str(p['BETA'])+'.h5'), measurements=listobs, verbose=True)
    grouped = pyalps.groupSets(pyalps.flatten(data), ['iteration'])
    nd=[]
    for group in grouped:
        r = pyalps.DataSet()
        r.y = np.array(group[0].y)
        r.x = np.array([e*group[0].props['BETA']/float(group[0].props['N']) for e in group[0].x])
        r.props = group[0].props
        r.props['label'] = 'it'+r.props['iteration']
        nd.append( r )
    plt.figure()
    plt.xlabel(r'$\tau$')
    plt.ylabel(r'$G_{flavor=0}(\tau)$')
    plt.title('DMFT-03: ' + r'$\beta = %.4s$' %nd[0].props['BETA'])
    pyalps.plot.plot(nd)
    plt.legend()

plt.show()
