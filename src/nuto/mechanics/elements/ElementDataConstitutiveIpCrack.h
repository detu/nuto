// $Id$
#ifndef ELEMENTDATACONSTITUTIVEIPCRACK_H_
#define ELEMENTDATACONSTITUTIVEIPCRACK_H_

#include "nuto/mechanics/elements/ElementDataConstitutiveBase.h"
#include "nuto/mechanics/elements/ElementDataCrackBase.h"
#include "nuto/mechanics/elements/ElementDataIpBase.h"

namespace NuTo
{

//! @author Daniel Arnold
//! @date October 2010
//! @brief Standart class for ElementData IP, Constitutive and Cracks
class ElementDataConstitutiveIpCrack : public ElementDataConstitutiveBase, public ElementDataCrackBase, public ElementDataIpBase
{

public:
	//! @brief constructor
	ElementDataConstitutiveIpCrack(const ElementBase *rElement, const NuTo::IntegrationTypeBase* rIntegrationType, NuTo::IpData::eIpDataType rIpDataType);

	//! @brief destructor
	virtual ~ElementDataConstitutiveIpCrack();

	//! @brief updates the data related to changes of the constitutive model (e.g. reallocation of static data, nonlocal weights etc.)
    //! @param rElement element
    virtual void InitializeUpdatedConstitutiveLaw(const ElementBase* rElement);

    //! @brief returns the enum of element data type
    //! @return enum of ElementDataType
    const NuTo::ElementData::eElementDataType GetElementDataType()const;

protected:

};
}


#endif /* ELEMENTDATACONSTITUTIVEIPCRACK_H_ */