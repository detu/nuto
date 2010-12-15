#ifndef TETRAHEDRON4N_H_
#define TETRAHEDRON4N_H_

#include <vector>
#include "nuto/mechanics/elements/Solid.h"

namespace NuTo
{
class NodeBase;
class Tetrahedron4N : public Solid
{
public:
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION
    Tetrahedron4N(NuTo::StructureBase* rStructure, std::vector<NuTo::NodeBase* >& rNodes,
    		ElementData::eElementDataType rElementDataType, IpData::eIpDataType rIpDataType);
    virtual ~Tetrahedron4N();

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif  // ENABLE_SERIALIZATION

    //! @brief returns the enum (type of the element)
    //! @return enum
    NuTo::Element::eElementType GetEnumType()const
    {
        return NuTo::Element::TETRAHEDRON4N;
    }

    //! @brief returns the number of nodes in this element
    //! @return number of nodes
    int GetNumNodes() const
    {
        return 4;
    }

    //! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still identical
    //! @param rOldPtr old node ptr
    //! @param rNewPtr new node ptr
    void ExchangeNodePtr(NodeBase* rOldPtr, NodeBase* rNewPtr);

    //! @brief returns the number of shape functions
    //! this is required for the calculation of the derivatives of the shape functions
    //! whose size is GetLocalDimension*GetNumShapeFunctions
    //! @return local dimension
    virtual int GetNumShapeFunctions() const
    {
        return 4;
    }

    //! @brief calculates the shape functions
    //! @param rLocalCoordinates local coordinates of the integration point
    //! @param shape functions for all the nodes
    void CalculateShapeFunctions(const double rLocalCoordinates[3],
                                 std::vector<double>& rShapeFunctions) const;

    //! @brief calculates the derivative of the shape functions with respect to local coordinates
    //! @param rLocalCoordinates local coordinates of the integration point
    //! @param derivative of the shape functions for all the nodes,
    //! first all the directions for a single node, and then for the next node
    void CalculateDerivativeShapeFunctionsLocal(
        const double rLocalCoordinates[3],
        std::vector<double>& rDerivativeShapeFunctions) const;

    //! @brief returns the number of local degrees of freedom
    //! @return number of local degrees of freedom
    inline int GetNumDofs() const
    {
        return 12;
    }

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber)
    {
        assert(rLocalNodeNumber>=0 && rLocalNodeNumber<=3);
        return mNodes[rLocalNodeNumber];
    }

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber) const
    {
        assert(rLocalNodeNumber>=0 && rLocalNodeNumber<=3);
        return mNodes[rLocalNodeNumber];
    }

    //! @brief sets the rLocalNodeNumber-th node of the element
    //! @param local node number
    //! @param pointer to the node
    void SetNode(int rLocalNodeNumber, NodeBase* rNode)
    {
        assert(rLocalNodeNumber>=0 && rLocalNodeNumber<=3);
        mNodes[rLocalNodeNumber] = rNode;
    }

    //! @brief returns the enum of the standard integration type for this element
    NuTo::IntegrationType::eIntegrationType GetStandardIntegrationType();
protected:
    //! @brief ... just for serialization
    Tetrahedron4N(){};

    //! @brief ... reorder nodes such that the sign of the length of the element changes
    void ReorderNodes();

    //! @brief element nodes
    NodeBase* mNodes[4];
};

}
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::Tetrahedron4N)
#endif // ENABLE_SERIALIZATION

#endif /*TETRAHEDRON4N_H_*/
