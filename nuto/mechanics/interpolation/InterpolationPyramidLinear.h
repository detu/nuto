#pragma once
#include "nuto/mechanics/interpolation/InterpolationSimple.h"
#include "nuto/math/shapes/Pyramid.h"

namespace NuTo
{
class InterpolationPyramidLinear : public InterpolationSimple
{
public:
    std::unique_ptr<InterpolationSimple> Clone() const override
    {
        return std::make_unique<InterpolationPyramidLinear>(*this);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static Eigen::Matrix<double, 3, 1> NodeCoordinatesPyramidOrder1(int rNodeIndex);

    static Eigen::Matrix<double, 5, 1> ShapeFunctionsPyramidOrder1(const Eigen::VectorXd& rCoordinates);

    static Eigen::Matrix<double, 5, 3> DerivativeShapeFunctionsPyramidOrder1(const Eigen::VectorXd& rCoordinates);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ShapeFunctions GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctionsPyramidOrder1(naturalIpCoords);
    }

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return DerivativeShapeFunctionsPyramidOrder1(naturalIpCoords);
    }

    NaturalCoords GetLocalCoords(int nodeId) const override
    {
        return NodeCoordinatesPyramidOrder1(nodeId);
    }

    int GetNumNodes() const override
    {
        return 5;
    }

    const Shape& GetShape() const override
    {
        return mShape;
    }

private:
    Pyramid mShape;
};
} /* NuTo */
