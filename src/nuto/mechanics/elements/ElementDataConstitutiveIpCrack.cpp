// $Id$

#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/elements/ElementDataConstitutiveIpCrack.h"
#include "nuto/mechanics/elements/ElementDataEnum.h"
#include "nuto/mechanics/elements/IpDataBase.h"
#include "nuto/mechanics/MechanicsException.h"

NuTo::ElementDataConstitutiveIpCrack::ElementDataConstitutiveIpCrack(const ElementBase *rElement,
		const NuTo::IntegrationTypeBase* rIntegrationType, NuTo::IpData::eIpDataType rIpDataType) :
   NuTo::ElementDataBase::ElementDataBase(), ElementDataConstitutiveBase(), ElementDataCrackBase() , ElementDataIpBase(rElement,rIntegrationType,rIpDataType)
{
//	DBG_POSITION
}

NuTo::ElementDataConstitutiveIpCrack::ElementDataConstitutiveIpCrack(const ElementBase *rElement,
		int rNumIp, NuTo::IpData::eIpDataType rIpDataType) :
   NuTo::ElementDataBase::ElementDataBase(), ElementDataConstitutiveBase(), ElementDataCrackBase() , ElementDataIpBase(rElement,rNumIp,rIpDataType)
{
//	DBG_POSITION
}

NuTo::ElementDataConstitutiveIpCrack::~ElementDataConstitutiveIpCrack()
{
//	DBG_POSITION
}

//! @brief updates the data related to changes of the constitutive model (e.g. reallocation of static data, nonlocal weights etc.)
//! @param rElement element
void NuTo::ElementDataConstitutiveIpCrack::InitializeUpdatedConstitutiveLaw(const ElementBase* rElement)
{
	//reinitialize ip data (f.e. if different static data or nonlocal data are required with another constitutive model)
	for (int theIp=0; theIp<(int)mIpData.size();theIp++)
	{
		mIpData[theIp].Initialize(rElement,mConstitutiveLaw);
	}
}

//! @brief updates the data related to changes of the constitutive model (e.g. reallocation of static data, nonlocal weights etc.)
//! @param rElement element
//! @param rIp Ip
void NuTo::ElementDataConstitutiveIpCrack::InitializeUpdatedConstitutiveLaw(const ElementBase* rElement,int rIp)
{
	throw MechanicsException("[NuTo::ElementDataConstitutiveIpCrack::InitializeUpdatedConstitutiveLaw] only one constitutive law is assigned at one element.");
}

//! @brief returns the enum of element data type
//! @return enum of ElementDataType
const NuTo::ElementData::eElementDataType NuTo::ElementDataConstitutiveIpCrack::GetElementDataType()const
{
    return NuTo::ElementData::eElementDataType::CONSTITUTIVELAWIPCRACK;
}
