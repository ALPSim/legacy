/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2010 - 2012 by Lukas Gamper <gamperl@gmail.com>                   *
 *                                                                                 *
 * This software is part of the ALPS libraries, published under the ALPS           *
 * Library License; you can use, redistribute it and/or modify it under            *
 * the terms of the license, either version 1 or (at your option) any later        *
 * version.                                                                        *
 *                                                                                 *
 * You should have received a copy of the ALPS Library License along with          *
 * the ALPS Libraries; see the file LICENSE.txt. If not, the license is also       *
 * available from http://alps.comp-phys.org/.                                      *
 *                                                                                 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT       *
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE       *
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,     *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER     *
 * DEALINGS IN THE SOFTWARE.                                                       *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <alps/ngs/api.hpp>
#include <alps/ngs/hdf5.hpp>
#include <alps/ngs/config.hpp>
#include <alps/ngs/params.hpp>
#include <alps/ngs/hdf5/map.hpp>
#include <alps/ngs/boost_mpi.hpp>
#include <alps/ngs/hdf5/pair.hpp>
#include <alps/ngs/hdf5/vector.hpp>
#include <alps/ngs/hdf5/multi_array.hpp>
#include <alps/ngs/alea/accumulator_set.hpp>
#include <alps/ngs/scheduler/check_schedule.hpp>

#include <alps/random/mersenne_twister.hpp>

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#ifndef ALPS_NGS_USE_NEW_ALEA
#error "this example only works with the new alea"
#endif

class multi_array_sim_mpi {

    public:
        
        typedef alps::params parameters_type;
        typedef std::map<std::string, std::pair<boost::uint64_t, alps::multi_array<double, 2> > > results_type;
        typedef std::vector<std::string> result_names_type;

        multi_array_sim_mpi(parameters_type const & parameters, boost::mpi::communicator const & comm, double t_min = 1, double t_max = 600)
            : communicator(comm)
            , params(parameters)
            , random(boost::mt19937((parameters["SEED"] | 42) + comm.rank()), boost::uniform_real<>())
            , schedule(t_min, t_max)
            , clone(comm.rank())
            , steps(0)
        {
            using namespace alps::accumulator;
            measurements << make_accumulator(
                  "M"
                , detail::accumulator_wrapper(accumulator<alps::multi_array<double, 2>, features<tag::mean> >())
            );
        }

        void update() {};

        void measure() {
            static const unsigned size = 100;
            ++steps;
            alps::multi_array<double, 2> values(size, size);
            std::generate(values.data(), values.data() + size * size, random);
            measurements["M"] << values;
        };

        double fraction_completed() const {
            return fraction;
        }

        void save(boost::filesystem::path const & filename) const {
            alps::hdf5::archive ar(filename, "w");
            ar << *this;
        }

        void load(boost::filesystem::path const & filename) {
            alps::hdf5::archive ar(filename);
            ar >> *this;
        }

        bool run(boost::function<bool ()> const & stop_callback) {
            bool done = false, stopped = false;
            do {
                update();
                measure();
                if ((stopped = stop_callback()) || schedule.pending()) {
                    static const double total = 100.;
                    double local_fraction = (stopped
                        ? 1.
                        : steps / total
                    );
                    schedule.update(fraction = boost::mpi::all_reduce(communicator, local_fraction, std::plus<double>()));
                    done = fraction >= 1.;
                }
            } while(!done);
            return !stopped;
        }
        
        result_names_type result_names() const {
            result_names_type names;
            for(alps::accumulator::accumulator_set::const_iterator it = measurements.begin(); it != measurements.end(); ++it)
                names.push_back(it->first);
            return names;
        }

        result_names_type unsaved_result_names() const {
            return result_names_type(); 
        }

        results_type collect_results() const {
            return collect_results(result_names());
        }

        results_type collect_results(result_names_type const & names) const {
            results_type partial_results;
            for(result_names_type::const_iterator it = names.begin(); it != names.end(); ++it) {
                alps::multi_array<double, 2> sum = measurements[*it].get<alps::multi_array<double, 2> >().mean();
                sum *= measurements[*it].count();
                if (communicator.rank() == 0) {
                    boost::mpi::reduce(communicator, measurements[*it].count(), partial_results[*it].first, std::plus<boost::uint64_t>(), 0);
                    partial_results[*it].second = sum;
                    boost::mpi::reduce(communicator, sum.data(), sum.num_elements(), partial_results[*it].second.data(), std::plus<double>(), 0);
                    partial_results[*it].second /= partial_results[*it].first;
                } else {
                    boost::mpi::reduce(communicator, measurements[*it].count(), std::plus<boost::uint64_t>(), 0);
                    boost::mpi::reduce(communicator, sum.data(), sum.num_elements(), std::plus<double>(), 0);
                }
            }
            return partial_results;
        }

        void save(alps::hdf5::archive & ar) const {
            ar["/parameters"] << params;

            std::string context = ar.get_context();
            ar.set_context("/simulation/realizations/0/clones/0");
            ar["measurements"] << measurements;

            ar.set_context("checkpoint");
            ar["steps"] << steps;

            {
                std::ostringstream os;
                os << random.engine();
                ar["engine"] << os.str();
            }

            ar.set_context(context);
        }

        void load(alps::hdf5::archive & ar) {
            ar["/parameters"] >> params;

            std::string context = ar.get_context();
            ar.set_context("/simulation/realizations/0/clones/0");
            ar["measurements"] >> measurements;

            ar.set_context("checkpoint");
            ar["steps"] >> steps;

            {
                std::string state;
                ar["engine"] >> state;
                std::istringstream is(state);
                is >> random.engine();
            }

            ar.set_context(context);
        }

    private:
    
        boost::mpi::communicator communicator;

        parameters_type params;
        boost::variate_generator<boost::mt19937, boost::uniform_real<> > mutable random;
        alps::accumulator::accumulator_set measurements;

        alps::check_schedule schedule;
        double fraction;
        int clone;

        int steps;
};

bool stop_callback() {
    return false;
}

int main(int argc, char *argv[]) {

    try {
        boost::mpi::environment env(argc, argv);
        boost::mpi::communicator comm;

        alps::parameters_type<multi_array_sim_mpi>::type parameters;
        parameters["SEED"] = 66;

        multi_array_sim_mpi sim(parameters, comm);

        sim.run(&stop_callback);
        
        sim.save("checkpoint" + boost::lexical_cast<std::string>(comm.rank()) + ".h5");
        
        using alps::collect_results;
        alps::results_type<multi_array_sim_mpi>::type results = collect_results(sim);

        if (comm.rank() == 0) {
            alps::hdf5::archive ar("result.h5", "w");
            ar["/parameters"] << parameters;
            ar["/simulation/results"] << results;
        }

    } catch (std::exception const & e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Caught unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
