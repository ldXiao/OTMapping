// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2016 Michael Rabinovich
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef SLIM_H
#define SLIM_H

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <string>

#include <igl/jet.h>
#include <igl/readOBJ.h>
#include <igl/facet_components.h>
#include <igl/slice.h>


namespace bcclean{
namespace SLIM
{
class WeightedGlobalLocal;
// Compute a SLIM map as derived in "Scalable Locally Injective Maps" [Rabinovich et al. 2016].
struct SLIMData {

  // Input
  Eigen::MatrixXd V; // #V by 3 list of mesh vertex positions
  Eigen::MatrixXi F; // #F by 3/3 list of mesh faces (triangles/tets)
  enum SLIM_ENERGY {
    ARAP,
    LOG_ARAP,
    SYMMETRIC_DIRICHLET,
    CONFORMAL,
    EXP_CONFORMAL,
    EXP_SYMMETRIC_DIRICHLET
  };
  SLIM_ENERGY slim_energy;

  // Optional Input
    // soft constraints
    Eigen::VectorXi b;
    Eigen::MatrixXd bc;
    double soft_const_p;
    // whether it is hard constraint
    bool is_hard_const;

  double exp_factor; // used for exponential energies, ignored otherwise
  bool mesh_improvement_3d; // only supported for 3d

  // Output
  Eigen::MatrixXd V_o; // #V by dim list of mesh vertex positions (dim = 2 for parametrization, 3 otherwise)
  double energy; // objective value

  // INTERNAL
  Eigen::VectorXd M;
  double mesh_area;
  double avg_edge_length;
  int v_num;
  int f_num;
  double proximal_p;

  WeightedGlobalLocal* wGlobalLocal;
};

  // Compute necessary information to start using SLIM
  // Inputs:
  //		V           #V by 3 list of mesh vertex positions
  //		F           #F by 3/3 list of mesh faces (triangles/tets)
  //    b           list of boundary indices into V
  //    bc          #b by dim list of boundary conditions
  //    soft_p      Soft penalty factor (can be zero)
  //    is_hard_const  whether the constraints are hard
  //    slim_energy Energy to minimize
    void slim_precompute(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& V_init, SLIMData& data,
   SLIMData::SLIM_ENERGY slim_energy, Eigen::VectorXi& b, Eigen::MatrixXd& bc, double soft_p, bool is_hard_const);

// Run iter_num iterations of SLIM
// Outputs:
//    V_o (in SLIMData): #V by dim list of mesh vertex positions
    void slim_solve(SLIMData& data, int iter_num);

}
}


#endif // SLIM_H
