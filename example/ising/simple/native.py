 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
 #                                                                                 #
 # ALPS Project: Algorithms and Libraries for Physics Simulations                  #
 #                                                                                 #
 # ALPS Libraries                                                                  #
 #                                                                                 #
 # Copyright (C) 2010 - 2011 by Lukas Gamper <gamperl@gmail.com>                   #
 #                                                                                 #
 # This software is part of the ALPS libraries, published under the ALPS           #
 # Library License; you can use, redistribute it and/or modify it under            #
 # the terms of the license, either version 1 or (at your option) any later        #
 # version.                                                                        #
 #                                                                                 #
 # You should have received a copy of the ALPS Library License along with          #
 # the ALPS Libraries; see the file LICENSE.txt. If not, the license is also       #
 # available from http://alps.comp-phys.org/.                                      #
 #                                                                                 #
 #  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     #
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        #
 # FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT       #
 # SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE       #
 # FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,     #
 # ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER     #
 # DEALINGS IN THE SOFTWARE.                                                       #
 #                                                                                 #
 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

import pyalps.ngs as ngs
import numpy as np
import sys, time

class isingSim:
    def __init__(self, params):
        self.random = lambda: 0.4 # TODO: implement: random(boost::mt19937((params['SEED'] | 42)), boost::uniform_real<>())
        self.parameters = params
        self.measurements = {
            'Energy':  ngs.createRealObservable('Energy'),
            'Magnetization': ngs.createRealObservable('Magnetization'),
            'Magnetization^2': ngs.createRealObservable('Magnetization^2'),
            'Magnetization^4': ngs.createRealObservable('Magnetization^4'),
            'Correlations': ngs.createRealVectorObservable('Correlations')
        }

        self.length = int(params['L']);
        self.sweeps = 0
        self.thermalization_sweeps = long(params['THERMALIZATION'])
        self.total_sweeps = long(params['SWEEPS'])
        self.beta = 1. / float(params['T'])
        self.spins = np.array([(-x if self.random() < 0.5 else x) for x in np.ones(self.length)])
        
        self.realization = '0'
        self.clone = '0'

    def update(self):
        for j in range(self.length):
            i = int(float(self.length) * self.random());
            right = i + 1 if i + 1 < self.length else 0
            left = self.length - 1 if i - 1 < 0 else i - 1
            p = np.exp(2. * self.beta * self.spins[i] * (self.spins[right] + self.spins[left]))
            if p >= 1. or self.random() < p:
                self.spins[i] =- self.spins[i]

    def measure(self):
        self.sweeps += 1
        if self.sweeps > self.thermalization_sweeps:
            tmag = 0
            ten = 0
            sign = 1
            corr = np.zeros(self.length)
            for i in range(self.length):
                tmag += self.spins[i]
                sign *= self.spins[i]
                ten += -self.spins[i] * self.spins[i + 1 if i + 1 < self.length else 0]
            for d in range(self.length):
                corr[d] = np.inner(self.spins, np.roll(self.spins, d)) / float(self.length)
            ten /= self.length
            tmag /= self.length
            self.measurements['Energy'] << ten
            self.measurements['Magnetization'] << tmag
            self.measurements['Magnetization^2'] << tmag**2
            self.measurements['Magnetization^4'] << tmag**4
            self.measurements['Correlations'] << corr

    def fraction_completed(self):
        return 0 if self.sweeps < self.thermalization_sweeps else (self.sweeps - self.thermalization_sweeps) / float(self.total_sweeps)

# TODO: implement
#    void save(boost::filesystem::path const & filename) const {
#        alps::hdf5::archive ar(filename, "w");
#        ar << *this;
#    }

# TODO: implement
#    void load(boost::filesystem::path const & filename) {
#        alps::hdf5::archive ar(filename);
#        ar >> *this;
#    }

    def run(self, stopCallback):
        stopped = False
        while True:
            self.update()
            self.measure()
            stopped = stopCallback()
            if (stopped or self.fraction_completed() >= 1.):
                return not stopped

    def result_names(self):
        return self.measurements.keys()

# TODO: implement
#    result_names_type unsaved_result_names() const {
#        return result_names_type(); 
#    }

    def collectResults(self, names = None):
        if names == None:
            names = self.result_names()
        partial_results = {}
        for name in names:
            partial_results[name] = ngs.observable2result(self.measurements[name])
        return partial_results

# TODO: implement
#    void save(alps::hdf5::archive & ar) const {
#        ar["/parameters"] << params;
#        std::string context = ar.get_context();
#        ar.set_context("/simulation/realizations/" + realization + "/clones/" + clone);
#
#        ar["length"] << length; // TODO: where to put the checkpoint informations?
#        ar["sweeps"] << sweeps;
#        ar["thermalization_sweeps"] << thermalization_sweeps;
#        ar["beta"] << beta;
#        ar["spins"] << spins;
#        ar["measurements"] << measurements;
#
#        {
#            std::ostringstream os;
#            os << random.engine();
#            ar["engine"] << os.str();
#        }
#
#        ar.set_context(context);
#    }

# TODO: implement
#    void load(alps::hdf5::archive & ar) {
#        ar["/parameters"] >> params; // TODO: do we want to load the parameters?
#
#        std::string context = ar.get_context();
#        ar.set_context("/simulation/realizations/" + realization + "/clones/" + clone);
#
#        ar["length"] >> length;
#        ar["sweeps"] >> sweeps;
#        ar["thermalization_sweeps"] >> thermalization_sweeps;
#        ar["beta"] >> beta;
#        ar["spins"] >> spins;
#        ar["measurements"] >> measurements;
#
#        {
#            std::string state;
#            ar["engine"] >> state;
#            std::istringstream is(state);
#            is >> random.engine();
#        }
#
#        ar.set_context(context);
#    }

#TODO: implement nice argv parsing ...
def main(limit, resume, output):

    sim = isingSim(ngs.params({
        'L': 100,
        'THERMALIZATION': 1000,
        'SWEEPS': 10000,
        'T': 2
    }))

#    if resume == 't':
#        try:
#            sim.load(output[0:output.rfind('.h5')] + 'clone0.h5')
#        except ArchiveNotFound: pass

    if limit == 0:
        sim.run()
    else:
        start = time.time()
        sim.run(lambda: time.time() > start + float(limit))

#    if resume == 't':
#        ngs.archive(output[0:output.rfind('.h5')] + 'clone0.h5', 'w')

    results = sim.collectResults() # TODO: how should we do that?
    for key, value in results.iteritems():
        print "{}: {}".format(key, value)

    ar = ngs.archive(output, 'w')
    ar['/parameters'] = sim.parameters;
    ar['/simulation/results'] = results;

if __name__ == '__main__':
    apply(main, sys.argv[1:])
