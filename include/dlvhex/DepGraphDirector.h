/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
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
 * @file GraphBuilder.h
 * @author Roman Schindlauer
 * @date Wed Jan 18 17:43:21 CET 2006
 *
 * @brief Abstract strategy class for finding the dependency edges of a program.
 *
 *
 */

#if !defined(_DLVHEX_DEPGRAPHDIRECTOR_H)
#define _DLVHEX_DEPGRAPHDIRECTOR_H

#include "dlvhex/PlatformDefinitions.h"

#include "dlvhex/DepGraphBuilder.h"
#include "dlvhex/BaseVisitor.h"
#include "dlvhex/Error.h"

DLVHEX_NAMESPACE_BEGIN

// forward declarations
class PluginContainer;


/**
 * @brief Class for building a dependency graph from a given program.
 *
 * Takes a set of rules and builds the according node graph.
 *
 * This nodegraph will contain the entire dependency graph of the program,
 * including any artificial nodes that had to be created for auxiliary
 * rules, e.g., for external atoms with variable input parameters.
 */
template<class DepGraph, class Vertex, class VProp, class EProp>
class DLVHEX_EXPORT DepGraphDirector : public BaseVisitor
{
 public:
  typedef DepGraphBuilder<DepGraph, Vertex, VProp, EProp> DGBuilder;

 protected:
  DGBuilder& builder;
  PluginContainer& container;

  void
  constructHead(const RuleHead_t&) throw (PluginError);

  void
  constructBody(const RuleBody_t&) throw (PluginError);


 public:
  DepGraphDirector(DGBuilder&, PluginContainer&);

  virtual void
  visit(Program* const);

  virtual void
  visit(AtomSet* const);

  virtual void
  visit(Rule* const);

  virtual void
  visit(WeakConstraint* const);

  virtual void
  visit(Literal* const);

  virtual void
  visit(Atom* const);

  virtual void
  visit(ExternalAtom* const);

  virtual void
  visit(BuiltinPredicate* const);

  virtual void
  visit(AggregateAtom* const);

};

DLVHEX_NAMESPACE_END

#endif /* _DLVHEX_DEPGRAPHDIRECTOR_H */

#include "dlvhex/DepGraphDirector.tcc"

// Local Variables:
// mode: C++
// End:
