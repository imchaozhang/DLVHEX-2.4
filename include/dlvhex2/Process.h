/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
 * Copyright (C) 2011, 2012, 2013 Christoph Redl
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/**
 * @file   Process.h
 * @author Thomas Krennwallner
 * @date   
 * 
 * @brief  
 * 
 * 
 */


#if !defined(_DLVHEX_PROCESS_H)
#define _DLVHEX_PROCESS_H

#include "dlvhex2/PlatformDefinitions.h"

#include <string>
#include <vector>

DLVHEX_NAMESPACE_BEGIN


/**
 * @brief Base class for solver processes
 */
class DLVHEX_EXPORT Process
{
public:
  virtual
  ~Process() { }

  virtual void
  addOption(const std::string&) = 0;

  virtual std::string
  path() const = 0;

  virtual std::vector<std::string>
  commandline() const = 0;

  virtual void
  spawn() = 0;

  virtual void
  spawn(const std::vector<std::string>&) = 0;

  virtual void
  endoffile() = 0;

  virtual int
  close(bool kill=false) = 0;

  virtual std::ostream&
  getOutput() = 0;

  virtual std::istream&
  getInput() = 0;
};

DLVHEX_NAMESPACE_END

#endif // _DLVHEX_PROCESS_H


// Local Variables:
// mode: C++
// End:
