/*****************************************************************************
*
* ALPS Project Applications
*
* Copyright (C) 2005 by Matthias Troyer <troyer@comp-phys.org>,
*                       Andreas Streich <astreich@student.ethz.ch>
*
* This software is part of the ALPS Applications, published under the ALPS
* Application License; you can use, redistribute it and/or modify it under
* the terms of the license, either version 1 or (at your option) any later
* version.
* 
* You should have received a copy of the ALPS Application License along with
* the ALPS Applications; see the file LICENSE.txt. If not, the license is also
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

/* $Id$ */

#include <stdexcept>
#include "factory.h"
#include "fitting_scheduler.h"
#include "alps/osiris.h"

/**
 * Main function to be called to start the fitting process.
 * Starting from a parameter file, the scheduler is started.
 */
int main(int argc, char** argv)
{
  #ifndef BOOST_NO_EXCEPTIONS
    try {
  #endif
     // choose the right factory, depending on the input!
     return start_fitting(argc,argv,SpinFactory());
  #ifndef BOOST_NO_EXCEPTIONS
    }
    catch (std::exception& exc) {
      std::cerr << exc.what() << "\n";
      alps::comm_exit(true);
      return -1;
    }
    catch (...) {
      std::cerr << "Fatal Error: Unknown Exception!\n";
      return -2;
    }
  #endif
}
