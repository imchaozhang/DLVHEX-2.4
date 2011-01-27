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
 * @file   BaseModelGenerator.hpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Base class for model generators, implementing common functionality.
 */

#ifndef BASE_MODEL_GENERATOR_HPP_INCLUDED__09112010
#define BASE_MODEL_GENERATOR_HPP_INCLUDED__09112010

#include "dlvhex/PlatformDefinitions.h"
#include "dlvhex/fwd.hpp"
#include "dlvhex/ModelGenerator.hpp"
#include "dlvhex/Interpretation.hpp"
#include "dlvhex/ASPSolverManager.h"

#include <list>

DLVHEX_NAMESPACE_BEGIN

class BaseModelGenerator:
  public ModelGeneratorBase<Interpretation>
{
protected:
  struct EmptyResults:
    public ASPSolverManager::Results
  {
    EmptyResults() {}
    virtual ~EmptyResults() {}
    virtual AnswerSet::Ptr getNextAnswerSet() { return AnswerSet::Ptr(); }
  };

  struct SingularResults:
    public ASPSolverManager::Results
  {
    SingularResults(AnswerSet::Ptr as): ASPSolverManager::Results(), ret(as) {}
    virtual ~SingularResults() {}
    virtual AnswerSet::Ptr getNextAnswerSet()
      { AnswerSet::Ptr p = ret; ret.reset(); return p; };
    AnswerSet::Ptr ret;
  };

  // members
public:
  BaseModelGenerator(InterpretationConstPtr input):
    ModelGeneratorBase<Interpretation>(input) {}
  virtual ~BaseModelGenerator() {}

protected:
  // projects input interpretation for predicate inputs
  // calculates constant input tuples from auxiliary input predicates and from given constants
  // calls eatom function with each input tuple
  // reintegrates output tuples as auxiliary atoms into outputi
  // (inputi and outputi may point to the same interpretation)
  virtual void evaluateExternalAtom(RegistryPtr reg,
    const ExternalAtom& eatom,
    InterpretationConstPtr inputi, InterpretationPtr outputi) const;

  // calls evaluateExternalAtom for each atom in eatoms
  virtual void evaluateExternalAtoms(RegistryPtr reg,
    const std::vector<ID>& eatoms,
    InterpretationConstPtr inputi, InterpretationPtr outputi) const;

  //
  // helper methods used by evaluateExternalAtom
  //

  // returns false iff tuple does not unify with eatom output pattern
  // (the caller must decide whether to throw an exception or ignore the tuple)
  virtual bool verifyEAtomAnswerTuple(RegistryPtr reg,
    const ExternalAtom& eatom, const Tuple& t) const;

  // project a given interpretation to all predicates that are predicate inputs in the given eatom
  // return this as a new interpretation
  virtual InterpretationPtr projectEAtomInputInterpretation(RegistryPtr reg,
    const ExternalAtom& eatom, InterpretationConstPtr full) const;

  // from auxiliary input predicates and the eatom,
  // calculate all tuples that are inputs to the eatom and store them into "inputs"
  virtual void buildEAtomInputTuples(RegistryPtr reg,
    const ExternalAtom& eatom, InterpretationConstPtr i, std::list<Tuple>& inputs) const;
};

//
// a model generator factory provides model generators
// for a certain types of interpretations
//
class BaseModelGeneratorFactory:
  public ModelGeneratorFactoryBase<Interpretation>
{
  // methods
public:
  BaseModelGeneratorFactory() {}
  virtual ~BaseModelGeneratorFactory() {}

protected:
  // rewrite all eatoms in body to auxiliary replacement atoms
  // store into registry and return id
  virtual ID convertRule(RegistryPtr reg, ID ruleid);
};

DLVHEX_NAMESPACE_END

#endif // BASE_MODEL_GENERATOR_HPP_INCLUDED__09112010
