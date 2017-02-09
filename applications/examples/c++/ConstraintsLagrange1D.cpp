#include "math/FullMatrix.h"
#include "math/FullVector.h"
#include "mechanics/structures/unstructured/Structure.h"
#include "mechanics/MechanicsException.h"

#include "math/SparseDirectSolverMUMPS.h"
#include "math/SparseMatrixCSRVector2General.h"

#define MAXNUMNEWTONITERATIONS 20
//#define MAXNORMRHS 100
#define PRINTRESULT false
#define MIN_DELTA_STRAIN_FACTOR 1e-7

int main()
{
try
{
    //create structure
    NuTo::Structure myStructure(1);

    double lx(1.);

    //2 nodes 1 element grid
    //create nodes
    NuTo::FullVector<double,Eigen::Dynamic> Coordinates(1);
    Coordinates(0,0) = 0.0;
    int node1 = 0;
    myStructure.NodeCreate(node1, Coordinates);

    Coordinates(0,0) = lx;
    int node2 = 1;
    myStructure.NodeCreate(node2, Coordinates);

    int interpolationType = myStructure.InterpolationTypeCreate("TRUSS1D");
    myStructure.InterpolationTypeAdd(interpolationType, NuTo::Node::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
    myStructure.InterpolationTypeAdd(interpolationType, NuTo::Node::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);

    NuTo::FullVector<int,Eigen::Dynamic> Incidence(2);
    Incidence(0,0) = node1;
    Incidence(1,0) = node2;
    myStructure.ElementCreate(interpolationType,Incidence);

    myStructure.ElementTotalConvertToInterpolationType(1e-6,3);

    //create constitutive law
    int myMatLin = myStructure.ConstitutiveLawCreate("LinearElasticEngineeringStress");
    myStructure.ConstitutiveLawSetParameterDouble(myMatLin,NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS,1);
    myStructure.ConstitutiveLawSetParameterDouble(myMatLin,NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO,0);

    //create section
    int mySection = myStructure.SectionCreate("TRUSS");
    double area(1.);
    myStructure.SectionSetArea(mySection,area);

    myStructure.ElementTotalSetSection(mySection);
    myStructure.ElementTotalSetConstitutiveLaw(myMatLin);

	//Create groups to apply the boundary conditions
	//left boundary
	int GrpNodesLeftBoundary = myStructure.GroupCreate("Nodes");
	int direction = 0; //either 0,1,2
	double min(0.);
	double max(0.);
	myStructure.GroupAddNodeCoordinateRange(GrpNodesLeftBoundary,direction,min,max);

	int GrpNodesRightBoundary = myStructure.GroupCreate("Nodes");
    direction = 0; //either 0,1,2
    min = lx;
    max = lx;
    myStructure.GroupAddNodeCoordinateRange(GrpNodesRightBoundary,direction,min,max);

    //fix bottom left corner node
    NuTo::FullVector<double,Eigen::Dynamic> DirectionX(1);
	DirectionX.SetValue(0,0,1.0);

    int constraintLHS = myStructure.ConstraintLagrangeSetDisplacementNodeGroup(GrpNodesLeftBoundary,DirectionX, std::string("EQUAL"),0.0);
    myStructure.ConstraintLagrangeSetPenaltyStiffness(constraintLHS,1.);
    int constraintRHS = myStructure.ConstraintLagrangeSetDisplacementNodeGroup(GrpNodesRightBoundary,DirectionX, std::string("SMALLER"),0.5);
    myStructure.ConstraintLagrangeSetPenaltyStiffness(constraintRHS,1.);

#ifdef ENABLE_VISUALIZE
        int visualizationGroup = myStructure.GroupCreate(NuTo::Groups::eGroupId::Elements);
        myStructure.GroupAddElementsTotal(visualizationGroup);

        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::DISPLACEMENTS);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::ENGINEERING_STRAIN);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::ENGINEERING_STRESS);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::SECTION);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::CONSTITUTIVE);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::DAMAGE);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::ENGINEERING_PLASTIC_STRAIN);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::VisualizeBase::PRINCIPAL_ENGINEERING_STRESS);

        myStructure.ElementTotalUpdateTmpStaticData();
        myStructure.ExportVtkDataFileElements("ConstraintsLagrange1D.vtk");
#endif

    // init some result data
    NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> PlotData(1,7);

    // start analysis
    double maxDisp(1);
    double deltaDispFactor(0.2);
    double maxDeltaDispFactor(0.2);
    double curDispFactor(0.2);

	//update conre mat
	myStructure.NodeBuildGlobalDofs();

	//update tmpstatic data with zero displacements
	myStructure.ElementTotalUpdateTmpStaticData();
    
    //calculate maximum independent sets
	myStructure.CalculateMaximumIndependentSets();

	//init some auxiliary variables
	NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrixCSRVector2;
	NuTo::FullVector<double,Eigen::Dynamic> dispForceVector;
	NuTo::FullVector<double,Eigen::Dynamic> intForceVector;
	NuTo::FullVector<double,Eigen::Dynamic> extForceVector;
	NuTo::FullVector<double,Eigen::Dynamic> rhsVector;

	//allocate solver
	NuTo::SparseDirectSolverMUMPS mySolver;
#ifdef SHOW_TIME
	if (PRINTRESULT)
	    mySolver.SetShowTime(true);
#endif //SHOW_TIME
    //calculate stiffness
	myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);

    //apply initial displacement
	double curDisp(maxDisp*curDispFactor);
    myStructure.ConstraintSetRHS(constraintLHS,curDisp);

    //update conre mat
    myStructure.NodeBuildGlobalDofs();

    //update displacements of all nodes according to the new conre mat
    {
    	NuTo::FullVector<double,Eigen::Dynamic> displacementsActiveDOFsCheck;
    	NuTo::FullVector<double,Eigen::Dynamic> displacementsDependentDOFsCheck;
        myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
        myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
        myStructure.ElementTotalUpdateTmpStaticData();
    }

	//build global external load vector and RHS vector
	myStructure.BuildGlobalExternalLoadVector(1,extForceVector);
	//extForceVector.Info(10,13);
    myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);
    //intForceVector.Info(10,13);
	rhsVector = extForceVector + dispForceVector - intForceVector;

	//calculate absolute tolerance for matrix entries to be not considered as zero
	double maxValue, minValue, ToleranceZeroStiffness;
	stiffnessMatrixCSRVector2.Max(maxValue);
	stiffnessMatrixCSRVector2.Min(minValue);
	//std::cout << "min and max " << minValue << " , " << maxValue << std::endl;

	ToleranceZeroStiffness = (1e-14) * (fabs(maxValue)>fabs(minValue) ?  fabs(maxValue) : fabs(minValue));
	myStructure.SetToleranceStiffnessEntries(ToleranceZeroStiffness);
	int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
	int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
	if (PRINTRESULT)
	    std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;


	//repeat until max displacement is reached
	bool convergenceStatusLoadSteps(false);
    int loadstep(1);
    NuTo::FullVector<double,Eigen::Dynamic> displacementsActiveDOFsLastConverged,displacementsDependentDOFsLastConverged;
	while (!convergenceStatusLoadSteps)
    {

        double normResidual(1);
        double maxResidual(1);
        int numNewtonIterations(0);
		double normRHS(1.);
		double alpha(1.);
		int convergenceStatus(0);
		//0 - not converged, continue Newton iteration
		//1 - converged
		//2 - stop iteration, decrease load step
		while(convergenceStatus==0)
		{
			numNewtonIterations++;

			if (numNewtonIterations>MAXNUMNEWTONITERATIONS && alpha<0.5)
			{
				if (PRINTRESULT)
		    	{
				    std::cout << "numNewtonIterations (" << numNewtonIterations << ") > MAXNUMNEWTONITERATIONS (" << MAXNUMNEWTONITERATIONS << ")" << std::endl;
		    	}
				convergenceStatus = 2; //decrease load step
				break;
			}

			normRHS = rhsVector.Norm();

			// solve
			NuTo::FullVector<double,Eigen::Dynamic> deltaDisplacementsActiveDOFs;
			NuTo::FullVector<double,Eigen::Dynamic> oldDisplacementsActiveDOFs;
			NuTo::FullVector<double,Eigen::Dynamic> displacementsActiveDOFs;
			NuTo::FullVector<double,Eigen::Dynamic> displacementsDependentDOFs;
			if (PRINTRESULT)
			{
                NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> stiffnessMatrixCSRVector2Full(stiffnessMatrixCSRVector2);
                stiffnessMatrixCSRVector2Full.Info(20,10);
			}

			NuTo::SparseMatrixCSRGeneral<double> stiffnessMatrixCSR(stiffnessMatrixCSRVector2);
			stiffnessMatrixCSR.SetOneBasedIndexing();
			mySolver.Solve(stiffnessMatrixCSR, rhsVector, deltaDisplacementsActiveDOFs);
			//deltaDisplacementsActiveDOFs.Trans().Info();

			// write displacements to node
			myStructure.NodeExtractDofValues(oldDisplacementsActiveDOFs, displacementsDependentDOFs);

			//perform a linesearch
			alpha = 1.;
			do
			{
				//add new displacement state
				displacementsActiveDOFs = oldDisplacementsActiveDOFs + deltaDisplacementsActiveDOFs*alpha;
				myStructure.NodeMergeActiveDofValues(displacementsActiveDOFs);
				myStructure.ElementTotalUpdateTmpStaticData();

				// calculate residual
				myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);
				rhsVector = extForceVector - intForceVector;
				normResidual = rhsVector.Norm();
				if (PRINTRESULT)
				    std::cout << "alpha " << alpha << ", normResidual " << normResidual << ", normResidualInit "<< normRHS << ", normRHS*(1-0.5*alpha) " << normRHS*(1-0.5*alpha) << std::endl;
				alpha*=0.5;
			}
			while(alpha>1e-3 && normResidual>normRHS*(1-0.5*alpha) && normResidual>1e-5);
			if (normResidual>normRHS*(1-0.5*alpha) && normResidual>1e-5)
			{
			    convergenceStatus=2;
			    break;
			}

		    maxResidual = rhsVector.Abs().Max();

			if (PRINTRESULT)
			{
                std::cout << std::endl << "Newton iteration " << numNewtonIterations << ", final alpha " << 2*alpha << ", normResidual " << normResidual<< ", maxResidual " << maxResidual<<std::endl;
                char cDummy[100]="";
                std::cin.getline(cDummy, 100);
			}

			//check convergence
			if (normResidual<1e-5 || maxResidual<1e-5)
			{
				if (PRINTRESULT)
		    	{
					std::cout << "Convergence after " << numNewtonIterations << " Newton iterations, curdispFactor " << curDispFactor << ", deltaDispFactor "<< deltaDispFactor << std::endl<< std::endl;
		    	}
				convergenceStatus=1;
				break;
			}

			//convergence status == 0 (continue Newton iteration)
			//build new stiffness matrix
			myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
			int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
			if (PRINTRESULT)
			{
                int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
                std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;
			}
/*		    //check stiffness
		    {
		        NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrixCSRVector2_1, stiffnessMatrixCSRVector2_2;
		        NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> dispForceVector_1, intForceVector_1, intForceVector_2;
		        myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2_1, dispForceVector_1);
		        NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> stiffnessMatrixFull(stiffnessMatrixCSRVector2);
		        NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> stiffnessMatrixCDFull(stiffnessMatrixCSRVector2);
		        stiffnessMatrixFull.Info(10,13);
		        myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector_1);
		        NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> displacementsActiveDOFsCheck;
		        NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> displacementsDependentDOFsCheck;
		        myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
		        double delta=1e-5;
		        for (int theDOF=0; theDOF<stiffnessMatrixFull.GetNumRows(); theDOF++)
		        {
		            displacementsActiveDOFsCheck(theDOF,0)=displacementsActiveDOFsCheck(theDOF,0)+delta;
		            myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
		            myStructure.ElementTotalUpdateTmpStaticData();
		            myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector_2);
		            stiffnessMatrixCDFull.SetColumn(theDOF,(intForceVector_2-intForceVector_1)*(1/delta));
		            displacementsActiveDOFsCheck(theDOF,0)=displacementsActiveDOFsCheck(theDOF,0)-delta;
		        }
		        stiffnessMatrixCDFull.Info(10,13);
		        myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
		        myStructure.ElementTotalUpdateTmpStaticData();
		    }
*/
		}

		if (deltaDispFactor<1e-7)
		    throw NuTo::MechanicsException("Example ConcurrentMultiscale : No convergence, delta strain factor < 1e-7");

		if (convergenceStatus==1)
		{
			myStructure.ElementTotalUpdateStaticData();

            // visualize results
#ifdef ENABLE_VISUALIZE
            std::stringstream ss;
            ss << "ConstraintsLagrange1D" << loadstep << ".vtk";
            myStructure.ExportVtkDataFileElements(ss.str());
#endif
 			//store result/plot data
            NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> SinglePlotData(1,7);

            //displacements
            NuTo::FullVector<double,Eigen::Dynamic> dispNode;
            myStructure.NodeGetDisplacements(node1,dispNode);
            SinglePlotData(0,0) = dispNode(0,0);
            myStructure.NodeGetDisplacements(node2,dispNode);
            SinglePlotData(0,1) = dispNode(0,0);

            //boundary force
            NuTo::FullVector<double,Eigen::Dynamic> SupportingForce;
            myStructure.NodeGroupInternalForce(GrpNodesLeftBoundary,SupportingForce);
            SinglePlotData(0,2) = SupportingForce(0,0);
            myStructure.NodeGroupInternalForce(GrpNodesRightBoundary,SupportingForce);
            SinglePlotData(0,3) = SupportingForce(0,0);

            //lagrange multiplier
            NuTo::FullVector<double,Eigen::Dynamic> lagrangeMultiplier;
            myStructure.ConstraintLagrangeGetMultiplier(constraintLHS,lagrangeMultiplier);
            SinglePlotData(0,4) = lagrangeMultiplier(0,0);
            myStructure.ConstraintLagrangeGetMultiplier(constraintRHS,lagrangeMultiplier);
            SinglePlotData(0,5) = lagrangeMultiplier(0,0);

            //number of Newton iterations
            SinglePlotData(0,6) = numNewtonIterations;

			PlotData.AppendRows(SinglePlotData);
		    PlotData.WriteToFile("ConstraintsLagrange1DLoadDisp.txt"," ","#disp left and right, boundary force left and right, lagrange multiplier left and right","  ");
            if (PRINTRESULT)
            {
                std::cout << "disp left and right, boundary force left and right, lagrange multiplier left and right" << std::endl;
                SinglePlotData.Trans().Info();
            }

		    myStructure.NodeExtractDofValues(displacementsActiveDOFsLastConverged,displacementsDependentDOFsLastConverged);
            if (curDispFactor==1)
                convergenceStatusLoadSteps=true;
            else
            {
                //eventually increase load step
                if (numNewtonIterations<MAXNUMNEWTONITERATIONS/3)
                {
                    deltaDispFactor*=1.5;
                    if (deltaDispFactor>maxDeltaDispFactor)
                        deltaDispFactor = maxDeltaDispFactor;
                }

                //increase displacement
                curDispFactor+=deltaDispFactor;
                if (curDispFactor>1)
                {
                    deltaDispFactor -= curDispFactor -1.;
                    curDispFactor=1;
                }

                curDisp = maxDisp*curDispFactor;

                //old stiffness matrix is used in first step of next load increment in order to prevent spurious problems at the boundary
                //std::cout << "press enter to next load increment, delta disp factor " << deltaDispFactor << " max delta disp factor " <<  maxDeltaDispFactor << std::endl << std::endl;
                //char cDummy[100]="";
                //std::cin.getline(cDummy, 100);
            }
            loadstep++;
        }
		else
		{
            assert(convergenceStatus==2);
			//calculate stiffness of previous loadstep (used as initial stiffness in the next load step)
			//this is done within the loop in order to ensure, that for the first step the stiffness matrix of the previous step is used
			//otherwise, the additional boundary displacements will result in an artificial localization in elements at the boundary
			curDispFactor-=deltaDispFactor;
            curDisp = maxDisp*curDispFactor;

            myStructure.ConstraintSetRHS(constraintLHS,curDisp);

			// build global dof numbering
			myStructure.NodeBuildGlobalDofs();

			//set previous converged displacements
			myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsLastConverged);
			myStructure.ElementTotalUpdateTmpStaticData();

			//decrease load step
			deltaDispFactor*=0.5;
			curDispFactor+=deltaDispFactor;
			curDisp = maxDisp*curDispFactor;

			//check for minimum delta (this mostly indicates an error in the software
			if (deltaDispFactor<MIN_DELTA_STRAIN_FACTOR)
			{
			    throw NuTo::MechanicsException("Example ConstraintsLagrange1D : No convergence, delta strain factor < 1e-7");
			}

			std::cout << "press enter to reduce load increment" << std::endl;
			char cDummy[100]="";
			std::cin.getline(cDummy, 100);;
		}

		if (!convergenceStatusLoadSteps)
		{
            //update new displacement of RHS
		    myStructure.ConstraintSetRHS(constraintLHS,curDisp);

            // build global dof numbering and conre mat
            myStructure.NodeBuildGlobalDofs();

            //update stiffness in order to calculate new dispForceVector
            myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
            int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
            int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
            if (PRINTRESULT)
                std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;

            //update displacements of all nodes according to the new conre mat
            NuTo::FullVector<double,Eigen::Dynamic> displacementsActiveDOFsCheck;
            NuTo::FullVector<double,Eigen::Dynamic> displacementsDependentDOFsCheck;
            myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
            myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
            myStructure.ElementTotalUpdateTmpStaticData();

            // calculate initial residual for next load step
            myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);

            //update rhs vector for next Newton iteration
            rhsVector = dispForceVector + extForceVector - intForceVector;

		}
    }
	NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> PlotDataRef(6,7);
	PlotDataRef(0,0) = 0.0;
    PlotDataRef(1,0) = 0.2;
    PlotDataRef(2,0) = 0.4;
    PlotDataRef(3,0) = 0.6;
    PlotDataRef(4,0) = 0.8;
    PlotDataRef(5,0) = 1.0;

    PlotDataRef(0,0) = 0.0;
    PlotDataRef(1,1) = 0.2;
    PlotDataRef(2,1) = 0.4;
    PlotDataRef(3,1) = 0.5;
    PlotDataRef(4,1) = 0.5;
    PlotDataRef(5,1) = 0.5;

    PlotDataRef(0,2) = 0.0;
    PlotDataRef(1,2) = 0.0;
    PlotDataRef(2,2) = 0.0;
    PlotDataRef(3,2) = 0.1;
    PlotDataRef(4,2) = 0.3;
    PlotDataRef(5,2) = 0.5;

    PlotDataRef(0,3) = 0.0;
    PlotDataRef(1,3) = 0.0;
    PlotDataRef(2,3) = 0.0;
    PlotDataRef(3,3) = -0.1;
    PlotDataRef(4,3) = -0.3;
    PlotDataRef(5,3) = -0.5;

    PlotDataRef(0,4) = 0.0;
    PlotDataRef(1,4) = 0.0;
    PlotDataRef(2,4) = 0.0;
    PlotDataRef(3,4) = -0.1;
    PlotDataRef(4,4) = -0.3;
    PlotDataRef(5,4) = -0.5;

    PlotDataRef(0,5) = 0.0;
    PlotDataRef(1,5) = 0.0;
    PlotDataRef(2,5) = 0.0;
    PlotDataRef(3,5) = 0.1;
    PlotDataRef(4,5) = 0.3;
    PlotDataRef(5,5) = 0.5;

    PlotDataRef(0,6) = 0;
    PlotDataRef(1,6) = 1;
    PlotDataRef(2,6) = 1;
    PlotDataRef(3,6) = 2;
    PlotDataRef(4,6) = 1;
    PlotDataRef(5,6) = 1;

    if ((PlotDataRef-PlotData).cwiseAbs().maxCoeff()>1e-4)
    {
        std::cout<< "final results stored in load disp file as well" << std::endl;
        PlotData.Info();
        std::cout<< "reference results" << std::endl;
        PlotDataRef.Info();
        std::cout << "[ConstraintLagrange1D] result is not correct." << std::endl;
    }
    else
        std::cout<< "[ConstraintLagrange1D] nice, result is correct" << std::endl;
}
catch (NuTo::Exception& e)
{
    std::cout << e.ErrorMessage() << std::endl;
}
return 0;
}