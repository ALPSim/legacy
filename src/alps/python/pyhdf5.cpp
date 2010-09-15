/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 1994-2010 by Lukas Gamper <gamperl@gmail.com>,
*                            Matthias Troyer <troyer@itp.phys.ethz.ch>
*
* This software is part of the ALPS libraries, published under the ALPS
* Library License; you can use, redistribute it and/or modify it under
* the terms of the license, either version 1 or (at your option) any later
* version.
*
* You should have received a copy of the ALPS Library License along with
* the ALPS Libraries; see the file LICENSE.txt. If not, the license is also
* available from http://alps.comp-phys.org/.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

/* $Id: pyalea.cpp 3520 2010-04-09 16:49:53Z gamperl $ */

#define PY_ARRAY_UNIQUE_SYMBOL pyhdf5_PyArrayHandle
#define ALPS_HDF5_CLOSE_GREEDY

#include <alps/hdf5.hpp>
#include <alps/python/make_copy.hpp>
#include <alps/python/numpy_array.hpp>

#include <boost/python.hpp>

#include <numpy/arrayobject.h>

namespace alps { 
    namespace hdf5 {

        namespace detail {
            std::string extract_string(boost::python::object const & obj) {
                boost::python::extract<std::string> str(obj);
                if (!str.check()) {
                    PyErr_SetString(PyExc_RuntimeError, "Invalid path");
                    boost::python::throw_error_already_set();
                }
                return str();
            }
        }

        template<typename Archive> struct pyarchive {
            public:

                pyarchive(std::string const & filename): archive_(create_archive(filename)) {}

                virtual ~pyarchive() {
                    if (!--mem[filename_].second) {
                        delete mem[filename_].first;
                        mem.erase(filename_);
                    }
                }

                boost::python::object is_group(boost::python::object const & path) const {
                    return boost::python::object(archive_.is_group(detail::extract_string(path)));
                }

                boost::python::object is_data(boost::python::object const & path) const {
                    return boost::python::object(archive_.is_data(detail::extract_string(path)));
                }

                boost::python::object is_attribute(boost::python::object const & path) const {
                    return boost::python::object(archive_.is_attribute(detail::extract_string(path)));
                }

                boost::python::list extent(boost::python::object const & path) const {
                    return boost::python::list(archive_.extent(detail::extract_string(path)));
                }

                boost::python::object dimensions(boost::python::object const & path) const {
                    return boost::python::object(archive_.dimensions(detail::extract_string(path)));
                }

                boost::python::object is_scalar(boost::python::object const & path) const {
                    return boost::python::object(archive_.is_scalar(detail::extract_string(path)));
                }

                boost::python::object is_null(boost::python::object const & path) const {
                    return boost::python::object(archive_.is_null(detail::extract_string(path)));
                }

                boost::python::list list_children(boost::python::object const & path) const {
                    boost::python::list result;
                    std::vector<std::string> children = archive_.list_children(detail::extract_string(path));
                    for (std::vector<std::string>::const_iterator it = children.begin(); it != children.end(); ++it)
                        result.append(boost::python::str(*it));
                    return result;
                }

                boost::python::list list_attr(boost::python::object const & path) const {
                    boost::python::list result;
                    std::vector<std::string> children = archive_.list_attr(detail::extract_string(path));
                    for (std::vector<std::string>::const_iterator it = children.begin(); it != children.end(); ++it)
                        result.append(boost::python::str(*it));
                    return result;
                }

            protected:

                Archive & archive_;

            private:

                Archive & create_archive(std::string const & filename) {
                    filename_ = filename;
                    if (mem.find(filename_) == mem.end())
                        mem[filename_] = std::make_pair(new Archive(filename_), 1);
                    else
                        ++mem[filename_].second;
                    return *mem[filename_].first;
                }

                std::string filename_;
                static std::map<std::string, std::pair<Archive *, std::size_t> > mem;

        };

        template<typename Archive> std::map<std::string, std::pair<Archive *, std::size_t> > pyarchive<Archive>::mem;

        struct pyoarchive : public pyarchive<alps::hdf5::oarchive> {
            public:

                pyoarchive(std::string const & filename): pyarchive<alps::hdf5::oarchive>(filename) {}

                void write(boost::python::object path, boost::python::object const & data) {
                    alps::python::numpy::import();
                    std::size_t size = PyArray_Size(data.ptr());
                    double * data_ = static_cast<double *>(PyArray_DATA(data.ptr()));
                    using namespace alps;
                    archive_ << make_pvp(detail::extract_string(path), data_, size);
                }
        };

        struct pyiarchive : public pyarchive<alps::hdf5::iarchive> {
            public:

                pyiarchive(std::string const & filename): pyarchive<alps::hdf5::iarchive>(filename) {}

                boost::python::numeric::array read(boost::python::object const & path) {
                    alps::python::numpy::import();
                    std::vector<double> data;
                    archive_ >> make_pvp(detail::extract_string(path), data);
                    npy_intp size = data.size();
                    boost::python::object obj(boost::python::handle<>(PyArray_SimpleNew(1, &size, PyArray_DOUBLE)));
                    void * data_ = PyArray_DATA((PyArrayObject *)obj.ptr());
                    memcpy(data_, &data.front(), PyArray_ITEMSIZE((PyArrayObject *)obj.ptr()) * size);
                    return boost::python::extract<boost::python::numeric::array>(obj);
                }

        };
    }
}

BOOST_PYTHON_MODULE(pyhdf5_c) {

    boost::python::class_<alps::hdf5::pyoarchive>("oArchive", boost::python::init<std::string>())
        .def("__deepcopy__", &alps::python::make_copy<alps::hdf5::pyoarchive>)
        .def("is_group", &alps::hdf5::pyoarchive::is_group)
        .def("is_data", &alps::hdf5::pyoarchive::is_data)
        .def("is_attribute", &alps::hdf5::pyoarchive::extent)
        .def("extent", &alps::hdf5::pyoarchive::dimensions)
        .def("dimensions", &alps::hdf5::pyoarchive::dimensions)
        .def("is_scalar", &alps::hdf5::pyoarchive::is_scalar)
        .def("is_null", &alps::hdf5::pyoarchive::is_null)
        .def("list_children", &alps::hdf5::pyoarchive::list_children)
        .def("write", &alps::hdf5::pyoarchive::write)
    ;

    boost::python::class_<alps::hdf5::pyiarchive>("iArchive", boost::python::init<std::string>())
        .def("__deepcopy__", &alps::python::make_copy<alps::hdf5::pyiarchive>)
        .def("is_group", &alps::hdf5::pyiarchive::is_group)
        .def("is_data", &alps::hdf5::pyiarchive::is_data)
        .def("is_attribute", &alps::hdf5::pyiarchive::extent)
        .def("extent", &alps::hdf5::pyiarchive::dimensions)
        .def("dimensions", &alps::hdf5::pyiarchive::dimensions)
        .def("is_scalar", &alps::hdf5::pyiarchive::is_scalar)
        .def("is_null", &alps::hdf5::pyiarchive::is_null)
        .def("list_children", &alps::hdf5::pyiarchive::list_children)
        .def("read", &alps::hdf5::pyiarchive::read)
    ;

}
