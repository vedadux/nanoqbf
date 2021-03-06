//
// Created by vedad on 18/06/18.
//

#ifndef NANOQBF_SATSOLVER_H
#define NANOQBF_SATSOLVER_H

#include "ipasir.h"
#include "types/Assignment.h"
#include "Logger.h"
#include "types/Clause.h"
#include <vector>

/// A SAT solver interface using IPASIR
class SatSolver
{
public:
  /// SatSolver Constructor
  inline SatSolver() : solver_(ipasir_init()), num_vars(0), mem_error(false){ }
  
  /// SatSolver Destructor
  inline ~SatSolver() { ipasir_release(solver_); }
  
  /// Destroys the currently used #solver_ and requests a new one
  inline void reset();
  
  /// Solves the given CNF
  inline int solve();
  
  /// Adds a literal to the current clause in the formula inside #solver_
  inline void add(Lit l);
  
  /// Pushes the current clause into #solver_
  inline void push();
  
  /// Adds a clause from a Clause object to the formula inside #solver_
  inline void addClause(const Clause* clause);
  
  /// Gets the Lit representing the found value of \a v
  inline Lit getValue(Var v);
  
  /// Reserves \num variable indexes inside #solver_
  inline Var reserveVars(unsigned num);
  
  /// Checks whether there was a memory error
  inline bool hasMemory() const { return !mem_error; }
  
private:
  void* solver_;     ///< IPASIR solver object
  unsigned num_vars; ///< number of variables in #solver_
  bool mem_error;    ///< records that an out of memory error occured
};

inline void SatSolver::reset()
{
  ipasir_release(solver_);
  solver_ = ipasir_init();
  num_vars = 0;
  mem_error = false;
}

inline int SatSolver::solve()
{
  if (mem_error) return -1;
  return ipasir_solve(solver_);
}

inline void SatSolver::add(Lit l)
{
  assert(l != 0);
  mem_error |= ipasir_add(solver_, l);
  if(mem_error) throw std::bad_alloc();
}

inline void SatSolver::push()
{
  mem_error |= ipasir_add(solver_, 0);
  if(mem_error) throw std::bad_alloc();
}

inline void SatSolver::addClause(const Clause* clause)
{
  for(const_lit_iterator l_iter = clause->begin_e(); l_iter != clause->end_e(); l_iter++)
    mem_error |= ipasir_add(solver_, *l_iter);
  for(const_lit_iterator l_iter = clause->begin_a(); l_iter != clause->end_a(); l_iter++)
    mem_error |= ipasir_add(solver_, *l_iter);
  mem_error |= ipasir_add(solver_, 0);
  if(mem_error) throw std::bad_alloc();
}

inline Lit SatSolver::getValue(Var v)
{
  return ipasir_val(solver_, v);
}

inline Var SatSolver::reserveVars(unsigned num)
{
  Var res = num_vars + 1;
  num_vars += num;
  return res;
}

#endif //NANOQBF_SATSOLVER_H
