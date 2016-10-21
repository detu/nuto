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

    using Type = typename StaticData::DataContainer<TLaw>::Type;

    //! @brief constructor
    //! @param rLaw underlying constitutive law
    IPConstitutiveLaw(TLaw& rLaw, const Type& rData)
    : mLaw(rLaw), mData(rData) {}

    //! @brief default copy constructor
    IPConstitutiveLaw(const IPConstitutiveLaw& ) = default;

    //! @brief default move constructor
    IPConstitutiveLaw(      IPConstitutiveLaw&&) = default;

    //! @brief default destuctor
    ~IPConstitutiveLaw() = default;

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

protected:
    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    eError Evaluate1D(const ConstitutiveInputMap& rConstitutiveInput,
                      const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return mLaw.Evaluate<1>(rConstitutiveInput, rConstitutiveOutput, mData);
    }

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    eError Evaluate2D(const ConstitutiveInputMap& rConstitutiveInput,
                      const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return mLaw.Evaluate<2>(rConstitutiveInput, rConstitutiveOutput, mData);
    }

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    eError Evaluate3D(const ConstitutiveInputMap& rConstitutiveInput,
                      const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return mLaw.Evaluate<3>(rConstitutiveInput, rConstitutiveOutput, mData);
    }

private:

    TLaw& mLaw;
    StaticData::DataContainer<TLaw> mData;
};


} // namespace Constitutive
} // namespace NuTo