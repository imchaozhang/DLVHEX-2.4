/* -*- C++ -*- */

/**
 * @file   ProgramBuilder.h
 * @author Roman Schindlauer
 * @date   Sun Sep 4 12:39:40 2005
 * 
 * @brief  Builders for logic program representations.
 * 
 * 
 */

#ifndef _PROGRAMBUILDER_H
#define _PROGRAMBUILDER_H

#include <string>
#include <sstream>

#include "dlvhex/Program.h"

//
// forward declaration
//
class PrintVisitor;


/**
* @brief Base Builder for building logic programs.
*/
class ProgramBuilder
{
protected:
    std::ostringstream stream;

    /// Ctor
    ProgramBuilder();

    /// Dtor
    virtual
    ~ProgramBuilder();

public:

    /**
     * Building method implemented by the children of ProgramBuilder.
     *
     * @param rule
     */
    virtual void
    buildRule(const Rule* rule) = 0;

    /**
     * @brief Build Facts.
     */
    virtual void
    buildFacts(const AtomSet&) = 0;

    std::string
    getString();

    void
    clearString();

};


/**
 * @brief A Builder for programs to be evaluated with DLV.
 *
 * Uses a DLVPrintVisitor (in first order mode) or a HOPrintVisitor
 * (in higher order mode) for building logic programs.
 *
 * @todo This whole builder could be effectively replaced by the
 * corresponding visitors.
 */
class ProgramDLVBuilder : public ProgramBuilder
{
public:
    /// Ctor
    ProgramDLVBuilder(bool ho);

    /// Dtor
    virtual
    ~ProgramDLVBuilder();

    /**
     * @brief Build rule for DLV.
     */
    virtual void
    buildRule(const Rule*);

    /**
     * @brief Build facts for DLV.
     */
    virtual void
    buildFacts(const AtomSet&);

    /**
     * @brief Build program for DLV from set of rules.
     */
    virtual void
    buildProgram(const Program&);

private:
    /**
     * @brief either a DLVPrintVisitor or a HOPrintVisitor.
     */
    PrintVisitor* pv;
};



#endif /* _PROGRAMBUILDER_H */
