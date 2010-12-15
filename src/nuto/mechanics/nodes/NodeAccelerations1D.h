// $Id$

#ifndef NODE_ACCELERATIONS_1D_H_
#define NODE_ACCELERATIONS_1D_H_

#include "nuto/mechanics/nodes/NodeBase.h"

namespace NuTo
{
//! @author Stefan Eckardt, IFF
//! @date July 2010
//! @brief ... standard class for nodes having accelerations degrees of freedom
class NodeAccelerations1D : public virtual NuTo::NodeBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief default constructor
    NodeAccelerations1D();

    //! @brief constructor
    //! @param rAccelerations ... given accelerations
    NodeAccelerations1D(const double rAccelerations[1]);

    //! @brief returns the number of accelerations of the node
    //! @return number of accelerations
    int GetNumAccelerations()const;

    //! @brief set the accelerations
    //! @param rAccelerations  given accelerations
    void SetAccelerations1D(const double rAccelerations[1]);

    //! @brief writes the accelerations of a node to the prescribed pointer
    //! @param rAccelerations accelerations
    void GetAccelerations1D(double rAccelerations[1])const;

    //! @brief returns the type of the node
    //! @return type
    virtual std::string GetNodeTypeStr()const;

    //! @brief returns the type of node as an enum (all the data stored at the node)
    //! @return enum
    virtual Node::eNodeType GetNodeType()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION
protected:
    double mAccelerations[1]; //!< node velocity
};
}
#endif // NODE_ACCELERATIONS_1D_H_
