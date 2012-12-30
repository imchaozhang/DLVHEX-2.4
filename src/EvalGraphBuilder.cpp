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
 * @file EvalGraphBuilder.cpp
 * @author Peter Schüller
 *
 * @brief Implementation of the eval graph builder.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dlvhex2/EvalGraphBuilder.h"
#include "dlvhex2/PlainModelGenerator.h"
#include "dlvhex2/WellfoundedModelGenerator.h"
#include "dlvhex2/GuessAndCheckModelGenerator.h"
#include "dlvhex2/GenuinePlainModelGenerator.h"
#include "dlvhex2/GenuineWellfoundedModelGenerator.h"
#include "dlvhex2/GenuineGuessAndCheckModelGenerator.h"
#include "dlvhex2/GenuineGuessAndCheckModelGeneratorAsync.h"
#include "dlvhex2/Logger.h"
#include "dlvhex2/Registry.h"
#include "dlvhex2/ProgramCtx.h"
#include "dlvhex2/PluginInterface.h"

#include <boost/range/iterator_range.hpp>

#include <sstream>

DLVHEX_NAMESPACE_BEGIN

namespace
{
  typedef ComponentGraph::ComponentSet ComponentSet;
  typedef ComponentGraph::ComponentInfo ComponentInfo;
  typedef ComponentGraph::DependencyInfo DependencyInfo;
  typedef ComponentGraph::DepMap DepMap;
}

EvalGraphBuilder::EvalGraphBuilder(
    ProgramCtx& ctx, 
		ComponentGraph& cg,
    EvalGraphT& eg,
    ASPSolverManager::SoftwareConfigurationPtr externalEvalConfig):
  ctx(ctx),
	//cg(cg),
  clonedcgptr(cg.clone()),
  cgcopy(*clonedcgptr),
  eg(eg),
  externalEvalConfig(externalEvalConfig),
	mapping(),
  unusedEdgeFilter(&cgcopy, &mapping),
  unusedVertexFilter(&mapping),
  cgrest(cgcopy.getInternalGraph(), unusedEdgeFilter, unusedVertexFilter)
{
}

EvalGraphBuilder::~EvalGraphBuilder()
{
}

RegistryPtr EvalGraphBuilder::registry()
{
  return ctx.registry();
}

namespace
{
  typedef FinalEvalGraph::EvalUnitPropertyBundle EvalUnitProperties;
  typedef FinalEvalGraph::EvalUnitDepPropertyBundle EvalUnitDepProperties;

  EvalUnitProperties eup_empty;
}

EvalGraphBuilder::Component EvalGraphBuilder::getComponentForUnit(EvalGraphBuilder::EvalUnit u) const
{
  ComponentEvalUnitMapping::right_map::const_iterator it =
    mapping.right.find(u);
  if( it == mapping.right.end() )
    throw std::runtime_error("tried to get component for unit not created here!");
  return it->second;
}


EvalGraphBuilder::EvalUnit
EvalGraphBuilder::createEvalUnit(
		const std::list<Component>& comps, const std::list<Component>& ccomps)
{
  LOG_SCOPE(ANALYZE,"cEU",true);
  DBGLOG(DBG,"= EvalGraphBuilder::createEvalUnit(" <<
			printrange(comps) << "," << printrange(ccomps) << ")");

  // collapse components into new eval unit
	// (this verifies necessary conditions and computes new dependencies)
  #warning substitute name cgcopy by name cg once this works
  Component newComp;
	{
		// TODO perhaps directly take ComponentSet as inputs
		ComponentSet scomps(comps.begin(), comps.end());
		ComponentSet sccomps(ccomps.begin(), ccomps.end());
    //was: calculateNewEvalUnitInfos(scomps, sccomps, newUnitDependsOn, newUnitInfo);
		//cgcopy.computeCollapsedComponentInfos(
    //    scomps, sccomps, newInDeps, newOutDeps, newUnitInfo);
		newComp = cgcopy.collapseComponents(scomps, sccomps);
	}
	const ComponentInfo& newUnitInfo = cgcopy.propsOf(newComp);

  // create eval unit
  EvalUnit u = eg.addUnit(eup_empty);
  LOG(DBG,"created unit " << u << " for new comp " << newComp);

  // associate new comp with eval unit
  {
    typedef ComponentEvalUnitMapping::value_type MappedPair;
		bool success = mapping.insert(MappedPair(newComp, u)).second;
		assert(success); // component must not already exist here
	}

  // configure unit
  EvalUnitProperties& uprops = eg.propsOf(u);

  // configure model generator factory, depending on type of component
  {
    const ComponentGraph::ComponentInfo& ci = newUnitInfo;
    if( ci.innerEatoms.empty() )
    {
      // no inner external atoms -> plain model generator factory
      LOG(DBG,"configuring plain model generator factory for eval unit " << u);
      if (ctx.config.getOption("GenuineSolver") > 0){
        uprops.mgf.reset(new GenuinePlainModelGeneratorFactory(
              ctx, ci, externalEvalConfig));
      }else{
        uprops.mgf.reset(new PlainModelGeneratorFactory(
              ctx, ci, externalEvalConfig));
      }
    }
    else
    {
      if( !ci.innerEatomsNonmonotonic && !ci.negativeDependencyBetweenRules && !ci.disjunctiveHeads )
      {
        // inner external atoms and only in positive cycles and monotonic and no disjunctive rules
				// -> wellfounded/fixpoint model generator factory
        LOG(DBG,"configuring wellfounded model generator factory for eval unit " << u);
        if (ctx.config.getOption("GenuineSolver") > 0){
          uprops.mgf.reset(new GenuineWellfoundedModelGeneratorFactory(
                ctx, ci, externalEvalConfig));
        }else{
          uprops.mgf.reset(new WellfoundedModelGeneratorFactory(
                ctx, ci, externalEvalConfig));
        }
      }
      else
      {
        // everything else -> guess and check model generator factory
        LOG(DBG,"configuring guess and check model generator factory for eval unit " << u);
        if (ctx.config.getOption("GenuineSolver") > 0){
          if (ctx.config.getOption("MultiThreading")){
            uprops.mgf.reset(new GenuineGuessAndCheckModelGeneratorAsyncFactory(
                  ctx, ci, externalEvalConfig));
          }else{
            uprops.mgf.reset(new GenuineGuessAndCheckModelGeneratorFactory(
                  ctx, ci, externalEvalConfig));
          }
        }else{
          uprops.mgf.reset(new GuessAndCheckModelGeneratorFactory(
                ctx, ci, externalEvalConfig));
        }
      }
    }
  }

  // create dependencies
  unsigned joinOrder = 0; // TODO define join order in a more intelligent way?
  ComponentGraph::PredecessorIterator dit, dend;
  for(boost::tie(dit, dend) = cgcopy.getDependencies(newComp);
      dit != dend; dit++)
  {
    Component tocomp = cgcopy.targetOf(*dit);

    // get eval unit corresponding to tocomp
    ComponentEvalUnitMapping::left_map::iterator itdo =
      mapping.left.find(tocomp);
    assert(itdo != mapping.left.end());
    EvalUnit dependsOn = itdo->second;

    const DependencyInfo& di = cgcopy.propsOf(*dit);
    DBGLOG(DBG,"adding dependency to unit " << dependsOn << " with joinOrder " << joinOrder);
    eg.addDependency(u, dependsOn, EvalUnitDepProperties(joinOrder));
    joinOrder++;
	}

  return u;
}

DLVHEX_NAMESPACE_END

// Local Variables:
// mode: C++
// End:
