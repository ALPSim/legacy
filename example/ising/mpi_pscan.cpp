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

#include <boost/lexical_cast.hpp>

#include <alps/ngs/scheduler/proto/mpisim.hpp>

using namespace alps;

typedef mpisim_ng<ising_sim> sim_type;

int main(int argc, char *argv[]) {

    mcoptions options(argc, argv);
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator comm_world;

    std::size_t color = comm_world.rank() % 2;

    boost::mpi::communicator comm_local = comm_world.split(color);

    parameters_type<sim_type>::type params;
    if (comm_local.rank() == 0)
        hdf5::archive(options.input_file)["/parameters"] >> params;
    
    
    if (!comm_local.rank()) {
        hdf5::archive ar(options.input_file + "." + boost::lexical_cast<std::string>(color));
        ar["/parameters"] >> params;
    }
    broadcast(comm_local, params);
    
    std::cout << "color: " << color << ", "
              << "global rank: " << boost::lexical_cast<std::string>(comm_world.rank()) << ", "
              << "local rank: " << boost::lexical_cast<std::string>(comm_local.rank()) << ", "
              << "L: " << params["L"] << std::endl;

    sim_type sim(params, comm_local);

    if (options.resume)
        sim.load(
              (params["DUMP"] | "checkpoint")
            + "." + boost::lexical_cast<std::string>(color)
        );

    sim.run(boost::bind(&stop_callback, options.time_limit));

    sim.save(
          (params["DUMP"] | "checkpoint")
        + "." + boost::lexical_cast<std::string>(color)
    );

    results_type<sim_type>::type results = collect_results(sim);

    if (!comm_local.rank()) {
        std::cout << results << std::endl;
        save_results(results, params, options.output_file + "." + boost::lexical_cast<std::string>(color), "/simulation/results");
    }
}
