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
 * @file   HexGrammar.h
 * @author Peter Schüller
 * @date   Wed Jul  8 14:00:48 CEST 2009
 * 
 * @brief  Grammar for parsing HEX using boost::spirit
 *
 * We code everything as intended by boost::spirit (use templates)
 * however we explicitly instantiate the template paramters in
 * a separate compilation unit HexGrammar.cpp to
 * 1) have faster compilation, and
 * 2) allow us to extend parsers by plugins from shared libraries
 *    (i.e., during runtime).
 */

#ifndef DLVHEX_HEX_PARSER_MODULE_H_INCLUDED
#define DLVHEX_HEX_PARSER_MODULE_H_INCLUDED

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/HexGrammar.h"

DLVHEX_NAMESPACE_BEGIN

class HexParserModule
{
public:
  enum Type
  {
    TOPLEVEL,
    BODYATOM,
    HEADATOM,
    TERM
  };

  virtual ~HexParserModule() {};

  Type getType() const { return type; }
  HexParserModule(Type type): type(type) {}

  virtual HexParserModuleGrammarPtr createGrammarModule() = 0;

protected:
  Type type;
};

DLVHEX_NAMESPACE_END

#endif // DLVHEX_HEX_PARSER_MODULE_H_INCLUDED

// Local Variables:
// mode: C++
// End:
