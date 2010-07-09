import nuto
import sys
import os

#call of the test file, e.g.
#/usr/local/bin/python ~/develop/nuto/test/mechanics/Plane2D3N.py Linux x86_64 ~/develop/nuto/test/mechanics

#if set to true, the result will be generated (for later use in the test routine)
#otherwise, the current result will be compared to the stored result
#createResult = True
createResult = False

#show the results on the screen
#printResult = True
printResult = False

#system name and processor
system = sys.argv[1]+sys.argv[2]

#path in the original source directory and current filename at the end
pathToResultFiles = os.path.join(sys.argv[3],"results",system,os.path.basename(sys.argv[0]))

#remove the extension
fileExt = os.path.splitext(sys.argv[0])[1]
pathToResultFiles = pathToResultFiles.replace(fileExt,'')

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#% no error in file, modified, if error is detected              %
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
error = False

#create structure
myStructure = nuto.Structure(2)

#create nodes
myNode1 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(2,1,(-1,-1)))
myNode4 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(2,1,(+2,-1)))
myNode3 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(2,1,(+2,+1)))
myNode2 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(2,1,(-1,+1)))

#create element
myStructure.GroupCreate("ElementGroup","Elements")
myElement1 = myStructure.ElementCreate("PLANE2D3N",nuto.IntFullMatrix(3,1,(myNode1,myNode2,myNode3)))
myStructure.GroupAddElement("ElementGroup",myElement1)
myElement2 = myStructure.ElementCreate("PLANE2D3N",nuto.IntFullMatrix(3,1,(myNode1,myNode3,myNode4)))
myStructure.GroupAddElement("ElementGroup",myElement2)

#create constitutive law
myMatLin = myStructure.ConstitutiveLawCreate("LinearElastic")
myStructure.ConstitutiveLawSetYoungsModulus(myMatLin,10)
myStructure.ConstitutiveLawSetPoissonsRatio(myMatLin,0.25)

#create section
myStructure.SectionCreate("mySection","Plane_Strain")
myStructure.SectionSetThickness("mySection",1)

#assign constitutive law 
#myStructure.ElementSetIntegrationType(myElement1,"3D8NGauss1Ip")
myStructure.ElementGroupSetConstitutiveLaw("ElementGroup",myMatLin)
myStructure.ElementGroupSetSection("ElementGroup","mySection")


#set displacements of right node
myStructure.NodeSetDisplacements(myNode2,nuto.DoubleFullMatrix(2,1,(0.1,0.2)))
myStructure.NodeSetDisplacements(myNode3,nuto.DoubleFullMatrix(2,1,(0.1,0.2)))

#calculate element stiffness matrix
Ke = nuto.DoubleFullMatrix(0,0)
rowIndex = nuto.IntFullMatrix(0,0)
colIndex = nuto.IntFullMatrix(0,0)
myStructure.ElementStiffness(myElement1,Ke,rowIndex,colIndex)
if (printResult):
    print "Ke"
    Ke.Info()

#correct stiffness matrix
if createResult:
   print pathToResultFiles+"Stiffness.txt"
   Ke.WriteToFile(pathToResultFiles+"Stiffness.txt"," ","#Correct result","  ")
else:
   KeCorrect = nuto.DoubleFullMatrix(24,24)
   KeCorrect.ReadFromFile(pathToResultFiles+"Stiffness.txt",1," ")
   if (printResult):
       print "KeCorrect"
       KeCorrect.Info()
   if ((Ke-KeCorrect).Abs().Max()[0]>1e-8):
       print '[' + system,sys.argv[0] + '] : stiffness is not correct.'
       error = True;

#calculate internal force vector
Fi = nuto.DoubleFullMatrix(0,0)
rowIndex = nuto.IntFullMatrix(0,0)
myStructure.ElementGradientInternalPotential(myElement1,Fi,rowIndex)
if (printResult):
    print "Internal Force"
    Fi.Info()

#correct resforce vector
if createResult:
    print pathToResultFiles+"Internalforce.txt"
    Fi.WriteToFile(pathToResultFiles+"Internalforce.txt"," ","#Correct result","  ")
else:
    FiCorrect = nuto.DoubleFullMatrix(24,1)
    FiCorrect.ReadFromFile(pathToResultFiles+"Internalforce.txt",1," ")
    if (printResult):
        print "FiCorrect"
        FiCorrect.Info()
    if ((Fi-FiCorrect).Abs().Max()[0]>1e-8):
        print '[' + system,sys.argv[0] + '] : internal force is not correct.'
        error = True;

#calculate engineering strain of myelement1 at all integration points
#the size the matrix is not important and reallocated within the procedure
EngineeringStrain = nuto.DoubleFullMatrix(6,1)
myStructure.ElementGetEngineeringStrain(myElement1, EngineeringStrain)

#correct strain
EngineeringStrainCorrect = nuto.DoubleFullMatrix(6,3,(
0.0,0.1,0,0.05,0,0,
0.0,0.1,0,0.05,0,0,
0.0,0.1,0,0.05,0,0
))

if (printResult):
    print "EngineeringStrainCorrect"
    EngineeringStrainCorrect.Info()
    print "EngineeringStrain"
    EngineeringStrain.Info()

if ((EngineeringStrain-EngineeringStrainCorrect).Abs().Max()[0]>1e-8):
        print '[' + system,sys.argv[0] + '] : strain is not correct.'
        error = True;

#calculate engineering strain of myelement1 at all integration points
EngineeringStress = nuto.DoubleFullMatrix(6,3)
myStructure.ElementGetEngineeringStress(myElement1, EngineeringStress)
#correct stress
EngineeringStressCorrect = nuto.DoubleFullMatrix(6,3,(
0.4,1.2,0.4,0.2,0,0,
0.4,1.2,0.4,0.2,0,0,
0.4,1.2,0.4,0.2,0,0
))

if (printResult):
    print "EngineeringStressCorrect"
    EngineeringStressCorrect.Info()
    print "EngineeringStress"
    EngineeringStress.Info()

if ((EngineeringStress-EngineeringStressCorrect).Abs().Max()[0]>1e-8):
        print '[' + system,sys.argv[0] + '] : stress is not correct.'
        error = True;

# visualize results
myStructure.AddVisualizationComponentDisplacements()
myStructure.AddVisualizationComponentEngineeringStrain()
myStructure.AddVisualizationComponentEngineeringStress()
myStructure.ExportVtkDataFile("Plane2D3N.vtk")

if (error):
    sys.exit(-1)
else:
    sys.exit(0)