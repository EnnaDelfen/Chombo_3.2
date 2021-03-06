#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#ifndef _SMOOTHUNION_H_
#define _SMOOTHUNION_H_

#include "MayDay.H"
#include "RealVect.H"
#include "Vector.H"

#include "BaseIF.H"

#include "NamespaceHeader.H"

///
/**
   This implicit function is the maximum of two (of more) implicit
   functions with which it is constructed.  This results in an implicit
   function whose interior is the intersection of the interiors of the given
   implicit functions.
   For the maximum, it uses the smooth absolute value Ae(a,b) \approx |a-b|
   min(a,b) = 0.5*(a+b - | a-b |)
*/
class SmoothUnion: public BaseIF
{
public:

  ///
  /**
     Constructor specifying any number of implicit functions as inputs.
  */
  SmoothUnion(const Vector<BaseIF*>& a_impFuncs,
                     const Real           & a_delta);

  /// Destructor
  virtual ~SmoothUnion();

  ///
  /**
      Return the value of the function at a_point.
   */
  virtual Real value(const RealVect& a_point) const;

  ///
  virtual Real derivative(const  IntVect& a_deriv,
                          const RealVect& a_point) const;


  virtual BaseIF* newImplicitFunction() const;

  //min(a,b) = 0.5*(a+b - | a-b |)
  virtual Real smoothMin(const  IntVect & a_deriv,
                         const RealVect & a_point,
                         const  int     & a_closestIF,
                         const  int     & a_nextClosestIF) const;

  void findClosest(int            & a_closestIF, 
                   int            & a_nextClosestIF,
                   int            & a_numWithinDelta,
                   const RealVect & a_point) const;
protected:
  int             m_numFuncs; // number of implicit functions
  Vector<BaseIF*> m_impFuncs; // implicit functions
  Real            m_delta   ; //smoothing length scale

private:
  SmoothUnion()
  {
    MayDay::Abort("SmoothUnion uses strong construction");
  }

  SmoothUnion(const SmoothUnion& a_inputIF)
  {
    MayDay::Abort("SmoothUnion does not have copy construction");
  }

  void operator=(const SmoothUnion& a_inputIF)
  {
    MayDay::Abort("SmoothUnion doesn't allow assignment");
  }
};

#include "NamespaceFooter.H"
#endif
