// $Id$
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/math/FullMatrix.h"
#include <iostream>

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/base/Logger.h"
#include "nuto/mechanics/MechanicsException.h"

// constructor
NuTo::ConstitutiveBase::ConstitutiveBase()
{
    this->mParametersValid = false;
}

//! @brief ... evaluate the constitutive relation in 1D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::ConstitutiveBase::Evaluate1D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const NuTo::ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, NuTo::ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	std::cout << "[ConstitutiveBase::Evaluate1D] make this function pure virtual." << std::endl;
	throw std::string("[ConstitutiveBase::Evaluate] make this function pure virtual.");
	return Error::SUCCESSFUL;
}

//! @brief ... evaluate the constitutive relation in 2D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::ConstitutiveBase::Evaluate2D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const NuTo::ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, NuTo::ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	std::cout << "[ConstitutiveBase::Evaluate1D] make this function pure virtual." << std::endl;
	throw std::string("[ConstitutiveBase::Evaluate] make this function pure virtual.");
	return Error::SUCCESSFUL;
}

//! @brief ... evaluate the constitutive relation in 3D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::ConstitutiveBase::Evaluate3D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::Input::eInput, const NuTo::ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::Output::eOutput, NuTo::ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	std::cout << "[ConstitutiveBase::Evaluate3D] make this function pure virtual." << std::endl;
	throw std::string("[ConstitutiveBase::Evaluate] make this function pure virtual.");
    return Error::SUCCESSFUL;
}

//! @brief ... gets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @return ... value of the requested variable
bool NuTo::ConstitutiveBase::GetVariableBool(NuTo::Constitutive::eConstitutiveVariable rIdentifier) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetVariableBool] This constitutive law has no variables of type bool.");
}

//! @brief ... sets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetVariableBool(NuTo::Constitutive::eConstitutiveVariable rIdentifier, bool rValue)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetVariableBool] This constitutive law has no variables of type bool.");
}

//! @brief ... gets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @return ... value of the requested variable
double NuTo::ConstitutiveBase::GetVariableDouble(Constitutive::eConstitutiveVariable rIdentifier) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetVariableDouble] This constitutive law has no variables of type double.");
}

//! @brief ... sets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetVariableDouble(NuTo::Constitutive::eConstitutiveVariable rIdentifier, double rValue)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetVariableDouble] This constitutive law has no variables of type double.");
}

//! @brief ... gets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @return ... value of the requested variable
NuTo::FullVector<double, Eigen::Dynamic> NuTo::ConstitutiveBase::GetVariableFullVectorDouble(NuTo::Constitutive::eConstitutiveVariable rIdentifier) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetVariableFullVectorDouble] This constitutive law has no variables of type NuTo::FullVector<double, Eigen::Dynamic>.");
}

//! @brief ... sets a variable of the constitutive law which is selected by an enum
//! @param rIdentifier ... Enum to identify the requested variable
//! @param rValue ... new value for requested variable
void NuTo::ConstitutiveBase::SetVariableFullVectorDouble(NuTo::Constitutive::eConstitutiveVariable rIdentifier, NuTo::FullVector<double, Eigen::Dynamic> rValue)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetVariableFullVectorDouble] This constitutive law has no variables of type NuTo::FullVector<double, Eigen::Dynamic>.");
}

// set density
void NuTo::ConstitutiveBase::SetDensity(double rRho)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetDensity] The constitutive relationship does not have a parameter density.");
}

// get density
double NuTo::ConstitutiveBase::GetDensity() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetDensity] The constitutive relationship does not have a parameter density.");
}


// set Young's modulus
void NuTo::ConstitutiveBase::SetYoungsModulus(double rE)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetYoungsModulus] The constitutive relationship does not have a parameter Young's modulus.");
}

// get Young's modulus
double NuTo::ConstitutiveBase::GetYoungsModulus() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYoungsModulus] The constitutive relationship does not have a parameter Young's modulus.");
}

//! @brief ... get thermal expansion coefficient
//! @return ... thermal expansion coefficient
double NuTo::ConstitutiveBase::GetThermalExpansionCoefficient() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetThermalExpansionCoefficient] The constitutive relationship does not have a thermal expansion coefficient.");
}

//! @brief ... set thermal expansion coefficient
//! @param rAlpha ... thermal expansion coefficient
void NuTo::ConstitutiveBase::SetThermalExpansionCoefficient(double rAlpha)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetThermalExpansionCoefficient] The constitutive relationship does not have a thermal expansion coefficient.");
}

//! @brief ... get factor to modify Young's modulus (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorYoungsModulus(const ElementBase* rElement,int rIp) const
{
    return 1;
}

// set Poisson's ratio
void NuTo::ConstitutiveBase::SetPoissonsRatio(double rNu)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetPoissonsRatio] The constitutive relationship does not have a parameter Poisson's ratio.");
}

// get Poisson's ratio
double NuTo::ConstitutiveBase::GetPoissonsRatio() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetPoissonsRatio] The constitutive relationship does not have a parameter Poisson's ratio.");
}

//! @brief ... get factor to modify Poisson's ratio (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorPoissonsRatio(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get initial yield strength
//! @return ... yield strength
double NuTo::ConstitutiveBase::GetInitialYieldStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetInitialYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... set initial yield strength
//! @param rSigma ...  yield strength
void NuTo::ConstitutiveBase::SetInitialYieldStrength(double rSigma)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetInitialYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get yield strength for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding yield strength
NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> NuTo::ConstitutiveBase::GetYieldStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... add yield strength
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  yield strength
void NuTo::ConstitutiveBase::AddYieldStrength(double rEpsilon, double rSigma)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AddYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get factor to modify yield strength (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorYieldStrength(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get initial hardening modulus
//! @return ... hardening modulus
double NuTo::ConstitutiveBase::GetInitialHardeningModulus() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetHardeningModulus] The constitutive relationship does not have a parameter hardening modulus.");
}

//! @brief ... set initial hardening modulus
//! @param rH ...  hardening modulus
void NuTo::ConstitutiveBase::SetInitialHardeningModulus(double rH)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetHardeningModulus] The constitutive relationship does not have a parameter hardening modulus.");
}

//! @brief ... get hardening value
//! @return ... hardening value
double NuTo::ConstitutiveBase::GetHardeningValue() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetHardeningValue] The constitutive relationship does not have a parameter hardening value.");
}

//! @brief ... set hardening value
//! @param rH ...  hardening value
void NuTo::ConstitutiveBase::SetHardeningValue(double rHardening)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetHardeningValue] The constitutive relationship does not have a parameter hardening value.");
}

//! @brief ... get hardening exponent
//! @return ... hardening exponent
double NuTo::ConstitutiveBase::GetHardeningExponent() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetHardeningExponent] The constitutive relationship does not have a parameter hardening exponent.");
}

//! @brief ... set hardening exponent
//! @param rHardeningExponent ...  hardening exponent
void NuTo::ConstitutiveBase::SetHardeningExponent(double rHardeningExponent)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetHardeningExponent] The constitutive relationship does not have a parameter hardening exponent.");
}

//! @brief ... get hardening modulus for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding hardening modulus
NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> NuTo::ConstitutiveBase::GetHardeningModulus() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... add hardening modulus
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  hardening modulus
void NuTo::ConstitutiveBase::AddHardeningModulus(double rEpsilon, double rH)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AddYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get factor to modify hardening modulus (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorHardeningModulus(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get nonlocal radius
//! @return ... nonlocal radius
double NuTo::ConstitutiveBase::GetNonlocalRadius() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetNonlocalRadius] The constitutive relationship does not have a nonlocal radius.");
}

//! @brief ... set nonlocal radius
//! @param rRadius ...  nonlocal radius
void NuTo::ConstitutiveBase::SetNonlocalRadius(double rRadius)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetNonlocalRadius] The constitutive relationship does not have a nonlocal radius.");
}

//! @brief ... get nonlocal radius parameter
//! @return ... nonlocal radius parameter
double NuTo::ConstitutiveBase::GetNonlocalRadiusParameter() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetNonlocalRadiusParameter] The constitutive relationship does not have a nonlocal radius parameter.");
}

//! @brief ... set nonlocal radius parameter
//! @param rRadius ...  nonlocal radius parameter
void NuTo::ConstitutiveBase::SetNonlocalRadiusParameter(double rRadiusParameter)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetNonlocalRadiusParameter] The constitutive relationship does not have a nonlocal radius parameter.");
}

//! @brief ... get tensile strength
//! @return ... tensile strength
double NuTo::ConstitutiveBase::GetTensileStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetTensileStrength] The constitutive relationship does not have a tensile strength.");
}

//! @brief ... set tensile strength
//! @param rTensileStrength...  tensile strength
void NuTo::ConstitutiveBase::SetTensileStrength(double rTensileStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetTensileStrength] The constitutive relationship does not have a tensile strength.");
}

//! @brief ... get shear strength
//! @return ... shear strength
double NuTo::ConstitutiveBase::GetShearStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetShearStrength] The constitutive relationship does not have a shear strength.");
}

//! @brief ... set shear strength
//! @param rShearStrength...  shear strength
void NuTo::ConstitutiveBase::SetShearStrength(double rShearStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetShearStrength] The constitutive relationship does not have a shear strength.");
}

//! @brief ... get compressive strength
//! @return ... compressive strength
double NuTo::ConstitutiveBase::GetCompressiveStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetCompressiveStrength] The constitutive relationship does not have a compressive strength.");
}

//! @brief ... set compressive strength
//! @param rCompressiveStrength...  compressive strength
void NuTo::ConstitutiveBase::SetCompressiveStrength(double rCompressiveStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetCompressiveStrength] The constitutive relationship does not have a compressive strength.");
}

//! @brief ... get biaxial compressive strength
//! @return ... biaxial compressive strength
double NuTo::ConstitutiveBase::GetBiaxialCompressiveStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetBiaxialCompressiveStrength] The constitutive relationship does not have a biaxial compressive strength.");
}

//! @brief ... set biaxial compressive strength
//! @param rBiaxialCompressiveStrength...  biaxial compressive strength
void NuTo::ConstitutiveBase::SetBiaxialCompressiveStrength(double rBiaxialCompressiveStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetBiaxialCompressiveStrength] The constitutive relationship does not have a biaxial compressive strength.");
}

//! @brief ... get fracture energy
//! @return ... fracture energy
double NuTo::ConstitutiveBase::GetFractureEnergy() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetFractureEnergy] The constitutive relationship does not have a fracture energy.");
}

//! @brief ... set fracture energy
//! @param rFractureEnergy... fracture energy
void NuTo::ConstitutiveBase::SetFractureEnergy(double rFractureEnergy)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetFractureEnergy] The constitutive relationship does not have a fracture energy.");
}

//! @brief ... get friction coefficient
//! @return ... friction coefficient
double NuTo::ConstitutiveBase::GetFrictionCoefficient() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetFrictionCoefficient] The constitutive relationship does not have a friction coefficient.");
}


//! @brief ... set friction coefficient
//! @param rFrictionCoefficient... friction coefficient
void NuTo::ConstitutiveBase::SetFrictionCoefficient(double rFrictionCoefficient)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetFrictioncoefficient] The constitutive relationship does not have a friction coefficient.");
}


//! @brief ... get HeatCapacity
//! @return ... HeatCapacity
double NuTo::ConstitutiveBase::GetHeatCapacity() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetHeatCapacity] The constitutive relationship does not have this parameter.");
}

//! @brief ... set HeatCapacity
//! @param rHeatCapacity ... HeatCapacity
void NuTo::ConstitutiveBase::SetHeatCapacity(double rHeatCapacity)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetHeatCapacity] The constitutive relationship does not have this parameter.");
}

//! @brief ... get thermal conductivity
//! @return ... thermal conductivity
double NuTo::ConstitutiveBase::GetThermalConductivity() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetThermalConductivity] The constitutive relationship does not have this parameter.");
}

//! @brief ... set thermal conductivity
//! @param ... thermal conductivity
void NuTo::ConstitutiveBase::SetThermalConductivity(double rThermalConductivity)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetThermalConductivity] The constitutive relationship does not have this parameter.");
}

//! @brief ... get viscosity
//! @return ... viscosity
double NuTo::ConstitutiveBase::GetViscosity() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetViscosity] The constitutive relationship does not have this parameter.");
}

//! @brief ... set viscosity
//! @param ... viscosity
void NuTo::ConstitutiveBase::SetViscosity(double rViscosity)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetViscosity] The constitutive relationship does not have this parameter.");
}

//! @brief ... get viscosity exponent
//! @return ... viscosity exponent
double NuTo::ConstitutiveBase::GetViscosityExponent() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetViscosityExponent] The constitutive relationship does not have this parameter.");
}

//! @brief ... set viscosity exponent
//! @param ... viscosity exponent
void NuTo::ConstitutiveBase::SetViscosityExponent(double rViscosityExponent)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetViscosityExponent] The constitutive relationship does not have this parameter.");
}

//! @brief ... get damage distribution (determines the portion of damage via viscoplasticity and plasticity)
//! @return ... damage distribution
double NuTo::ConstitutiveBase::GetDamageDistribution() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetDamageDistribution] The constitutive relationship does not have this parameter.");
}

//! @brief ... set damage distribution (determines the portion of damage via viscoplasticity and plasticity)
//! @param ... damage distribution
void NuTo::ConstitutiveBase::SetDamageDistribution(double rDamageDistribution)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetDamageDistribution] The constitutive relationship does not have this parameter.");
}

//! @brief ... get damage law
//! @return ... damage law
NuTo::FullVector<double, Eigen::Dynamic> NuTo::ConstitutiveBase::GetDamageLaw() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetDamageLaw] The constitutive relationship does not have this parameter.");

}

//! @brief ... set damage law
//! @param rDamageLaw ... damage law
void NuTo::ConstitutiveBase::SetDamageLaw(const NuTo::FullVector<double, Eigen::Dynamic> rDamageLaw)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetDamageLaw] The constitutive relationship does not have this parameter.");
}


//! @brief ... get viscoplastic yield surface offset with respect to the plastic yield surface
//! @return ... viscoplastic yield surface offset
double NuTo::ConstitutiveBase::GetViscoplasticYieldSurfaceOffset() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetViscoplasticYieldSurfaceOffset] The constitutive relationship does not have this parameter.");
}

//! @brief ... set viscoplastic yield surface offset with respect to the plastic yield surface
//! @param ... viscoplastic yield surface offset
void NuTo::ConstitutiveBase::SetViscoplasticYieldSurfaceOffset(double rViscoplasticYieldSurfaceOffset)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetViscoplasticYieldSurfaceOffset] The constitutive relationship does not have this parameter.");
}

//! @brief ... get fatigue extrapolation flag
//! @param ... fatigue extrapolation flag
bool NuTo::ConstitutiveBase::GetFatigueExtrapolation() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetFatigueExtrapolation] The constitutive relationship does not have this option.");
}

//! @brief ... set fatigue extrapolation flag
//! @param ... fatigue extrapolation flag
void NuTo::ConstitutiveBase::SetFatigueExtrapolation(bool rFatigueExtrapolation)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetFatigueExtrapolation] The constitutive relationship does not have this option.");
}

//! @brief ... gets the equilibrium water volume fraction depend on the relative humidity
//! @param rRelativeHumidity ... relative humidity
//! @return ... equilibrium water volume fraction
double NuTo::ConstitutiveBase::GetEquilibriumWaterVolumeFraction(double rRelativeHumidity, NuTo::FullVector<double,Eigen::Dynamic> rCoeffs) const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetEquilibriumWaterVolumeFraction] The constitutive relationship does not have this parameter.");
}




// modify parameter validity flag
void NuTo::ConstitutiveBase::SetParametersValid()
{
    try
    {
        this->CheckParameters();
    }
    catch (NuTo::MechanicsException)
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

//! @brief ... avoid dynamic cast
//! @return ... see brief explanation
NuTo::ConstitutiveEngineeringStressStrain* NuTo::ConstitutiveBase::AsConstitutiveEngineeringStressStrain()
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AsConstitutiveEngineeringStressStrain] Constitutive Law is not of type EngineeringStressStrain.");
}

//! @brief ... avoid dynamic cast
//! @return ... see brief explanation
const NuTo::ConstitutiveEngineeringStressStrain* NuTo::ConstitutiveBase::AsConstitutiveEngineeringStressStrain()const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AsConstitutiveEngineeringStressStrain] Constitutive Law is not of type EngineeringStressStrain.");
}

//! @brief ... avoid dynamic cast
//! @return ... see brief explanation
NuTo::ConstitutiveLatticeStressStrain* NuTo::ConstitutiveBase::AsConstitutiveLatticeStressStrain()
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AsConstitutiveLatticeStressStrain] Constitutive Law is not of type LatticeStressStrain.");
}

//! @brief ... avoid dynamic cast
//! @return ... see brief explanation
const NuTo::ConstitutiveLatticeStressStrain* NuTo::ConstitutiveBase::AsConstitutiveLatticeStressStrain()const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AsConstitutiveLatticeStressStrain] Constitutive Law is not of type LatticeStressStrain.");
}


// info routine
void NuTo::ConstitutiveBase::Info(unsigned short rVerboseLevel, Logger& rLogger) const
{
    std::cout << "    parameter validity flag: " << this->mParametersValid << std::endl;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain1D(const ElementBase* rElement)const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain1D] Allocate routine for 1D EngineeringStressStrain not implemented.");
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain2D(const ElementBase* rElement)const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain2D] Allocate routine for 2D EngineeringStressStrain not implemented.");
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain3D(const ElementBase* rElement)const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AllocateStaticDataEngineeringStress_EngineeringStrain3D] Allocate routine for 3D EngineeringStressStrain not implemented.");
}


#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::ConstitutiveBase::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstitutiveBase::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize constitutive Base" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_NVP(mParametersValid);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize Constitutive Base" << std::endl;
#endif
}
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::ConstitutiveBase)
#endif // ENABLE_SERIALIZATION
