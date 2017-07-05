#include "mechanics/constitutive/ConstitutiveBase.h"
#include "mechanics/constitutive/ConstitutiveEnum.h"

#include <iostream>

#include "base/Logger.h"
#include "mechanics/MechanicsException.h"

//! @brief ... checks if the constitutive law has a specific parameter
//! @param rIdentifier ... Enum to identify the requested parameter
//! @return ... true/false
bool NuTo::ConstitutiveBase::CheckHaveParameter(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    throw NuTo::MechanicsException(
            "[NuTo::ConstitutiveBase::CheckHaveParameter] Not implemented for this constitutive law.");
}

//! @brief ... gets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @return ... value of the requested variable
bool NuTo::ConstitutiveBase::GetParameterBool(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    throw NuTo::MechanicsException(
            "[NuTo::ConstitutiveBase::GetParameterBool] This constitutive law has no variables of type bool.");
}

//! @brief ... sets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetParameterBool(NuTo::Constitutive::eConstitutiveParameter rIdentifier, bool rValue)
{
    throw NuTo::MechanicsException(
            "[NuTo::ConstitutiveBase::GetParameterBool] This constitutive law has no variables of type bool.");
}

//! @brief ... gets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @return ... value of the requested variable
double NuTo::ConstitutiveBase::GetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier) const
{
    throw NuTo::MechanicsException(
            "[NuTo::ConstitutiveBase::GetParameterDouble] This constitutive law has no variables of type double.");
}

//! @brief ... sets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetParameterDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier, double rValue)
{
    throw NuTo::MechanicsException(
            "[NuTo::ConstitutiveBase::SetParameterDouble] This constitutive law has no variables of type double.");
}

//! @brief ... sets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetParameterFunction(std::function<std::array<double, 2>(double)>)
{
    throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "This constitutive law has no variables of type double.");
}

void NuTo::ConstitutiveBase::SetDamageLaw(std::shared_ptr<NuTo::Constitutive::DamageLaw> damageLaw)
{
    throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "This constitutive law has no damage law.");
}

//! @brief ... gets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @return ... value of the requested variable
Eigen::VectorXd
NuTo::ConstitutiveBase::GetParameterFullVectorDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetParameterFullVectorDouble] This constitutive law has "
                                   "no variables of type Eigen::VectorXd.");
}

//! @brief ... sets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetParameterFullVectorDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier,
                                                          Eigen::VectorXd rValue)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetParameterFullVectorDouble] This constitutive law has "
                                   "no variables of type Eigen::VectorXd.");
}

//! @brief ... gets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @return ... value of the requested variable
Eigen::MatrixXd
NuTo::ConstitutiveBase::GetParameterMatrixDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetParameterMatrixDouble] This constitutive law has no "
                                   "variables of type Eigen::MatrixXd.");
}

//! @brief ... sets a parameter of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested parameter
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetParameterMatrixDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier,
                                                      Eigen::MatrixXd rValue)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetParameterMatrixDouble] This constitutive law has no "
                                   "variables of type Eigen::MatrixXd.");
}

//! @brief checks parameters, throws if the check failed
void NuTo::ConstitutiveBase::CheckParameterDouble(Constitutive::eConstitutiveParameter rIdentifier, double rValue)
{
    switch (rIdentifier)
    {
    case Constitutive::eConstitutiveParameter::NONLOCAL_RADIUS:
    case Constitutive::eConstitutiveParameter::TENSILE_STRENGTH:
    case Constitutive::eConstitutiveParameter::COMPRESSIVE_STRENGTH:
    case Constitutive::eConstitutiveParameter::FRACTURE_ENERGY:
    case Constitutive::eConstitutiveParameter::YOUNGS_MODULUS:
    case Constitutive::eConstitutiveParameter::DENSITY:
    case Constitutive::eConstitutiveParameter::HEAT_CAPACITY:
    case Constitutive::eConstitutiveParameter::THERMAL_CONDUCTIVITY:
    {
        if (rValue < 0.)
            throw NuTo::MechanicsException(__PRETTY_FUNCTION__,
                                           Constitutive::ConstitutiveParameterToString(rIdentifier) +
                                                   " must be > 0. (value: " + std::to_string(rValue) + ").");
    }
    break;
    case Constitutive::eConstitutiveParameter::POISSONS_RATIO:
    {
        if (rValue <= -1.0)
            throw NuTo::MechanicsException(__PRETTY_FUNCTION__,
                                           "Poisson's ratio must be greater or equal to -1.0 (value: " +
                                                   std::to_string(rValue) + ").");
        if (rValue >= 0.5)
            throw NuTo::MechanicsException(__PRETTY_FUNCTION__,
                                           "Poisson's ratio must be smaller or equal to 0.5 (value: " +
                                                   std::to_string(rValue) + ").");
    }
    break;

    case Constitutive::eConstitutiveParameter::THERMAL_EXPANSION_COEFFICIENT:
        break;

    default:
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "material parameter check not implemented.");
    }
}


//! @brief ... gets the equilibrium water volume fraction depend on the relative humidity
//! @param rRelativeHumidity ... relative humidity
//! @return ... equilibrium water volume fraction
double NuTo::ConstitutiveBase::GetEquilibriumWaterVolumeFraction(double rRelativeHumidity,
                                                                 Eigen::VectorXd rCoeffs) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetEquilibriumWaterVolumeFraction] The constitutive "
                                   "relationship does not have this parameter.");
}


//! @brief ... checks if a constitutive law has an specific output
//! @return ... true/false
bool NuTo::ConstitutiveBase::CheckOutputTypeCompatibility(Constitutive::eOutput rOutputEnum) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::CheckOutputTypeCompatibility] Function not implemented "
                                   "for this constitutive law.");
}

// modify parameter validity flag
void NuTo::ConstitutiveBase::SetParametersValid()
{
    try
    {
        this->CheckParameters();
    }
    catch (NuTo::MechanicsException& e)
    {
        this->mParametersValid = false;
        return;
    }
    catch (...)
    {
        throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetParametersValid] Unhandled exception");
    }
    this->mParametersValid = true;
}


// info routine
void NuTo::ConstitutiveBase::Info(unsigned short rVerboseLevel, Logger& rLogger) const
{
    std::cout << "    parameter validity flag: " << this->mParametersValid << std::endl;
}
