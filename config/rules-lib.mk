###########################################################################
# PALM++ library
#
# config/rules-lib.mk
#
# $Id$
#
# Copyright (C) 1994-2002 by Matthias Troyer <troyer@comp-phys.org>,
#                            Synge Todo <wistaria@comp-phys.org>,
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
###########################################################################

# library

$(LIB) : $(OBJ)
	$(ARXX) $(cxxflags) $(ldflags) -o $@ $(OBJ) $(libs)
