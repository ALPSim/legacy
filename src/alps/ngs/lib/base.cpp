/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2010 - 2011 by Lukas Gamper <gamperl@gmail.com>                   *
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
#include <alps/ngs/base.hpp>

namespace alps {

    void base::save(alps::hdf5::archive & ar) const {
        ar
            << make_pvp("/parameters", params)
            << make_pvp("/simulation/realizations/0/clones/0/results", measurements)
        ;
    }

    void base::load(alps::hdf5::archive  & ar) {
        ar 
            >> make_pvp("/simulation/realizations/0/clones/0/results", measurements)
        ;
    }

    bool base::run(boost::function<bool ()> const & stop_callback) {
        do {
            do_update();
            do_measurements();
        } while(!complete_callback(stop_callback));
        return !stop_callback();
    }

    base::result_names_type base::result_names() const {
        result_names_type names;
        for(mcobservables::const_iterator it = measurements.begin(); it != measurements.end(); ++it)
            names.push_back(it->first);
        return names;
    }

    base::result_names_type base::unsaved_result_names() const {
        return result_names_type(); 
    }

    base::results_type base::collect_results() const {
        return collect_results(result_names());
    }

    base::results_type base::collect_results(result_names_type const & names) const {
        results_type partial_results;
        for(result_names_type::const_iterator it = names.begin(); it != names.end(); ++it)
            partial_results.insert(*it, mcresult(measurements[*it]));
        return partial_results;
    }

    bool base::complete_callback(boost::function<bool ()> const & stop_callback) {
        if (boost::posix_time::second_clock::local_time() > check_time) {
            fraction = fraction_completed();
            next_check = std::min(
                2. * next_check, 
                std::max(
                      double(next_check)
                    , 0.8 * (boost::posix_time::second_clock::local_time() - start_time).total_seconds() / fraction * (1 - fraction)
                )
            );
           check_time = boost::posix_time::second_clock::local_time() + boost::posix_time::seconds(next_check);
        }
        return (stop_callback() || fraction >= 1.);
    }

}
