/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
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
 * @file ProgramCtx.cpp
 * @author Thomas Krennwallner
 * @date 
 *
 * @brief Program context.
 *
 *
 *
 */


#include "dlvhex/ProgramCtx.h"

#include "dlvhex/PluginContainer.h"

#if 0
#include "dlvhex/Program.h"
#include "dlvhex/AtomSet.h"
#endif

//#include "dlvhex/NodeGraph.h"
//#include "dlvhex/DependencyGraph.h"
//#include "dlvhex/ResultContainer.h"
//#include "dlvhex/OutputBuilder.h"
#include "dlvhex/State.h"
#include "dlvhex/DLVProcess.h"

#include <boost/shared_ptr.hpp>

#include <sstream>
#include <iostream>

DLVHEX_NAMESPACE_BEGIN

	
ProgramCtx::ProgramCtx():
		maxint(0)
{
}


ProgramCtx::~ProgramCtx()
{
  DBGLOG(DBG,"resetting state");
  state.reset();

  DBGLOG(DBG,"resetting callbacks");
  modelCallbacks.clear();
  finalCallbacks.clear();

  DBGLOG(DBG,"resetting modelBuilder");
  modelBuilder.reset();

  DBGLOG(DBG,"resetting parser");
  parser.reset();

  DBGLOG(DBG,"resetting evalgraph");
  evalgraph.reset();

  DBGLOG(DBG,"resetting compgraph");
  compgraph.reset();

  DBGLOG(DBG,"resetting depgraph");
  depgraph.reset();

  DBGLOG(DBG,"resetting edb");
  edb.reset();

  DBGLOG(DBG,"resetting inputProvider");
  inputProvider.reset();

  DBGLOG(DBG,"resetting aspsoftware");
  aspsoftware.reset();

  DBGLOG(DBG,"resetting registry, usage count was " << _registry.use_count() << " (it should be 2)");
  _registry.reset();
  DBGLOG(DBG,"resetting pluginContainer, usage count was " << _pluginContainer.use_count() << " (it should be 1)");
  _pluginContainer.reset();
}
  

void
ProgramCtx::changeState(const boost::shared_ptr<State>& s)
{
  state = s;
}

// must be setup together
// pluginContainer must be associated to registry
void ProgramCtx::setupRegistryPluginContainer(
    RegistryPtr registry, PluginContainerPtr pluginContainer)
{
  assert(!pluginContainer ||
      (pluginContainer->getRegistry() == registry &&
      "PluginContainer in ProgramCtx must be associated to registry of programCtx"));
  _registry = registry;
  _pluginContainer = pluginContainer;
}

ASPSolverManager::SoftwareConfigurationPtr
ProgramCtx::getASPSoftware() const
{
  assert(aspsoftware != 0);
  return aspsoftware;
}

void ProgramCtx::setASPSoftware(ASPSolverManager::SoftwareConfigurationPtr software)
{
  aspsoftware = software;
}

#if 0

void
ProgramCtx::setPluginContainer(PluginContainer* c)
{
  if (this->container != c)
    {
      this->container = c;
    }
}


PluginContainer*
ProgramCtx::getPluginContainer() const
{
  return this->container;
} 


std::vector<PluginInterface*>*
ProgramCtx::getPlugins() const
{
  return this->plugins;
}


void
ProgramCtx::addPlugins(const std::vector<PluginInterface*>& p)
{
  plugins->insert(plugins->end(), p.begin(), p.end());
}


Program*
ProgramCtx::getIDB() const
{
  return IDB;
}


AtomSet*
ProgramCtx::getEDB() const
{
  return EDB;
}


NodeGraph*
ProgramCtx::getNodeGraph() const
{
  return nodegraph;
}


void
ProgramCtx::setNodeGraph(NodeGraph* ng)
{
  if (ng != this->nodegraph)
    {
      delete this->nodegraph;
      this->nodegraph = ng;
    }
}


DependencyGraph*
ProgramCtx::getDependencyGraph() const
{
  return depgraph;
}


void
ProgramCtx::setDependencyGraph(DependencyGraph* dg)
{
  if (dg != this->depgraph)
    {
      delete this->depgraph;
      this->depgraph = dg;
    }
}



ResultContainer*
ProgramCtx::getResultContainer() const
{
  return result;
}


void
ProgramCtx::setResultContainer(ResultContainer* c)
{
  if (this->result != c)
    {
      delete this->result;
      this->result = c;
    }
}


OutputBuilder*
ProgramCtx::getOutputBuilder() const
{
  return outputbuilder;
}


void
ProgramCtx::setOutputBuilder(OutputBuilder* o)
{
  if (this->outputbuilder != o)
    {
      delete this->outputbuilder;
      this->outputbuilder = o;
    }
}
#endif


void ProgramCtx::showPlugins() { state->showPlugins(this); }
void ProgramCtx::convert() { state->convert(this); }
void ProgramCtx::parse() { state->parse(this); }
void ProgramCtx::rewriteEDBIDB() { state->rewriteEDBIDB(this); }
void ProgramCtx::safetyCheck() { state->safetyCheck(this); }
void ProgramCtx::createDependencyGraph() { state->createDependencyGraph(this); }
void ProgramCtx::optimizeEDBDependencyGraph() { state->optimizeEDBDependencyGraph(this); }
void ProgramCtx::createComponentGraph() { state->createComponentGraph(this); }
void ProgramCtx::strongSafetyCheck() { state->strongSafetyCheck(this); }
void ProgramCtx::createEvalGraph() { state->createEvalGraph(this); }
void ProgramCtx::setupProgramCtx() { state->setupProgramCtx(this); }
void ProgramCtx::evaluate() { state->evaluate(this); }
void ProgramCtx::postProcess() { state->postProcess(this); }

DLVHEX_NAMESPACE_END

// Local Variables:
// mode: C++
// End:
