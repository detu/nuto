#pragma once

#include <Eigen/Core>
#include <vector>
#include <cassert>

#define NOT_SET -1

namespace NuTo
{
//! @brief Store node values and its dof
//! @todo fix sized nodes?
class NodeCoordinates
{
public:
    //! @brief initizalizes the nodes coordintes with `values` and initializes the coordinate numbers to NOT_SET
    //! @param values inititial coordinates
    NodeCoordinates(Eigen::VectorXd values)
        : mCoordinates({values})
        , mCoordinateNumbers(Eigen::VectorXi::Constant(values.rows(), NOT_SET))
    {
    }

    //! @brief initializes a 1D coordinate node with `value` and a coordinate number NOT_SET
    //! @param value initial coordinate value
    NodeCoordinates(double value)
        : mCoordinates(Eigen::VectorXd::Constant(1, value))
        , mCoordinateNumbers(Eigen::VectorXi::Constant(1, NOT_SET))
    {
    }


    const Eigen::VectorXd& GetCoordinates() const
    {
        return mCoordinates;
    }

    int GetCoordinateNumber(int component) const
    {
        assert(component < mCoordinateNumbers.rows());
        return mCoordinateNumbers[component];
    }

    void SetCoordinates(Eigen::VectorXd coordinates)
    {
        assert(coordinates.size() == mCoordinates.size());
        mCoordinates = coordinates;
    }

    void SetCoordinate(int component, double value)
    {
        assert(component < mCoordinateNumbers.rows());
        mCoordinates[component] = value;
    }

    void SetCoordinateNumber(int component, int coordinateNumber)
    {
        assert(component < mCoordinateNumbers.rows());
        mCoordinateNumbers[component] = coordinateNumber;
    }

    int GetNumCoordinates() const
    {
        return mCoordinates.rows();
    }

private:
    Eigen::VectorXd mCoordinates;
    Eigen::VectorXi mCoordinateNumbers;
};
} /* NuTo */

#undef NOT_SET