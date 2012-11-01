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

#include "src/ising.hpp"

#include <alps/ngs/scheduler/proto/mpisim.hpp>

#include <boost/lexical_cast.hpp>

using namespace alps;

typedef mpisim_ng<ising_sim> sim_type;

int main(int argc, char *argv[]) {

    mcoptions options(argc, argv);
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator c;

    parameters_type<sim_type>::type params;
    if (c.rank() == 0)
        hdf5::archive(options.input_file)["/parameters"] >> params;
    broadcast(c, params);

    sim_type sim(params, c);

    if (options.resume)
        sim.load(params["DUMP"] | "checkpoint");

    sim.run(boost::bind(&stop_callback, options.time_limit));

    sim.save(params["DUMP"] | "checkpoint");

    results_type<sim_type>::type results = collect_results(sim);

    if (c.rank() == 0) {
        std::cout << results << std::endl;
        save_results(results, params, options.output_file, "/simulation/results");
    }
}
