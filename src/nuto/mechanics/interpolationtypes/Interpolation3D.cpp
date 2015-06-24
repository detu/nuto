/*
 * InterpolationType3D.cpp
 *
 *  Created on: 19 May 2015
 *      Author: ttitsche
 */

#include "nuto/mechanics/interpolationtypes/Interpolation3D.h"

NuTo::Interpolation3D::Interpolation3D(NuTo::Node::eAttributes rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder) :
        InterpolationBase::InterpolationBase(rDofType, rTypeOrder)
{

}

int NuTo::Interpolation3D::GetNumDofsPerNode() const
{
    switch (mDofType)
    {
    case NuTo::Node::COORDINATES:
        return 3;
    case NuTo::Node::DISPLACEMENTS:
        return 3;
    case NuTo::Node::TEMPERATURES:
        return 1;
    case NuTo::Node::NONLOCALEQSTRAIN:
        return 1;
    default:
        throw NuTo::MechanicsException("[NuTo::Interpolation3D::GetNumDofsPerNode] dof type not found.");
    }
}