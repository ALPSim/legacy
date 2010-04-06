# ****************************************************************************
# 
# ALPS Project: Algorithms and Libraries for Physics Simulations
# 
# ALPS Libraries
# 
# Copyright (C) 1994-2010 by Bela Bauer <bauerb@phys.ethz.ch>
#                            Brigitte Surer <surerb@phys.ethz.ch> 
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

import core.modules.module_registry
import core.modules.basic_modules as basic
from core.modules.vistrails_module import Module, ModuleError, NotCacheable
from core.modules.python_source_configure import PythonSourceConfigurationWidget
from packages.controlflow.list_module import ListOfElements

import urllib, copy
import matplotlib.pyplot as plt
import numpy as np
import h5py
from scipy import optimize

from dataset_core import *
from dataset_exceptions import *
from pyalps.load import Hdf5Loader
from pyalps.hlist import flatten, depth
import pyalps

class Loader:
    def __init__(self,filename,label,xcolumn,ycolumns,props={}):
        self.sets = []
        self.read_set(filename,label,xcolumn,ycolumns,props)
    
    def __init__(self):
        self.sets = []
    
    def read_set(self,filename,label,xcolumn,ycolumns,props={}):
        raw = np.loadtxt(filename).transpose()
        
        if len(raw.shape) == 0:
            res = DataSet()
            res.x = np.array([0])
            res.y = np.array([raw])
            res.props.update(props)
            res.props.update({'column':0,'label':copy.deepcopy(label),'filename':filename})
            self.sets.append(res)
            return
        
        for iyc in ycolumns:
            res = DataSet()
            res.props['filename'] = filename
            
            if xcolumn >= 0:
                res.x = raw[xcolumn]
                res.y = raw[iyc]
            else:
                if len(raw.strides) == 1:
                    res.x = np.arange(0,len(raw))
                    res.y = raw
                else:
                    res.y = raw[iyc]
                    res.x = np.arange(0,len(res.y))
            
            res.props['column'] = iyc
            res.props['label'] = copy.deepcopy(label)
            res.props.update(props)
            
            self.sets.append(res)

class LoadDataSet(Module):
    """Load dataset from text data. Description of input ports:
    @file: The file, of which only the name is used.
    @label: A possible label for the dataset
    @x-column: The column of the data used for x data. Default: 0
    @y-columns: A ListOfElements of the columns. Default: 1
    Catch: if the file only has one columns, set x-column to -1!
    The following properties are set: filename, [label], column"""
    my_input_ports = [
        PortDescriptor("file",basic.File),
        PortDescriptor("label",basic.String),
        PortDescriptor("x-column",basic.Integer),
        PortDescriptor("y-columns",ListOfElements)
    ]

    my_output_ports = [
        PortDescriptor("data",DataSets)
    ]

    def compute(self):
        if self.hasInputFromPort('file'):
            f = self.getInputFromPort('file')
            filename = f.name
            
            xc = 0
            yc = [1]
            if self.hasInputFromPort('x-column'):
                xc = int(self.getInputFromPort('x-column'))
                if xc < 0:
                    yc = [0]
            if self.hasInputFromPort('y-columns'):
                yc = self.getInputFromPort('y-columns')
                yc = [int(x) for x in yc]
            
            label = ''
            if self.hasInputFromPort('label'):
                label = self.getInputFromPort('label')
            
            l = Loader()
            l.read_set(filename,label,xc,yc)
            self.setResult('data', l.sets)

class CustomLoader(Module):
    my_input_ports = [
        PortDescriptor("source",basic.String,use_python_source=True),
        PortDescriptor("base_path",basic.String)
    ]
    my_output_ports = [
        PortDescriptor("data",DataSets)
    ]
    
    def compute(self):
        if self.hasInputFromPort('source'):
            code = self.getInputFromPort('source')
            proc_code = urllib.unquote(str(code))
            exec proc_code

class LoadProperties(Module):
    my_input_ports = [
        PortDescriptor('ResultFiles', ResultFiles),
        PortDescriptor('PropertyPath',basic.String) 
    ]
    
    my_output_ports = [
        PortDescriptor('Props', ResultFiles)
    ]    
    
    def compute(self):
        propPath= self.getInputFromPort('PropertyPath') if self.hasInputFromPort('PropertyPath') else "/parameters"
        if self.hasInputFromPort('ResultFiles'):
            flist = [f.props["filename"] for f in self.getInputFromPort('ResultFiles')]
        loader = Hdf5Loader()
        dictlist = loader.GetProperties(flist)
        self.setResult('Props', dictlist)
    
class LoadAlpsHdf5(Module):
    my_input_ports = [
        PortDescriptor('ResultFiles',ResultFiles),
        PortDescriptor('Measurements',ListOfElements),
        PortDescriptor('StatisticalVariables',ListOfElements),
        PortDescriptor('PropertyPath',basic.String),
        PortDescriptor('ResultPath',basic.String)
    ]
    
    my_output_ports = [
        PortDescriptor('data',DataSets)
    ]    
    
    def compute(self):
        try:
            propPath= self.getInputFromPort('PropertyPath') if self.hasInputFromPort('PropertyPath') else "/parameters"
            resPath= self.getInputFromPort('ResultPath') if self.hasInputFromPort('ResultPath') else "/simulation/results"
            loader = Hdf5Loader()
            if self.hasInputFromPort('ResultFiles'):
                files = [f.props["filename"] for f in self.getInputFromPort('ResultFiles')]
            datasets = []
            statisticalvariables = []
            if self.hasInputFromPort('StatisticalVariables'):
                statisticalvariables = self.getInputFromPort('StatisticalVariables')    
            if self.hasInputFromPort('Measurements'):
                datasets = loader.ReadMeasurementFromFile(files,statisticalvariables,measurements=self.getInputFromPort('Measurements'),proppath=propPath,respath=resPath)
            else:
                datasets = loader.ReadMeasurementFromFile(files,statisticalvariables,measurements=None,proppath=propPath,respath=resPath)
            self.setResult('data',datasets)
        except Exception, (exc):
            from traceback import print_exc
            print_exc()
            raise exc

class LoadBinningAnalysis(Module):
    my_input_ports = [
        PortDescriptor('ResultFiles',ResultFiles),
        PortDescriptor('Measurements',ListOfElements),
        PortDescriptor('PropertyPath',basic.String),
        PortDescriptor('ResultPath',basic.String)
    ]
    
    my_output_ports = [
        PortDescriptor('data',DataSets)
    ]    
    
    def compute(self):
        propPath= self.getInputFromPort('PropertyPath') if self.hasInputFromPort('PropertyPath') else "/parameters"
        resPath= self.getInputFromPort('ResultPath') if self.hasInputFromPort('ResultPath') else None
        loader = Hdf5Loader()
        if self.hasInputFromPort('ResultFiles'):
            files = [f.props["filename"] for f in self.getInputFromPort('ResultFiles')]
        datasets = []
        if self.hasInputFromPort('Measurements'):
            datasets = loader.ReadBinningAnalysis(files,propPath,resPath,self.getInputFromPort('Measurements'))
        else:
            datasets = loader.ReadMeasurementFromFile(files,propPath,resPath)
        self.setResult('data',datasets)


class LoadSpectrumHdf5(Module):
    my_input_ports = [
        PortDescriptor('ResultFiles',ResultFiles),
        PortDescriptor('PropertyPath',basic.String),
        PortDescriptor('SpectrumPath',basic.String)
    ]
    
    my_output_ports = [
        PortDescriptor('data',DataSets)
    ]    
    
    def compute(self):
        propPath= self.getInputFromPort('PropertyPath') if self.hasInputFromPort('PropertyPath') else "/parameters"
        resPath= self.getInputFromPort('SpectrumPath') if self.hasInputFromPort('SpectrumPath') else "/spectrum"
        loader = Hdf5Loader()
        if self.hasInputFromPort('ResultFiles'):
            files = [f.props["filename"] for f in self.getInputFromPort('ResultFiles')]
        datasets = []
        datasets = loader.ReadSpectrumFromFile(files,propPath,resPath)
        self.setResult('data',datasets)
        
class LoadAlpsDiagData(Module):
    my_input_ports = [
        PortDescriptor('ResultFiles',ResultFiles),
        PortDescriptor('PropertyPath',basic.String),
        PortDescriptor('SpectrumPath',basic.String),
        PortDescriptor('Measurements',ListOfElements),
        PortDescriptor('index',basic.Integer)        
    ]
    
    my_output_ports = [
        PortDescriptor('data',DataSets)
    ]    
    
    def compute(self):
        propPath= self.getInputFromPort('PropertyPath') if self.hasInputFromPort('PropertyPath') else "/parameters"
        resPath= self.getInputFromPort('SpectrumPath') if self.hasInputFromPort('SpectrumPath') else "/spectrum"
        loader = Hdf5Loader()
        if self.hasInputFromPort('ResultFiles'):
            files = [f.props["filename"] for f in self.getInputFromPort('ResultFiles')]
        datasets = []
        if self.hasInputFromPort('Measurements') and self.hasInputFromPort('index'):
            datasets = loader.ReadDiagDataFromFile(files,propPath,resPath, self.getInputFromPort('Measurements'),self.getInputFromPort('index'))
        elif self.hasInputFromPort('Measurements'):
            datasets = loader.ReadDiagDataFromFile(files,propPath,resPath, self.getInputFromPort('Measurements'))
        elif self.hasInputFromPort('index'):
            datasets = loader.ReadDiagDataFromFile(files,propPath,resPath,None,self.getInputFromPort('index'))
        else:
            datasets = loader.ReadDiagDataFromFile(files,propPath,resPath)
            
        self.setResult('data',datasets)

class GroupBy(Module):
    my_input_ports = [
        PortDescriptor('for-each',ListOfElements),
        PortDescriptor('input',DataSets)
    ]
    my_output_ports = [
        PortDescriptor('output',DataSets)
    ]
    
    def compute(self):
        if self.hasInputFromPort('input'):
            groups = self.getInputFromPort('input')
            dd = depth(groups)
            
            if dd > 1:
                hgroups = flatten(groups, -1)
                hgroups_idcs = hgroups.indices()
            else:
                hgroups = [groups]
                hgroups_idcs = [0]
            
            for idx in hgroups_idcs:
                sets = hgroups[idx]
                
                for_each = []
                if self.hasInputFromPort('for-each'):
                    for_each = self.getInputFromPort('for-each')
                
                for_each_sets = {}
                for iset in sets:
                    fe_par_set = tuple((iset.props[m] for m in for_each))
                    
                    if fe_par_set in for_each_sets:
                        for_each_sets[fe_par_set].append(iset)
                    else:
                        for_each_sets[fe_par_set] = [iset]
                
                hgroups[idx] = for_each_sets.values()
            
            if dd > 1:
                self.setResult('output', groups)
            else:
                self.setResult('output', hgroups[0])
                
        else:
            raise EmptyInputPort('for-each || observable')

class CollectXY(Module):
    my_input_ports = [
        PortDescriptor('for-each',ListOfElements),
        PortDescriptor('y',basic.String),
        PortDescriptor('x',basic.String),
        PortDescriptor('input',DataSets)
    ]
    my_output_ports = [
        PortDescriptor('output',DataSets)
    ]
    
    def compute(self):
        if self.hasInputFromPort('y') \
        and self.hasInputFromPort('x') \
        and self.hasInputFromPort('input'):
            # find all possible values for each for-each
            sets = self.getInputFromPort('input')
            observable = self.getInputFromPort('y')
            versus = self.getInputFromPort('x')
            
            for_each = []
            if self.hasInputFromPort('for-each'):
                for_each = self.getInputFromPort('for-each')
            
            self.setResult('output',pyalps.collectXY(sets,versus,observable,for_each))
                
        else:
            raise EmptyInputPort('for-each || observable')
