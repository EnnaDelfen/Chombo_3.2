#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#ifndef _MOMENTITERATOR_H_
#define _MOMENTITERATOR_H_

#include "IndexedMoments.H"

#include "NamespaceHeader.H"

///class to enable template containment
template <int D>
class BaseMomentIterator
{
public:
  ///
  /**
  */
  BaseMomentIterator()
  {
  }

  ///
  virtual ~BaseMomentIterator()
  {
  }
  
  ///
  virtual void reset() = 0;

  ///
  virtual void operator++()= 0;

  ///
  virtual const IndexTM<int, D>& operator () () const = 0;

  ///
  virtual bool ok() = 0;

  virtual int size() const = 0;
  
};

///iterates through the indices of a IndexedMoment
/**
   MomentIterator iterates through the multi-indices of a IndexedMoments.
*/
template <int D, int P>
class MomentIterator: public BaseMomentIterator<D>
{
public:
  ///
  /**
  */
  MomentIterator()
  {
    m_indicies = &(IndexedMoments<D,P>::getMonomialPowers());
    m_current = 0;
  }

  ///
  virtual ~MomentIterator()
  {
  }
  
  ///
  virtual void reset()
  {
    m_current = 0;
  }

  ///
  virtual void operator ++ ()
  {
    m_current++;
  }

  ///
  /**
     Returns the moment of the current multi-index of this MomentIterator.
  */
  virtual const IndexTM<int, D>& operator () () const
  {
    return (*m_indicies)[m_current];
  }

  ///
  /**
     Returns true if this MomentIterator's location is within its IndexedMoment.
  */
  virtual bool ok()
  {
    bool retval = m_current < m_indicies->size();
    return retval;
  }

  virtual int size() const
  {
    return m_indicies->size();
  }
protected:
  

  /// points to the (static) powers that live in IndexedMoments
  const Vector<IndexTM<int,D> >* m_indicies;

  ///
  int m_current;


};

#include "NamespaceFooter.H"
#endif
