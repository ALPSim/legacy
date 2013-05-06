import numpy;
import scipy;
import pyalps;

def thermalized(h5_outfile, observables, tolerance=0.01, includeLog=False):
  if isinstance(observables, str):
    observables = [observables];

  results = [];
  for observable in observables:
    timeseries = pyalps.hdf5.iArchive(h5_outfile).read("/simulation/results/" + observable)['timeseries']['data']; 
    mean = timeseries.mean();

    index = scipy.linspace(0, timeseries.size-1, timeseries.size);
    timeseries = scipy.polyval(scipy.polyfit(index, timeseries, 1), index);  # timeseries get fitted

    percentage_increment = (timeseries[-1] - timeseries[0])/mean;
    result = abs(percentage_increment) < tolerance;

    if not includeLog:
      results.append(result);
    else:
      results.append({'observable': observable, 'percentage_increment' : percentage_increment, 'thermalized': result})

  return results;


def converged(h5_outfile, observables, includeLog=False):
  if isinstance(observables, str):
    observables = [observables];

  results = [];
  for observable in observables:
    measurements = pyalps.hdf5.iArchive(h5_outfile).read("/simulation/results/" + observable);

    result = (measurements['mean']['error_convergence'] == 0);

    if not includeLog:
      results.append(result);
    else:
      mean  = measurements['mean']['value'];
      error = measurements['mean']['error'];
      tau   = measurements['tau']['value'];
      count = measurements['count'];
      results.append({'observable': observable, 'converged': result, 'mean': mean, 'error': error, 'tau': tau, 'count': count});

  return results;


def tau(h5_outfile, observables):
  if isinstance(observables, str):
    observables = [observables];

  results = [];
  for observable in observables:
    measurements = pyalps.hdf5.iArchive(h5_outfile).read("/simulation/results/" + observable);
    results.append(measurements['tau']['value']);

  return results;



def recursiveRun(cmd, cmd_lang='command_line', follow_up_script=None, n=None, break_if=None, loc=None):
  ### 
  ### Either recursively run cmd for n times, or until the break_if condition holds true.
  ###
  ###  Note:
  ###    1. cmd              : command to be recursively run (quoted as a python str)
  ###    2. cmd_lang         : language of cmd, either "command_line" (default), or "python".
  ###    3. n                : number of recursions 
  ###    4. break_if         : condition to break recursion loop (quoted as a python str, interpreted as python command)     
  ###    5. follow_up_script : script to be run after command (""")
  ###    6. loc              : Python dict of local variables 
  ###

  if loc != None:
    locals().update(loc);

  if cmd_lang == 'command_line':
    pyalps.executeCommand(cmd.split());
  elif cmd_lang == 'python':
    eval(cmd);
  else:
    print "Error: The options for cmd_lang are 1) 'command_line' (default), or 2) 'python'."
    return;

  if follow_up_script != None:  
    eval(follow_up_script);

  if n != None:             # if n exists
    if isinstance(n, int):  # if n is a python integer
      if n <= 1:
        return;
      else:
        return recursiveRun(cmd, cmd_lang=cmd_lang, follow_up_script=follow_up_script, n=n-1, loc=locals()); 

  elif break_if != None:    # otherwise, if break_if exists
    if reduce(lambda x,y: x*y, eval(break_if)):
      return;
    else:
      return recursiveRun(cmd, cmd_lang=cmd_lang, follow_up_script=follow_up_script, break_if=break_if, loc=locals());

  else:                     # otherwise, recursiveRun only runs once
    return;

