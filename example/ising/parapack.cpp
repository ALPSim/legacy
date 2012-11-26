/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                                 *
* ALPS Project: Algorithms and Libraries for Physics Simulations                  *
*                                                                                 *
* ALPS Libraries                                                                  *
*                                                                                 *
* Copyright (C) 1997-2012 by Synge Todo <wistaria@comp-phys.org>,
*                            Ryo Igarashi <rigarash@issp.u-tokyo.ac.jp>,
*                            Haruhiko Matsuo <halm@rist.or.jp>,
*                            Tatsuya Sakashita <t-sakashita@issp.u-tokyo.ac.jp>,
*                            Yuichi Motoyama <yomichi@looper.t.u-tokyo.ac.jp>
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
#include <alps/ngs/parapack/parapack.h>

int main(int argc, char *argv[]) {
  return alps::ngs_parapack::start<ising_sim>(argc, argv);
  // return alps::ngs_parapack::start<single_worker, parallel_worker>(argc, argv);
  // return alps::ngs_parapack::start<single_worker, alps::no_worker>(argc, argv);
  // return alps::ngs_parapack::start<alps::no_worker, parallel_worker>(argc, argv);
}
