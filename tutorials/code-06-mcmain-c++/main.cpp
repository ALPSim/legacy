/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2010 - 2013 by Lukas Gamper <gamperl@gmail.com>                   *
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

#include "ising.hpp"

#include <alps/ngs.hpp>

#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <iostream>
#include <stdexcept>

// function object to determine when to stop the simulation
struct stop_callback {
    stop_callback(std::size_t timelimit)
        : limit(timelimit)
        , start(boost::chrono::high_resolution_clock::now())
    {}

    // stop the simulation eighter if a signal was sent to the program or if the timlimit was reached
    bool operator()() {
        return !signals.empty() 
            || (limit.count() > 0 && boost::chrono::high_resolution_clock::now() > start + limit);
    }

    boost::chrono::duration<std::size_t> limit;
    alps::ngs::signal signals;
    boost::chrono::high_resolution_clock::time_point start;
};

int main(int argc, char *argv[]) {

    // this tutorial expect exactly 3 arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " timelimit parameter-file" << std::endl;
        return EXIT_FAILURE;
    }

    try {

        // extract parameters from argv
        std::size_t timelimit = boost::lexical_cast<std::size_t>(argv[1]);
        boost::filesystem::path input_file = argv[2];
        std::string basename = std::string(argv[2]).substr(0, std::string(argv[2]).find_last_of('.'));

        // create path for ouput and checkpoint files from the paramter file
        boost::filesystem::path checkpoint_file = basename + ".clone0.h5";
        boost::filesystem::path output_file = basename +  ".out.h5";

        // parse parameter file
        alps::parameters_type<ising_sim>::type parameters(input_file);

        // create simulation object
        ising_sim sim(parameters);

        // if checkpoint file exists resume, else start from scratch
        if (boost::filesystem::exists(checkpoint_file))
            sim.load(checkpoint_file);

        // run simulation
        sim.run(stop_callback(timelimit));

        // save checkpoint
        sim.save(checkpoint_file);

        // convert measurements to results
        using alps::collect_results;
        alps::results_type<ising_sim>::type results = collect_results(sim);

        // output results and save results to output_file
        std::cout << results << std::endl;
        alps::hdf5::archive ar(output_file, "w");
        ar["/parameters"] << parameters;
        ar["/simulation/results"] << results;

    } catch (std::exception const & e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
