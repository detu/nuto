#pragma once

#include "nuto/mechanics/elements/ElementBase.h"


namespace NuTo
{

class NodeBase;
class SectionBase;
class ElementOutputBase;
class ElementOutputIpData;

template <typename T>
class BlockFullVector;
template <typename T>
class BlockFullMatrix;
template <int TDim>
class ContinuumBoundaryElement;

template <int TDim>
struct EvaluateDataContinuum;

template <int TDim>
class ContinuumElement : public ElementBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;

protected:
    ContinuumElement() = default;
#endif // ENABLE_SERIALIZATION

    friend class ContinuumBoundaryElement<TDim>;

public:
    ContinuumElement(const NuTo::StructureBase* rStructure, const std::vector<NuTo::NodeBase*>& rNodes,
                     ElementData::eElementDataType rElementDataType, IpData::eIpDataType rIpDataType,
                     InterpolationType* rInterpolationType);

    ContinuumElement(const ContinuumElement&) = default;
    ContinuumElement(ContinuumElement&&) = default;

    virtual ~ContinuumElement() = default;

    //! @brief calculates output data for the element
    //! @param rInput ... constitutive input map for the constitutive law
    //! @param rOutput ...  coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which
    //! includes inertia terms)
    eError Evaluate(const ConstitutiveInputMap& rInput,
                    std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rOutput) override;

    //! @brief returns the enum (type of the element)
    //! @return enum
    NuTo::Element::eElementType GetEnumType() const override;

    //! @brief returns the local dimension of the element
    //! this is required to check, if an element can be used in a 1d, 2D or 3D Structure
    //! @return local dimension
    int GetLocalDimension() const override
    {
        return TDim;
    }

    //! @brief Allocates static data for an integration point of an element
    //! @param rConstitutiveLaw constitutive law, which is called to allocate the static data object
    ConstitutiveStaticDataBase* AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber) override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber) const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType) override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType) const override;

    //! @brief sets the rLocalNodeNumber-th node of the element
    //! @param local node number
    //! @param pointer to the node
    void SetNode(int rLocalNodeNumber, NodeBase* rNode) override;

    //! @brief resizes the node vector
    //! @param rNewNumNodes new number of nodes
    void ResizeNodes(int rNewNumNodes) override;

    //! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still
    //! identical
    void ExchangeNodePtr(NodeBase* rOldPtr, NodeBase* rNewPtr) override;

    virtual Eigen::VectorXd ExtractNodeValues(int rTimeDerivative, Node::eDof rDofType) const override;

    //! @brief sets the section of an element
    //! implemented with an exception for all elements, reimplementation required for those elements
    //! which actually need a section
    //! @param rSection pointer to section
    void SetSection(const SectionBase* rSection) override
    {
        mSection = rSection;
    }

    //! @brief returns a pointer to the section of an element
    //! implemented with an exception for all elements, reimplementation required for those elements
    //! which actually need a section
    //! @return pointer to section
    const SectionBase* GetSection() const override
    {
        return mSection;
    }

    //! @brief calculates the volume of an integration point (weight * detJac)
    //! @return rVolume  vector for storage of the ip volumes (area in 2D, length in 1D)
    const Eigen::VectorXd GetIntegrationPointVolume() const override;

    //! @brief Calculates the the inverse of the Jacobian and its determinant
    //! @param rDerivativeShapeFunctions Derivatives of the shape functions (dN0dx & dN0dy \\ dN1dx & dN1dy \\ ..
    //! @param rNodeCoordinates Node coordinates (X1 \\ Y1 \\ X2 \\Y2 \\ ...
    //! @param rDetJac determinant of the Jacobian (return value)
    //! @return inverse Jacobian matrix
    Eigen::Matrix<double, TDim, TDim> CalculateJacobian(const Eigen::MatrixXd& rDerivativeShapeFunctions,
                                                        const Eigen::VectorXd& rNodeCoordinates) const;

    virtual Eigen::VectorXd CalculateJacobianSurface(const Eigen::VectorXd& rParameter,
                                                     const Eigen::VectorXd& rNodalCoordinates, int rSurfaceId) const;

    //! @brief Calculates the the jacobian of the mapping between the refernce element and parametric space (knots)
    //! @param rKnots ... knots of the element
    //! @return the jacobian
    virtual Eigen::Matrix<double, TDim, TDim> CalculateJacobianParametricSpaceIGA() const;

    Eigen::MatrixXd CalculateMatrixB(Node::eDof rDofType, const Eigen::MatrixXd& rDerivativeShapeFunctions,
                                     const Eigen::Matrix<double, TDim, TDim> rInvJacobian) const;

    Eigen::VectorXd InterpolateDofGlobalCurrentConfiguration(int rTimeDerivative,
                                                             const Eigen::VectorXd& rNaturalCoordinates,
                                                             Node::eDof rDofTypeInit,
                                                             Node::eDof rDofTypeCurrent) const override;

    Eigen::VectorXd InterpolateDof(int rTimeDerivative, int rTheIP, Node::eDof rDofType) const override;

    const ContinuumElement<1>& AsContinuumElement1D() const override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<1>.");
    }

    const ContinuumElement<2>& AsContinuumElement2D() const override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<2>.");
    }

    const ContinuumElement<3>& AsContinuumElement3D() const override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<3>.");
    }

    ContinuumElement<1>& AsContinuumElement1D() override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<1>.");
    }

    ContinuumElement<2>& AsContinuumElement2D() override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<2>.");
    }

    ContinuumElement<3>& AsContinuumElement3D() override
    {
        throw NuTo::MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                       "] Element is not of type ContinuumElement<3>.");
    }

protected:
    //! @todo boost ptr container?
    std::vector<NodeBase*> mNodes;
    const SectionBase* mSection;


    //! @brief ... check if the element is properly defined (check node dofs, nodes are reordered if the element
    //! length/area/volum is negative)
    void CheckElement() override;

    void ExtractAllNecessaryDofValues(EvaluateDataContinuum<TDim>& data);

    ConstitutiveOutputMap
    GetConstitutiveOutputMap(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput) const;

    virtual void FillConstitutiveOutputMapInternalGradient(ConstitutiveOutputMap& rConstitutiveOutput,
                                                           BlockFullVector<double>& rInternalGradient) const;
    virtual void FillConstitutiveOutputMapHessian0(ConstitutiveOutputMap& rConstitutiveOutput,
                                                   BlockFullMatrix<double>& rHessian0) const;
    virtual void FillConstitutiveOutputMapHessian1(ConstitutiveOutputMap& rConstitutiveOutput,
                                                   BlockFullMatrix<double>& rHessian1) const;
    virtual void FillConstitutiveOutputMapHessian2(ConstitutiveOutputMap& rConstitutiveOutput,
                                                   BlockFullMatrix<double>& rHessian2) const;
    virtual void FillConstitutiveOutputMapIpData(ConstitutiveOutputMap& rConstitutiveOutput,
                                                 ElementOutputIpData& rIpData) const;

    ConstitutiveInputMap GetConstitutiveInputMap(const ConstitutiveOutputMap& rConstitutiveOutput) const;

    //! @brief ... extract global dofs from nodes (mapping of local row ordering of the element matrices to the global
    //! dof ordering)
    virtual void CalculateGlobalRowDofs(BlockFullVector<int>& rGlobalRowDofs) const;

    //! @brief ... extract global dofs from nodes (mapping of local column ordering of the element matrices to the
    //! global dof ordering)
    void CalculateGlobalColumnDofs(BlockFullVector<int>& rGlobalDofMapping) const;


    virtual void CalculateNMatrixBMatrixDetJacobian(EvaluateDataContinuum<TDim>& data, int rTheIP) const;


    //! @brief Turns rDerivativeShapeFunctions into the B-Matrix for the displacements
    //! @remark: (N0,x & N0,y \\ ...)   --> (N0,x & 0 \\ 0 & N0,y \\ N0,y & N0,x)
    void BlowToBMatrixEngineeringStrain(Eigen::MatrixXd& rDerivativeShapeFunctions) const;

    void CalculateConstitutiveInputs(ConstitutiveInputMap& rConstitutiveInput, EvaluateDataContinuum<TDim>& rData);

    void CalculateElementOutputs(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput,
                                 EvaluateDataContinuum<TDim>& rData, int rTheIP,
                                 const ConstitutiveInputMap& constitutiveInput,
                                 const ConstitutiveOutputMap& constitutiveOutput) const;

    virtual void CalculateElementOutputInternalGradient(BlockFullVector<double>& rInternalGradient,
                                                        EvaluateDataContinuum<TDim>& rData, int rTheIP,
                                                        const ConstitutiveInputMap& constitutiveInput,
                                                        const ConstitutiveOutputMap& constitutiveOutput) const;
    virtual void CalculateElementOutputHessian0(BlockFullMatrix<double>& rHessian0, EvaluateDataContinuum<TDim>& rData,
                                                int rTheIP, const ConstitutiveOutputMap& constitutiveOutput) const;
    virtual void CalculateElementOutputHessian1(BlockFullMatrix<double>& rHessian1, EvaluateDataContinuum<TDim>& rData,
                                                int rTheIP, const ConstitutiveOutputMap& constitutiveOutput) const;
    virtual void CalculateElementOutputHessian2(BlockFullMatrix<double>& rHessian2, EvaluateDataContinuum<TDim>& rData,
                                                int rTheIP) const;
    virtual void CalculateElementOutputIpData(ElementOutputIpData& rIpData, EvaluateDataContinuum<TDim>& rData,
                                              int rTheIP, const ConstitutiveOutputMap& constitutiveOutput) const;

    virtual double CalculateDetJxWeightIPxSection(double rDetJacobian, int rTheIP) const;


#ifdef ENABLE_SERIALIZATION
private:
    //! @brief serializes the class, this is the load routine
    //! @param ar         archive
    //! @param version    version
    template <class Archive>
    void load(Archive& ar, const unsigned int version);

    //! @brief serializes the class, this is the save routine
    //! @param ar         archive
    //! @param version    version
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    //! @brief NodeBase-Pointer are not serialized to avoid cyclic dependencies, but are serialized as Pointer-Address
    //! (uintptr_t)
    //! Deserialization of the NodeBase-Pointer is done by searching and casting back the Address in the map
    //! @param mNodeMapCast   std::map containing the old and new Addresses
    virtual void SetNodePtrAfterSerialization(const std::map<std::uintptr_t, std::uintptr_t>& mNodeMapCast) override;

#endif // ENABLE_SERIALIZATION
};

} /* namespace NuTo */
