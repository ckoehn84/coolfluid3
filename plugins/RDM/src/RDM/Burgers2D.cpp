// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "RDM/Burgers2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Burgers2D::Burgers2D()
{
}

/////////////////////////////////////////////////////////////////////////////////////

Burgers2D::~Burgers2D()
{
}

/////////////////////////////////////////////////////////////////////////////////////

Real Burgers2D::Lu(const RealVector2 & point, const RealVector2 & grad, const Real & u)
{  
   return u*grad[XX] + grad[YY];
}


} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
