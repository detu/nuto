// $Id: ConstitutiveStaticDataNonlocalDamagePlasticity3D.h 87 2009-11-06 10:35:39Z unger3 $

#ifndef CONSTITUTIVESTATICDATANONLOCALDAMAGEPLASTICITY2DPLANESTRAIN_H
#define CONSTITUTIVESTATICDATANONLOCALDAMAGEPLASTICITY2DPLANESTRAIN_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/mechanics/constitutive/mechanics/ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain.h"

//! @brief ... base class, storing the static data (history variables) of a constitutive relationship
//! @author Jörg F. Unger, ISM
//! @date December 2009
namespace NuTo
{

class ConstitutiveStaticDataNonlocalDamagePlasticity2DPlaneStrain : public ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
    friend class NonlocalDamagePlasticity;
public:
	//! @brief constructor
    ConstitutiveStaticDataNonlocalDamagePlasticity2DPlaneStrain();

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:
    //! @brief accumulated plastic strain scaled by nonlocal radius and a factor considering the fracture energy
    double mKappa;

    //! @brief nonlocal damage variable
    double mOmega;

    //! @brief plastic strain
    double mEpsilonP[4];

    //! @brief derivative of local plastic strain with respect to local total strain (row wise storage 4x4)
    double mDEpsilonPdEpsilon[16];
};

}

#endif // CONSTITUTIVESTATICDATANONLOCALDAMAGEPLASTICITY3D_H