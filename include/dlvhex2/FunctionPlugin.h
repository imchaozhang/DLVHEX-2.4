/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Thomas Krennwallner
 * Copyright (C) 2009, 2010, 2011 Peter Schüller
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
 * @file FunctionPlugin.h
 * @author Christoph Redl
 *
 * @brief Support for function symbol handling via external atoms
 */

#ifndef FUNCTION_PLUGIN__HPP_INCLUDED_
#define FUNCTION_PLUGIN__HPP_INCLUDED_

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/PluginInterface.h"
#include <set>

DLVHEX_NAMESPACE_BEGIN

class FunctionPlugin:
  public PluginInterface
{
public:
  // stored in ProgramCtx, accessed using getPluginData<ExistsPlugin>()
  class CtxData:
    public PluginData
  {
  public:
    // maximal input arity
    int maxArity;

    // rewrite function symbols to external atoms
    bool rewrite;

    CtxData();
    virtual ~CtxData() {};
  };

public:
  FunctionPlugin();
  virtual ~FunctionPlugin();

	// output help message for this plugin
	virtual void printUsage(std::ostream& o) const;

  // accepted options: --exists-enable  --exists-maxarity=<N>
  //
	// processes options for this plugin, and removes recognized options from pluginOptions
  // (do not free the pointers, the const char* directly come from argv)
	virtual void processOptions(std::list<const char*>& pluginOptions, ProgramCtx&);

  // rewrite program by adding auxiliary constraints
  virtual PluginRewriterPtr createRewriter(ProgramCtx&);

  // plugin atoms
  virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx& ctx) const;

  // change model callback (print auxiliaries as negative atoms)
  virtual void setupProgramCtx(ProgramCtx&);

  // no atoms!
};

DLVHEX_NAMESPACE_END

#endif
