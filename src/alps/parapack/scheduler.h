/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 1997-2008 by Synge Todo <wistaria@comp-phys.org>
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

#ifndef PARAPACK_SCHEDULER_H
#define PARAPACK_SCHEDULER_H

#include "worker_factory.h"
#include "option.h"
#include "job.h"
#include <iostream>

namespace alps {

/// return the compile date of ALPS/parapack
std::string compile_date();

namespace parapack {

int start(int argc, char **argv);

int evaluate(int argc, char **argv);

int run_sequential(int argc, char **argv);

namespace scheduler {

int start(int argc, char **argv);

void print_copyright(std::ostream& os = std::cout);

void print_license(std::ostream& os = std::cout);

std::string alps_version();

std::string log_header();

std::string clone_name(alps::tid_t tid, alps::cid_t cid);

std::string pg_name(alps::gid_t gid);

void print_taskinfo(std::ostream& os, std::vector<alps::task> const& tasks);

// return 1 for job XML (<JOB>) file or 2 for task XML (<SIMULATION>)
int load_filename(boost::filesystem::path const& file, std::string& file_in_str,
  std::string& file_out_str);

void load_version(boost::filesystem::path const& file,
  std::vector<std::pair<std::string, std::string> >& versions);

void load_tasks(boost::filesystem::path const& file_in,
  boost::filesystem::path const& file_out, boost::filesystem::path const& basedir,
  bool check_parameter, std::string& simname, std::vector<alps::task>& tasks);

void save_tasks(boost::filesystem::path const& file, std::string const& simname,
  std::string const& file_in_str, std::string const& file_out_str, std::vector<alps::task>& tasks);

} // namespace scheduler
} // namespace parapack
} // namespace alps

#endif // PARAPACK_SCHEDULER_H
