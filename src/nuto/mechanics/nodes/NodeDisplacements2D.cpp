#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION
#include "nuto/mechanics/nodes/NodeDisplacements2D.h"

//! @brief constructor
NuTo::NodeDisplacements2D::NodeDisplacements2D() : NodeBase ()
{
    mDisplacements[0]=0;
    mDisplacements[1]=0;
}

//! @brief constructor
NuTo::NodeDisplacements2D::NodeDisplacements2D (const double rDisplacements[2])  : NodeBase ()
{
    mDisplacements[0]= rDisplacements[0];
    mDisplacements[1]= rDisplacements[1];
}

#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template<class Archive>
void NuTo::NodeDisplacements2D::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NodeBase)
       & BOOST_SERIALIZATION_NVP(mDisplacements);
}
#endif // ENABLE_SERIALIZATION

//! @brief returns the number of displacements of the node
//! @return number of displacements
int NuTo::NodeDisplacements2D::GetNumDisplacements()const
{
    return 2;
}

//! @brief gives the global DOF of a displacement component
//! @param rComponent component
//! @return global DOF
int NuTo::NodeDisplacements2D::GetDofDisplacement(int rComponent)const
{
	if (rComponent<0 || rComponent>1)
		throw MechanicsException("[NuTo::NodeDisplacements2D::GetDofDisplacement] Node has only two displacement components.");
    return mDOF[rComponent];
}

//! @brief set the displacements
//! @param rDisplacements  given displacements
void NuTo::NodeDisplacements2D::SetDisplacements2D(const double rDisplacements[2])
{
    mDisplacements[0] = rDisplacements[0];
    mDisplacements[1] = rDisplacements[1];
}

//! @brief writes the displacements of a node to the prescribed pointer
//! @param rDisplacements displacements
void NuTo::NodeDisplacements2D::GetDisplacements2D(double rDisplacements[2])const
{
    rDisplacements[0] = mDisplacements[0];
    rDisplacements[1] = mDisplacements[1];
}

//! @brief sets the global dofs
//! @param rDOF current maximum DOF, this variable is increased within the routine
void NuTo::NodeDisplacements2D::SetGlobalDofs(int& rDOF)
{
    mDOF[0]=rDOF++;
    mDOF[1]=rDOF++;
}

//! @brief write dof values to the node (based on global dof number)
//! @param rActiveDofValues ... active dof values
//! @param rDependentDofValues ... dependent dof values
void NuTo::NodeDisplacements2D::SetGlobalDofValues(const FullMatrix<double>& rActiveDofValues, const FullMatrix<double>& rDependentDofValues)
{
    assert(rActiveDofValues.GetNumColumns() == 1);
    assert(rDependentDofValues.GetNumColumns() == 1);
    for (int count=0; count<2; count++)
    {
        int dof = this->mDOF[count];
        double value;
        if (dof >= rActiveDofValues.GetNumRows())
        {
            dof -= rActiveDofValues.GetNumRows();
            assert(dof < rDependentDofValues.GetNumRows());
            value = rDependentDofValues(dof,0);
        }
        else
        {
            value = rActiveDofValues(dof,0);
        }
        this->mDisplacements[count] = value;
    }
}

//! @brief extract dof values from the node (based on global dof number)
//! @param rActiveDofValues ... active dof values
//! @param rDependentDofValues ... dependent dof values
void NuTo::NodeDisplacements2D::GetGlobalDofValues(FullMatrix<double>& rActiveDofValues, FullMatrix<double>& rDependentDofValues) const
{
    assert(rActiveDofValues.GetNumColumns() == 1);
    assert(rDependentDofValues.GetNumColumns() == 1);
    for (int count=0; count<2; count++)
    {
        int dof = this->mDOF[count];
        double value = this->mDisplacements[count];
        if (dof >= rActiveDofValues.GetNumRows())
        {
            dof -= rActiveDofValues.GetNumRows();
            assert(dof < rDependentDofValues.GetNumRows());
            rDependentDofValues(dof,0) = value;
        }
        else
        {
            rActiveDofValues(dof,0) = value;
        }
    }
}

//! @brief renumber the global dofs according to predefined ordering
//! @param rMappingInitialToNewOrdering ... mapping from initial ordering to the new ordering
void NuTo::NodeDisplacements2D::RenumberGlobalDofs(std::vector<int>& rMappingInitialToNewOrdering)
{
    mDOF[0]=rMappingInitialToNewOrdering[mDOF[0]];
    mDOF[1]=rMappingInitialToNewOrdering[mDOF[1]];
}

//! @brief returns the type of the node
//! @return type
std::string NuTo::NodeDisplacements2D::GetNodeTypeStr()const
{
	return std::string("NodeDisplacements2D");
}