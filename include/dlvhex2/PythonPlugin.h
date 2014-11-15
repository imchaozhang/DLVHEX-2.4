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
 * @file PythonPlugin.h
 * @author Christoph Redl
 *
 * @brief Supports Python-implemented plugins.
 */



/**
 * \defgroup pythonpluginframework The Python Plugin Framework
 *
 * \section introduction Introduction
 *
 * dlvhex evaluates Answer Set Programs with external atoms. One important
 * design principle was to provide a mechanism to easily add further external
 * atoms without having to recompile the main application.
 * With dlvhex version 2.4.0 a \em Python plugin interface was introduced,
 * which supports Python scripts that provide functions to realise custom external atoms.
 *
 * This Section gives an overview of dlvhex Python plugins and external atoms.
 *
 * \section extatom The External Atom Function
 *
 * Formally, an external atom is defined to evaluate to \e true or \e
 * false, depending on a number of parameters:
 * - An interpretation (a set of facts)
 * - A list of input constants
 * - A list of output constants
 * .
 * However, it is more intuitive and convenient to think of an external atom not
 * as being boolean, but rather functional: Depending on a given interpretation
 * and a list of input constants, it returns output tuples. For instance, the
 * external atom to import triples from RDF files has this form:
 *
 * \code
 *   &rdf[uri](X,Y,Z)
 * \endcode
 *
 * where \c uri stands for a string denoting the RDF-source and \c X, \c Y, and
 * \c Z are variables that represent an RDF-triple. The function associated with
 * this atom simply returns all RDF-triples from the specified source.
 * Obviously, in this case the interpretation is ignored.
 *
 * \subsection informationflow Information Flow
 *
 * The interface that is used by dlvhex to access a plugin follows very closely
 * these semantics. For each atom, a retrieval function has to be implemented,
 * which receives a query-object and has to return an an answer-object. The
 * query-object carries the input interpretation as well as the ground input
 * parameters of the external atom call, while the answer object is a container
 * for the output tuples of the external atom's function.
 *
 * \subsection inputtypes Types of Input Parameters
 *
 * Theoretically, it is completely up to the atom function how to process the
 * interpretation together with the input constants, which are basically only
 * names. In practice however, only parts of the interpretation might be needed
 * (if at all). Considering this as well as for efficiency reasons, we created
 * two (in the implementation three, see below) categories of input parameters:
 * - the Constant parameter and
 * - the Predicate parameter.
 * A third category called Tuple exists, which is a meta category standing for
 * an arbitrary mount of Constant input parameters. This is useful, e.g.,
 * for &concat[s1,s2,...](Out).
 * 
 * A parameter of type "Constant" is not related to the interpretation at all,
 * like in the previous example of the RDF-atom. A parameter is of type
 * "Predicate" means that all facts in the interpretation with this predicate
 * are necessary for the atom. Let's assume, we have an external atom that
 * calculates the overall price of a number of books given by their ISBN number:
 *
 * \code
 *   &overallbookprice[isbn](X)
 * \endcode
 *
 * The single input parameter of this atom would be of type "Predicate", meaning
 * that not the constant itself is necessary for the atom's function, but the part
 * of the interpretation with this predicate.  So if we have, e.g.,
 * \f[
 * I=\{{\rm isbn}({\rm 0{-}19{-}824183{-}6}), {\rm isbn}({\rm 0{-}201{-}99954{-}4}), p(a), q(b),\ldots\}
 * \f]
 * the atom's function will be called with a "reduced" interpretation:
 * \f[
 * I=\{{\rm isbn}({\rm 0{-}19{-}824183{-}6}), {\rm isbn}({\rm 0{-}201{-}99954{-}4})\}
 * \f]
 * Specifying the type of input parameters not only helps to single out the
 * relevant part of the interpretation, but also supports dlvhex in calculating
 * the dependencies within a HEX-program.
 *
 * \section writingpythonplugin Writing a Python Plugin
 * We wanted to keep the interface between dlvhex and the plugins as lean as
 * possible.
 * Necessary tasks are:
 * - Write a new Python script which contains a \em register function and imports the package \em dlvhex
 * - Write another function for each external atom, and export this function using the \em register function
 *
 * The \em register function has the following form:
 * \code
 * def register():
 *   dlvhex.addAtom ("concat", (dlvhex.CONSTANT, dlvhex.CONSTANT), 1) )
 * \endcode
 * It adds one \em entry for each external atom. Each \em entry is
 * again a tuple of arity 3: the first element is the external predicate name,
 * the third element is the output arity, and the second is another tuple of input parameter types (dlvhex.CONSTANT, dlvhex.PREDICATE, dlvhex.TUPLE).
 *
 * Each external predicate name (e.g. \em concat) needs to be implemented in form of another Python function with an appropriate number of input parameters.
 *
 * Example:
 * \code
 * def concat(a, b):
 *   dlvhex.outputValues(dlvhex.getValue(a), dlvhex.getValue(b))
 * \endcode
 * The function just takes the values of these parameters and outputs their string concatenation.
 * Here, \em a and \em b are the input parameters (of type constant). If an external atom specifies an input parameter of type TUPLE,
 * the elements will be passed as a Python tuple.
 *
 * Example:
 * \code
 * def concat(tup):
 *   ret = ""
 *   for x in tup:
 *     ret = reg + x
 *   dlvhex.outputValues((ret, ))
 * \endcode
 * Note that akin to the \ref pluginframework "C++ API", terms and atoms are represented by IDs and the retrieval of the value behind
 *
 * usually requires the use of the \em getValue method; some methods combine this with other functionalities (see method list below).
 *
 * In more detail, the \em dlvhex Python module provides the following methods:
 * <ul>
 *   <li><em>tuple getTuple(aID)</em>: Return the IDs of the elements of a dlvhex atom identified by ID \em aID.</li>
 *   <li><em>tuple getTupleValues(aID)</em>: Return the values of the elements of a dlvhex atom identified by ID \em aID.</li>
 *   <li><em>string getValue(id)</em>: Return the value of an atom or term ID \em id.</li>
 *   <li><em>int getIntValue(id)</em>: Return the value of an integer term ID \em id as integer.</li>
 *   <li><em>string getValue(tup)</em>: Print the tuple \em tup recursively, i.e., the elements of the tuple can be further tuples or IDs. IDs \em id are printed by calling <em>dlvhex.getValue(id)</em>, they are delimited by <em>,</em> and the output is enclosed in curly braces.</li>
 *   <li><em>tuple getExtension(id)</em>: Returns all tuples \em tup in the extension of the predicate represented by \em id (wrt. the input interpretation).</li>
 *   <li><em>dlvhex.ID storeString(str)</em>: Stores a string \em str as dlvhex object and returns its ID.</li>
 *   <li><em>dlvhex.ID storeInteger(int)</em>: Stores an integer \em int as dlvhex object and returns its ID.</li>
 *   <li><em>dlvhex.ID storeAtom(args)</em>: Transforms a sequence of terms or values \em args into a dlvhex atom.</li>
 *   <li><em>dlvhex.ID negate(aID)</em>: Negates an atom ID \em aID.</li>
 *   <li><em>bool learn(tup)</em>: Learns a nogood as a tuple of atom IDs or their negations \em tup; returns if learning was enabled.</li>
 *   <li><em>ID storeOutputAtom(args)</em>: Constructs an output atom from IDs or values \em args (for learning purposes) and returns its ID.</li>
 *   <li><em>void output(args)</em>: Adds a tuple of IDs or values \em args to the external source output.</li>
 *   <li><em>tuple getInputAtoms([pred])</em>: Returns a tuple of \em all input atoms (\em not only true ones!) to this external atom; \em pred is an optional predicate ID, which allows for restricting the tuple to atoms over this predicate.</li>
 *   <li><em>tuple getTrueInputAtoms([pred])</em>: Returns a tuple of all input atoms to this external atom <em>which are currently true</em>; \em pred is an optional predicate ID, which allows for restricting the tuple to atoms over this predicate.</li>
 *   <li><em>int getInputAtomCount()</em>: Returns the number of \em input atoms (\em not only true ones!).</li>
 *   <li><em>int getTrueInputAtomCount()</em>: Returns the number of input atoms <em>which are currently true</em>.</li>
 *   <li><em>bool isInputAtom(id)</em>: Checks if atom \em id belongs to the input of the current external atom.</li>
 *   <li><em>bool isAssigned(id)</em>: Checks if an input atom identified by ID \em id is assigned.</li>
 *   <li><em>bool isTrue(id)</em>: Checks if an input atom identified by ID \em id is assigned to true.</li>
 *   <li><em>bool isFalse(id)</em>: Checks if an input atom identified by ID \em id is assigned to false.</li>
 *   <li><em>void addAtom(name, args, ar, [prop])</em>: Add external atom \em name with arguments \em args (see above), output arity \em ar and external source properties \em prop ("prop" is optional).</li>
 *   <li><em>void storeExternalAtom(pred, input, output)</em>: Stores an external atom with predicate \em pred, input parameters \em input and output parameters \em output (can be terms or their IDs) and returns its ID.</li>
 *   <li><em>void storeRule(head, pbody, nbody)</em>: Stores a rule with head atoms \em head, positive body atoms \em pbody and negative body atoms \em nbody; all parameters need to be a tuples of IDs.</li>
 *   <li><em>tuple evaluateSubprogram(tup)</em>: Evaluates the subprogram specified by a tuple <em>facts, rules</em> consisting of facts \em facts (tuple of IDs of ground atoms) and rules \em rules (tuple of rule IDs) and returns the number of answer sets; the result is a tuple of answer sets, where each answer set is again a tuple of the ground atom IDs which are true in the respective answer set.</li>
 *   <li><em>tuple loadSubprogram(filename)</em>: Loads the program stored in file \em filename and returns a pair <em>(edb, idb)</em> consisting of a tuple \em edb of facts (ground atom IDs) and a tuple \em idb of rule IDs.</li>
 * </ul>
 *
 * External source properties \em prop are of type <em>dlvhex.ExtSourceProperties</em> and can be configured using the following methods:
 * <ul>
 *   <li><em>void prop.addMonotonicInputPredicate(index)</em>: Declare argument \em index as monotonic predicate parameter.</li>
 *   <li><em>void prop.addAntimonotonicInputPredicate(index)</em>: Declare argument \em index as antimonotonic predicate parameter.</li>
 *   <li><em>void prop.addPredicateParameterNameIndependence(index)</em>: Declare argument \em index as independent of the predicate name (only its extension is relevant).</li>
 *   <li><em>void prop.addFiniteOutputDomain(index)</em>: Declare that output argument \em index has a finite domain.</li>
 *   <li><em>void prop.addRelativeFiniteOutputDomain(index1, index2)</em>: Declare that output argument \em index2 has a finite domain wrt. input argument \em index1.</li>
 *   <li><em>void prop.setFunctional(value)</em>: Declare the source as functional.</li>
 *   <li><em>void prop.setFunctionalStart(index)</em>: Declare the source as functional beginning at term index + 1.</li>
 *   <li><em>void prop.setSupportSets(value)</em>: Declare that the source provides support sets.</li>
 *   <li><em>void prop.setCompletePositiveSupportSets(value)</em>: Declare that the source provides complete positive support sets.</li>
 *   <li><em>void prop.setCompleteNegativeSupportSets(value)</em>: Declare that the source provides complete negative support sets.</li>
 *   <li><em>void prop.setVariableOutputArity(value)</em>: Declare that the source has a variable output arity.</li>
 *   <li><em>void prop.setCaresAboutAssigned(value)</em>: Declare that the sources wants to know the assigned values.</li>
 *   <li><em>void prop.setCaresAboutChanged(value)</em>: Declare that the sources wants to know the values which potentially changed since the previous call.</li>
 *   <li><em>void prop.setAtomlevellinear(value)</em>: Declare the source as linear on the atom level.</li>
 *   <li><em>void prop.setUsesEnvironment(value)</em>: Declare the source as linear on the tuple level.</li>
 *   <li><em>void prop.setFiniteFiber(value)</em>: Declare that the source has a finite fiber.</li>
 *   <li><em>void prop.addWellorderingStrlen(index1, index2)</em>: Declare that output argument \em index1 has a string length wellordering wrt. input argument \em index2.</li>
 *   <li><em>void prop.addWellorderingNatural(index1, index2)</em>: Declare that output argument \em index1 has a natural wellordering wrt. input argument \em index2.</li>
 * </ul>
 * 
 * Moreover, for an ID object \em id, there are the following shortcuts:
 * <ul>
 *   <li><em>id.value()</em> for <em>dlvhex.getValue(id)</em></li>
 *   <li><em>id.extension()</em> for <em>dlvhex.getExtension(id)</em></li>
 *   <li><em>id.intValue()</em> for <em>dlvhex.getIntValue(id)</em></li>
 *   <li><em>id.tuple()</em> for <em>dlvhex.getTuple(id)</em></li>
 *   <li><em>id.tupleValues()</em> for <em>dlvhex.getTupleValues(id)</em></li>
 *   <li><em>id.negate()</em> for <em>dlvhex.negate(id)</em></li>
 *   <li><em>id.isInputAtom()</em> for <em>dlvhex.isInputAtom(id)</em></li>
 *   <li><em>id.isAssigned()</em> for <em>dlvhex.isAssigned(id)</em></li>
 *   <li><em>id.isTrue()</em> for <em>dlvhex.isTrue(id)</em></li>
 *   <li><em>id.isFalse()</em> for <em>dlvhex.isFalse(id)</em></li>
 * </ul>
 * 
 * In order to load a Python-implemented plugin stored in file PATH,
 * pass the additional option \code --pythonplugin=PATH \endcode to dlvhex.
 */
#ifndef PYTHON_PLUGIN__HPP_INCLUDED
#define PYTHON_PLUGIN__HPP_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/PluginInterface.h"

#include <string>
#include <vector>

#ifdef HAVE_PYTHON

//#include <Python.h>

DLVHEX_NAMESPACE_BEGIN

class PythonPlugin:
  public PluginInterface
{
public:
	// stored in ProgramCtx, accessed using getPluginData<PythonPlugin>()
	class CtxData:
	public PluginData
	{
		public:

		std::vector<std::string> pythonScripts;

		CtxData();
		virtual ~CtxData() {}
	};

public:
	PythonPlugin();
	virtual ~PythonPlugin();

	// output help message for this plugin
	virtual void printUsage(std::ostream& o) const;

	// accepted options: --aggregate-enable
	//
	// processes options for this plugin, and removes recognized options from pluginOptions
	// (do not free the pointers, the const char* directly come from argv)
	virtual void processOptions(std::list<const char*>& pluginOptions, ProgramCtx&);

	// rewrite program: rewrite aggregate atoms to external atoms
	virtual PluginRewriterPtr createRewriter(ProgramCtx&);

	// register model callback which transforms all auxn(p,t1,...,tn) back to p(t1,...,tn)
	virtual void setupProgramCtx(ProgramCtx&);

	virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx&) const;

	void runPythonMain(std::string filename);

	// no atoms!
};

DLVHEX_NAMESPACE_END

#endif

#endif

