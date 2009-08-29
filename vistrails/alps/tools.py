# VisTrails package for ALPS, Algorithms and Libraries for Physics Simulations
#
# Get ALPS at http://alps.comp-phys.org/
#
##############################################################################

from core.modules.vistrails_module import Module, ModuleError, NotCacheable
import core.bundles
import core.modules.basic_modules
import core.modules.module_registry
import os
import os.path
import tempfile
import copy
import glob

import parameters
import alpscore
import system
from parameters import Parameters, ParameterList
from packages.controlflow.list_module import ListOfElements

basic = core.modules.basic_modules

##############################################################################



class MakeParameterFile(Module):
    """Creates a parameter file.
    """
    def compute(self):
        if (self.hasInputFromPort('file')):
          o = self.getInputFromPort('file')
          if self.hasInputFromPort('simulationid'):
            raise ModuleError(self, 'Cannot specify a full path and a simulationid')
        else:
          o = self.interpreter.filePool.create_file()
        if self.hasInputFromPort('simulationid'):
            o = self.interpreter.filePool.create_file()
            o.name = os.path.join(os.path.dirname(o.name),self.getInputFromPort('simulationid'))
        f = file(o.name,'w')
        if self.hasInputFromPort('parmlist'):
          input_values = self.forceGetInputListFromPort('parmlist')
          for p in input_values:
            p.write(f);
        if self.hasInputFromPort('parms'):
          input_values = self.forceGetInputListFromPort('parms')
          for p in input_values:
            p.write(f);
        f.close()
        self.setResult('file', o)
        self.setResult('file_name', o.name)
        self.setResult('simulationid',os.path.basename(o.name))
    _input_ports = [('parmlist',[ParameterList]),
                    ('parms', [Parameters]),
                    ('file', [basic.File]),
                    ('simulationid',[system.SimulationID])]
    _output_ports=[('file', [basic.File]),
                   ('file_name',[basic.String]),
                   ('simulationid',[system.SimulationID])]


class Parameter2XML(alpscore.SystemCommandLogged):
    def compute(self):
        if self.hasInputFromPort('output_dir'):
          o = self.getInputFromPort('output_dir')
        else: 
          o = self.interpreter.filePool.create_file()
          os.unlink(o.name)
          os.mkdir(o.name)
        input_file = self.getInputFromPort("parameter")
        base_name = os.path.basename(input_file.name)
        print "Running"
        self.execute([alpscore._get_path('parameter2xml'),
                            input_file.name,
                            o.name + "/" + base_name])
        # Things would be easier on our side if ALPS somehow
        # advertised all the files it creates. Right now, it will be
        # hard to make sure all temporary files that are created will
        # be cleaned up. We are trying to keep all the temporaries
        # inside a temp directory, but the lack of control over where
        # the files are created hurt.
        ofile = basic.File()
        ofile.name = o.name + '/' + base_name + '.in.xml'
        self.setResult("output_dir", o)
        self.setResult("output_file", ofile)
    _input_ports = [('parameter', [basic.File]),
                    ('output_dir', [basic.File])]
    _output_ports = [('output_file', [basic.File]),
                     ('output_dir', [basic.File]),
                     ('log_file',[basic.File])]

class ExpandWildcards(Module):
    def expand(self,name):
        l = glob.glob(name)
        print "Found: ", l
        self.setResult('value',l)
        self.setResult('value_as_string',str(l))
    def compute(self):
      self.expand(self.getInputFromPort('input_file').name)
    _input_ports = [('input_file',[basic.File])]
    _output_ports = [('value',[ListOfElements]),
                     ('value_as_string',[basic.String])]

class GetRunFiles(ExpandWildcards):
    def compute(self):
        tasks = '*'
        runs = '*[0-9]'
        if (self.hasInputFromPort('tasks')):
          tasks = str(self.getInputFromPort('tasks'))
        if (self.hasInputFromPort('runs')):
          tasks = str(self.getInputFromPort('runs'))
        input_file = self.getInputFromPort('input_file')
        self.expand(input_file.name.replace('.out.xml', '.task' + tasks + '.out.run' +runs))
    _input_ports = [('tasks',[basic.String]),
                    ('runs',[basic.String])]

class GetResultFiles(ExpandWildcards):
    def compute(self):
        tasks = '*'
        if (self.hasInputFromPort('tasks')):
          tasks = self.getInputFromPort('tasks')
        input_file = self.getInputFromPort('input_file')
        self.expand(input_file.name.replace('.out.xml', '.task' + tasks + '.out.xml'))
    _input_ports = [('tasks',[basic.String])]

class Convert2XML(alpscore.SystemCommandLogged):
    def compute(self):
        input_file = self.getInputFromPort('input_file')
        self.execute([alpscore._get_path('convert2xml')] + input_file)
        ilist = copy.copy(input_file)
        olist = []
        for q in ilist:
          olist +=  [q + '.xml']
        self.setResult('value', olist)
        self.setResult('value_as_string', str(olist))
    _input_ports = [('input_file', [ListOfElements])]
    _output_ports = [('value', [ListOfElements]),
                     ('value_as_string',[basic.String]),
                     ('log_file',[basic.File])]
 
class Convert2Text(alpscore.SystemCommand):
    def compute(self):
        input_file = self.getInputFromPort('input_file')
        output_file = self.interpreter.filePool.create_file(suffix='.txt')
        self.execute([alpscore._get_path('convert2xml'), input_file.name, '>' , output_file.name])
        self.setResult('output_file', output_file)
    _input_ports = [('input_file', [basic.File])]
    _output_ports = [('output_file', [basic.File])]

class XML2HTML(alpscore.SystemCommand):
    def compute(self):
        input_file = self.getInputFromPort('input_file')
        output_file = self.interpreter.filePool.create_file(suffix='.html')
        cmdlist = [alpscore._get_path('xslttransform')]
        if self.hasInputFromPort('stylesheet'):
          cmdlist += [self.getInputFromPort('stylesheet').name]
        cmdlist += [input_file.name, '>' , output_file.name]
        self.execute(cmdlist)
        self.setResult('output_file', output_file)
    _input_ports = [('input_file', [basic.File]),
                    ('stylesheet',[basic.File])]
    _output_ports = [('output_file', [basic.File])]




def initialize(): pass

def selfRegister():

  reg = core.modules.module_registry.get_module_registry()
  
  reg.add_module(MakeParameterFile,namespace="Tools")
  reg.add_module(Parameter2XML,namespace="Tools")
  
  reg.add_module(ExpandWildcards,namespace="Tools")
  reg.add_module(GetRunFiles,namespace="Tools")
  reg.add_module(GetResultFiles,namespace="Tools")
  
  reg.add_module(Convert2XML,namespace="Tools")
  reg.add_module(Convert2Text,namespace="Tools")
  reg.add_module(XML2HTML,namespace="Tools")

