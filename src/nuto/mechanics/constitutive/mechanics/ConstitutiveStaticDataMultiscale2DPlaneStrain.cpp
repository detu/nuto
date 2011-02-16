// $Id: ConstitutiveStaticDataMultiscale2DPlaneStrain.cpp 342 2010-10-18 12:39:08Z arnold2 $
// ConstitutiveStaticDataMultiscale2DPlaneStrain.cpp
// created May 6, 2010 by Joerg F. Unger

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION


#include <cctype>
#include <algorithm>

#include "nuto/mechanics/constitutive/mechanics/ConstitutiveStaticDataMultiscale2DPlaneStrain.h"
#include "nuto/mechanics/constitutive/mechanics/ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain.h"
#include "nuto/mechanics/structures/unstructured/StructureIp.h"

//! @brief constructor
NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::ConstitutiveStaticDataMultiscale2DPlaneStrain()
   : NuTo::ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain::ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain()
{
    mStructure = 0;
    mNonlinearSolutionOn = false;
}

#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstitutiveStaticDataMultiscale2DPlaneStrain" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveStaticDataPrevEngineeringStressStrain2DPlaneStrain)
       & BOOST_SERIALIZATION_NVP(mStructure)
       & BOOST_SERIALIZATION_NVP(mNonlinearSolutionOn);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstitutiveStaticDataMultiscale2DPlaneStrain" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain)
#endif // ENABLE_SERIALIZATION

//!@ brief reinterpret as nonlocal damage2d static data
NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain* NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::AsMultiscale2DPlaneStrain()
{
    return this;
}

//!@ brief reinterpret as nonlocal damage2d static data
const NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain* NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::AsMultiscale2DPlaneStrain()const
{
    return this;
}

//! @brief return structure
NuTo::StructureIp* NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::GetFineScaleStructure()
{
    return mStructure;
}

//! @brief return structure
const NuTo::StructureIp* NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::GetFineScaleStructure()const
{
    return mStructure;
}

//! @brief sets the fine scale model (deserialization from a binary file)
void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel(std::string rFileName)
{
    // open file
    std::ifstream ifs ( rFileName.c_str(), std::ios_base::binary );
    if(! ifs.is_open())
    {
        throw MechanicsException("[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel] Error opening file for restoring the structure.");
    }

    if (mStructure!=0)
        delete mStructure;
    // if this should be implemented for 3D, the original structure has to be given in order to determine the dimension
    mStructure = new StructureIp(2);

    std::string typeIdString;

#ifdef ENABLE_SERIALIZATION
    boost::archive::binary_iarchive oba ( ifs, std::ios::binary );
    oba & boost::serialization::make_nvp ( "Object_type", typeIdString );
    if ( typeIdString != mStructure->GetTypeId() )
    {
        throw MechanicsException ( "[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel] Data type of object in file ("+typeIdString+") is not identical to data type of object to read ("+mStructure->GetTypeId() +")." );
    }
    oba & boost::serialization::make_nvp(typeIdString.c_str(), *mStructure);
    if (mStructure->GetDimension()!=2)
        throw MechanicsException ( "[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel] Read in structure should have dimension 2");
#else
    throw MechanicsException("[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel] Serialization is switch off, can't restore the IP structure.");
#endif //ENABLE_SERIALIZATION

    //transform boundary nodes to multiscale nodes
    mStructure->TransformBoundaryNodes();

    //check all the sections of the elements to a plane strain section
    std::cout<<"[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleModel] Section type check still to be implemented." << std::endl;
}

//! @brief sets the parameters of the fine scale model
void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleParameter(const std::string& rName, double rParameter)
{
    std::string upperCaseName(rName);
    std::transform(upperCaseName.begin(), upperCaseName.end(), upperCaseName.begin(), (int(*)(int)) std::toupper);
    if (upperCaseName=="CRACKTRANSITIONZONE")
        mStructure->SetCrackTransitionZone(rParameter);
    else if (upperCaseName=="PENALTYSTIFFNESSCRACKANGLE")
        mStructure->SetPenaltyStiffnessCrackAngle(rParameter);
    else if (upperCaseName=="PENALTYSTIFFNESSSCALINGFACTORCRACKANGLE")
        mStructure->SetPenaltyStiffnessScalingFactorCrackAngle(rParameter);
    else if (upperCaseName=="TOLERANCEELASTICCRACKANGLELOW")
        mStructure->SetToleranceElasticCrackAngleLow(rParameter);
    else if (upperCaseName=="TOLERANCEELASTICCRACKANGLEHIGH")
        mStructure->SetToleranceElasticCrackAngleHigh(rParameter);
    else if (upperCaseName=="CONSTRAINTPENALTYSTIFFNESSCRACKANGLE")
        mStructure->ConstraintNonlinearCrackAngle(rParameter,2.*M_PI);
    else if (upperCaseName=="AUGMENTEDLAGRANGECRACKOPENING")
        mStructure->ConstraintLagrangeCrackOpening(rParameter);
    else if (upperCaseName=="CONSTRAINTPENALTYSTIFFNESSTANGENTIALCRACKOPENING")
        mStructure->ConstraintNonlinearTangentialCrackOpening(rParameter, 0.);
    else if (upperCaseName=="PENALTYSTIFFNESSTANGENTIALCRACKOPENING")
        mStructure->SetPenaltyStiffnessTangentialCrackOpening(rParameter);
    else if (upperCaseName=="PENALTYSTIFFNESSSCALINGFACTORTANGENTIALCRACKOPENING")
        mStructure->SetPenaltyStiffnessScalingFactorTangentialCrackOpening(rParameter);
    else if (upperCaseName=="USENONLINEARSOLUTION")
        UseNonlinearSolution();
    else
        throw MechanicsException("[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::SetFineScaleParameter] Parameter " + upperCaseName + " not a valid expression.");
}

//! @brief in case the fine scale model has not been initialized,
//! an initial linear elastic model is used
//! with this routine, the transition to the actual fine scale model is used
//! with the initialization of the crack angle based on the previous elastic solution
void NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::UseNonlinearSolution()
{
    if (mNonlinearSolutionOn==false)
    {
        double alpha = mStructure->CalculateInitialCrackAngleElastic();
        mStructure->SetPrevCrackAngle(alpha);
        mStructure->SetCrackAngle(alpha);
        mNonlinearSolutionOn = true;
        std::cout << "initial alpha is " << alpha*180./M_PI << std::endl;
    }
    else
    {
        throw MechanicsException("[NuTo::ConstitutiveStaticDataMultiscale2DPlaneStrain::InitAlpha] Nonlinear solution is already turned on.");
    }
}