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

#ifndef ALPS_NGS_MCOBSERVABLES_HPP
#define ALPS_NGS_MCOBSERVABLES_HPP

#include <alps/ngs/hdf5.hpp>
#include <alps/ngs/config.hpp>
#include <alps/ngs/mcobservable.hpp>

#include <alps/alea/observable_fwd.hpp>
#include <alps/ngs/alea/wrapper/accumulator_wrapper_fwd.hpp>

#include <map>
#include <string>

namespace alps {
    #ifdef ALPS_NGS_USE_NEW_ALEA
        namespace alea
        {
            class ALPS_DECL accumulator_set {
                
            public: 
                typedef std::map<std::string, boost::shared_ptr<detail::accumulator_wrapper> > map_type;
                //TODO
                typedef map_type::iterator iterator;
                typedef map_type::const_iterator const_iterator;

                detail::accumulator_wrapper & operator[](std::string const & name);

                detail::accumulator_wrapper const & operator[](std::string const & name) const;

                bool has(std::string const & name) const;
                
                void insert(std::string const & name, boost::shared_ptr<detail::accumulator_wrapper> ptr);

                void save(hdf5::archive & ar) const;

                void load(hdf5::archive & ar);

                void merge(accumulator_set const &);

                void output(std::ostream & os) const;
                
                iterator begin();
                iterator end();
                const_iterator begin() const;
                const_iterator end() const;
                
            private:
                std::map<std::string, boost::shared_ptr<detail::accumulator_wrapper> > storage;
            };
        } //end namespace alea
    #endif
    class ALPS_DECL mcobservables : public std::map<std::string, mcobservable> {

        public: 

            mcobservable & operator[](std::string const & name);

            mcobservable const & operator[](std::string const & name) const;

            bool has(std::string const & name) const;

            void insert(std::string const & name, mcobservable obs);

            void insert(std::string const & name, Observable const * obs);

            void reset(bool equilibrated = false);

            void save(hdf5::archive & ar) const;

            void load(hdf5::archive & ar);

            void merge(mcobservables const &);

            void output(std::ostream & os) const;

            // DO NOT USE! use set << ngs::RealObservable(name);
            // TODO: shold be solved using friends
            void create_RealObservable(std::string const & name, uint32_t binnum = 0);

            // DO NOT USE! use set << ngs::RealVectorObservable(name);
            // TODO: shold be solved using friends
            void create_RealVectorObservable(std::string const & name, uint32_t binnum = 0);

            // DO NOT USE! use set << ngs::SimpleRealObservable(name);
            // TODO: shold be solved using friends
            void create_SimpleRealObservable(std::string const & name);

            // DO NOT USE! use set << ngs::SimpleRealVectorObservable(name);
            // TODO: shold be solved using friends
            void create_SimpleRealVectorObservable(std::string const & name);

            // DO NOT USE! use set << ngs::SignedRealObservable(name, sign);
            // TODO: shold be solved using friends
            void create_SignedRealObservable(std::string const & name, std::string sign = "Sign", uint32_t binnum = 0);

            // DO NOT USE! use set << ngs::SignedRealVectorObservable(name, sign);
            // TODO: shold be solved using friends
            void create_SignedRealVectorObservable(std::string const & name, std::string sign = "Sign", uint32_t binnum = 0);

            // DO NOT USE! use set << ngs::SignedSimpleRealObservable(name, sign);
            // TODO: shold be solved using friends
            void create_SignedSimpleRealObservable(std::string const & name, std::string sign = "Sign");

            // DO NOT USE! use set << ngs::SignedSimpleRealVectorObservable(name, sign);
            // TODO: shold be solved using friends
            void create_SignedSimpleRealVectorObservable(std::string const & name, std::string sign = "Sign");

    };

    std::ostream & operator<<(std::ostream & os, mcobservables const & observables);
}

#endif
