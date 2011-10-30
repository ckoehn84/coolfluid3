// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/RegistLibrary.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/WriteMesh.hpp"

namespace cf3 {
namespace mesh {

using namespace cf3::common;

cf3::common::RegistLibrary<LibMesh> libMesh;

////////////////////////////////////////////////////////////////////////////////

LibMesh::~LibMesh()
{
  if(m_is_initiated)
    terminate_impl();
}


void LibMesh::initiate()
{
  if(m_is_initiated)
    return;
  
  initiate_impl();
  m_is_initiated = true;
}


void LibMesh::terminate()
{
  if(!m_is_initiated)
    return;
  
  terminate_impl();
  m_is_initiated = false;
}


void LibMesh::initiate_impl()
{
  Core::instance().tools().create_component_ptr<mesh::LoadMesh>( "LoadMesh" )
      ->mark_basic();

  Core::instance().tools().create_component_ptr<mesh::WriteMesh>( "WriteMesh" )
      ->mark_basic();
}

void LibMesh::terminate_impl()
{
  Core::instance().tools().remove_component("LoadMesh");
  Core::instance().tools().remove_component("WriteMesh");
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
