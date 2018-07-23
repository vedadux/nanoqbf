/* Copyright (C) 2011 - 2014, Armin Biere, Johannes Kepler University, Linz */
/* Copyright (C) 2014, Mathias Preiner, Johannes Kepler University, Linz */
/* Copyright (C) 2018, Vedad Hadzic, Technical University Graz, Linz */

/* This file contains code original developped for the Boolector project,
 * but is also available under the standard IPASIR license.
 */
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

#include "core/Solver.h"
#include "ipasir.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

using namespace std;
using namespace Minisat;

extern "C" {
static const char* sig = "maplesat";
#include <sys/resource.h>
#include <sys/time.h>
static double getime(void)
{
  struct rusage u;
  double res;
  if (getrusage(RUSAGE_SELF, &u)) return 0;
  res = u.ru_utime.tv_sec + 1e-6 * u.ru_utime.tv_usec;
  res += u.ru_stime.tv_sec + 1e-6 * u.ru_stime.tv_usec;
  return res;
}
};

class IPAsirMapleSAT :
public Solver
{
  vec <Lit> assumptions, clause;
  int szfmap;
  unsigned char* fmap;
  bool nomodel;
  unsigned long long calls;
  
  void reset()
  { if (fmap) delete[] fmap, fmap = 0, szfmap = 0; }
  
  Lit import(int lit)
  {
    while (abs(lit) > nVars()) (void) newVar();
    return mkLit(Var(abs(lit) - 1), (lit < 0));
  }
  
  void ana()
  {
    fmap = new unsigned char[szfmap = nVars()];
    memset(fmap, 0, szfmap);
    for (int i = 0; i < conflict.size(); i++)
    {
      int tmp = var(conflict[i]);
      assert(0 <= tmp && tmp < szfmap);
      fmap[tmp] = 1;
    }
  }
  
  double ps(double s, double t)
  { return t ? s / t : 0; }

public:
  IPAsirMapleSAT() :
  szfmap(0), fmap(0), nomodel(false), calls(0)
  {
    // MapleSAT by default produces non standard conforming messages.
    // So either we have to set this to '0' or patch the sources.
    verbosity = 0;
  }
  
  ~IPAsirMapleSAT()
  { reset(); }
  
  void add(int lit)
  {
    reset();
    nomodel = true;
    if (lit) clause.push(import(lit));
    else addClause(clause), clause.clear();
  }
  
  void assume(int lit)
  {
    reset();
    nomodel = true;
    assumptions.push(import(lit));
  }
  
  int solve()
  {
    calls++;
    reset();
    lbool res = solveLimited(assumptions);
    assumptions.clear();
    nomodel = (res != l_True);
    return (res == l_Undef) ? 0 : (res == l_True ? 10 : 20);
  }
  
  int val(int lit)
  {
    assert(!nomodel);
    unsigned v = std::abs(lit);
    if (nVars() < v) return -v;
    lbool res = modelValue(import(lit));
    int sign = (res == l_False);
    return (lit ^ (-sign)) + sign;
  }
  
  int failed(int lit)
  {
    if (!fmap) ana();
    int tmp = var(import(lit));
    assert(0 <= tmp && tmp < nVars());
    return fmap[tmp] != 0;
  }
  
  void stats()
  {
    double t = getime();
    printf(
    "c [%s]\n"
    "c [%s]        calls %12llu   %9.1f per second\n"
    "c [%s]     restarts %12llu   %9.1f per second\n"
    "c [%s]    conflicts %12llu   %9.1f per second\n"
    "c [%s]    decisions %12llu   %9.1f per second\n"
    "c [%s] propagations %12llu   %9.1f per second\n"
    "c [%s]\n",
    sig,
    sig, (unsigned long long) calls, ps(calls, t),
    sig, (unsigned long long) starts, ps(starts, t),
    sig, (unsigned long long) conflicts, ps(conflicts, t),
    sig, (unsigned long long) decisions, ps(decisions, t),
    sig, (unsigned long long) propagations, ps(propagations, t),
    sig);
    fflush(stdout);
  }
};

static IPAsirMapleSAT* import(void* s)
{ return (IPAsirMapleSAT*) s; }

const char* ipasir_signature()
{ return sig; }

void* ipasir_init()
{ return new IPAsirMapleSAT(); }

void ipasir_release(void* s)
{
  import(s)->stats();
  delete import(s);
}

int ipasir_solve(void* s)
{ return import(s)->solve(); }

void ipasir_add(void* s, int l)
{ import(s)->add(l); }

void ipasir_assume(void* s, int l)
{ import(s)->assume(l); }

int ipasir_val(void* s, int l)
{ return import(s)->val(l); }

int ipasir_failed(void* s, int l)
{ return import(s)->failed(l); }
