// $Id$ 
#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include <string>

namespace NuTo
{

enum class eVisualizeWhat;

//! @author Joerg F. Unger
//! @date Apr 27, 2010
//! @brief a base class to store additional information about the data to be plotted (e.g. the element and ip for nonlocal weights or the stress/strain/displacement component, if not all should be exported to the file
class VisualizeComponentBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    VisualizeComponentBase(){};

    virtual ~VisualizeComponentBase(){};

    virtual NuTo::eVisualizeWhat GetComponentEnum()const=0;

    virtual std::string GetComponentName()const=0;

    virtual int GetElementId()const;

    virtual int GetIp()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:

};
}

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::VisualizeComponentBase)
#endif // ENABLE_SERIALIZATION