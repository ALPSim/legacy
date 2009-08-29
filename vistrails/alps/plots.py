# VisTrails package for ALPS, Algorithms and Libraries for Physics Simulations
#
# Get ALPS at http://alps.comp-phys.org/
#
##############################################################################

from core.modules.vistrails_module import Module, ModuleError, NotCacheable
import core.bundles
import core.modules.basic_modules
import core.modules.module_registry
import tempfile

import alpscore
from packages.pylab.plot import MplPlot, MplPlotConfigurationWidget

from packages.controlflow.list_module import ListOfElements

basic = core.modules.basic_modules

##############################################################################

class DisplayXMGRPlot(NotCacheable, alpscore.SystemCommand):
    """ open the file using  xmgr command """
    def compute(self):
        self.execute(['nohup','/sw/bin/xmgrace', self.getInputFromPort('file').name,'&'])
    _input_ports = [('file', [basic.File])]

class PlotDescription(basic.File):
    """ a plot desription file """

class PlotDescriptionXML(Module):
    def compute(self):
        self = self.interpreter.filePool.create_file(suffix='.txt')
        f = file(self.name,'w')
        f.write(str(self.getInputFromPort('xml')))
        f.close()
        self.setResult('value',self)
    _input_ports = [('xml', [basic.String])]
    _output_ports = [('value',PlotDescription)]

class PlotScalarVersusParameter(PlotDescription):
    def compute(self):
        self.name = self.interpreter.filePool.create_file(suffix='.xsl').name
        f = file(self.name,'w')
        f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        if (self.hasInputFromPort('title')):
          f.write('<plot name="'+str(self.getInputFromPort('title')) + '">\n')
        else:
          f.write('<plot>\n')
        if (self.hasInputFromPort('x-label')):
          xlabel = self.getInputFromPort('x-label')
        else:
          xlabel = self.getInputFromPort('parameter')
        if (self.hasInputFromPort('y-label')):
          ylabel = self.getInputFromPort('y-label')
        else:
          ylabel = self.getInputFromPort('observable')
        f.write('  <xaxis label="' + str(xlabel) + '" type="PARAMETER" name="' + str(self.getInputFromPort('parameter')) + '"/>\n')
        f.write('  <yaxis label="' + str(ylabel) + '" type="SCALAR_AVERAGE" name="' + str(self.getInputFromPort('observable')) + '"/>\n')
        f.write('</plot>\n')
        f.close()
        self.setResult('value',self)
    _input_ports = [('parameter', [basic.String]),
                    ('observable', [basic.String]),
                    ('x-label', [basic.String]),
                    ('y-label', [basic.String]),
                    ('title',[basic.String])]
    _output_ports = [('value',PlotDescription)]

class PlotFile(basic.File):
   """ a plot in XML """

#class MakePlot(PlotFile,SystemCommand):
#    _input_ports = [('datafiles',[basic.File]),
#                    ('plotdescription',[PlotDescription])]

class BasicExtract(alpscore.SystemCommand):
    def compute(self):
        outputfile = self.interpreter.filePool.create_file(suffix=self.suffix)
        cmdlist = [alpscore._get_path(self.extractapp), 
                   self.getInputFromPort('plotdescription').name ]
        cmdlist += self.getfiles()
        cmdlist += [ '>', outputfile.name]
        print cmdlist
        self.execute(cmdlist)
        self.setResult('file',outputfile)
        f = file(outputfile.name,'r')
        self.setResult('source',f.read())
    _input_ports = [('data',[ListOfElements]),
                    ('plotdescription',[PlotDescription])]
    _output_ports = [('file',[basic.File]),
                     ('source',[basic.String])]

class GetSingleFile:
   def getfiles(self):
       return self.getInputFromPort('data').name

class GetFileList:
   def getfiles(self):
       return self.getInputFromPort('data')
       
       
class ExtractXMGR(BasicExtract,GetFileList):
    suffix='xmgr'
    extractapp='extractxmgr'

class ExtractText(BasicExtract,GetFileList):
    suffix='txt'
    extractapp='extracttext'

class ExtractMpl(BasicExtract,GetFileList):
    suffix='py'
    extractapp='extractmpl'
    
    
class Plot2XMGR(BasicExtract,GetSingleFile):
    suffix='xmgr'
    extractapp='plot2xmgr'

class Plot2Text(BasicExtract,GetSingleFile):
    suffix='txt'
    extractapp='plot2text'

class Plot2Mpl(BasicExtract,GetSingleFile):
    suffix='py'
    extractapp='plot2mpl'

class AlpsMplPlot(MplPlot):
    _input_ports=[('source', basic.String)]
  
def initialize(): pass

def selfRegister():

  reg = core.modules.module_registry.get_module_registry()

  reg.add_module(PlotDescription,namespace="Plots")
  reg.add_output_port(PlotDescription, "value", PlotDescription)
  reg.add_output_port(PlotDescription, "self", PlotDescription, True)
  reg.add_module(PlotDescriptionXML,namespace="Plots")
  reg.add_module(PlotScalarVersusParameter,namespace="Plots")
  reg.add_module(PlotFile,namespace="Plots")

  reg.add_module(BasicExtract,namespace="Plots",abstract=True)
  reg.add_module(ExtractXMGR,namespace="Plots")
  reg.add_module(ExtractText,namespace="Plots")
  reg.add_module(ExtractMpl,namespace="Plots")
  reg.add_module(Plot2XMGR,namespace="Plots")
  reg.add_module(Plot2Text,namespace="Plots")
  reg.add_module(Plot2Mpl,namespace="Plots")
  
  reg.add_module(DisplayXMGRPlot,namespace="Plots")
  reg.add_module(AlpsMplPlot,namespace="Plots")
  
 # reg.add_module(MakePlot)
