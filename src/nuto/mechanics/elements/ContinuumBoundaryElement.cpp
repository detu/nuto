/*
 * ContinuumBoundaryElementBase.cpp
 *
 *  Created on: 5 Mar 2016
 *      Author: vhirtham
 */
#include "nuto/base/ErrorEnum.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/dofSubMatrixStorage/BlockFullMatrix.h"
#include "nuto/mechanics/elements/ContinuumBoundaryElement.h"
#include "nuto/mechanics/elements/ContinuumElement.h"
#include "nuto/mechanics/elements/ElementDataBase.h"
#include "nuto/mechanics/elements/ElementEnum.h"
#include "nuto/mechanics/elements/ElementOutputBase.h"
#include "nuto/mechanics/elements/ElementOutputIpData.h"
#include "nuto/mechanics/elements/EvaluateDataContinuumBoundary.h"
#include "nuto/mechanics/elements/IpDataEnum.h"
#include "nuto/mechanics/integrationtypes/IntegrationTypeBase.h"
#include "nuto/mechanics/interpolationtypes/InterpolationBase.h"
#include "nuto/mechanics/interpolationtypes/InterpolationType.h"
#include "nuto/mechanics/nodes/NodeEnum.h"
#include "nuto/mechanics/sections/SectionTruss.h"
#include "nuto/mechanics/sections/SectionPlane.h"
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/interpolationtypes/InterpolationTypeEnum.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveScalar.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveVector.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStress.h"

template <int TDim>
NuTo::ContinuumBoundaryElement<TDim>::ContinuumBoundaryElement(const ContinuumElement<TDim>* rBaseElement,
                                                               int rSurfaceId)
    : ElementBase::ElementBase(rBaseElement->GetStructure(), rBaseElement->GetElementDataType(),
                               rBaseElement->GetIpDataType(0), rBaseElement->GetInterpolationType())
    , mBaseElement(rBaseElement)
    , mSurfaceId(rSurfaceId)
    , mAlphaUserDefined(-1)
{
}

template <int TDim>
NuTo::eError NuTo::ContinuumBoundaryElement<TDim>::Evaluate(
        const ConstitutiveInputMap& rInput,
        std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput)
{
    EvaluateDataContinuumBoundary<TDim> data;
    ExtractAllNecessaryDofValues(data);

    auto constitutiveOutput = GetConstitutiveOutputMap(rElementOutput);
    auto constitutiveInput = GetConstitutiveInputMap(constitutiveOutput);
    constitutiveInput.Merge(rInput);

    for (int theIP = 0; theIP < GetNumIntegrationPoints(); theIP++)
    {
        CalculateNMatrixBMatrixDetJacobian(data, theIP);
        CalculateConstitutiveInputs(constitutiveInput, data);


        eError error = EvaluateConstitutiveLaw<TDim>(constitutiveInput, constitutiveOutput, theIP);
        if (error != eError::SUCCESSFUL)
            return error;
        CalculateElementOutputs(rElementOutput, data, theIP, constitutiveInput, constitutiveOutput);
    }
    return eError::SUCCESSFUL;
}

template <int TDim>
NuTo::Element::eElementType NuTo::ContinuumBoundaryElement<TDim>::GetEnumType() const
{
    return Element::eElementType::CONTINUUMBOUNDARYELEMENT;
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::ExtractAllNecessaryDofValues(EvaluateDataContinuumBoundary<TDim>& rData)
{
    // needs optimization,
    // not all dofs might be needed...

    const std::set<Node::eDof>& dofs = mInterpolationType->GetDofs();
    for (auto dof : dofs)
        if (mInterpolationType->IsConstitutiveInput(dof))
            rData.mNodalValues[dof] = mBaseElement->ExtractNodeValues(0, dof);

    rData.mNodalValues[Node::eDof::COORDINATES] = mBaseElement->ExtractNodeValues(0, Node::eDof::COORDINATES);

    if (mStructure->GetNumTimeDerivatives() >= 1)
        for (auto dof : dofs)
            if (mInterpolationType->IsConstitutiveInput(dof))
                rData.mNodalValues_dt1[dof] = mBaseElement->ExtractNodeValues(1, dof);
}


template <int TDim>
NuTo::ConstitutiveOutputMap NuTo::ContinuumBoundaryElement<TDim>::GetConstitutiveOutputMap(
        std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput) const
{
    ConstitutiveOutputMap constitutiveOutput;

    for (auto it : rElementOutput)
    {
        switch (it.first)
        {
        case Element::eOutput::INTERNAL_GRADIENT:
            FillConstitutiveOutputMapInternalGradient(constitutiveOutput, it.second->GetBlockFullVectorDouble());
            break;

        case Element::eOutput::HESSIAN_0_TIME_DERIVATIVE:
            FillConstitutiveOutputMapHessian0(constitutiveOutput, it.second->GetBlockFullMatrixDouble());
            break;

        case Element::eOutput::HESSIAN_1_TIME_DERIVATIVE:
            FillConstitutiveOutputMapHessian1(constitutiveOutput, it.second->GetBlockFullMatrixDouble());
            break;

        case Element::eOutput::HESSIAN_2_TIME_DERIVATIVE:
            FillConstitutiveOutputMapHessian2(constitutiveOutput, it.second->GetBlockFullMatrixDouble());
            break;
        case Element::eOutput::LUMPED_HESSIAN_2_TIME_DERIVATIVE:
        {
            auto activeDofs = mInterpolationType->GetActiveDofs();
            if (activeDofs.size() > 1 && activeDofs.find(Node::eDof::DISPLACEMENTS) == activeDofs.end())
                throw MechanicsException(__PRETTY_FUNCTION__, "Lumped Hessian2 is only implemented for displacements.");
            int numDofs = mInterpolationType->Get(Node::eDof::DISPLACEMENTS).GetNumDofs();
            it.second->GetBlockFullVectorDouble()[Node::eDof::DISPLACEMENTS].Resize(numDofs);
            it.second->GetBlockFullVectorDouble()[Node::eDof::DISPLACEMENTS].setZero();
            break;
        }
        case Element::eOutput::UPDATE_STATIC_DATA:
            constitutiveOutput[Constitutive::eOutput::UPDATE_STATIC_DATA] = 0;
            break;

        case Element::eOutput::UPDATE_TMP_STATIC_DATA:
            constitutiveOutput[Constitutive::eOutput::UPDATE_TMP_STATIC_DATA] = 0;
            break;

        case Element::eOutput::IP_DATA:
            FillConstitutiveOutputMapIpData(constitutiveOutput, it.second->GetIpData());
            break;

        case Element::eOutput::GLOBAL_ROW_DOF:
            mBaseElement->CalculateGlobalRowDofs(it.second->GetBlockFullVectorInt());
            break;

        case Element::eOutput::GLOBAL_COLUMN_DOF:
            mBaseElement->CalculateGlobalColumnDofs(it.second->GetBlockFullVectorInt());
            break;

        default:
            throw MechanicsException(__PRETTY_FUNCTION__, "element  output not implemented.");
        }
    }
    return constitutiveOutput;
}

template <int TDim>
NuTo::ConstitutiveInputMap
NuTo::ContinuumBoundaryElement<TDim>::GetConstitutiveInputMap(const ConstitutiveOutputMap& rConstitutiveOutput) const
{
    ConstitutiveInputMap constitutiveInput =
            GetConstitutiveLaw(0)->GetConstitutiveInputs(rConstitutiveOutput, *GetInterpolationType());

    for (auto& itInput : constitutiveInput)
    {
        itInput.second = ConstitutiveIOBase::makeConstitutiveIO<TDim>(itInput.first);
    }
    return constitutiveInput;
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateNMatrixBMatrixDetJacobian(
        EvaluateDataContinuumBoundary<TDim>& rData, int rTheIP) const
{

    const InterpolationBase& interpolationTypeCoords = mInterpolationType->Get(Node::eDof::COORDINATES);

    Eigen::Matrix<double, TDim - 1, 1> ipCoordsSurface = CalculateIPCoordinatesSurface(rTheIP);
    Eigen::Matrix<double, TDim, 1> ipCoordsNatural =
            interpolationTypeCoords.CalculateNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);

    // #######################################
    // ##  Calculate the surface jacobian
    // ## = || [dX / dXi] * [dXi / dAlpha] ||
    // #######################################
    Eigen::MatrixXd derivativeShapeFunctionsNatural =
            interpolationTypeCoords.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);
    const Eigen::Matrix<double, TDim, TDim> jacobian = mBaseElement->CalculateJacobian(
            derivativeShapeFunctionsNatural, rData.mNodalValues[Node::eDof::COORDINATES]); // = [dX / dXi]

    const Eigen::MatrixXd derivativeNaturalSurfaceCoordinates =
            interpolationTypeCoords.CalculateDerivativeNaturalSurfaceCoordinates(ipCoordsSurface,
                                                                                 mSurfaceId); // = [dXi / dAlpha]
    rData.mDetJacobian = (jacobian * derivativeNaturalSurfaceCoordinates).norm();

    if (rData.mDetJacobian == 0)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Determinant of the Jacobian is zero, no inversion possible.");
    }

    const Eigen::Matrix<double, TDim, TDim> invJacobian = jacobian.inverse();


    for (auto dof : mInterpolationType->GetDofs())
    {
        if (dof == Node::eDof::COORDINATES)
            continue;
        const InterpolationBase& interpolationType = mInterpolationType->Get(dof);
        rData.mN[dof] = interpolationType.CalculateMatrixN(ipCoordsNatural);

        rData.mB[dof] = mBaseElement->CalculateMatrixB(
                dof, interpolationType.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural), invJacobian);
    }
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateConstitutiveInputs(const ConstitutiveInputMap& rConstitutiveInput,
                                                                       EvaluateDataContinuumBoundary<TDim>& rData)
{
    constexpr int voigtDim = ConstitutiveIOBase::GetVoigtDim(TDim);
    for (auto& it : rConstitutiveInput)
    {
        switch (it.first)
        {
        case Constitutive::eInput::ENGINEERING_STRAIN:
        {
            auto& strain = *static_cast<ConstitutiveVector<voigtDim>*>(it.second.get());
            strain.AsVector() =
                    rData.mB.at(Node::eDof::DISPLACEMENTS) * rData.mNodalValues.at(Node::eDof::DISPLACEMENTS);
            break;
        }
        case Constitutive::eInput::NONLOCAL_EQ_STRAIN:
        {
            if (mAlphaUserDefined == -1) // just to put it somewhere...
                rData.mAlpha = CalculateAlpha();
            else
                rData.mAlpha = mAlphaUserDefined;

            auto& nonLocalEqStrain = *static_cast<ConstitutiveScalar*>(it.second.get());
            nonLocalEqStrain.AsScalar() =
                    rData.mN.at(Node::eDof::NONLOCALEQSTRAIN) * rData.mNodalValues.at(Node::eDof::NONLOCALEQSTRAIN);
            break;
        }
        case Constitutive::eInput::RELATIVE_HUMIDITY:
        {
            auto& relativeHumidity = *static_cast<ConstitutiveScalar*>(it.second.get());
            relativeHumidity.AsScalar() =
                    rData.mN.at(Node::eDof::RELATIVEHUMIDITY) * rData.mNodalValues.at(Node::eDof::RELATIVEHUMIDITY);
            break;
        }
        case Constitutive::eInput::WATER_VOLUME_FRACTION:
        {
            auto& waterVolumeFraction = *static_cast<ConstitutiveScalar*>(it.second.get());
            waterVolumeFraction.AsScalar() = rData.mN.at(Node::eDof::WATERVOLUMEFRACTION) *
                                             rData.mNodalValues.at(Node::eDof::WATERVOLUMEFRACTION);
            break;
        }
        case Constitutive::eInput::TIME_STEP:
        case Constitutive::eInput::CALCULATE_STATIC_DATA:
            break;

        default:
            throw MechanicsException(__PRETTY_FUNCTION__, "Constitutive input for " +
                                                                  Constitutive::InputToString(it.first) +
                                                                  " not implemented.");
        }
    }
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateElementOutputs(
        std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput,
        EvaluateDataContinuumBoundary<TDim>& rData, int rTheIP, const ConstitutiveInputMap& constitutiveInput,
        const ConstitutiveOutputMap& constitutiveOutput) const
{
    rData.mDetJxWeightIPxSection =
            CalculateDetJxWeightIPxSection(rData.mDetJacobian, rTheIP); // formerly known as "factor"

    for (auto it : rElementOutput)
    {
        switch (it.first)
        {
        case Element::eOutput::INTERNAL_GRADIENT:
            CalculateElementOutputInternalGradient(it.second->GetBlockFullVectorDouble(), rData, constitutiveInput,
                                                   constitutiveOutput, rTheIP);
            break;

        case Element::eOutput::HESSIAN_0_TIME_DERIVATIVE:
            CalculateElementOutputHessian0(it.second->GetBlockFullMatrixDouble(), rData, constitutiveOutput, rTheIP);
            break;

        case Element::eOutput::HESSIAN_1_TIME_DERIVATIVE:
            break;

        case Element::eOutput::HESSIAN_2_TIME_DERIVATIVE:
            break;

        case Element::eOutput::LUMPED_HESSIAN_2_TIME_DERIVATIVE:
            break;

        case Element::eOutput::UPDATE_STATIC_DATA:
        case Element::eOutput::UPDATE_TMP_STATIC_DATA:
            break;
        case Element::eOutput::IP_DATA:
            CalculateElementOutputIpData(it.second->GetIpData(), constitutiveOutput, rTheIP);
            break;
        case Element::eOutput::GLOBAL_ROW_DOF:
        case Element::eOutput::GLOBAL_COLUMN_DOF:
            break;
        default:
            throw MechanicsException(__PRETTY_FUNCTION__, "element output not implemented.");
        }
    }
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateElementOutputInternalGradient(
        BlockFullVector<double>& rInternalGradient, EvaluateDataContinuumBoundary<TDim>& rData,
        const ConstitutiveInputMap& constitutiveInput, const ConstitutiveOutputMap& constitutiveOutput,
        int rTheIP) const
{
    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        switch (dofRow)
        {
        case Node::eDof::DISPLACEMENTS:
            break;

        case Node::eDof::NONLOCALEQSTRAIN:
        {
            const auto& localEqStrain = *static_cast<ConstitutiveScalar*>(
                    constitutiveOutput.at(Constitutive::eOutput::LOCAL_EQ_STRAIN).get());
            const auto& nonlocalEqStrain = *static_cast<ConstitutiveScalar*>(
                    constitutiveInput.at(Constitutive::eInput::NONLOCAL_EQ_STRAIN).get());
            rInternalGradient[dofRow] += rData.mDetJxWeightIPxSection / rData.mAlpha *
                                         rData.mN.at(Node::eDof::NONLOCALEQSTRAIN).transpose() *
                                         (nonlocalEqStrain[0] - localEqStrain[0]);
            break;
        }

        case Node::eDof::RELATIVEHUMIDITY:
        {
            const auto& internalGradientRH_Boundary_N = *static_cast<ConstitutiveScalar*>(
                    constitutiveOutput.at(Constitutive::eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_BOUNDARY_N).get());
            rInternalGradient[dofRow] +=
                    rData.mDetJxWeightIPxSection * rData.mN.at(dofRow).transpose() * internalGradientRH_Boundary_N;
            break;
        }
        case Node::eDof::WATERVOLUMEFRACTION:
        {
            const auto& internalGradientWV_Boundary_N = *static_cast<ConstitutiveScalar*>(
                    constitutiveOutput.at(Constitutive::eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_BOUNDARY_N)
                            .get());
            rInternalGradient[dofRow] +=
                    rData.mDetJxWeightIPxSection * rData.mN.at(dofRow).transpose() * internalGradientWV_Boundary_N;
            break;
        }
        default:
            throw MechanicsException(__PRETTY_FUNCTION__, "Element output INTERNAL_GRADIENT for " +
                                                                  Node::DofToString(dofRow) + " not implemented.");
        }
    }
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateElementOutputHessian0(
        BlockFullMatrix<double>& rHessian0, EvaluateDataContinuumBoundary<TDim>& rData,
        const ConstitutiveOutputMap& constitutiveOutput, int rTheIP) const
{
    constexpr int VoigtDim = ConstitutiveIOBase::GetVoigtDim(TDim);

    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        for (auto dofCol : mInterpolationType->GetActiveDofs())
        {
            if (!GetConstitutiveLaw(rTheIP)->CheckDofCombinationComputable(dofRow, dofCol, 0))
                continue;
            auto& hessian0 = rHessian0(dofRow, dofCol);
            switch (Node::CombineDofs(dofRow, dofCol))
            {

            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::DISPLACEMENTS):
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::NONLOCALEQSTRAIN):
                break;

            case Node::CombineDofs(Node::eDof::NONLOCALEQSTRAIN, Node::eDof::DISPLACEMENTS):
            {
                const auto& tangentLocalEqStrainStrain = *static_cast<ConstitutiveVector<VoigtDim>*>(
                        constitutiveOutput.at(Constitutive::eOutput::D_LOCAL_EQ_STRAIN_XI_D_STRAIN).get());
                hessian0 -= rData.mDetJxWeightIPxSection * rData.mAlpha * rData.mN.at(dofRow).transpose() *
                            tangentLocalEqStrainStrain.transpose() * rData.mB.at(dofCol);
                break;
            }
            case Node::CombineDofs(Node::eDof::NONLOCALEQSTRAIN, Node::eDof::NONLOCALEQSTRAIN):
            {
                hessian0 += rData.mN.at(dofRow).transpose() * rData.mN.at(dofRow) * rData.mDetJxWeightIPxSection /
                            rData.mAlpha;
                break;
            }
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::RELATIVEHUMIDITY):
            {
                const auto& internalGradientRH_dRH_Boundary_NN_H0 = *static_cast<ConstitutiveScalar*>(
                        constitutiveOutput.at(Constitutive::eOutput::D_INTERNAL_GRADIENT_RH_D_RH_BOUNDARY_NN_H0).get());
                hessian0 += rData.mDetJxWeightIPxSection * rData.mN.at(dofRow).transpose() *
                            internalGradientRH_dRH_Boundary_NN_H0 * rData.mN.at(dofCol);
                break;
            }
            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::WATERVOLUMEFRACTION):
            {
                const auto& internalGradientWV_dWV_Boundary_NN_H0 = *static_cast<ConstitutiveScalar*>(
                        constitutiveOutput.at(Constitutive::eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BOUNDARY_NN_H0).get());
                hessian0 += rData.mDetJxWeightIPxSection * rData.mN.at(dofRow).transpose() *
                            internalGradientWV_dWV_Boundary_NN_H0 * rData.mN.at(dofCol);
                break;
            }
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::WATERVOLUMEFRACTION):
            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::RELATIVEHUMIDITY):
                break;
            default:
            /*******************************************************\
            |         NECESSARY BUT UNUSED DOF COMBINATIONS         |
            \*******************************************************/
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::RELATIVEHUMIDITY):
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::WATERVOLUMEFRACTION):
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::DISPLACEMENTS):
            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::DISPLACEMENTS):
                continue;
                throw MechanicsException(__PRETTY_FUNCTION__, "Element output HESSIAN_0_TIME_DERIVATIVE for "
                                                              "(" + Node::DofToString(dofRow) +
                                                                      "," + Node::DofToString(dofCol) +
                                                                      ") not implemented.");
            }
        }
    }
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::CalculateElementOutputIpData(ElementOutputIpData& rIpData,
                                                                        const ConstitutiveOutputMap& constitutiveOutput,
                                                                        int rTheIP) const
{
    for (auto& it :
         rIpData.GetIpDataMap()) // this reference here is _EXTREMLY_ important, since the GetIpDataMap() contains a
    { // FullMatrix VALUE and you want to access this value by reference. Without the &, a tmp copy would be made.
        switch (it.first)
        {
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_STRAIN:
            it.second.col(rTheIP) = *static_cast<EngineeringStrain<3>*>(
                    constitutiveOutput.at(Constitutive::eOutput::ENGINEERING_STRAIN_VISUALIZE).get());
            break;
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_STRESS:
            it.second.col(rTheIP) = *static_cast<EngineeringStress<3>*>(
                    constitutiveOutput.at(Constitutive::eOutput::ENGINEERING_STRESS_VISUALIZE).get());
            break;
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_PLASTIC_STRAIN:
            it.second.col(rTheIP) = *static_cast<EngineeringStrain<3>*>(
                    constitutiveOutput.at(Constitutive::eOutput::ENGINEERING_PLASTIC_STRAIN_VISUALIZE).get());
            break;
        case NuTo::IpData::eIpStaticDataType::DAMAGE:
            it.second.col(rTheIP) =
                    *static_cast<ConstitutiveScalar*>(constitutiveOutput.at(Constitutive::eOutput::DAMAGE).get());
            break;
        case NuTo::IpData::eIpStaticDataType::EXTRAPOLATION_ERROR:
            it.second.col(rTheIP) = *static_cast<ConstitutiveScalar*>(
                    constitutiveOutput.at(Constitutive::eOutput::EXTRAPOLATION_ERROR).get());
            break;
        case NuTo::IpData::eIpStaticDataType::LOCAL_EQ_STRAIN:
            it.second.col(rTheIP) = *static_cast<ConstitutiveScalar*>(
                    constitutiveOutput.at(Constitutive::eOutput::LOCAL_EQ_STRAIN).get());
            break;
        default:
            throw MechanicsException(std::string("[") + __PRETTY_FUNCTION__ + "] Ip data not implemented.");
        }
    }
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::FillConstitutiveOutputMapInternalGradient(
        ConstitutiveOutputMap& rConstitutiveOutput, BlockFullVector<double>& rInternalGradient) const
{
    using namespace NuTo::Constitutive;
    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        rInternalGradient[dofRow].Resize(mInterpolationType->Get(dofRow).GetNumDofs());
        switch (dofRow)
        {
        case Node::eDof::DISPLACEMENTS:
            break;

        case Node::eDof::NONLOCALEQSTRAIN:
            rConstitutiveOutput[eOutput::LOCAL_EQ_STRAIN] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::LOCAL_EQ_STRAIN);
            break;

        case Node::eDof::RELATIVEHUMIDITY:
            rConstitutiveOutput[eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_BOUNDARY_N] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(
                            eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_BOUNDARY_N);
            break;

        case Node::eDof::WATERVOLUMEFRACTION:
            rConstitutiveOutput[eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_BOUNDARY_N] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(
                            eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_BOUNDARY_N);
            break;

        default:
            throw MechanicsException(__PRETTY_FUNCTION__, "Constitutive output INTERNAL_GRADIENT for " +
                                                                  Node::DofToString(dofRow) + " not implemented.");
        }
    }
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::FillConstitutiveOutputMapHessian0(ConstitutiveOutputMap& rConstitutiveOutput,
                                                                             BlockFullMatrix<double>& rHessian0) const
{
    using namespace NuTo::Constitutive;
    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        for (auto dofCol : mInterpolationType->GetActiveDofs())
        {
            NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& dofSubMatrix = rHessian0(dofRow, dofCol);
            dofSubMatrix.Resize(mInterpolationType->Get(dofRow).GetNumDofs(),
                                mInterpolationType->Get(dofCol).GetNumDofs());
            dofSubMatrix.setZero();
            if (!GetConstitutiveLaw(0)->CheckDofCombinationComputable(dofRow, dofCol, 0))
                continue;

            switch (Node::CombineDofs(dofRow, dofCol))
            {

            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::DISPLACEMENTS):
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::NONLOCALEQSTRAIN):
                break;

            case Node::CombineDofs(Node::eDof::NONLOCALEQSTRAIN, Node::eDof::DISPLACEMENTS):
                rConstitutiveOutput[eOutput::D_LOCAL_EQ_STRAIN_XI_D_STRAIN] =
                        ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::D_LOCAL_EQ_STRAIN_XI_D_STRAIN);
                break;

            case Node::CombineDofs(Node::eDof::NONLOCALEQSTRAIN, Node::eDof::NONLOCALEQSTRAIN):
                break;

            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::RELATIVEHUMIDITY):
                rConstitutiveOutput[eOutput::D_INTERNAL_GRADIENT_RH_D_RH_BOUNDARY_NN_H0] =
                        ConstitutiveIOBase::makeConstitutiveIO<TDim>(
                                eOutput::D_INTERNAL_GRADIENT_RH_D_RH_BOUNDARY_NN_H0);
                break;

            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::WATERVOLUMEFRACTION):
                rConstitutiveOutput[eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BOUNDARY_NN_H0] =
                        ConstitutiveIOBase::makeConstitutiveIO<TDim>(
                                eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BOUNDARY_NN_H0);
                break;


            /*******************************************************\
            |         NECESSARY BUT UNUSED DOF COMBINATIONS         |
            \*******************************************************/
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::RELATIVEHUMIDITY):
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::WATERVOLUMEFRACTION):
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::WATERVOLUMEFRACTION):
            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::RELATIVEHUMIDITY):
                continue;
            default:
                throw MechanicsException(__PRETTY_FUNCTION__, "Constitutive output HESSIAN_0_TIME_DERIVATIVE for "
                                                              "(" + Node::DofToString(dofRow) +
                                                                      "," + Node::DofToString(dofCol) +
                                                                      ") not implemented.");
            }
        }
    }
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::FillConstitutiveOutputMapHessian1(ConstitutiveOutputMap& rConstitutiveOutput,
                                                                             BlockFullMatrix<double>& rHessian0) const
{
    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        for (auto dofCol : mInterpolationType->GetActiveDofs())
        {
            NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& dofSubMatrix = rHessian0(dofRow, dofCol);
            dofSubMatrix.Resize(mInterpolationType->Get(dofRow).GetNumDofs(),
                                mInterpolationType->Get(dofCol).GetNumDofs());
            dofSubMatrix.setZero();
            if (!GetConstitutiveLaw(0)->CheckDofCombinationComputable(dofRow, dofCol, 1))
                continue;

            switch (Node::CombineDofs(dofRow, dofCol))
            {


            /*******************************************************\
            |         NECESSARY BUT UNUSED DOF COMBINATIONS         |
            \*******************************************************/
            //            case Node::CombineDofs(Node::eDof::DISPLACEMENTS,         Node::eDof::DISPLACEMENTS):
            //            case Node::CombineDofs(Node::eDof::DISPLACEMENTS,         Node::eDof::RELATIVEHUMIDITY):
            //            case Node::CombineDofs(Node::eDof::DISPLACEMENTS,         Node::eDof::WATERVOLUMEFRACTION):
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::RELATIVEHUMIDITY):
            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION, Node::eDof::WATERVOLUMEFRACTION):
            //            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY,      Node::eDof::DISPLACEMENTS):
            case Node::CombineDofs(Node::eDof::RELATIVEHUMIDITY, Node::eDof::WATERVOLUMEFRACTION):
                //            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION,   Node::eDof::DISPLACEMENTS):
                //            case Node::CombineDofs(Node::eDof::WATERVOLUMEFRACTION,   Node::eDof::RELATIVEHUMIDITY):
                continue;
            default:
                throw MechanicsException(__PRETTY_FUNCTION__, "Constitutive output HESSIAN_1_TIME_DERIVATIVE for "
                                                              "(" + Node::DofToString(dofRow) +
                                                                      "," + Node::DofToString(dofCol) +
                                                                      ") not implemented.");
            }
        }
    }
}


template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::FillConstitutiveOutputMapHessian2(ConstitutiveOutputMap& rConstitutiveOutput,
                                                                             BlockFullMatrix<double>& rHessian2) const
{
    for (auto dofRow : mInterpolationType->GetActiveDofs())
    {
        for (auto dofCol : mInterpolationType->GetActiveDofs())
        {
            NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& dofSubMatrix = rHessian2(dofRow, dofCol);
            dofSubMatrix.Resize(mInterpolationType->Get(dofRow).GetNumDofs(),
                                mInterpolationType->Get(dofCol).GetNumDofs());
            dofSubMatrix.setZero();
            if (!GetConstitutiveLaw(0)->CheckDofCombinationComputable(dofRow, dofCol, 2))
                continue;
            switch (Node::CombineDofs(dofRow, dofCol))
            {
            case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::DISPLACEMENTS):
                break;
            default:
                throw MechanicsException("Not implemented.");
            }
        }
    }
}

template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::FillConstitutiveOutputMapIpData(ConstitutiveOutputMap& rConstitutiveOutput,
                                                                           ElementOutputIpData& rIpData) const
{
    using namespace NuTo::Constitutive;
    for (auto& it :
         rIpData.GetIpDataMap()) // this reference here is _EXTREMLY_ important, since the GetIpDataMap() contains a
    { // FullMatrix VALUE and you want to access this value by reference. Without the &, a tmp copy would be made.
        switch (it.first)
        {
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_STRAIN:
            it.second.Resize(6, GetNumIntegrationPoints());
            rConstitutiveOutput[eOutput::ENGINEERING_STRAIN_VISUALIZE] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::ENGINEERING_STRAIN_VISUALIZE);
            break;
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_STRESS:
            it.second.Resize(6, GetNumIntegrationPoints());
            rConstitutiveOutput[eOutput::ENGINEERING_STRESS_VISUALIZE] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::ENGINEERING_STRESS_VISUALIZE);
            break;
        case NuTo::IpData::eIpStaticDataType::ENGINEERING_PLASTIC_STRAIN:
            it.second.Resize(6, GetNumIntegrationPoints());
            rConstitutiveOutput[eOutput::ENGINEERING_PLASTIC_STRAIN_VISUALIZE] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::ENGINEERING_PLASTIC_STRAIN_VISUALIZE);
            break;
        case NuTo::IpData::eIpStaticDataType::DAMAGE:
            it.second.Resize(1, GetNumIntegrationPoints());
            rConstitutiveOutput[eOutput::DAMAGE] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::DAMAGE);
            break;
        case NuTo::IpData::eIpStaticDataType::LOCAL_EQ_STRAIN:
            it.second.Resize(1, GetNumIntegrationPoints());
            rConstitutiveOutput[eOutput::LOCAL_EQ_STRAIN] =
                    ConstitutiveIOBase::makeConstitutiveIO<TDim>(eOutput::LOCAL_EQ_STRAIN);
            break;
        default:
            throw MechanicsException(std::string("[") + __PRETTY_FUNCTION__ +
                                     "] this ip data type is not implemented.");
        }
    }
}


template <int TDim>
const Eigen::Vector3d NuTo::ContinuumBoundaryElement<TDim>::GetGlobalIntegrationPointCoordinates(int rIpNum) const
{
    Eigen::VectorXd naturalSurfaceIpCoordinates;
    switch (GetStructure()->GetDimension() - 1)
    {
    case 1:
    {
        double ipCoordinate;
        GetIntegrationType()->GetLocalIntegrationPointCoordinates1D(rIpNum, ipCoordinate);
        naturalSurfaceIpCoordinates.resize(1);
        naturalSurfaceIpCoordinates(0) = ipCoordinate;
        break;
    }
    case 2:
    {
        double ipCoordinates[2];
        GetIntegrationType()->GetLocalIntegrationPointCoordinates2D(rIpNum, ipCoordinates);
        naturalSurfaceIpCoordinates.resize(2);
        naturalSurfaceIpCoordinates(0) = ipCoordinates[0];
        naturalSurfaceIpCoordinates(1) = ipCoordinates[1];
        break;
    }
    default:
        throw MechanicsException(std::string("[") + __PRETTY_FUNCTION__ + "] the maximum dimension is 2.");
        break;
    }

    Eigen::VectorXd naturalIpCoordinates =
            mInterpolationType->Get(Node::eDof::COORDINATES)
                    .CalculateNaturalSurfaceCoordinates(naturalSurfaceIpCoordinates, mSurfaceId);

    Eigen::MatrixXd matrixN = mInterpolationType->Get(Node::eDof::COORDINATES).CalculateMatrixN(naturalIpCoordinates);
    Eigen::VectorXd nodeCoordinates = ExtractNodeValues(0, Node::eDof::COORDINATES);

    Eigen::Vector3d globalIntegrationPointCoordinates = Eigen::Vector3d::Zero();
    globalIntegrationPointCoordinates.segment(0, GetLocalDimension()) = matrixN * nodeCoordinates;

    return globalIntegrationPointCoordinates;
}


#ifdef ENABLE_VISUALIZE
template <int TDim>
void NuTo::ContinuumBoundaryElement<TDim>::Visualize(
        VisualizeUnstructuredGrid& rVisualize,
        const std::list<std::shared_ptr<NuTo::VisualizeComponent>>& rVisualizationList)
{
    if (GetStructure()->GetVerboseLevel() > 10)
        std::cout << __PRETTY_FUNCTION__ << "Pleeeaaase, implement the visualization for me!!!" << std::endl;
}
#endif


namespace NuTo
{
template <>
NuTo::ConstitutiveStaticDataBase*
ContinuumBoundaryElement<1>::AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const
{
    return rConstitutiveLaw->AllocateStaticData1D(this);
}
template <>
NuTo::ConstitutiveStaticDataBase*
ContinuumBoundaryElement<2>::AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const
{
    return rConstitutiveLaw->AllocateStaticData2D(this);
}
template <>
NuTo::ConstitutiveStaticDataBase*
ContinuumBoundaryElement<3>::AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const
{
    return rConstitutiveLaw->AllocateStaticData3D(this);
}

template <int TDim>
int ContinuumBoundaryElement<TDim>::GetLocalDimension() const
{
    return mBaseElement->GetLocalDimension();
}

template <int TDim>
int ContinuumBoundaryElement<TDim>::GetNumNodes() const
{
    return mInterpolationType->GetNumSurfaceNodes(mSurfaceId);
}

template <int TDim>
NodeBase* ContinuumBoundaryElement<TDim>::GetNode(int rLocalNodeNumber)
{
    int nodeId = mInterpolationType->GetSurfaceNodeIndex(mSurfaceId, rLocalNodeNumber);
    return const_cast<NodeBase*>(mBaseElement->GetNode(nodeId));
}

template <int TDim>
const NodeBase* ContinuumBoundaryElement<TDim>::GetNode(int rLocalNodeNumber) const
{
    int nodeId = mInterpolationType->GetSurfaceNodeIndex(mSurfaceId, rLocalNodeNumber);
    return mBaseElement->GetNode(nodeId);
}

template <int TDim>
int ContinuumBoundaryElement<TDim>::GetNumInfluenceNodes() const
{
    return mBaseElement->GetNumNodes();
}

template <int TDim>
const NodeBase* ContinuumBoundaryElement<TDim>::GetInfluenceNode(int rLocalNodeNumber) const
{
    return mBaseElement->GetNode(rLocalNodeNumber);
}

template <int TDim>
int ContinuumBoundaryElement<TDim>::GetNumNodes(Node::eDof rDofType) const
{
    return mInterpolationType->Get(rDofType).GetNumSurfaceNodes(mSurfaceId);
}

template <int TDim>
NodeBase* ContinuumBoundaryElement<TDim>::GetNode(int rLocalNodeNumber, Node::eDof rDofType)
{
    int nodeId = mInterpolationType->Get(rDofType).GetSurfaceNodeIndex(mSurfaceId, rLocalNodeNumber);
    return const_cast<NodeBase*>(mBaseElement->GetNode(nodeId));
}

template <int TDim>
const NodeBase* ContinuumBoundaryElement<TDim>::GetNode(int rLocalNodeNumber, Node::eDof rDofType) const
{
    int nodeId = mInterpolationType->Get(rDofType).GetSurfaceNodeIndex(mSurfaceId, rLocalNodeNumber);
    return mBaseElement->GetNode(nodeId);
}

template <int TDim>
const SectionBase* ContinuumBoundaryElement<TDim>::GetSection() const
{
    return mBaseElement->GetSection();
}


template <int TDim>
Eigen::VectorXd ContinuumBoundaryElement<TDim>::ExtractNodeValues(int rTimeDerivative, Node::eDof rDof) const
{
    return mBaseElement->ExtractNodeValues(rTimeDerivative, rDof);
}

template <int TDim>
double ContinuumBoundaryElement<TDim>::CalculateAlpha()
{
    int theIP = 0; // This is a bit of a hack... I am sorry.

    if (GetConstitutiveLaw(theIP)->GetParameterDouble(
                Constitutive::eConstitutiveParameter::NONLOCAL_RADIUS_PARAMETER) != 0)
        throw MechanicsException(__PRETTY_FUNCTION__, "The case c != const is currently not supported. Set "
                                                      "eConstitutiveParameter::NONLOCAL_RADIUS_PARAMETER to 0.");

    double c = GetConstitutiveLaw(theIP)->GetParameterDouble(Constitutive::eConstitutiveParameter::NONLOCAL_RADIUS);
    return std::sqrt(c);
}


template <>
Eigen::Matrix<double, 0, 1> ContinuumBoundaryElement<1>::CalculateIPCoordinatesSurface(int rTheIP) const
{
    return Eigen::Matrix<double, 0, 1>();
}

template <>
Eigen::Matrix<double, 1, 1> ContinuumBoundaryElement<2>::CalculateIPCoordinatesSurface(int rTheIP) const
{
    double tmp;
    GetIntegrationType()->GetLocalIntegrationPointCoordinates1D(rTheIP, tmp);
    Eigen::Matrix<double, 1, 1> ipCoordinatesSurface;
    ipCoordinatesSurface(0) = tmp;
    return ipCoordinatesSurface;
}

template <>
Eigen::Matrix<double, 2, 1> ContinuumBoundaryElement<3>::CalculateIPCoordinatesSurface(int rTheIP) const
{
    double tmp[2];
    GetIntegrationType()->GetLocalIntegrationPointCoordinates2D(rTheIP, tmp);
    Eigen::Matrix<double, 2, 1> ipCoordinatesSurface;
    ipCoordinatesSurface(0) = tmp[0];
    ipCoordinatesSurface(1) = tmp[1];
    return ipCoordinatesSurface;
}

template <int TDim>
const Eigen::VectorXd ContinuumBoundaryElement<TDim>::GetIntegrationPointVolume() const
{
    const InterpolationBase& interpolationTypeCoords = mInterpolationType->Get(Node::eDof::COORDINATES);
    Eigen::MatrixXd nodeCoordinates = this->ExtractNodeValues(0, Node::eDof::COORDINATES);

    Eigen::VectorXd volume(GetNumIntegrationPoints());
    for (int theIP = 0; theIP < GetNumIntegrationPoints(); theIP++)
    {
        Eigen::Matrix<double, TDim - 1, 1> ipCoordsSurface = CalculateIPCoordinatesSurface(theIP);
        Eigen::Matrix<double, TDim, 1> ipCoordsNatural =
                interpolationTypeCoords.CalculateNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);

        // #######################################
        // ##  Calculate the surface jacobian
        // ## = || [dX / dXi] * [dXi / dAlpha] ||
        // #######################################
        Eigen::MatrixXd derivativeShapeFunctionsNatural =
                interpolationTypeCoords.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);
        const Eigen::Matrix<double, TDim, TDim> jacobian =
                mBaseElement->CalculateJacobian(derivativeShapeFunctionsNatural, nodeCoordinates); // = [dX / dXi]

        const Eigen::MatrixXd derivativeNaturalSurfaceCoordinates =
                interpolationTypeCoords.CalculateDerivativeNaturalSurfaceCoordinates(ipCoordsSurface,
                                                                                     mSurfaceId); // = [dXi / dAlpha]
        double detJacobian = (jacobian * derivativeNaturalSurfaceCoordinates).norm();
        volume[theIP] = detJacobian * mElementData->GetIntegrationType()->GetIntegrationPointWeight(theIP);
    }

    return volume;
}

template <>
const Eigen::VectorXd ContinuumBoundaryElement<3>::GetIntegrationPointVolume() const
{
    const InterpolationBase& interpolationTypeCoords = mInterpolationType->Get(Node::eDof::COORDINATES);
    Eigen::MatrixXd nodeCoordinates = this->ExtractNodeValues(0, Node::eDof::COORDINATES);

    Eigen::VectorXd volume(GetNumIntegrationPoints());
    for (int theIP = 0; theIP < GetNumIntegrationPoints(); theIP++)
    {
        Eigen::Matrix<double, 2, 1> ipCoordsSurface = CalculateIPCoordinatesSurface(theIP);
        Eigen::Matrix<double, 3, 1> ipCoordsNatural =
                interpolationTypeCoords.CalculateNaturalSurfaceCoordinates(ipCoordsSurface, mSurfaceId);

        Eigen::MatrixXd derivativeShapeFunctionsNatural =
                interpolationTypeCoords.CalculateDerivativeShapeFunctionsNatural(ipCoordsNatural);

        const Eigen::Matrix<double, 3, 3> jacobian =
                mBaseElement->CalculateJacobian(derivativeShapeFunctionsNatural, nodeCoordinates); // = [dX / dXi]

        const Eigen::MatrixXd derivativeNaturalSurfaceCoordinates =
                interpolationTypeCoords.CalculateDerivativeNaturalSurfaceCoordinates(ipCoordsSurface,
                                                                                     mSurfaceId); // = [dXi / dAlpha]
        // #######################################
        // ##  Calculate the surface jacobian
        // ## = || [dX / dXi] x [dXi / dAlpha] ||
        // #######################################
        Eigen::Vector3d dXdAlpha = jacobian * derivativeNaturalSurfaceCoordinates.col(0);
        Eigen::Vector3d dXdBeta = jacobian * derivativeNaturalSurfaceCoordinates.col(1);

        NuTo::FullVector<double, 3> surfaceNormalVector =
                dXdAlpha.cross(dXdBeta); // = || [dX / dXi] * [dXi / dAlpha] ||

        double detJacobian = surfaceNormalVector.Norm();

        volume[theIP] = detJacobian * mElementData->GetIntegrationType()->GetIntegrationPointWeight(theIP);
    }

    return volume;
}

template <>
double NuTo::ContinuumBoundaryElement<1>::CalculateDetJxWeightIPxSection(double rDetJacobian, int rTheIP) const
{

    return mBaseElement->mSection->GetArea();
}

template <>
double NuTo::ContinuumBoundaryElement<2>::CalculateDetJxWeightIPxSection(double rDetJacobian, int rTheIP) const
{
    return rDetJacobian * mElementData->GetIntegrationType()->GetIntegrationPointWeight(rTheIP) *
           mBaseElement->mSection->GetThickness();
}

template <>
double NuTo::ContinuumBoundaryElement<3>::CalculateDetJxWeightIPxSection(double rDetJacobian, int rTheIP) const
{
    return rDetJacobian * mElementData->GetIntegrationType()->GetIntegrationPointWeight(rTheIP);
}


template <>
const ContinuumBoundaryElement<1>& ContinuumBoundaryElement<1>::AsContinuumBoundaryElement1D() const
{
    return *this;
}

template <>
const ContinuumBoundaryElement<2>& ContinuumBoundaryElement<2>::AsContinuumBoundaryElement2D() const
{
    return *this;
}

template <>
const ContinuumBoundaryElement<3>& ContinuumBoundaryElement<3>::AsContinuumBoundaryElement3D() const
{
    return *this;
}

template <>
ContinuumBoundaryElement<1>& ContinuumBoundaryElement<1>::AsContinuumBoundaryElement1D()
{
    return *this;
}

template <>
ContinuumBoundaryElement<2>& ContinuumBoundaryElement<2>::AsContinuumBoundaryElement2D()
{
    return *this;
}

template <>
ContinuumBoundaryElement<3>& ContinuumBoundaryElement<3>::AsContinuumBoundaryElement3D()
{
    return *this;
}

} // namespace NuTo

template class NuTo::ContinuumBoundaryElement<1>;
template class NuTo::ContinuumBoundaryElement<2>;
template class NuTo::ContinuumBoundaryElement<3>;
