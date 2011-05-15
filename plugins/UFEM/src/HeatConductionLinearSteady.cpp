// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"

#include "Solver/CEigenLSS.hpp"

#include "HeatConductionLinearSteady.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

CF::Common::ComponentBuilder < UFEM::HeatConductionLinearSteady, LinearSystem, LibUFEM > aHeatConductionLinearSteady_Builder;

HeatConductionLinearSteady::HeatConductionLinearSteady(const std::string& name) : LinearSystem(name)
{
}

void HeatConductionLinearSteady::add_actions()
{
  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  MeshTerm<1, ScalarField> heat("Heat", "q");
  
  StoredReference<Real> k = add_configurable_constant("k", "Thermal conductivity (J/(mK))", 1.);

  append_elements_action
  (
    "HeatEquation",
    ASSEMBLY,
    group <<
    (
      
      _A = _0, _T = _0,
      element_quadrature <<
      (
        _A(temperature) += k * transpose(nabla(temperature)) * nabla(temperature),
        _T(temperature) += transpose(N(temperature))*N(temperature)
      ),
      system_matrix( lss() ) +=  _A,
      system_rhs( lss() ) += _T * nodal_values(heat)
    )
  );
}

} // UFEM
} // CF
