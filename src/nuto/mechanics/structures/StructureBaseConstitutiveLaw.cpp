// $Id$

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/mechanics/LinearElastic.h"
#include "nuto/mechanics/constitutive/mechanics/ConstitutiveMisesPlasticity.h"
#include "nuto/mechanics/structures/StructureBase.h"

// create a new constitutive law
int NuTo::StructureBase::ConstitutiveLawCreate(const std::string& rType)
{
    // convert section type string to upper case
    std::string ConstitutiveLawTypeString;
    std::transform(rType.begin(), rType.end(), std::back_inserter(ConstitutiveLawTypeString), (int(*)(int)) toupper);

    // get section type from string
    ConstitutiveBase::eConstitutiveType ConstitutiveLawType;
    if (ConstitutiveLawTypeString == "LINEARELASTIC")
    {
        ConstitutiveLawType = ConstitutiveBase::LINEAR_ELASTIC;
    }
    else if (ConstitutiveLawTypeString == "MISESPLASTICITY")
    {
        ConstitutiveLawType = ConstitutiveBase::MISES_PLASTICITY;
    }
    else
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawCreate] invalid type of constitutive law.");
    }

	//find unused integer id
	int constitutiveNumber(mConstitutiveLawMap.size());
	boost::ptr_map<int,ConstitutiveBase>::iterator it = mConstitutiveLawMap.find(constitutiveNumber);
	while (it!=mConstitutiveLawMap.end())
	{
		constitutiveNumber++;
		it = mConstitutiveLawMap.find(constitutiveNumber);
	}

    this->ConstitutiveLawCreate(constitutiveNumber, ConstitutiveLawType);
    return constitutiveNumber;
}

// create a new constitutive law
void NuTo::StructureBase::ConstitutiveLawCreate(int rIdent, ConstitutiveBase::eConstitutiveType rType)
{
    // check if constitutive law identifier exists
    boost::ptr_map<int,ConstitutiveBase>::iterator it = this->mConstitutiveLawMap.find(rIdent);
    if (it == this->mConstitutiveLawMap.end())
    {
        // create new constitutive law
        ConstitutiveBase* ConstitutiveLawPtr;
        switch (rType)
        {
        case NuTo::ConstitutiveBase::LINEAR_ELASTIC:
            ConstitutiveLawPtr = new NuTo::LinearElastic();
            break;
        case NuTo::ConstitutiveBase::MISES_PLASTICITY:
            ConstitutiveLawPtr = new NuTo::ConstitutiveMisesPlasticity();
            break;
         default:
            throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawCreate] invalid type of constitutive law.");
        }

        // add section to map (insert does not allow const keys!!!!)
        this->mConstitutiveLawMap.insert(rIdent, ConstitutiveLawPtr);
    }
    else
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawCreate] Constitutive law already exists.");
    }
}

// delete an existing constitutive law
void NuTo::StructureBase::ConstitutiveLawDelete(int rIdent)
{
    // find constitutive law identifier in map
    boost::ptr_map<int,ConstitutiveBase>::iterator it = this->mConstitutiveLawMap.find(rIdent);
    if (it == this->mConstitutiveLawMap.end())
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawDelete] Constitutive law does not exist.");
    }
    else
    {
        this->mConstitutiveLawMap.erase(it);
    }
}

// get constitutive law pointer from constitutive law identifier
NuTo::ConstitutiveBase* NuTo::StructureBase::ConstitutiveLawGetConstitutiveLawPtr(int rIdent)
{
    boost::ptr_map<int,ConstitutiveBase>::iterator it = this->mConstitutiveLawMap.find(rIdent);
    if (it == this->mConstitutiveLawMap.end())
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawGetConstitutiveLawPtr] Constitutive law does not exist.");
    }
    return it->second;
}

// get constitutive law pointer from constitutive law identifier
const NuTo::ConstitutiveBase* NuTo::StructureBase::ConstitutiveLawGetConstitutiveLawPtr(int rIdent) const
{
    boost::ptr_map<int,ConstitutiveBase>::const_iterator it = this->mConstitutiveLawMap.find(rIdent);
    if (it == this->mConstitutiveLawMap.end())
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::ConstitutiveLawGetConstitutiveLawPtr] Constitutive law does not exist.");
    }
    return it->second;
}

// get constitutive law identifier from constitutive law pointer
int NuTo::StructureBase::ConstitutiveLawGetId(const NuTo::ConstitutiveBase* rConstitutiveLawPtr) const
{
    for (boost::ptr_map<int,ConstitutiveBase>::const_iterator it = mConstitutiveLawMap.begin(); it!= mConstitutiveLawMap.end(); it++)
    {
        if (it->second == rConstitutiveLawPtr)
        {
            return it->first;
        }
    }
    throw MechanicsException("[NuTo::StructureBase::ConstitutiveLawGetId] Constitutive law does not exist.");
}

// info routines
void NuTo::StructureBase::ConstitutiveLawInfo(unsigned short rVerboseLevel) const
{
    std::cout << "Number of constitutive laws: " << this->GetNumConstitutiveLaws() << std::endl;
    for (boost::ptr_map<int,ConstitutiveBase>::const_iterator it = mConstitutiveLawMap.begin(); it!= mConstitutiveLawMap.end(); it++)
    {
        std::cout << "  Constitutive law: " << it->first << std::endl;
        it->second->Info(rVerboseLevel);
    }
}

void NuTo::StructureBase::ConstitutiveLawInfo(int rIdent, unsigned short rVerboseLevel) const
{
    const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
    std::cout << "  Constitutive law: " << rIdent << std::endl;
    ConstitutiveLawPtr->Info(rVerboseLevel);
}

// set Young's modulus
void NuTo::StructureBase::ConstitutiveLawSetYoungsModulus(int rIdent, double rE)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->SetYoungsModulus(rE);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawSetYoungsModulus] error setting Young's modulus.");
        throw e;
    }
}

// get Young's modulus
double NuTo::StructureBase::ConstitutiveLawGetYoungsModulus(int rIdent) const
{
    double youngs_modulus;
    try
    {
        const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        youngs_modulus = ConstitutiveLawPtr->GetYoungsModulus();
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetYoungsModulus] error getting Young's modulus.");
        throw e;
    }
    return youngs_modulus;
}

// set Poisson's ratio
void NuTo::StructureBase::ConstitutiveLawSetPoissonsRatio(int rIdent, double rNu)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->SetPoissonsRatio(rNu);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawSetPoissonsRatio] error setting Poisson's ratio.");
        throw e;
    }
}

// get Poisson's ratio
double NuTo::StructureBase::ConstitutiveLawGetPoissonsRatio(int rIdent) const
{
    double nu;
    try
    {
        const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        nu = ConstitutiveLawPtr->GetPoissonsRatio();
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetPoissonsRatio] error getting Poisson's ratio.");
        throw e;
    }
    return nu;
}

//! @brief ... get initial yield strength
//! @return ... yield strength
double NuTo::StructureBase::ConstitutiveLawGetInitialYieldStrength(int rIdent) const
{
	try
	{
		const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
		return ConstitutiveLawPtr->GetInitialYieldStrength();
	}
	catch (NuTo::MechanicsException& e)
	{
		e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetInitialYieldStrength] error getting initial yield strength.");
		throw e;
	}
}

//! @brief ... set initial yield strength
//! @param rSigma ...  yield strength
void NuTo::StructureBase::ConstitutiveLawSetInitialYieldStrength(int rIdent, double rSigma)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->SetInitialYieldStrength(rSigma);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawSetInitialYieldStrength] error setting initial yield strength.");
        throw e;
    }
}

//! @brief ... get yield strength for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding yield strength
NuTo::FullMatrix<double> NuTo::StructureBase::ConstitutiveLawGetYieldStrength(int rIdent) const
{
	try
	{
		const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
		return ConstitutiveLawPtr->GetYieldStrength();
	}
	catch (NuTo::MechanicsException& e)
	{
		e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetYieldStrength] error getting yield strength.");
		throw e;
	}
}

//! @brief ... add yield strength
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  yield strength
void NuTo::StructureBase::ConstitutiveLawAddYieldStrength(int rIdent, double rEpsilon, double rSigma)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->AddYieldStrength(rEpsilon, rSigma);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawAddYieldStrength] error adding yield strength.");
        throw e;
    }
}

//! @brief ... get initial hardening modulus
//! @return ... hardening modulus
double NuTo::StructureBase::ConstitutiveLawGetInitialHardeningModulus(int rIdent) const
{
	try
	{
		const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
		return ConstitutiveLawPtr->GetInitialHardeningModulus();
	}
	catch (NuTo::MechanicsException& e)
	{
		e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetInitialHardeningModulus] error getting initial hardening modulus.");
		throw e;
	}
}

//! @brief ... set hardening modulus
//! @param rH ...  hardening modulus
void NuTo::StructureBase::ConstitutiveLawSetInitialHardeningModulus(int rIdent, double rH)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->SetInitialHardeningModulus(rH);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawSetInitialHardeningModulus] error setting initial hardening modulus.");
        throw e;
    }
}

//! @brief ... get hardening modulus for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding hardening modulus
NuTo::FullMatrix<double> NuTo::StructureBase::ConstitutiveLawGetHardeningModulus(int rIdent) const
{
	try
	{
		const ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
		return ConstitutiveLawPtr->GetHardeningModulus();
	}
	catch (NuTo::MechanicsException& e)
	{
		e.AddMessage("[NuTo::StructureBase::ConstitutiveLawGetHardeningModulus's ratio] error getting hardening moduli.");
		throw e;
	}
}

//! @brief ... add hardening modulus
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  hardening modulus
void NuTo::StructureBase::ConstitutiveLawAddHardeningModulus(int rIdent, double rEpsilon, double rH)
{
    try
    {
        ConstitutiveBase* ConstitutiveLawPtr = this->ConstitutiveLawGetConstitutiveLawPtr(rIdent);
        ConstitutiveLawPtr->AddHardeningModulus(rEpsilon, rH);
    }
    catch (NuTo::MechanicsException& e)
    {
        e.AddMessage("[NuTo::StructureBase::ConstitutiveLawAddHardeningModulus] error adding hardening modulus.");
        throw e;
    }
}

