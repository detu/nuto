// $Id$
#pragma once

#include "nuto/mechanics/elements/ElementDataConstitutiveBase.h"
#include "nuto/mechanics/elements/ElementDataIpBase.h"
#include "nuto/mechanics/MechanicsException.h"


namespace NuTo
{
class IntegrationTypeBase;
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... class for elements with a single material per element static data for each integration point
class ElementDataConstitutiveIp : public ElementDataConstitutiveBase, public ElementDataIpBase
{
#ifdef ENABLE_SERIALIZATION
	friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION

public:
    //! @brief constructor
	ElementDataConstitutiveIp(const ElementBase *rElement, const NuTo::IntegrationTypeBase* rIntegrationType, NuTo::IpData::eIpDataType rIpDataType);

	//! @brief constructor
	ElementDataConstitutiveIp(const ElementBase *rElement, int rNumIp, NuTo::IpData::eIpDataType rIpDataType);

	virtual ~ElementDataConstitutiveIp();

    //! @brief updates the data related to changes of the constitutive model (e.g. reallocation of static data, nonlocal weights etc.)
    //! @param rElement element
   virtual void InitializeUpdatedConstitutiveLaw(const ElementBase* rElement);

   //! @brief updates the data related to changes of the constitutive model (e.g. reallocation of static data, nonlocal weights etc.)
   //! @param rElement element
   //! @param rIp Ip
   virtual void InitializeUpdatedConstitutiveLaw(const ElementBase* rElement,int rIp)
   {
	   throw MechanicsException("[NuTo::ElementDataConstitutiveBase::InitializeUpdatedConstitutiveLaw] only one constitutive law is assigned at one element.");

   }

   //! @brief returns the enum of element data type
   //! @return enum of ElementDataType
   virtual const NuTo::ElementData::eElementDataType GetElementDataType()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif  // ENABLE_SERIALIZATION

protected:
    //! @brief ...just for serialization
    ElementDataConstitutiveIp(){}
};
}
#ifdef ENABLE_SERIALIZATION
namespace boost{
template<>
struct is_virtual_base_of<NuTo::ElementDataConstitutiveBase, NuTo::ElementDataConstitutiveIp>: public mpl::true_ {};
template<>
struct is_virtual_base_of<NuTo::ElementDataIpBase, NuTo::ElementDataConstitutiveIp>: public mpl::true_ {};
}
BOOST_CLASS_EXPORT_KEY(NuTo::ElementDataConstitutiveIp)
#endif // ENABLE_SERIALIZATION

