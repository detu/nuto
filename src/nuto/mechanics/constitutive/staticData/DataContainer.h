//
// Created by Thomas Titscher on 10/20/16.
//
#pragma once
#include <vector>
#include "nuto/mechanics/MechanicsException.h"

namespace NuTo
{
namespace Constitutive
{
namespace StaticData
{

//! @brief Wrapper for a StaticDataType container
template <typename TLaw>
class DataContainer
{
public:

    typedef typename TLaw::StaticDataType Type;

    //! @brief Set a new value for the current static data.
    //! @param rNewData New value for current static data.
    void SetData(const Type& rNewData)
    {
        if (mData.empty())
            mData.push_back(rNewData);
        else
            mData.at(0) = rNewData;
    }

    //! @brief Get the data at `timestep`.
    //! @param rTimeStep Timestep at which to retrieve the data. The timestep is optional and defaults to 0, that is,
    //!                 the current time step.
    Type& GetData(int rTimeStep = 0)
    {
        if (rTimeStep > GetNumData() - 1)
            throw MechanicsException(__PRETTY_FUNCTION__, "You requested time step " + std::to_string(rTimeStep) + ". Number of allocated time steps: " + std::to_string(GetNumData()));
        return mData.at(rTimeStep);
    }

    //! @brief Copies the "current" ([0]) static data `rNumAdditionalData` times.
    void AllocateAdditionalData(int rNumAdditionalData)
    {
        if (mData.empty())
            throw MechanicsException(__PRETTY_FUNCTION__, "No static data allocated yet.");

        for (int i = 0; i < rNumAdditionalData; ++i)
        {
            mData.push_back(mData[0]);
        }
    }

    //! @brief Puts current static data to previous static data, previous to pre-previous, etc.
    //! The current data is copied to the previous data, all others are moved.
    void ShiftToPast()
    {
        if (GetNumData() < 2)
            throw MechanicsException(__PRETTY_FUNCTION__, "There need to be at least two time steps allocated.");

        mData.pop_back();
        mData.insert(mData.begin(), mData[0]);
    }

    //! @brief Puts previous static data to current static data, pre-previous to previous, etc.
    void ShiftToFuture()
    {
        if (GetNumData() < 2)
            throw MechanicsException(__PRETTY_FUNCTION__, "There need to be at least two time steps allocated.");

        mData.erase(mData.begin());
        mData.push_back(mData.back());
    }

    //! @brief Returns the total number of static data sets.
    int GetNumData() const
    {
        return mData.size();
    }

    //! @brief defines the serialization of this class
    //! @param rStream serialize output stream
    void NuToSerializeSave(SerializeStreamOut& rStream)
    {
        SerializeDataContainer(rStream);
    };

    //! @brief defines the serialization of this class
    //! @param rStream serialize input stream
    void NuToSerializeLoad(SerializeStreamIn& rStream)
    {
        SerializeDataContainer(rStream);
    };

private:
    //! @brief defines the serialization of this class
    //! @param rStream serialize input/output stream
    template <typename TStream>
    void SerializeDataContainer(TStream &rStream)
    {
        for (Type& data : mData)
            rStream.Serialize(data);
    }


    std::vector<Type> mData;
};

}   // namespace StaticData
}   // namespace Constitutive
}   // namespace NuTo

