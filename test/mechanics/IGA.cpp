﻿#include <iostream>
#include "mechanics/structures/StructureBase.h"
#include "mechanics/structures/unstructured/Structure.h"

#include <eigen3/Eigen/Core>

#include "math/SparseMatrixCSRGeneral.h"
#include "math/SparseMatrixCSRSymmetric.h"
#include "math/SparseDirectSolverMUMPS.h"
#include "math/SparseDirectSolverPardiso.h"

#include "mechanics/integrationtypes/IntegrationType1D2NLobatto3Ip.h"
#include "mechanics/integrationtypes/IntegrationType1D2NLobatto4Ip.h"
#include "mechanics/integrationtypes/IntegrationType1D2NLobatto5Ip.h"
#include "mechanics/integrationtypes/IntegrationType2D4NLobatto9Ip.h"
#include "mechanics/integrationtypes/IntegrationType2D4NLobatto16Ip.h"
#include "mechanics/integrationtypes/IntegrationType2D4NLobatto25Ip.h"

#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "mechanics/interpolationtypes/InterpolationTypeEnum.h"

#include "mechanics/IGA/BSplineCurve.h"
#include "mechanics/IGA/BSplineSurface.h"

#include "mechanics/nodes/NodeBase.h"
#include "mechanics/nodes/NodeEnum.h"

#include "mechanics/groups/GroupEnum.h"

#include <boost/filesystem.hpp>

#ifdef ENABLE_VISUALIZE
#include "visualize/VisualizeEnum.h"
#endif

#define PRINTRESULT true

/*  ||>*----*----*----*----*
       |    |    |    |    | -->
       |    |    |    |    |
    ||>*----*----*----*----*     Sigma
       |    |    |    |    | -->
       |    |    |    |    |
    ||>*----*----*----*----*
       ^
*/
NuTo::BSplineSurface buildRect2D(double x0, double y0, double Height, double Length)
{
    /** Knots and control points **/
    int numElementsX = 1;
    int numElementsY = 1;

    Eigen::Vector2i degree(2,2);

    int numKnotsX = 2*(degree(0)+1) + numElementsX - 1;
    int numKnotsY = 2*(degree(1)+1) + numElementsY - 1;

    Eigen::VectorXd knotsX(numKnotsX);
    Eigen::VectorXd knotsY(numKnotsY);

    for (int i = 0; i <= degree(0); i++) knotsX(i) = 0.;
    for (int i = degree(0) + 1; i <= degree(0) + numElementsX - 1 ; i++) knotsX(i) = knotsX(i-1) + 1./numElementsX;
    for (int i = degree(0) + numElementsX ; i < numKnotsX; i++) knotsX(i) = 1;

    for (int i = 0; i <= degree(1); i++) knotsY(i) = 0.;
    for (int i = degree(1) + 1; i <= degree(1) + numElementsY - 1 ; i++) knotsY(i) = knotsY(i-1) + 1./numElementsY;
    for (int i = degree(1) + numElementsY ; i < numKnotsY; i++) knotsY(i) = 1;

    int numControlPointsX = (numKnotsX -1) - degree(0);
    int numControlPointsY = (numKnotsY -1) - degree(1);

    Eigen::Matrix<Eigen::VectorXd, Eigen::Dynamic, Eigen::Dynamic> controlPoints(numControlPointsY, numControlPointsX);

    double incrx = (Length/(numControlPointsX-1));
    double incry = (Height/(numControlPointsY-1));
    for(int i = 0; i < numControlPointsY; i++)
    {
        for(int j = 0; j < numControlPointsX; j++)
        {
            controlPoints(i,j) = Eigen::Vector2d(x0 + incrx*j, y0 + incry*i);
        }
    }

    Eigen::MatrixXd weights(numControlPointsY, numControlPointsX);
    weights.setOnes(numControlPointsY, numControlPointsX);

    return NuTo::BSplineSurface(degree, knotsX, knotsY, controlPoints, weights);
}


NuTo::Structure* constantStress(double& DisplacementCorrect, int refinements, const std::string &resultDir)
{
    /** parameters **/
    double  YoungsModulus = 20000.;
    double  PoissonRatio = 0.3;
    double  Height = 5.;
    // there should be no dependency, because the pressure at the boundary is predefined
    double  Thickness = 2.123548;
    double  Length = 10;
    double  Stress = 10.;

    DisplacementCorrect = (Stress*Length)/YoungsModulus;

    NuTo::BSplineSurface surface = buildRect2D(0, 0, Height, Length);

    /** Structure 2D **/
    NuTo::Structure* myStructure = new NuTo::Structure(2);

#ifdef _OPENMP
    int numThreads = 1;
    myStructure->SetNumProcessors(numThreads);
#endif

    /** create section **/
    int mySection = myStructure->SectionCreate("Plane_Stress");
    myStructure->SectionSetThickness(mySection,Thickness);

    /** create constitutive law **/
    int myMatLin = myStructure->ConstitutiveLawCreate("LINEAR_ELASTIC_ENGINEERING_STRESS");
    myStructure->ConstitutiveLawSetParameterDouble(myMatLin,NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS,YoungsModulus);
    myStructure->ConstitutiveLawSetParameterDouble(myMatLin,NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO,PoissonRatio);

    for(int i = 0; i < refinements; i++)
    {
        surface.DuplicateKnots(0);
        surface.DuplicateKnots(1);
    }

    std::set<NuTo::Node::eDof> setOfDOFS;
    setOfDOFS.insert(NuTo::Node::eDof::COORDINATES);
    setOfDOFS.insert(NuTo::Node::eDof::DISPLACEMENTS);

    int groupNodes  = myStructure->GroupCreate("Nodes");
    int groupElements = myStructure->GroupCreate("Elements");

    surface.buildIGAStructure(*myStructure, setOfDOFS, groupElements, groupNodes);

    myStructure->Info();

    /** assign constitutive law **/
    myStructure->ElementTotalSetConstitutiveLaw(myMatLin);
    myStructure->ElementTotalSetSection(mySection);

    /** Boundary condition **/

    // Dirichlet
    Eigen::VectorXd direction(2);

    direction << 1, 0;
    for(int i = 0; i < surface.GetNumControlPoints(1); i++)
    {
        myStructure->ConstraintLinearSetDisplacementNode(i*surface.GetNumControlPoints(0), direction, 0.0);
    }

    direction << 0, 1;
    for(int i = 0; i <  surface.GetNumControlPoints(0); i++)
    {
        myStructure->ConstraintLinearSetDisplacementNode(i, direction, 0.0);
    }

    // right boundary
    int groupNumberNodesLeft = myStructure->GroupCreate("NODES");

    for(int i = 1; i <=  surface.GetNumControlPoints(1); i++)
    {
        myStructure->GroupAddNode(groupNumberNodesLeft, i*surface.GetNumControlPoints(0) - 1);
    }

    int groupNumberElementsLeft = myStructure->GroupCreate("ELEMENTS");
    for(int i = 1; i <= surface.GetNumIGAElements(1); i++)
    {
        myStructure->GroupAddElement(groupNumberElementsLeft, i*surface.GetNumIGAElements(0) - 1);
    }

    myStructure->SetNumLoadCases(1);

    myStructure->LoadSurfacePressureCreate2D(0, groupNumberElementsLeft, groupNumberNodesLeft, -Stress);

    myStructure->CalculateMaximumIndependentSets();
    myStructure->NodeBuildGlobalDofs();

    myStructure->NodeInfo(10);

    return myStructure;
}

Eigen::VectorXd exact_plate_hole_disp(Eigen::VectorXd pt)
{
    double a = 1.0;
    double T = -1.;
    double E = 1.e5;
    double nu = 0.3;
    Eigen::VectorXd disp(2);
    double r     = std::sqrt(pt(0)*pt(0) + pt(1)*pt(1));
    double theta = std::atan(pt(1)/pt(0));

    double ct = std::cos(theta);
    double c3t = std::cos(3*theta);
    double st = std::sin(theta);
    double s3t = std::sin(3*theta);

    double mu = E/(1. - 2.*nu);
    double k =  (3.- nu)/(1. + nu);

    disp(0) = ((T*a)/(8.*mu)) * ( (r/a)*(k + 1.)*ct + ((2.*a)/r)*( (1. + k)*ct + c3t ) - ((2.*a*a*a)/(r*r*r))*c3t );
    disp(1) = ((T*a)/(8.*mu)) * ( (r/a)*(k - 3.)*st + ((2.*a)/r)*( (1. - k)*st + s3t ) - ((2.*a*a*a)/(r*r*r))*s3t );

//    disp(0) = ((8.*a*(1 + nu)*T)/E) * ((r/a)*(2./(1+nu))*ct + (a/r)*((4./(1+nu))*ct + c3t) - (a/r)*(a/r)*(a/r)*c3t);
//    disp(1) = ((8.*a*(1 + nu)*T)/E) * ((r/a)*((-2.*nu)/(1+nu))*st + (a/r)*(((-4.*nu)/(1+nu))*st + s3t) - (a/r)*(a/r)*(a/r)*s3t);

    return disp;
}

Eigen::VectorXd exact_plate_hole(Eigen::VectorXd pt)
{
    double a = 1.0;
    Eigen::VectorXd stress(3);
    double r     = std::sqrt(pt(0)*pt(0) + pt(1)*pt(1));
    double theta = std::atan(pt(1)/pt(0));

    double c2t = std::cos(2*theta);
    double c4t = std::cos(4*theta);
    double s2t = std::sin(2*theta);
    double s4t = std::sin(4*theta);

    double fac1 = (a/r)*(a/r);
    double fac2 = fac1*fac1;
    stress(0) = 1. - fac1*(1.5*c2t+c4t)+1.5*fac2*c4t; // xx
    stress(1) =    - fac1*(0.5*c2t-c4t)-1.5*fac2*c4t; // yy
    stress(2) =    - fac1*(0.5*s2t+s4t)+1.5*fac2*s4t; // xy

    return stress;
}


Eigen::Vector2d exact_plate_hole_left(Eigen::VectorXd pt)
{
    Eigen::VectorXd stress = exact_plate_hole(pt);
    Eigen::Vector2d stressRet(-stress(0), -stress(2));
    return stressRet;
}

Eigen::Vector2d exact_plate_hole_upper(Eigen::VectorXd pt)
{
    Eigen::VectorXd stress = exact_plate_hole(pt);
    Eigen::Vector2d stressRet(-stress(2), -stress(1));
    return stressRet;
}


NuTo::Structure* buildPlateWithHole2DNeumann(const std::string &resultDir, int refine, int BC)
{
    NuTo::Structure* myStructure = new NuTo::Structure(2);

    /*********/
    // Mesh  //
    /*********/

    int noPtsX = 3;
    int noPtsY = 4;

    Eigen::Matrix<Eigen::VectorXd, Eigen::Dynamic, Eigen::Dynamic> controlPts(noPtsX, noPtsY);

    controlPts(0,0) = Eigen::Vector2d(-1, 0);
    controlPts(0,1) = Eigen::Vector2d(-1, 0.4142135623730951);
    controlPts(0,2) = Eigen::Vector2d(-0.4142135623730951, 1);
    controlPts(0,3) = Eigen::Vector2d(0,1);

    controlPts(1,0) = Eigen::Vector2d(-2.5,  0);
    controlPts(1,1) = Eigen::Vector2d(-2.5,  0.75);
    controlPts(1,2) = Eigen::Vector2d(-0.75, 2.5);
    controlPts(1,3) = Eigen::Vector2d( 0,    2.5);

    controlPts(2,0) = Eigen::Vector2d(-4,0);
    controlPts(2,1) = Eigen::Vector2d(-4,4);
    controlPts(2,2) = Eigen::Vector2d(-4,4);
    controlPts(2,3) = Eigen::Vector2d(0,4);

    Eigen::MatrixXd weights(noPtsX, noPtsY);
    weights.setOnes(noPtsX, noPtsY);
    weights(0,1) = (1. + 1./sqrt(2))/2.;
    weights(0,2) = (1. + 1./sqrt(2))/2.;


    Eigen::VectorXd knotsX(7);
    knotsX << 0, 0, 0, 0.5, 1, 1, 1;
    Eigen::VectorXd knotsY(6);
    knotsY << 0, 0, 0, 1, 1, 1;

    Eigen::Vector2i degree(2,2);

    NuTo::BSplineSurface surface(degree, knotsX, knotsY, controlPts, weights);

    for(int i = 0; i < refine; i++)
    {
        surface.DuplicateKnots(0);
        surface.DuplicateKnots(1);
    }

    /**************/
    // Structure  //
    /**************/

    std::set<NuTo::Node::eDof> setOfDOFS;
    setOfDOFS.insert(NuTo::Node::eDof::COORDINATES);
    setOfDOFS.insert(NuTo::Node::eDof::DISPLACEMENTS);

    int groupNodes  = myStructure->GroupCreate("Nodes");
    int groupElements = myStructure->GroupCreate("Elements");

    surface.buildIGAStructure(*myStructure, setOfDOFS, groupElements, groupNodes);

    /** create section **/
    double Thickness = 1.;
    int mySection = myStructure->SectionCreate("Plane_Stress");
    myStructure->SectionSetThickness(mySection,Thickness);

    /** create constitutive law **/
    double YoungsModulus = 1.e5;
    double PoissonRatio  = 0.3;
    int myMatLin = myStructure->ConstitutiveLawCreate("LINEAR_ELASTIC_ENGINEERING_STRESS");
    myStructure->ConstitutiveLawSetParameterDouble(myMatLin, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, YoungsModulus);
    myStructure->ConstitutiveLawSetParameterDouble(myMatLin, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, PoissonRatio);

    myStructure->ElementTotalSetConstitutiveLaw(myMatLin);
    myStructure->ElementTotalSetSection(mySection);

    /**********************/
    // Boundary condition //
    /**********************/

    int groupElementsLeft  = myStructure->GroupCreate("ELEMENTS");
    int groupElementsUpper = myStructure->GroupCreate("ELEMENTS");

    int start = (surface.GetNumIGAElements(1)-1) * surface.GetNumIGAElements(0);
    for(int i = start; i < start + surface.GetNumIGAElements(0)/2; i++)
    {
        myStructure->GroupAddElement(groupElementsLeft, i);
    }

    start += surface.GetNumIGAElements(0)/2;
    for(int i = start; i < surface.GetNumIGAElements(); i++)
    {
        myStructure->GroupAddElement(groupElementsUpper, i);
    }

    auto LambdaGetBoundaryNodesLeft = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= -4 - Tol && x <= -4 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda

    auto LambdaGetBoundaryNodesUpper = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
                                        if ((y >= 4 - Tol && y <= 4 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda


    auto LambdaGetBoundaryNodesLower = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
                                        if ((y >= 0 - Tol && y <= 0 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // LambdaGetBoundaryNodesLowerSymmetry

    auto LambdaGetBoundaryNodesRight = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= 0 - Tol && x <= 0 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // LambdaGetBoundaryNodesRight

    auto LambdaGetBoundaryNodesCorner = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
                                        if ((y >= 4 - Tol && y <= 4 + Tol) && (x >= -4 - Tol && x <= -4 + Tol) )
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda

    int groupNodesLeft  = myStructure->GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodesUpper = myStructure->GroupCreate(NuTo::eGroupId::Nodes);

    int groupNodesLower = myStructure->GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodesRight = myStructure->GroupCreate(NuTo::eGroupId::Nodes);

    int groupNodesCorner = myStructure->GroupCreate(NuTo::eGroupId::Nodes);

    myStructure->GroupAddNodeFunction(groupNodesLower, LambdaGetBoundaryNodesLower);
    myStructure->GroupAddNodeFunction(groupNodesRight, LambdaGetBoundaryNodesRight);

    myStructure->GroupAddNodeFunction(groupNodesLeft, LambdaGetBoundaryNodesLeft);
    myStructure->GroupAddNodeFunction(groupNodesUpper, LambdaGetBoundaryNodesUpper);

    myStructure->GroupAddNodeFunction(groupNodesCorner, LambdaGetBoundaryNodesCorner);

    std::vector<int> members;

    myStructure->NodeGroupGetMembers(groupNodesLower, members);

    int countDBC = 0;
    Eigen::VectorXd direction(2);
    direction << 0 ,1;
    for (int memberId : members)
    {
        countDBC = myStructure->ConstraintLinearSetDisplacementNode(memberId, direction, 0.0);
    }


    myStructure->NodeGroupGetMembers(groupNodesRight, members);

    direction << 1, 0;
    for (int memberId : members)
    {
        countDBC = myStructure->ConstraintLinearSetDisplacementNode(memberId, direction, 0.0);
    }


    myStructure->SetNumLoadCases(1);

    if(BC == 0)
    {
        std::function<Eigen::Vector2d(Eigen::Vector2d)> stress_left  = exact_plate_hole_left;
        std::function<Eigen::Vector2d(Eigen::Vector2d)> stress_upper = exact_plate_hole_upper;

        myStructure->LoadSurfacePressureFunctionCreate2D(0, groupElementsLeft,  groupNodesLeft, stress_left);
        myStructure->LoadSurfacePressureFunctionCreate2D(0, groupElementsUpper, groupNodesUpper, stress_upper);
    }
    else
    {
        double Stress = -10.;
        myStructure->LoadSurfacePressureCreate2D(0, groupElementsLeft, groupNodesLeft, Stress);
    }

    countDBC++;

//    myStructure->NodeGroupGetMembers(groupNodesCorner, members);
//    std::cout << "Node Members group Corner: \n" << members << std::endl;
//    for(int dof = 0; dof < 1; dof++)
//    {
//        myStructure->ConstraintLinearEquationCreate (countDBC, members(0), NuTo::Node::eDof::DISPLACEMENTS, dof, 1., 0.);
//        myStructure->ConstraintLinearEquationAddTerm(countDBC, members(1), NuTo::Node::eDof::DISPLACEMENTS, dof, -1.);
//        countDBC++;
//    }

    myStructure->CalculateMaximumIndependentSets();
    myStructure->NodeBuildGlobalDofs();

    myStructure->NodeInfo(10);

    return myStructure;
}


void solve(NuTo::Structure *myStructure, double solution, const std::string &resultDir, const std::string &name, bool exc, double tol = 1.e-6)
{
    myStructure->SolveGlobalSystemStaticElastic();

    double nodeDisp = myStructure->NodeGetNodePtr(myStructure->GetNumNodes()-1)->Get(NuTo::Node::eDof::DISPLACEMENTS)[0];

#ifdef ENABLE_VISUALIZE
    int visualizationGroup = myStructure->GroupGetElementsTotal();
    myStructure->AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure->AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRAIN);
    myStructure->AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRESS);

    myStructure->ExportVtkDataFileElements(resultDir+"/Elements" + name + ".vtu", true);
    myStructure->ExportVtkDataFileNodes(resultDir+"/Nodes" + name + ".vtu", true);
#endif

    if(exc)
    {
        if (std::fabs(nodeDisp - solution)/std::fabs(solution) > tol)
        {
            throw NuTo::Exception("[IGA] : displacement is not correct");
        }
    }

}


//! @brief Imports a mesh file and builds the hessian and the internal gradient.
void Neumann(const std::string &resultDir, const std::string &path, const std::string &fileName, int BC)
{
    NuTo::Structure myStructure(2);
    auto groupIndices = myStructure.ImportFromGmsh(path + fileName);

    int interpolationType = groupIndices[0].second;
    myStructure.InterpolationTypeAdd(interpolationType, NuTo::Node::eDof::DISPLACEMENTS,  NuTo::Interpolation::eTypeOrder::LOBATTO3);

    myStructure.SetVerboseLevel(10);
    myStructure.ElementConvertToInterpolationType(groupIndices[0].first);

//    myStructure.InterpolationTypeSetIntegrationType(interpolationType, NuTo::IntegrationType::IntegrationType2D3NGauss3Ip, NuTo::IpData::NOIPDATA);

    myStructure.InterpolationTypeInfo(0);

    myStructure.NodeBuildGlobalDofs();
    int section = myStructure.SectionCreate("PLANE_STRESS");

    double Thickness = 1.;
    myStructure.SectionSetThickness(section, Thickness);

    myStructure.ElementTotalSetSection(section);

    int constitutiveLaw = myStructure.ConstitutiveLawCreate("Linear_Elastic_Engineering_Stress");
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, 1.e5);
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, 0.3);
    myStructure.ElementTotalSetConstitutiveLaw(constitutiveLaw);

    myStructure.CalculateMaximumIndependentSets();

    auto LambdaGetBoundaryNodesLeft = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= -4 - Tol && x <= -4 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda

    auto LambdaGetBoundaryNodesUpper = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
                                        if ((y >= 4 - Tol && y <= 4 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda

    auto LambdaGetBoundaryNodesLowerSymmetry = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
                                        if ((y >= 0 - Tol && y <= 0 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // LambdaGetBoundaryNodesLowerSymmetry

    auto LambdaGetBoundaryNodesRightSymmetry = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= 0 - Tol && x <= 0 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // LambdaGetBoundaryNodesLowerSymmetry



    int groupNodeBCLeft  = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodeBCUpper = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodeBCRightSymmetry = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodeBCLowerSymmetry  = myStructure.GroupCreate(NuTo::eGroupId::Nodes);

    myStructure.GroupAddNodeFunction(groupNodeBCLeft, LambdaGetBoundaryNodesLeft);
    myStructure.GroupAddNodeFunction(groupNodeBCUpper, LambdaGetBoundaryNodesUpper);
    myStructure.GroupAddNodeFunction(groupNodeBCLowerSymmetry, LambdaGetBoundaryNodesLowerSymmetry);
    myStructure.GroupAddNodeFunction(groupNodeBCRightSymmetry, LambdaGetBoundaryNodesRightSymmetry);

    // Dirichlet symmetry //
    std::vector<int> members;

    myStructure.NodeGroupGetMembers(groupNodeBCLowerSymmetry, members);
    for (int memberId : members)
    {
        myStructure.ConstraintLinearSetDisplacementNode(memberId, Eigen::Vector2d::UnitY(), 0.0);
    }

    myStructure.NodeGroupGetMembers(groupNodeBCRightSymmetry, members);
    for (int memberId : members)
    {
        myStructure.ConstraintLinearSetDisplacementNode(memberId, Eigen::Vector2d::UnitX(), 0.0);
    }

    int groupelementBCLeft = myStructure.GroupCreate("ELEMENTS");
    int groupelementBCUpper = myStructure.GroupCreate("ELEMENTS");

    myStructure.GroupAddElementsFromNodes(groupelementBCLeft, groupNodeBCLeft, false);
    myStructure.GroupAddElementsFromNodes(groupelementBCUpper, groupNodeBCUpper, false);

    myStructure.SetNumLoadCases(1);

    if(BC == 0)
    {
        std::function<Eigen::Vector2d(Eigen::Vector2d)> stress_left  = exact_plate_hole_left;
        std::function<Eigen::Vector2d(Eigen::Vector2d)> stress_upper = exact_plate_hole_upper;

        myStructure.LoadSurfacePressureFunctionCreate2D(0, groupelementBCLeft,  groupNodeBCLeft, stress_left);
        myStructure.LoadSurfacePressureFunctionCreate2D(0, groupelementBCUpper, groupNodeBCUpper, stress_upper);
    }
    else
    {
        double Stress = -10.;
        myStructure.LoadSurfacePressureCreate2D(0, groupelementBCLeft, groupNodeBCLeft, Stress);
    }

    myStructure.CalculateMaximumIndependentSets();
    myStructure.NodeBuildGlobalDofs();

    myStructure.SolveGlobalSystemStaticElastic();

#ifdef ENABLE_VISUALIZE
    int visualizationGroup = myStructure.GroupGetElementsTotal();
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRAIN);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRESS);

    myStructure.ExportVtkDataFileElements(resultDir + "/Elements" + fileName + ".vtu", true);
    myStructure.ExportVtkDataFileNodes(resultDir + "/Nodes" + fileName + ".vtu", true);
#endif
}


//! @brief Imports a mesh file and builds the hessian and the internal gradient.
void Dirichlet(const std::string &resultDir, const std::string &path, const std::string &fileName)
{
    NuTo::Structure myStructure(2);
    auto groupIndices = myStructure.ImportFromGmsh(path + fileName);

    int interpolationType = groupIndices[0].second;
    myStructure.InterpolationTypeAdd(interpolationType, NuTo::Node::eDof::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::LOBATTO3);

    myStructure.SetVerboseLevel(10);
    myStructure.ElementConvertToInterpolationType(groupIndices[0].first);

//    myStructure.InterpolationTypeSetIntegrationType(interpolationType, NuTo::IntegrationType::IntegrationType2D3NGauss3Ip, NuTo::IpData::NOIPDATA);

    myStructure.InterpolationTypeInfo(0);

    myStructure.NodeBuildGlobalDofs();
    int section = myStructure.SectionCreate("PLANE_STRESS");
    myStructure.SectionSetThickness(section, 1.);

    myStructure.ElementTotalSetSection(section);

    int constitutiveLaw = myStructure.ConstitutiveLawCreate("Linear_Elastic_Engineering_Stress");
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, 1.e5);
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, 0.3);
    myStructure.ElementTotalSetConstitutiveLaw(constitutiveLaw);

    myStructure.CalculateMaximumIndependentSets();

    auto LambdaGetBoundaryNodesRight = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= 4.0 - Tol && x <= 4.0 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda

    auto LambdaGetBoundaryNodesLeft = [](NuTo::NodeBase* rNodePtr) -> bool
                                {
                                    double Tol = 1.e-6;
                                    if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
                                    {
                                        double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
                                        if ((x >= -4 - Tol && x <= -4 + Tol))
                                        {
                                            return true;
                                        }
                                    }
                                    return false;
                                };  // GetBoundaryNodesLambda



    int groupNodeBCRight = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    int groupNodeBCLeft  = myStructure.GroupCreate(NuTo::eGroupId::Nodes);

    myStructure.GroupAddNodeFunction(groupNodeBCRight, LambdaGetBoundaryNodesRight);
    myStructure.GroupAddNodeFunction(groupNodeBCLeft, LambdaGetBoundaryNodesLeft);

    myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCLeft,  Eigen::Vector2d::UnitX(), -0.01);
    myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCRight, Eigen::Vector2d::UnitX(),  0.01);

    myStructure.ConstraintLinearSetDisplacementNode(0, Eigen::Vector2d::UnitY(), 0.);

    myStructure.CalculateMaximumIndependentSets();
    myStructure.NodeBuildGlobalDofs();

    myStructure.SolveGlobalSystemStaticElastic();

#ifdef ENABLE_VISUALIZE
    int visualizationGroup = myStructure.GroupGetElementsTotal();
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRAIN);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRESS);

    myStructure.ExportVtkDataFileElements(resultDir + "/Elements" + fileName + ".vtu", true);
    myStructure.ExportVtkDataFileNodes(resultDir + "/Nodes" + fileName + ".vtu", true);
#endif
}


int main()
{
    double solution = 0;
    NuTo::Structure* myStructure = NULL;

    std::string resultDir = "./ResultsIGA";

    if (boost::filesystem::exists(resultDir))
    {
        if (boost::filesystem::is_directory(resultDir))
        {
            boost::filesystem::remove_all(resultDir);
        }
    }

    // create result directory
    boost::filesystem::create_directory(resultDir);

    // 2D constant stress
    myStructure = constantStress(solution, 1, resultDir);
    solve(myStructure, solution, resultDir, "Rectangle1", true);

    // plate with hole

//    Eigen::VectorXd vec = exact_plate_hole(Eigen::Vector2d(0.,1.));
//    std::cout << "Vector: " << vec << std::endl;

    myStructure = buildPlateWithHole2DNeumann(resultDir, 5, 0);
    solve(myStructure, solution, resultDir, "Hole5", false);

    std::string path = "./";

    std::string fileName = "PlateWithHole.msh";
    Neumann(resultDir, path, fileName, 0);
//    Dirichlet(resultDir, meshFile2D, fileName);

    return 0;
}
