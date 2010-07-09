#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"

//just for test
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain2D.h"
#include "nuto/mechanics/constitutive/mechanics/NonlocalDamagePlasticity.h"
#include "nuto/math/SparseDirectSolverMUMPS.h"
#include <eigen2/Eigen/Core>

#define MAXNUMNEWTONITERATIONS 30
#define PRINTRESULT false

int main()
{
try
{
	//create structure
	NuTo::Structure myStructure(2);

	int NumElementX(5);
	int NumElementY(1);
	double lX(10);
	double lY(2);

	int NumNodeX(NumElementX+1);
	int NumNodeY(NumElementY+1);

	double deltaX=lX/NumElementX;
	double deltaY=lY/NumElementY;

	//create nodes
    NuTo::FullMatrix<double> Coordinates(2,1);
    myStructure.GroupCreate("LeftSupport","Nodes");
    myStructure.GroupCreate("RightSupport","Nodes");
    double posX;
    double posY=0;
    int leftLowerNode;
	for (int theNodeY=0; theNodeY<NumNodeY;theNodeY++,posY+=deltaY)
	{
		posX=0.;
		for (int theNodeX=0 ; theNodeX<NumNodeX;theNodeX++, posX+=deltaX )
		{
			Coordinates(0,0) = posX;
			Coordinates(1,0) = posY;
			int theNode = myStructure.NodeCreate("displacements",Coordinates);
			if (posX==0)
			{
				if (posY==0)
					leftLowerNode = theNode;
				else
					myStructure.GroupAddNode("LeftSupport",theNode);
			}
			if (posX==lX)
				myStructure.GroupAddNode("RightSupport",theNode);
		}
	}

	//create elements
    NuTo::FullMatrix<int> Incidence(4,1);
	for (int theElementY=0; theElementY<NumElementY;theElementY++)
	{
		for (int theElementX=0; theElementX<NumElementX;theElementX++)
		{
			Incidence(0,0) = theElementX+NumNodeX*theElementY;
			Incidence(1,0) = theElementX+1+NumNodeX*theElementY;
			Incidence(2,0) = theElementX+1+NumNodeX*(theElementY+1);
			Incidence(3,0) = theElementX+NumNodeX*(theElementY+1);
		    myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
		}
	}

	//create constitutive law
	int myMatDamage = myStructure.ConstitutiveLawCreate("NonlocalDamagePlasticity");
	double YoungsModulus(20000);
	myStructure.ConstitutiveLawSetYoungsModulus(myMatDamage,YoungsModulus);
	myStructure.ConstitutiveLawSetPoissonsRatio(myMatDamage,0.2);
	myStructure.ConstitutiveLawSetNonlocalRadius(myMatDamage,deltaX*2);
	double fct(2);
	myStructure.ConstitutiveLawSetTensileStrength(myMatDamage,fct);
	myStructure.ConstitutiveLawSetCompressiveStrength(myMatDamage,fct*10);
	myStructure.ConstitutiveLawSetBiaxialCompressiveStrength(myMatDamage,fct*12.5);
	myStructure.ConstitutiveLawSetFractureEnergy(myMatDamage,0.1);

	//create section
	double thickness(0.5);
	myStructure.SectionCreate("mySection","Plane_Strain");
	myStructure.SectionSetThickness("mySection",thickness);
	myStructure.SectionCreate("mySectionweak","Plane_Strain");
	myStructure.SectionSetThickness("mySectionweak",0.99*thickness);

	//assign constitutive law 
	myStructure.ElementTotalSetSection("mySection");
	int weakElement((NumElementX-1)/2);
	//std::cout << "weak element " << weakElement << std::endl;
	myStructure.ElementSetSection(weakElement,"mySectionweak");
	myStructure.ElementTotalSetConstitutiveLaw(myMatDamage);

	//Build nonlocal elements
	myStructure.BuildNonlocalData(myMatDamage);

	//fix left support
	NuTo::FullMatrix<double> DirectionX(2,1);
	DirectionX.SetValue(0,0,1.0);
	DirectionX.SetValue(1,0,0.0);

	NuTo::FullMatrix<double>DirectionY(2,1);
	DirectionY.SetValue(0,0,0.0);
	DirectionY.SetValue(1,0,1.0);

	myStructure.ConstraintSetDisplacementNodeGroup("LeftSupport",DirectionX, 0);
	myStructure.ConstraintSetDisplacementNode(leftLowerNode,DirectionX, 0);
	myStructure.ConstraintSetDisplacementNode(leftLowerNode,DirectionY, 0);

	// update the RHS of the constrain equation with myStructure.ConstraintSetRHS
	int ConstraintRHS = myStructure.ConstraintSetDisplacementNodeGroup("RightSupport",DirectionX, 0);

    // start analysis
    double maxDisp(200*fct/YoungsModulus*lX);
    double deltaMaxDisp(0.5*fct/YoungsModulus*lX);
    double curDisp(0.80*fct/YoungsModulus*lX);
	char cDummy[100];

	#ifdef ENABLE_VISUALIZE
    myStructure.AddVisualizationComponentDisplacements();
    myStructure.AddVisualizationComponentEngineeringStrain();
    myStructure.AddVisualizationComponentEngineeringStress();
    myStructure.AddVisualizationComponentDamage();
    myStructure.AddVisualizationComponentEngineeringPlasticStrain();
#endif

    NuTo::FullMatrix<double> PlotData(1,6);
    double externalEnergy(0.);
    //increment for different load steps
	myStructure.ElementTotalUpdateTmpStaticData();
    while (curDisp<maxDisp)
    {
    	//update displacement of RHS
    	myStructure.ConstraintSetRHS(ConstraintRHS,curDisp);

    	if (PRINTRESULT)
    	    std::cout<<"curDisp " << curDisp << std::endl;
	    // build global dof numbering
	    myStructure.NodeBuildGlobalDofs();

        double normResidual(1);
        double maxResidual(1);
        int numNewtonIterations(0);
		while(maxResidual>1e-4 && numNewtonIterations<MAXNUMNEWTONITERATIONS)
		{
			numNewtonIterations++;
			// build global stiffness matrix and equivalent load vector which corresponds to prescribed boundary values
			NuTo::SparseMatrixCSRGeneral<double> stiffnessMatrix;
			NuTo::FullMatrix<double> dispForceVector;
			myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrix, dispForceVector);
			NuTo::FullMatrix<double> stiffnessMatrixFull(stiffnessMatrix);
			//std::cout<<"stiffnessMatrixFull" << std::endl;
			//stiffnessMatrixFull.Info();
			//std::cout<<"dispForceVector" << std::endl;
			//dispForceVector.Trans().Info();
			stiffnessMatrix.RemoveZeroEntries(0,1e-14);

			// build global external load vector
			NuTo::FullMatrix<double> extForceVector;
			myStructure.BuildGlobalExternalLoadVector(extForceVector);
			//std::cout<<"extForceVector" << std::endl;
			//extForceVector.Trans().Info();

			// build global internal load vector
			NuTo::FullMatrix<double> intForceVector;
			myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);

			// calculate right hand side
			NuTo::FullMatrix<double> rhsVector = dispForceVector + extForceVector - intForceVector;
			//std::cout<<"rhsVector" << std::endl;
			//rhsVector.Trans().Info();

			// solve
			NuTo::SparseDirectSolverMUMPS mySolver;
			NuTo::FullMatrix<double> deltaDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> oldDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsDependentDOFs;
			stiffnessMatrix.SetOneBasedIndexing();
			mySolver.Solve(stiffnessMatrix, rhsVector, deltaDisplacementsActiveDOFs);
			//std::cout<<"deltaDisplacementsActiveDOFs" << std::endl;
			//deltaDisplacementsActiveDOFs.Trans().Info();
			//double normDeltaDisp = deltaDisplacementsActiveDOFs.Norm();
			//std::cout << "norm DeltaDisp: " << normDeltaDisp << std::endl;

			// write displacements to node
			myStructure.NodeExtractDofValues(oldDisplacementsActiveDOFs, displacementsDependentDOFs);

			//perform a linesearch
			NuTo::FullMatrix<double> residualVector;
			double normRHS = rhsVector.Norm();
			double alpha(1);
			do
			{
				displacementsActiveDOFs = oldDisplacementsActiveDOFs + deltaDisplacementsActiveDOFs*alpha;
				myStructure.NodeMergeActiveDofValues(displacementsActiveDOFs);
				myStructure.ElementTotalUpdateTmpStaticData();
				//std::cout<<"displacementsActiveDOFs" << std::endl;
				//displacementsActiveDOFs.Trans().Info();

				// calculate residual
				myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);
				residualVector = extForceVector - intForceVector;
				normResidual = residualVector.Norm();
				alpha*=0.5;
			}
			while(alpha>1e-3 && normResidual>normRHS);
			maxResidual = residualVector.Max();

			//std::cout << "press enter to next step in Newton iteration, final alpha=" << 2*alpha << std::endl;
			//std::cin.getline(cDummy, 100);
			//std::cout << "Newton iteration, final alpha=" << 2*alpha << ", normResidual" << normResidual<< ", maxResidual" << maxResidual<<std::endl;
		}
		if (numNewtonIterations<MAXNUMNEWTONITERATIONS)
		{

			//NuTo::FullMatrix<double> singleNodeDisp;
			//myStructure.NodeGetDisplacements(NumNodeX-1, singleNodeDisp);
			//std::cout<<"singleNodeDisp" << std::endl;
			//singleNodeDisp.Info();
			myStructure.ElementTotalUpdateStaticData();

			//calculate engineering plastic strain of myelement at all integration points
			NuTo::FullMatrix<double> Stress;
			myStructure.ElementGetEngineeringStress(weakElement, Stress);
	    	if (PRINTRESULT)
	    	{
				std::cout << "stress in weak element " << weakElement << std::endl;
				Stress.Trans().Info();
				std::cout << "number of Newton iterations " << numNewtonIterations << ", delta disp "<< deltaMaxDisp << ", maxResidual " << maxResidual << ", normResidual " << normResidual << std::endl;
	    	}

			//store residual force
			NuTo::FullMatrix<double> SupportingForce;
			myStructure.NodeGroupForce("RightSupport",SupportingForce);
			NuTo::FullMatrix<double> SinglePlotData(1,6);
			SinglePlotData(0,0) = curDisp;
			SinglePlotData(0,1) = SupportingForce(0,0)/(thickness*lY);
			SinglePlotData(0,2) = SupportingForce(0,0);
			SinglePlotData(0,3) = Stress(0,0);
			SinglePlotData(0,4) = Stress(1,0);
			SinglePlotData(0,5) = Stress(2,0);
		    externalEnergy+=deltaMaxDisp*SupportingForce(0,0);

			PlotData.AppendRows(SinglePlotData);

			// visualize results
#ifdef ENABLE_VISUALIZE
			myStructure.ExportVtkDataFile("NonlocalDamagePlasticityBeamView.vtk");
#endif
			//std::cout << "press enter to next load increment" << std::endl;
			//std::cin.getline(cDummy, 100);;
			if (numNewtonIterations<MAXNUMNEWTONITERATIONS/3)
				deltaMaxDisp*=1.5;
	        curDisp+=deltaMaxDisp;
			if (curDisp>maxDisp)
				curDisp=maxDisp;
		}
		else
		{
			deltaMaxDisp*=0.5;
			curDisp-=deltaMaxDisp;

		}
    }
	if (PRINTRESULT)
        std::cout<< "numerical fracture energy "<< externalEnergy/(thickness*lY) << std::endl;
    PlotData.WriteToFile("NonlocalDamagePlasticityBeamLoadDisp.txt"," ","#load displacement curve, disp, stress, force, sxx in center element, syy in center element","  ");
}
catch (NuTo::Exception& e)
{
    std::cout << e.ErrorMessage() << std::endl;
}
return 0;
}