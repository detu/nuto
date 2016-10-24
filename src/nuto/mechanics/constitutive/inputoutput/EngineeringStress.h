#pragma once

#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveVector.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutivePlaneState.h"


namespace NuTo
{

//! @brief ... Engineering stress
/*!
 *  3D case:
 *  \f[
 *     \boldsymbol{\sigma} = \begin{bmatrix}
 *        \sigma_{x}\\
 *        \sigma_{y}\\
 *        \sigma_{z}\\
 *        \tau_{yz}\\
 *        \tau_{zx}\\
 *        \tau_{xy}
 *    \end{bmatrix} = \begin{bmatrix}
 *        \sigma_{xx}\\
 *        \sigma_{yy}\\
 *        \sigma_{zz}\\
 *        \sigma_{yz}\\
 *        \sigma_{zx}\\
 *        \sigma_{xy}
 *    \end{bmatrix}.
 *  \f]
 */
template <int TDim>
class EngineeringStress: public ConstitutiveVector<ConstitutiveIOBase::GetVoigtDim(TDim)>
{
public:

    EngineeringStress<3> As3D(ePlaneState rPlaneState = ePlaneState::PLANE_STRESS) const;

    double GetVonMisesStress(ePlaneState rPlaneState = ePlaneState::PLANE_STRESS) const;

};

} /* namespace NuTo */

