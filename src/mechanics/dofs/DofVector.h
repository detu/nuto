#pragma once

#include "mechanics/dofs/DofCalcContainer.h"

namespace NuTo
{
template <typename T>
using DofVector = DofCalcContainer<Eigen::Matrix<T, Eigen::Dynamic, 1>>;

template <typename T>
inline int TotalRows(const DofVector<T>& v, const std::vector<DofType>& dofs)
{
    int totalRows = 0;
    for (auto dof : dofs)
        totalRows += v[dof].rows();
    return totalRows;
}

//! @brief export the dofs entries of a DofVector to a Eigen::VectorXT
//! @tparam T numeric type
//! @param v dof vector to export
//! @param dofs dof types to export
//! @return continuous vector containing the combined subvectors of v
template <typename T>
inline Eigen::Matrix<T, Eigen::Dynamic, 1> ToEigen(const DofVector<T>& v, std::vector<DofType> dofs)
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> combined(TotalRows(v, dofs));

    int currentStartRow = 0;
    for (auto dof : dofs)
    {
        const int rows = v[dof].rows();
        combined.segment(currentStartRow, rows) = v[dof];
        currentStartRow += rows;
    }
    return combined;
}

//! @brief imports a values into a properly sized DofVector
//! @param rDestination properly sized dof vector
//! @param source eigen vector whose values are imported
//! @param dofs dof types to import
template <typename T>
inline void FromEigen(DofVector<T>& rDesination, const Eigen::Matrix<T, Eigen::Dynamic, 1>& source,
                      std::vector<DofType> dofs)
{
    assert(TotalRows(rDesination, dofs) == source.rows());
    int currentStartRow = 0;
    for (auto dof : dofs)
    {
        const int rows = rDesination[dof].rows();
        rDesination[dof] = source.segment(currentStartRow, rows);
        currentStartRow += rows;
    }
}

} /* NuTo */
