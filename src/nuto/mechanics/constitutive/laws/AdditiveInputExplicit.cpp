#include "AdditiveInputExplicit.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStress.h"

void NuTo::AdditiveInputExplicit::AddConstitutiveLaw(NuTo::ConstitutiveBase *rConstitutiveLaw, Constitutive::Input::eInput rModiesInput)
{
    if(mStaticDataAllocated)
        throw MechanicsException(__PRETTY_FUNCTION__,"All constitutive laws have to be attached before static data is allocated!");
    if(rModiesInput == Constitutive::Input::NONE)
    {
        if (mConstitutiveLawOutput != nullptr)
            throw MechanicsException(__PRETTY_FUNCTION__,std::string("There can be only one!!! --- This additive input law only accepts one law which calculates the output. All other laws ")+
                                     " are only allowed to modify the input to this law. Specify the modifying laws by providing the enum of the modified input as second function parameter.");
        mConstitutiveLawOutput = rConstitutiveLaw;
    }
    else
    {
        mModifiedInputs.insert(rModiesInput);
        mConstitutiveLawsModInput.push_back({rConstitutiveLaw,rModiesInput});
    }
    AddCalculableDofCombinations(rConstitutiveLaw);
}


bool NuTo::AdditiveInputExplicit::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow, NuTo::Node::eDof rDofCol, int rTimeDerivative) const
{
    if(mComputableDofCombinations[rTimeDerivative].find(std::pair<Node::eDof,Node::eDof>(rDofRow,rDofCol)) != mComputableDofCombinations[rTimeDerivative].end())
        return true;
    return false;
}

template <int TDim>
NuTo::Error::eError NuTo::AdditiveInputExplicit::EvaluateAdditiveInputExplicit(NuTo::ElementBase *rElement,
                                                                               int rIp,
                                                                               const NuTo::ConstitutiveInputMap &rConstitutiveInput,
                                                                               const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    static constexpr int VoigtDim = ConstitutiveIOBase::GetVoigtDim(TDim);
    Error::eError error = Error::SUCCESSFUL;

     NuTo::ConstitutiveInputMap copiedInputMap;

    // Need deep copy of all modified inputs, otherwise the modifications will possibly effect other constitutive laws that are coupled in a additive input or output law
    // So far only the engineering strain input is modified. If there are more modified Inputs in the future, a more general allocation method should be chosen
    EngineeringStrain<TDim>* engineeringStrain = nullptr;
    ConstitutiveVector<VoigtDim> d_EngineeringStrain_D_RH;
    ConstitutiveVector<VoigtDim> d_EngineeringStrain_D_WV;
    ConstitutiveVector<VoigtDim> d_EngineeringStrain_D_Temperature;

    d_EngineeringStrain_D_RH.setZero();
    d_EngineeringStrain_D_WV.setZero();
    d_EngineeringStrain_D_Temperature.setZero();

    copiedInputMap.Join(rConstitutiveInput);

    if (copiedInputMap.find(Constitutive::Input::ENGINEERING_STRAIN) != copiedInputMap.end())
    {
        switch(TDim)
        {
        case 1:
            copiedInputMap.at(Constitutive::Input::ENGINEERING_STRAIN)->AsEngineeringStrain1D() *=-1.0;
            break;
        case 2:
            copiedInputMap.at(Constitutive::Input::ENGINEERING_STRAIN)->AsEngineeringStrain2D() *=-1.0;
            break;
        case 3:
            copiedInputMap.at(Constitutive::Input::ENGINEERING_STRAIN)->AsEngineeringStrain3D() *=-1.0;
            break;
        default:
            throw MechanicsException(__PRETTY_FUNCTION__,"invalid dimension");
        }
        engineeringStrain = static_cast<EngineeringStrain<TDim>*>(copiedInputMap.at(Constitutive::Input::ENGINEERING_STRAIN).get()); 
    }

    for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
    {
        // Generate modified output map for constitutive law
        NuTo::ConstitutiveOutputMap modifiedOutputMap = rConstitutiveOutput;

        if(copiedInputMap.find(Constitutive::Input::ENGINEERING_STRAIN) != copiedInputMap.end())
        {
            modifiedOutputMap[Constitutive::Output::ENGINEERING_STRAIN] = copiedInputMap.at(Constitutive::Input::ENGINEERING_STRAIN).get();
        }
        if(modifiedOutputMap.find(Constitutive::Output::D_ENGINEERING_STRESS_D_RELATIVE_HUMIDITY) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Constitutive::Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Constitutive::Output::D_ENGINEERING_STRAIN_D_RELATIVE_HUMIDITY] = &d_EngineeringStrain_D_RH;
        }
        if(modifiedOutputMap.find(Constitutive::Output::D_ENGINEERING_STRESS_D_WATER_VOLUME_FRACTION) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Constitutive::Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Constitutive::Output::D_ENGINEERING_STRAIN_D_WATER_VOLUME_FRACTION] = &d_EngineeringStrain_D_WV;
        }
        if(modifiedOutputMap.find(Constitutive::Output::D_ENGINEERING_STRESS_D_TEMPERATURE) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Constitutive::Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Constitutive::Output::D_STRAIN_D_TEMPERATURE] = &d_EngineeringStrain_D_Temperature;
        }


        // evaluate constitutive law
        switch(TDim)
        {
        case 1:
            error = mConstitutiveLawsModInput[i].first->Evaluate1D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        case 2:
            error = mConstitutiveLawsModInput[i].first->Evaluate2D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        case 3:
            error = mConstitutiveLawsModInput[i].first->Evaluate3D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        default:
            throw MechanicsException(__PRETTY_FUNCTION__,"invalid dimension");
        }
        if(error != Error::SUCCESSFUL)
        {
            throw MechanicsException(__PRETTY_FUNCTION__,"Attached constitutive law returned an error code. Can't handle this");
        }
    }

    if (engineeringStrain != nullptr)
    {
        engineeringStrain->AsVector() = engineeringStrain->AsVector() * -1;
    }

    switch(TDim)
    {
    case 1:
        error = mConstitutiveLawOutput->Evaluate1D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    case 2:
        error = mConstitutiveLawOutput->Evaluate2D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    case 3:
        error = mConstitutiveLawOutput->Evaluate3D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    default:
        throw MechanicsException(__PRETTY_FUNCTION__,"invalid dimension");
    }

    for(auto itOutput : rConstitutiveOutput)
    {
        switch(itOutput.first)
        {
        case Constitutive::Output::D_ENGINEERING_STRESS_D_RELATIVE_HUMIDITY:
        {
            assert(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN) != rConstitutiveOutput.end());
            ConstitutiveMatrix<VoigtDim, VoigtDim>& tangentStressStrain = *(static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN)->second));
            if(d_EngineeringStrain_D_RH.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__,std::string("Necessary value to determine ")+Constitutive::OutputToString(itOutput.first)+" was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second))->AsVector() = tangentStressStrain.AsMatrix() * d_EngineeringStrain_D_RH.AsVector();
            break;
        }
        case Constitutive::Output::D_ENGINEERING_STRESS_D_WATER_VOLUME_FRACTION:
        {
            assert(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN)!=rConstitutiveOutput.end());
            ConstitutiveMatrix<VoigtDim, VoigtDim>& tangentStressStrain = *(static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN)->second));
            if(d_EngineeringStrain_D_WV.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__,std::string("Necessary value to determine ")+Constitutive::OutputToString(itOutput.first)+" was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second))->AsVector() = tangentStressStrain.AsMatrix() * d_EngineeringStrain_D_WV.AsVector();
            break;
        }
        case Constitutive::Output::D_ENGINEERING_STRESS_D_TEMPERATURE:
        {
            assert(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN) != rConstitutiveOutput.end());
            ConstitutiveMatrix<VoigtDim, VoigtDim>& tangentStressStrain = *(static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.find(Constitutive::Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN)->second));
            if(d_EngineeringStrain_D_Temperature.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__, "Necessary value to determine " + Constitutive::OutputToString(itOutput.first) + " was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second))->AsVector() = tangentStressStrain.AsMatrix() * d_EngineeringStrain_D_Temperature.AsVector();
            break;
        }
        default:
            continue;
        }
        itOutput.second->SetIsCalculated(true);
    }
    return error;
}



NuTo::ConstitutiveInputMap NuTo::AdditiveInputExplicit::GetConstitutiveInputs(
        const NuTo::ConstitutiveOutputMap &rConstitutiveOutput,
        const NuTo::InterpolationType &rInterpolationType) const
{
    ConstitutiveInputMap constitutiveInputMap;

    for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
    {

        ConstitutiveInputMap singleLawInputMap = mConstitutiveLawsModInput[i].first->GetConstitutiveInputs(rConstitutiveOutput,
                                                                                                           rInterpolationType);

        constitutiveInputMap.Join(singleLawInputMap);
    }

    ConstitutiveInputMap outputLawInputMap = mConstitutiveLawOutput->GetConstitutiveInputs(rConstitutiveOutput,
                                                                                           rInterpolationType);
    constitutiveInputMap.Join(outputLawInputMap);

    return constitutiveInputMap;
}



NuTo::Constitutive::Output::eOutput NuTo::AdditiveInputExplicit::GetOutputEnumFromInputEnum(NuTo::Constitutive::Input::eInput rInputEnum)
{
    switch(rInputEnum)
    {
    case Constitutive::Input::ENGINEERING_STRAIN:
        return Constitutive::Output::ENGINEERING_STRAIN;
    default:
        throw MechanicsException(__PRETTY_FUNCTION__,std::string("There is no output enum specified which is related to the input enum ")+Constitutive::InputToString(rInputEnum));
    }
}

void NuTo::AdditiveInputExplicit::AddCalculableDofCombinations(NuTo::ConstitutiveBase *rConstitutiveLaw)
{
    std::set<Node::eDof> allDofs = Node::GetDofSet();
    for (unsigned int i=0; i<mComputableDofCombinations.size(); ++i)
    for (auto itRow : allDofs)
        for (auto itCol : allDofs)
        {
            if (rConstitutiveLaw->CheckDofCombinationComputable(itRow,itCol,i))
                    mComputableDofCombinations[i].emplace(itRow,itCol);
        }
}
