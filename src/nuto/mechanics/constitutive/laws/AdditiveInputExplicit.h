#pragma once

#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/staticData/ConstitutiveStaticDataMultipleConstitutiveLaws.h"

#include <set>
#include <vector>

namespace NuTo
{

class AdditiveInputExplicit : public ConstitutiveBase
{
public:

    //! @brief constructor
    AdditiveInputExplicit()
        : ConstitutiveBase()
    {
        mComputableDofCombinations.resize(2); //VHIRTHAMTODO ---> Get number time derivatives during construction (as parameter)
    }

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    ConstitutiveStaticDataBase* AllocateStaticData1D(const ElementBase* rElement) const override
    {
        mStaticDataAllocated = true;
        std::vector<NuTo::ConstitutiveBase*> tempVec;
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            tempVec.push_back(mConstitutiveLawsModInput[i].first);
        }
        tempVec.push_back(mConstitutiveLawOutput);
        return new ConstitutiveStaticDataMultipleConstitutiveLaws(tempVec,rElement,1);
    }

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    ConstitutiveStaticDataBase* AllocateStaticData2D(const ElementBase* rElement) const override
    {
        mStaticDataAllocated = true;
        std::vector<NuTo::ConstitutiveBase*> tempVec;
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            tempVec.push_back(mConstitutiveLawsModInput[i].first);
        }
        tempVec.push_back(mConstitutiveLawOutput);
        return new ConstitutiveStaticDataMultipleConstitutiveLaws(tempVec,rElement,2);
    }

    //! @brief ... create new static data object for an integration point
    //! @return ... pointer to static data object
    ConstitutiveStaticDataBase* AllocateStaticData3D(const ElementBase* rElement) const override
    {
        mStaticDataAllocated = true;
        std::vector<NuTo::ConstitutiveBase*> tempVec;
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            tempVec.push_back(mConstitutiveLawsModInput[i].first);
        }
        tempVec.push_back(mConstitutiveLawOutput);
        return new ConstitutiveStaticDataMultipleConstitutiveLaws(tempVec,rElement,3);
    }

    //! @brief ... adds a constitutive law to a model that combines multiple constitutive laws (additive, parallel)
    //! @param rConstitutiveLaw ... additional constitutive law
    //! @param rModiesInput ... enum which defines wich input is modified by a constitutive law.
    virtual void  AddConstitutiveLaw(NuTo::ConstitutiveBase* rConstitutiveLaw, Constitutive::Input::eInput rModiesInput = Constitutive::Input::NONE) override;


    //! @brief ... determines which submatrices of a multi-doftype problem can be solved by the constitutive law
    //! @param rDofRow ... row dof
    //! @param rDofCol ... column dof
    //! @param rTimeDerivative ... time derivative
    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow,
                                               Node::eDof rDofCol,
                                               int rTimeDerivative) const override;

    //! @brief ... check compatibility between element type and type of constitutive relationship
    //! @param rElementType ... element type
    //! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
    virtual bool CheckElementCompatibility( Element::eElementType rElementType) const override
    {
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            if(!mConstitutiveLawsModInput[i].first->CheckElementCompatibility(rElementType))
                return false;
        }
        return true;
    }

    //! @brief ... check parameters of the constitutive relationship
    //! if one check fails, an exception is thrwon
    virtual void CheckParameters() const override
    {
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            mConstitutiveLawsModInput[i].first->CheckParameters();
        }
    }

    //! @brief ... evaluate the constitutive relation
    //! @param rElement ... element
    //! @param rIp ... integration point
    //! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
    //! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
    template <int TDim>
    NuTo::Error::eError EvaluateAdditiveInputExplicit(  ElementBase* rElement,
                                                        int rIp,
                                                        const ConstitutiveInputMap& rConstitutiveInput,
                                                        const ConstitutiveOutputMap& rConstitutiveOutput);


    //! @brief ... evaluate the constitutive relation of every attached constitutive law in 1D
    //! @param rElement ... element
    //! @param rIp ... integration point
    //! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
    //! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
    virtual NuTo::Error::eError Evaluate1D(ElementBase* rElement,
                                           int rIp,
                                           const ConstitutiveInputMap& rConstitutiveInput,
                                           const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return EvaluateAdditiveInputExplicit<1>(rElement,
                                                rIp,
                                                rConstitutiveInput,
                                                rConstitutiveOutput);
    }

    //! @brief ... evaluate the constitutive relation of every attached constitutive law in 2D
    //! @param rElement ... element
    //! @param rIp ... integration point
    //! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
    //! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
    virtual NuTo::Error::eError Evaluate2D(ElementBase* rElement,
                                           int rIp,
                                           const ConstitutiveInputMap& rConstitutiveInput,
                                           const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return EvaluateAdditiveInputExplicit<2>(rElement,
                                                rIp,
                                                rConstitutiveInput,
                                                rConstitutiveOutput);

    }

    //! @brief ... evaluate the constitutive relation of every attached constitutive law relation in 3D
    //! @param rElement ... element
    //! @param rIp ... integration point
    //! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
    //! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
    virtual NuTo::Error::eError Evaluate3D(ElementBase* rElement,
                                           int rIp,
                                           const ConstitutiveInputMap& rConstitutiveInput,
                                           const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        return EvaluateAdditiveInputExplicit<3>(rElement,
                                                rIp,
                                                rConstitutiveInput,
                                                rConstitutiveOutput);
    }



    //! @brief ... determines the constitutive inputs needed to evaluate the constitutive outputs
    //! @param rConstitutiveOutput ... desired constitutive outputs
    //! @param rInterpolationType ... interpolation type to determine additional inputs
    //! @return constitutive inputs needed for the evaluation
    virtual ConstitutiveInputMap GetConstitutiveInputs( const ConstitutiveOutputMap& rConstitutiveOutput,
                                                        const InterpolationType& rInterpolationType) const override;

    //! @brief ... get type of constitutive relationship
    //! @return ... type of constitutive relationship
    //! @sa eConstitutiveType
    virtual Constitutive::eConstitutiveType GetType() const override
    {
        return NuTo::Constitutive::ADDITIVE_INPUT_EXPLICIT;
    }

    //! @brief ... returns true, if a material model has tmp static data (which has to be updated before stress or stiffness are calculated)
    //! @return ... see brief explanation
    virtual bool HaveTmpStaticData() const override
    {
        for(unsigned int i=0; i<mConstitutiveLawsModInput.size(); ++i)
        {
            if(mConstitutiveLawsModInput[i].first->HaveTmpStaticData())
                return true;
        }
        return false;
    }

private:

    //! @brief Gets the output enum from an input enum
    Constitutive::Output::eOutput GetOutputEnumFromInputEnum(Constitutive::Input::eInput rInputEnum);

    //! @brief Adds all calculable dof combinations of an attached constitutive law to an internal storage
    //! @param rConstitutiveLaw ... constitutive law
    void AddCalculableDofCombinations(NuTo::ConstitutiveBase* rConstitutiveLaw);

    //! @brief ... pointer to constitutive law that delivers the output
    NuTo::ConstitutiveBase* mConstitutiveLawOutput = nullptr;

    //! @brief ... vector of pairs which store the pointers to input modifying constitutive laws together with the modified inputs enum.
    std::vector<std::pair<NuTo::ConstitutiveBase*,Constitutive::Input::eInput>> mConstitutiveLawsModInput;

    //! @brief ... set of all Inputs that should be modified
    std::set<Constitutive::Input::eInput> mModifiedInputs;

    std::vector<std::set<std::pair<Node::eDof,Node::eDof>>> mComputableDofCombinations;

    //! @brief debug variable to avoid that a constitutive law can be attached after allocation of static data.
    mutable bool mStaticDataAllocated = false;
};

}