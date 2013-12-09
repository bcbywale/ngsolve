/*********************************************************************/
/* File:   l2hofe.cpp                                                */
/* Author: Start                                                     */
/* Date:   6. Feb. 2003                                              */
/*********************************************************************/

#define FILE_L2HOFE_CPP


#include <fem.hpp>
#include "l2hofe.hpp"

#include <l2hofe_impl.hpp>
#include <tscalarfe_impl.hpp>


namespace ngfem
{


  // sollten eigentlich rein ...
  template <> inline void L2HighOrderFE<ET_POINT> :: 
  GetDiagMassMatrix (FlatVector<> mass) const
  {
    mass(0) = 1;
  }

  template <> inline void L2HighOrderFE<ET_SEGM> :: 
  GetDiagMassMatrix (FlatVector<> mass) const
  {
    for (int ix = 0; ix <= order; ix++)
      mass(ix) = 1.0 / (2*ix+1);
  }

  template <> inline void L2HighOrderFE<ET_TRIG> :: 
  GetDiagMassMatrix (FlatVector<> mass) const
  {
    for (int ix = 0, ii = 0; ix <= order; ix++)
      for (int iy = 0; iy <= order-ix; iy++, ii++)
        mass(ii) = 1.0 / ( (2*iy+1) * (2*ix+2*iy+2));
  }

  
} // namespace







