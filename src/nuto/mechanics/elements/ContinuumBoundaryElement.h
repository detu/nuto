/*
 * BoundaryElementBase.h
 *
 *  Created on: 5 Jun 2015
 *      Author: ttitsche
 */

#pragma once

#include "nuto/mechanics/elements/ElementBase.h"


namespace NuTo
{


class ElementOutputIpData;
template <int TDim>
class ContinuumElement;
template <typename T>
class BlockFullVector;
template <typename T>
class BlockFullMatrix;
template <int TDim>
struct EvaluateDataContinuumBoundary;


template <int TDim>
class ContinuumBoundaryElement : public ElementBase
{
public:
    ContinuumBoundaryElement(const ContinuumElement<TDim>* rBaseElement, int rSurfaceId);

    virtual ~ContinuumBoundaryElement() = default;

    //! @brief calculates output data for the element
    //! @param rInput ... constitutive input map for the constitutive law
    //! @param rOutput ...  coefficient matrix 0 1 or 2  (mass, damping and stiffness)
    //! and internal force (which includes inertia terms)
    eError Evaluate(const ConstitutiveInputMap& rInput,
                    std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rOutput) override;

    //! @brief returns the enum (type of the element)
    //! @return enum
    virtual NuTo::Element::eElementType GetEnumType() const override;

    //! @brief Allocates static data for an integration point of an element
    //! @param rConstitutiveLaw constitutive law, which is called to allocate the static data object
    virtual ConstitutiveStaticDataBase* AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const;


    //! @brief returns the local dimension of the element
    //! this is required to check, if an element can be used in a 1d, 2D or 3D Structure
    //! @return local dimension
    virtual int GetLocalDimension() const override;

    //! @brief returns the number of nodes in this element
    //! @return number of nodes
    int GetNumNodes() const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber) override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber) const override;

    //! @brief returns the number of nodes in this element that are influenced by it
    //! @remark overridden by boundary elements
    //! @return number of nodes
    int GetNumInfluenceNodes() const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @remark overridden by boundary elements
    //! @param local node number
    //! @return pointer to the node
    const NodeBase* GetInfluenceNode(int rLocalNodeNumber) const override;
    //! @brief returns the number of nodes in this element of a specific dof
    //! @brief rDofType dof type
    //! @return number of nodes
    int GetNumNodes(Node::eDof rDofType) const override;

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
    void SetNode(int rLocalNodeNumber, NodeBase* rNode) override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Probably not needed.");
    }

    //! @brief resizes the node vector
    //! @param rNewNumNodes new number of nodes
    void ResizeNodes(int rNewNumNodes)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Probably not needed.");
    }

    //! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed,
    //! but the restraints, elements etc. are still identical
    void ExchangeNodePtr(NodeBase* rOldPtr, NodeBase* rNewPtr) override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Probably not needed.");
    }

    //! @brief sets the section of an element
    //! implemented with an exception for all elements, reimplementation required for those elements
    //! which actually need a section
    //! @param rSection pointer to section
    virtual void SetSection(const SectionBase* rSection) override
    {
        return;
    }

    //! @brief returns a pointer to the section of an element
    //! @return pointer to section
    virtual const SectionBase* GetSection() const override;

    //! @brief calculates the volume of an integration point (weight * detJac)
    //! @return rVolume  vector for storage of the ip volumes (area in 2D, length in 1D)
    virtual const Eigen::VectorXd GetIntegrationPointVolume() const override;

    //! @brief getter for alpha parameter
    double GetAlpha() const
    {
        return mAlphaUserDefined;
    }

    //! @brief setter for alpha parameter
    void SetAlpha(double rAlpha)
    {
        mAlphaUserDefined = rAlpha;
    }


    virtual Eigen::VectorXd ExtractNodeValues(int rTimeDerivative, Node::eDof rDof) const override;

    virtual const Eigen::Vector3d GetGlobalIntegrationPointCoordinates(int rIpNum) const override;

    const ContinuumBoundaryElement<1>& AsContinuumBoundaryElement1D() const override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<1>.");
    }

    const ContinuumBoundaryElement<2>& AsContinuumBoundaryElement2D() const override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<2>.");
    }

    const ContinuumBoundaryElement<3>& AsContinuumBoundaryElement3D() const override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<3>.");
    }

    ContinuumBoundaryElement<1>& AsContinuumBoundaryElement1D() override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<1>.");
    }

    ContinuumBoundaryElement<2>& AsContinuumBoundaryElement2D() override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<2>.");
    }

    ContinuumBoundaryElement<3>& AsContinuumBoundaryElement3D() override
    {
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "Element is not of type ContinuumBoundaryElement<3>.");
    }


#ifdef ENABLE_VISUALIZE
    virtual void Visualize(VisualizeUnstructuredGrid& rVisualize,
                           const std::list<std::shared_ptr<NuTo::VisualizeComponent>>& rVisualizationList) override;
#endif // ENABLE_VISUALIZE

protected:
    //! @brief ... just for serialization
    ContinuumBoundaryElement()
    {
    }


    void ExtractAllNecessaryDofValues(EvaluateDataContinuumBoundary<TDim>& rData);

    virtual ConstitutiveOutputMap
    GetConstitutiveOutputMap(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput) const;

    void FillConstitutiveOutputMapInternalGradient(ConstitutiveOutputMap& rConstitutiveOutput,
                                                   BlockFullVector<double>& rInternalGradient) const;
    void FillConstitutiveOutputMapHessian0(ConstitutiveOutputMap& rConstitutiveOutput,
                                           BlockFullMatrix<double>& rHessian0) const;
    void FillConstitutiveOutputMapHessian1(ConstitutiveOutputMap& rConstitutiveOutput,
                                           BlockFullMatrix<double>& rHessian0) const;
    void FillConstitutiveOutputMapHessian2(ConstitutiveOutputMap& rConstitutiveOutput,
                                           BlockFullMatrix<double>& rHessian2) const;
    void FillConstitutiveOutputMapIpData(ConstitutiveOutputMap& rConstitutiveOutput,
                                         ElementOutputIpData& rIpData) const;

    ConstitutiveInputMap GetConstitutiveInputMap(const ConstitutiveOutputMap& rConstitutiveOutput) const;

    void CalculateConstitutiveInputs(const ConstitutiveInputMap& rConstitutiveInput,
                                     EvaluateDataContinuumBoundary<TDim>& rData);

    void CalculateElementOutputs(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput,
                                 EvaluateDataContinuumBoundary<TDim>& rData, int rTheIP,
                                 const ConstitutiveInputMap& constitutiveInput,
                                 const ConstitutiveOutputMap& constitutiveOutput) const;

    void CalculateElementOutputInternalGradient(BlockFullVector<double>& rInternalGradient,
                                                EvaluateDataContinuumBoundary<TDim>& rData,
                                                const ConstitutiveInputMap& constitutiveInput,
                                                const ConstitutiveOutputMap& constitutiveOutput, int rTheIP) const;

    void CalculateElementOutputHessian0(BlockFullMatrix<double>& rHessian0, EvaluateDataContinuumBoundary<TDim>& rData,
                                        const ConstitutiveOutputMap& constitutiveOutput, int rTheIP) const;

    void CalculateElementOutputIpData(ElementOutputIpData& rIpData, const ConstitutiveOutputMap& constitutiveOutput,
                                      int rTheIP) const;

    void CalculateNMatrixBMatrixDetJacobian(EvaluateDataContinuumBoundary<TDim>& rData, int rTheIP) const;

    Eigen::Matrix<double, TDim - 1, 1> CalculateIPCoordinatesSurface(int rTheIP) const;

    double CalculateDetJxWeightIPxSection(double rDetJacobian, int rTheIP) const;

    //! @brief calculates the alpha parameter as alpha = sqrt(nonlocal radius)
    double CalculateAlpha();

    //! @brief ... reorder nodes such that the sign of the length/area/volume of the element changes
    void ReorderNodes() override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Probably not needed.");
    }

    //! @brief ... check if the element is properly defined
    //! (check node dofs, nodes are reordered if the element length/area/volum is negative)
    void CheckElement() override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Probably not needed.");
    }

    // The real boundary element that is attached to the virtual boundary element
    const ContinuumElement<TDim>* mBaseElement;

    // surface id
    int mSurfaceId;

    // alpha parameter for the gradient damage boundary condition
    int mAlphaUserDefined;
};

} /* namespace NuTo */
