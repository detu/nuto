//
// Created by Thomas Titscher on 10/20/16.
//
#pragma once

#include "nuto/mechanics/constitutive/staticData/IPConstitutiveLawBase.h"
#include "nuto/mechanics/constitutive/staticData/DataContainer.h"
#include "nuto/base/serializeStream/SerializeStreamIn.h"
#include "nuto/base/serializeStream/SerializeStreamOut.h"


namespace NuTo
{
namespace Constitutive
{

template<typename TLaw>
class IPConstitutiveLaw: public IPConstitutiveLawBase
{
public:

    //! @brief constructor
    //! @param rLaw underlying constitutive law
    IPConstitutiveLaw(TLaw& rLaw)
    : mLaw(rLaw) {}

    //! @brief default copy constructor
    IPConstitutiveLaw(const IPConstitutiveLaw& ) = default;

    //! @brief default move constructor
    IPConstitutiveLaw(      IPConstitutiveLaw&&) = default;

    //! @brief default destuctor
    ~IPConstitutiveLaw() = default;


    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    eError Evaluate1D(const ConstitutiveInputMap& rConstitutiveInput,
                      const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return mLaw.Evaluate1D(rConstitutiveInput, rConstitutiveOutput, mData);
    }

    TLaw& GetConstitutiveLaw() const
    {
        return mLaw;
    }

    const StaticData::DataContainer<TLaw>& GetStaticData() const
    {
        return mData;
    }

    StaticData::DataContainer<TLaw>& GetStaticData()
    {
        return mData;
    }

    //! @brief defines the serialization of this class
    //! @param rStream serialize output stream
    virtual void NuToSerializeSave(SerializeStreamOut& rStream) override
    {
        IPConstitutiveLawBase::NuToSerializeSave(rStream);
        rStream.Serialize(mData);
    }

    //! @brief defines the serialization of this class
    //! @param rStream serialize input stream
    virtual void NuToSerializeLoad(SerializeStreamIn& rStream) override
    {
        IPConstitutiveLawBase::NuToSerializeLoad(rStream);
        rStream.Serialize(mData);
    }


private:

    StaticData::DataContainer<TLaw> mData;
    TLaw& mLaw;
};


} // namespace Constitutive
} // namespace NuTo