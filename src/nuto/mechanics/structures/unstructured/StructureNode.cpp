// $Id$

#include <boost/tokenizer.hpp>
#include <sstream>

#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/nodes/NodeDof.h"
#include "nuto/mechanics/nodes/NodeCoordinates.h"
#include "nuto/mechanics/nodes/NodeCoordinatesDof.h"

//! @brief returns the number of nodes
//! @return number of nodes
int NuTo::Structure::GetNumNodes() const
{
    return mNodeMap.size();
}

//! @brief a reference to a node
//! @param identifier
//! @return reference to a node
NuTo::NodeBase* NuTo::Structure::NodeGetNodePtr(int rIdent)
{
    boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.find(rIdent);
    if (it!=mNodeMap.end())
        return it->second;
    else
    {
    	std::stringstream out;
    	out << rIdent;
    	throw MechanicsException("[NuTo::Structure::NodeGetNodePtr] Node with id " + out.str() +" does not exist.");
    }
}

//! @brief a reference to a node
//! @param identifier
//! @return reference to a node
const NuTo::NodeBase* NuTo::Structure::NodeGetNodePtr(int rIdent)const
{
    boost::ptr_map<int,NodeBase>::const_iterator it = mNodeMap.find(rIdent);
    if (it!=mNodeMap.end())
        return it->second;
    else
    {
    	std::stringstream out;
    	out << rIdent;
    	throw MechanicsException("[NuTo::Structure::NodeGetNodePtr] Node with id " + out.str() +" does not exist.");
    }
}

//! @brief gives the identifier of a node
//! @param reference to a node
//! @return identifier
int NuTo::Structure::NodeGetId(const NodeBase* rNode)const
{
    for (boost::ptr_map<int,NodeBase>::const_iterator
            it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
    {
        if (it->second==rNode)
            return it->first;
    }
    throw MechanicsException("[NuTo::Structure::GetNodeId] Node does not exist.");
}

//! @brief ... store all element ids connected to this node in a vector
//! @param rNode (Input) 			... node id
//! @param rElementNumbers (Output) ... vector of element ids
void NuTo::Structure::NodeGetElements(const int rNodeId, NuTo::FullVector<int,Eigen::Dynamic>& rElementNumbers)
{
	const NuTo::NodeBase* nodePtr(NuTo::Structure::NodeGetNodePtr(rNodeId));
	std::vector<NuTo::ElementBase*> elementPtrs;
	this->NodeGetElements(nodePtr,elementPtrs);
	rElementNumbers.Resize(elementPtrs.size());
	size_t i=0;
	BOOST_FOREACH(NuTo::ElementBase* thisElPtr,elementPtrs)
		rElementNumbers(++i,1)=thisElPtr->ElementGetId();
}

//! @brief ... store all elements connected to this node in a vector
//! @param rNode (Input) 		... node pointer
//! @param rElements (Output) 	... vector of element pointers
void NuTo::Structure::NodeGetElements(const NuTo::NodeBase* rNodePtr, std::vector<NuTo::ElementBase*>& rElements)
{
	rElements.resize(0);
	boost::ptr_map<int,NuTo::ElementBase>::iterator ElementIter = this->mElementMap.begin();
	while (ElementIter != this->mElementMap.end())
	{
		for(size_t thisNode=ElementIter->second->GetNumNodes();thisNode--;){
			NuTo::NodeBase* thisNodePtr=ElementIter->second->GetNode(thisNode);
			if(thisNodePtr==rNodePtr)
			{
				rElements.push_back(ElementIter->second);
				break;
			}
		}
		ElementIter++;
	}
}


//! @brief info about the elements in the Structure
void NuTo::Structure::NodeInfo(int rVerboseLevel)const
{
    mLogger <<"number of nodes   : " << mNodeMap.size() <<"\n";
    if (rVerboseLevel>3)
    {
    	mLogger << "\t\tnodes :" <<"\n";
    	for (boost::ptr_map<int,NodeBase>::const_iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
    	{
    		mLogger << "\t\t" << it->first;
			if (rVerboseLevel>4)
			{
				mLogger << "\t c:";
				for(unsigned short iDof=0; iDof<it->second->GetNumCoordinates(); ++iDof)
					mLogger << "\t" << it->second->GetCoordinate(iDof);
                if (it->second->GetNumDisplacements()>0)
                {
                	mLogger << "\t d:";
                    for(unsigned short iDof=0; iDof<it->second->GetNumDisplacements(); ++iDof)
                    {
                    	mLogger << "\t" << it->second->GetDisplacement(iDof) ;
                       	mLogger << "("<< it->second->GetDofDisplacement(iDof)<< ")" ;
                    }
                }
                if (it->second->GetNumRotations()>0 )
                {
                	mLogger << "\t r:";
                    for(unsigned short iDof=0; iDof<it->second->GetNumRotations(); ++iDof)
                    {
                    	mLogger << "\t" << it->second->GetRotation(iDof) ;
                        mLogger << "("<< it->second->GetDofRotation(iDof)<< ")" ;
                    }
                }
                if (it->second->GetNumTemperatures()>0 )
                {
                	mLogger << "\t t:";
                    for(unsigned short iDof=0; iDof<it->second->GetNumTemperatures(); ++iDof)
                    {
                    	mLogger << "\t" << it->second->GetTemperature() ;
                        mLogger << "("<< it->second->GetDofTemperature()<< ")" ;
                    }
                }
                if (it->second->GetNumNonlocalTotalStrain()>0 )
                {
                	mLogger << "\t ns:";
                    for(unsigned short iDof=0; iDof<it->second->GetNumNonlocalTotalStrain(); ++iDof)
                    {
                    	mLogger << "\t" << it->second->GetNonlocalTotalStrain(iDof) ;
                        mLogger << "("<< it->second->GetDofNonlocalTotalStrain(iDof)<< ")" ;
                    }
                }
			}
			mLogger << "\n";
    	}

    }
}

//! creates a node at coordinate's origin
int NuTo::Structure::NodeCreate(std::string rDOFs)
{
	NuTo::FullVector<double,Eigen::Dynamic> coordinates(this->GetDimension());

	//return int identifier of the new node
    return NodeCreate(rDOFs,coordinates);
}

//! creates a node
int NuTo::Structure::NodeCreate(std::string rDOFs, NuTo::FullVector<double,Eigen::Dynamic>& rCoordinates)
{

    //find unused integer id
    int id(mNodeMap.size());
    boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.find(id);
    while (it!=mNodeMap.end())
    {
        id++;
        it = mNodeMap.find(id);
    }
    this->NodeCreate(id, rDOFs, rCoordinates, 0);

    //return int identifier of the new node
    return id;
}

//! creates a node
int NuTo::Structure::NodeCreate(std::string rDOFs, NuTo::FullVector<double,Eigen::Dynamic>& rCoordinates, int rNumTimeDerivatives)
{

    //find unused integer id
    int id(mNodeMap.size());
    boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.find(id);
    while (it!=mNodeMap.end())
    {
        id++;
        it = mNodeMap.find(id);
    }
    this->NodeCreate(id, rDOFs, rCoordinates, rNumTimeDerivatives);

    //return int identifier of the new node
    return id;
}

void NuTo::Structure::NodeCreate(int rNodeNumber, std::string rDOFs, NuTo::FullVector<double,Eigen::Dynamic>& rCoordinates, int rNumTimeDerivatives)
{
	// check node number
	boost::ptr_map<int,NodeBase>::iterator it = this->mNodeMap.find(rNodeNumber);
	if(it != this->mNodeMap.end())
	{
		throw MechanicsException("[NuTo::Structure::NodeCreate] Node already exists.");
	}

    // transform string to uppercase
    std::transform(rDOFs.begin(), rDOFs.end(), rDOFs.begin(), toupper);

    // check all values
    int attributes(1 << Node::COORDINATES);
    // bit 0 : coordinates
    // bit 1 : displacements
    // bit 2 : rotations
    // bit 3 : temperatures
    // bit 4 : nonlocal data
    // bit 5 : nonlocal eq. plastic strain (as a nodal value)
    // bit 6 : nonlocal total strain (as a nodal value)
    // bit 7 : nonlocal eq. strain (as a nodal value)
    // bit 8 :
    // (maximum number of bits: 30)

    boost::char_separator<char> sep(" ");
    boost::tokenizer< boost::char_separator<char> > tok(rDOFs, sep);
    for (boost::tokenizer< boost::char_separator<char>  >::iterator beg=tok.begin(); beg!=tok.end(); ++beg)
    {
        if (*beg=="COORDINATES")
        {
            attributes = attributes | 1 << Node::COORDINATES;
        }
        else if (*beg=="DISPLACEMENTS")
        {
            attributes = attributes | 1 << Node::DISPLACEMENTS;
        }
        else if (*beg=="ROTATIONS")
        {
            attributes = attributes | 1 << Node::ROTATIONS;
        }
        else if (*beg=="TEMPERATURES")
        {
            attributes = attributes | 1 << Node::TEMPERATURES;
        }
        else if (*beg=="NONLOCALDATA")
        {
            attributes = attributes | 1 << Node::NONLOCALDATA;
        }
        else if (*beg=="NONLOCALEQPLASTICSTRAIN")
        {
            attributes = attributes | 1 << Node::NONLOCALEQPLASTICSTRAIN;
        }
        else if (*beg=="NONLOCALTOTALSTRAIN")
        {
            attributes = attributes | 1 << Node::NONLOCALTOTALSTRAIN;
        }
        else
        {
    		throw MechanicsException("[NuTo::Structure::NodeCreate] invalid dof type: " + *beg +".");
        }
    }
    if (rCoordinates.GetNumRows()!=mDimension || rCoordinates.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::Structure::NodeCreate]\
 Dimension of the coordinate vector does not fit the dimension of the structure");

    NodeBase* nodePtr;

    switch (attributes)
    {
        // the << shifts the 1 bitwise to the left, so 1<<n = 2^n
        // it actually sets the n-th bit (from the right) to 1, and all the other to zero
    case (1 << Node::COORDINATES):
        // reference node having only coordinates
        switch (mDimension)
        {
        case 1:
        	nodePtr = new NuTo::NodeCoordinates<1>();
            break;
        case 2:
        	nodePtr = new NuTo::NodeCoordinates<2>();
            break;
        case 3:
        	nodePtr = new NuTo::NodeCoordinates<3>();
            break;
        default:
            throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
        }
    break;
    case (1 << Node::COORDINATES) | (1 << Node::DISPLACEMENTS):
        // coordinates and displacements
        switch (rNumTimeDerivatives)
        {
        case 0:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,0,1,0,0,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,0,2,0,0,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,0,3,0,0,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
	    case 2:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,2,1,0,0,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,2,2,0,0,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,2,3,0,0,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Coordinates and Displacements only implemented for 0 and 2 time derivatives.");
        }
    break;
	case (1 << Node::COORDINATES) | (1 << Node::DISPLACEMENTS) | (1 << Node::ROTATIONS):
		// coordinates and displacements and rotations
		switch (rNumTimeDerivatives)
		{
		case 0:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,0,1,0,0,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,0,2,1,0,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,0,3,3,0,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		case 2:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,2,1,0,0,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,2,2,1,0,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,2,3,3,0,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Coordinates, Displacements and Rotations only implemented for 0 and 2 time derivatives.");
		}
	break;
    case (1 << Node::COORDINATES) | (1 << Node::TEMPERATURES):
        // coordinates and temperatures
		switch (rNumTimeDerivatives)
		{
		case 0:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,0,0,0,1,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,0,0,0,1,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,0,0,0,1,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		case 1:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,1,0,0,1,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,1,0,0,1,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,1,0,0,1,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		case 2:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,2,0,0,1,0,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,2,0,0,1,0,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,2,0,0,1,0,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Coordinates, Displacements and Rotations only implemented for 0, 1 and 2 time derivatives.");
		}
    break;
	case (1 << Node::COORDINATES) | (1 << Node::DISPLACEMENTS) | (1 << Node::NONLOCALDATA):
		// coordinates, displacements and nonlocal data
		switch (mDimension)
		{
		case 1:
			throw MechanicsException("[NuTo::Structure::NodeCreate] A 1D node with nonlocal data is not implemented");
			break;
		case 2:
			throw MechanicsException("[NuTo::Structure::NodeCreate] nonlocal not yet implemented.");
			//nodePtr = new NuTo::NodeCoordinatesDisplacementsNonlocalData2D();
			break;
		//case 3:
		//	nodePtr = new NuTo::NodeCoordinatesDisplacementsNonlocalData3D();
		//	break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
		}
	break;
	case (1 << Node::COORDINATES) | (1 << Node::DISPLACEMENTS) | (1 << Node::NONLOCALEQPLASTICSTRAIN):
		// coordinates and displacements and rotations
		switch (rNumTimeDerivatives)
		{
		case 0:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,0,1,0,0,2,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,0,2,0,0,2,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,0,3,0,0,2,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		case 2:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,2,1,0,0,2,0,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,2,2,0,0,2,0,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,2,3,0,0,2,0,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Coordinates, Displacements and nonlocal eq. plastic strains only implemented for 0 and 2 time derivatives.");
		}
	break;
	case (1 << Node::COORDINATES) | (1 << Node::DISPLACEMENTS) | (1 << Node::NONLOCALTOTALSTRAIN):
		// coordinates and displacements and rotations
		switch (rNumTimeDerivatives)
		{
		case 0:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,0,1,0,0,0,1,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,0,2,0,0,0,3,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,0,3,0,0,0,6,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		case 2:
			switch (mDimension)
			{
			case 1:
				nodePtr = new NuTo::NodeCoordinatesDof<1,2,1,0,0,0,1,0>();
				break;
			case 2:
				nodePtr = new NuTo::NodeCoordinatesDof<2,2,2,0,0,0,3,0>();
				break;
			case 3:
				nodePtr = new NuTo::NodeCoordinatesDof<3,2,3,0,0,0,6,0>();
				break;
			default:
				throw MechanicsException("[NuTo::Structure::NodeCreate] Dimension of the structure is not valid.");
			}
		break;
		default:
			throw MechanicsException("[NuTo::Structure::NodeCreate] Coordinates, Displacements and onlocal total strains only implemented for 0 and 2 time derivatives.");
		}
		break;
    default:
        throw MechanicsException("[NuTo::Structure::NodeCreate] This combination of attributes is not implemented (just add in the source file the relevant combination).");
    }

    //add coordinates
    try
    {
        switch (mDimension)
        {
        case 1:
            nodePtr->SetCoordinates1D(rCoordinates.data());
            break;
        case 2:
            nodePtr->SetCoordinates2D(rCoordinates.data());
            break;
        case 3:
            nodePtr->SetCoordinates3D(rCoordinates.data());
            break;
        case 0:
            throw MechanicsException("[NuTo::StructureBase::NodeCreate] Node has no displacements.");
            break;
        }
    }
    catch (std::bad_cast & b)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeCreate] Node has no coordinates or its dimension is not equivalent to the dimension of the input matrix.");
    }
    catch (NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeCreate] Error setting coordinates.");
        throw b;
    }
    catch (...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeCreate] Error setting coordinates of node (unspecified exception).");
    }
    // add node to map
    this->mNodeMap.insert(rNodeNumber, nodePtr);

    //renumbering of dofs for global matrices required
    this->mNodeNumberingRequired  = true;
}

//! @brief create multiple nodes
//! @param reference to a FullMatrix containing the node coordinates (row->coordinate; col->nodes)
//! @return a FullMatrix<int,Eigen::Dynamic,Eigen::Dynamic> with the identifiers
NuTo::FullVector<int,Eigen::Dynamic> NuTo::Structure::NodesCreate(std::string rDOFs, NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic>& rCoordinates)
{
	std::vector<int> idVec;
	/// go through the nodes
	for(size_t i=0 ; i<(size_t)rCoordinates.GetNumColumns(); ++i)
	{
		NuTo::FullVector<double,Eigen::Dynamic> coordinate(rCoordinates.GetColumn(i));
		idVec.push_back(this->NodeCreate(rDOFs, coordinate));
	}

    //return int identifiers of the new nodes as FullMatrix
	NuTo::FullVector<int,Eigen::Dynamic> ids(idVec);

    return ids;
}

void NuTo::Structure::NodeDelete(int rNodeNumber)
{
	NodeDelete(rNodeNumber,true);
}

//! @brief Deletes a nodes
//! @param rNodeNumber identifier for the node
void NuTo::Structure::NodeDelete(int rNodeNumber, bool checkElements)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    boost::ptr_map<int,NodeBase>::iterator itNode = mNodeMap.find(rNodeNumber);
    if (itNode==this->mNodeMap.end())
    {
        throw MechanicsException("[NuTo::Structure::NodeDelete] Node with the given id does not exist.");
    }
    else
    {
    	if (checkElements)
    	{
    		NodeBase* nodePtr = itNode->second;
    	 	/// Search for node in elements: using a loop over all elements
			for(boost::ptr_map<int,ElementBase>::const_iterator elemIt=mElementMap.begin(); elemIt!=mElementMap.end(); ++elemIt)
			{
				// loop over all element nodes
				for(unsigned short iNode=0; iNode<elemIt->second->GetNumNodes(); ++iNode)
				{
					// if the id of the node to be deleted is found, throw an error
					if(nodePtr == elemIt->second->GetNode(iNode) )
					{
						std::stringstream outNode, outElement;
						outNode << rNodeNumber;
						outElement << (elemIt->first);
						throw MechanicsException( "[NuTo::Structure::NodeDelete] Node " + outNode.str() + " is used by element " + outElement.str() + ", delete element first");
					}
				}
			}
    	}

    	//! Search for node in groups
        //! using a loop over all groups
        //! remove the entry from all groups
        for(boost::ptr_map<int,GroupBase>::iterator groupIt=mGroupMap.begin();groupIt!=mGroupMap.end(); ++groupIt){
        	if(groupIt->second->GetType()==NuTo::Groups::Nodes){
        		if(groupIt->second->Contain(itNode->first))
        		{
        			groupIt->second->RemoveMember(itNode->first);
        		}
        	}
        }

        // delete element from map
        this->mNodeMap.erase(itNode);

        this->mNodeNumberingRequired = true;
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        mLogger<<"[NuTo::Structure::NodeDelete] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << "\n";
#endif
}

//! @brief number the dofs in the structure
void NuTo::Structure::NodeBuildGlobalDofs()
{
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
    try
    {
        // build initial node numbering
        this->mNumDofs = 0;

        //call routine that includes non standard DOFs' e.g. in StructureIp, for standard structures this routine is empty
        //mNumDofs is also increased
        NumberAdditionalGlobalDofs();

        //number Lagrange multipliers in constraint equations defined in StructureBase
        ConstraintNumberGlobalDofs(this->mNumDofs);

        for (boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
        {
            it->second->SetGlobalDofs(this->mNumDofs);
        }

        // build constraint matrix
        this->mNodeNumberingRequired = false;
        this->ConstraintGetConstraintMatrixBeforeGaussElimination(this->mConstraintMatrix);

        unsigned int numDependentDofs = this->mConstraintMatrix.GetNumRows();
        this->mNumActiveDofs = this->mNumDofs - numDependentDofs;

        //init RHSMatrix as a diagonal identity matrix
        mConstraintMappingRHS.Resize(numDependentDofs,numDependentDofs);
        for (unsigned int count=0; count<numDependentDofs ; count++)
        	mConstraintMappingRHS.AddValue(count,count,1.);

        // perform gauss algorithm
        std::vector<int> mappingInitialToNewOrdering;
        std::vector<int> mappingNewToInitialOrdering;

        this->mConstraintMatrix.Gauss(mConstraintMappingRHS, mappingNewToInitialOrdering, mappingInitialToNewOrdering);

        //calculate current rhs matrix
        this->ConstraintUpdateRHSAfterGaussElimination();

        // move dependent dofs at the end
        // Warning!!! after this loop mappingNewToInitialOrdering is no longer valid !!!
        std::vector<int> tmpMapping;
        for (unsigned int dependentDofCount = 0; dependentDofCount < numDependentDofs; dependentDofCount++)
        {
            tmpMapping.push_back(this->mNumActiveDofs + dependentDofCount);
            mappingInitialToNewOrdering[mappingNewToInitialOrdering[dependentDofCount]] += this->mNumActiveDofs;
        }
        for (int activeDofCount = numDependentDofs; activeDofCount < this->mNumDofs; activeDofCount++)
        {
            tmpMapping.push_back(activeDofCount - numDependentDofs);
            mappingInitialToNewOrdering[mappingNewToInitialOrdering[activeDofCount]] -= numDependentDofs;
        }
        mappingNewToInitialOrdering.clear();

        // reorder columns
        this->mConstraintMatrix.ReorderColumns(tmpMapping);
        //NuTo::FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> constraintMatrixFull(mConstraintMatrix);
        //constraintMatrixFull.Info(12,5);
        // remove columns of dependent dofs
        // check if the submatrix which is removed is a diagonal matrix
        const std::vector<int>& constraintMatrixRowIndex = this->mConstraintMatrix.GetRowIndex();
        const std::vector<int>& constraintMatrixColumns = this->mConstraintMatrix.GetColumns();
        for (int row = 0; row < this->mConstraintMatrix.GetNumRows(); row++)
        {
            for (int pos = constraintMatrixRowIndex[row]; pos < constraintMatrixRowIndex[row + 1]; pos++)
            {
                if ((constraintMatrixColumns[pos] > this->mNumActiveDofs) && (constraintMatrixColumns[pos] != row + this->mNumActiveDofs))
                {
                    throw MechanicsException("[NuTo::Structure::NodeBuildGlobalDofs] invalid matrix structure.");
                }
            }
        }
        this->mConstraintMatrix.RemoveLastColumns(numDependentDofs);

        //call routine that includes non standard DOFs' e.g. in StructureIp, for standard structures this routine is empty
        ReNumberAdditionalGlobalDofs(mappingInitialToNewOrdering);

        //renumber DOFS in constraints (Lagrange multiplier)
        ConstraintRenumberGlobalDofs(mappingInitialToNewOrdering);

        // renumber dofs
        for (boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
        {
            it->second->RenumberGlobalDofs(mappingInitialToNewOrdering);
        }

        mNodeNumberingRequired = false;
    }
    catch (MathException& e)
    {
        e.AddMessage("[NuTo::Structure::NodeBuildGlobalDofs] error building global dof numbering.");
        throw e;
    }
    catch (MechanicsException& e)
    {
        e.AddMessage("[NuTo::Structure::NodeBuildGlobalDofs] error building global dof numbering.");
        throw e;
    }
#ifdef SHOW_TIME
    end=clock();
    if (mShowTime)
        mLogger<<"[NuTo::Structure::NodeBuildGlobalDofs] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << "\n";
#endif
}

// merge dof values
void NuTo::Structure::NodeMergeActiveDofValues(int rTimeDerivative, const FullVector<double,Eigen::Dynamic>& rActiveDofValues)
{
    if (this->mNodeNumberingRequired)
    {
        throw MechanicsException("[NuTo::Structure::NodeMergeActiceDofValues] a valid dof numbering was not found (build dof numbering using NodeBuildGlobalDofs).");
    }
    if ((rActiveDofValues.GetNumRows() != this->mNumActiveDofs) || rActiveDofValues.GetNumColumns() != 1)
    {
        throw MechanicsException("[NuTo::Structure::NodeMergeActiceDofValues] invalid dimension of input object (number of active dofs,1).");
    }
	this->mUpdateTmpStaticDataRequired=true;

    // calculate dependent dof values
	FullVector<double,Eigen::Dynamic> dependentDofValues;
    if (mNumActiveDofs>0)
        dependentDofValues = this->mConstraintRHS - this->mConstraintMatrix * rActiveDofValues;
    else
        dependentDofValues = this->mConstraintRHS;

    // write dof values to the nodes
    for (boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
    {
        it->second->SetGlobalDofValues(rTimeDerivative, rActiveDofValues, dependentDofValues);
    }

    //write the dof values of the Lagrange multipliers
    ConstraintMergeGlobalDofValues(rActiveDofValues, dependentDofValues);

    //write dof values of additional DOFs
    NodeMergeAdditionalGlobalDofValues(rTimeDerivative, rActiveDofValues,dependentDofValues);
}

// merge dof values
void NuTo::Structure::NodeMergeDofValues(int rTimeDerivative, const FullVector<double,Eigen::Dynamic>& rActiveDofValues, const FullVector<double,Eigen::Dynamic>& rDependentDofValues)
{
    if (this->mNodeNumberingRequired)
    {
        throw MechanicsException("[NuTo::Structure::NodeMergeActiceDofValues] a valid dof numbering was not found (build dof numbering using NodeBuildGlobalDofs).");
    }
    if ((rActiveDofValues.GetNumRows() != this->mNumActiveDofs) || rActiveDofValues.GetNumColumns() != 1)
    {
        throw MechanicsException("[NuTo::Structure::NodeMergeActiceDofValues] invalid dimension of input object (number of active dofs,1).");
    }
    if ((rDependentDofValues.GetNumRows() != this->mNumDofs - this->mNumActiveDofs) || rDependentDofValues.GetNumColumns() != 1)
    {
        throw MechanicsException("[NuTo::Structure::NodeMergeActiceDofValues] invalid dimension of input object (number of dependent dofs,1).");
    }
	this->mUpdateTmpStaticDataRequired=true;


    // write dof values to the nodes
    for (boost::ptr_map<int,NodeBase>::iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
    {
        it->second->SetGlobalDofValues(rTimeDerivative, rActiveDofValues, rDependentDofValues);
    }

    //write the dof values of the Lagrange multipliers
    ConstraintMergeGlobalDofValues(rActiveDofValues, rDependentDofValues);

    //write dof values of additional DOFs
    NodeMergeAdditionalGlobalDofValues(rTimeDerivative,rActiveDofValues,rDependentDofValues);
}

// extract dof values
void NuTo::Structure::NodeExtractDofValues(int rTimeDerivative, FullVector<double,Eigen::Dynamic>& rActiveDofValues, FullVector<double,Eigen::Dynamic>& rDependentDofValues) const
{
    if (this->mNodeNumberingRequired)
    {
        throw MechanicsException("[NuTo::Structure::NodeExtractDofValues] a valid dof numbering was not found (build dof numbering using NodeBuildGlobalDofs).");
    }
    rActiveDofValues.Resize(this->mNumActiveDofs);
    rDependentDofValues.Resize(this->mNumDofs - this->mNumActiveDofs);

    // extract dof values from nodes
    for (boost::ptr_map<int,NodeBase>::const_iterator it = mNodeMap.begin(); it!= mNodeMap.end(); it++)
    {
        it->second->GetGlobalDofValues(rTimeDerivative, rActiveDofValues, rDependentDofValues);
    }
    //extract dof values of Lagrange multipliers
    ConstraintExtractGlobalDofValues(rActiveDofValues,rDependentDofValues);

    //extract dof values of additional DOFs
    NodeExtractAdditionalGlobalDofValues(rTimeDerivative, rActiveDofValues,rDependentDofValues);
}

// store all nodes of a structure in a vector
void NuTo::Structure::GetNodesTotal(std::vector<const NodeBase*>& rNodes) const
{
	rNodes.reserve(mNodeMap.size());
    rNodes.resize(0);
	boost::ptr_map<int,NodeBase>::const_iterator NodeIter = this->mNodeMap.begin();
    while (NodeIter != this->mNodeMap.end())
    {
    	rNodes.push_back(NodeIter->second);
        NodeIter++;
    }
}

// store all nodes of a structure in a vector
void NuTo::Structure::GetNodesTotal(std::vector<std::pair<int, const NodeBase*> >& rNodes) const
{
	rNodes.reserve(mNodeMap.size());
    rNodes.resize(0);
	boost::ptr_map<int,NodeBase>::const_iterator NodeIter = this->mNodeMap.begin();
    while (NodeIter != this->mNodeMap.end())
    {
    	rNodes.push_back(std::pair<int, const NodeBase*>(NodeIter->first,NodeIter->second));
        NodeIter++;
    }
}

// store all nodes of a structure in a vector
void NuTo::Structure::GetNodesTotal(std::vector<NodeBase*>& rNodes)
{
	rNodes.reserve(mNodeMap.size());
	rNodes.resize(0);
    boost::ptr_map<int,NodeBase>::iterator NodeIter = this->mNodeMap.begin();
    while (NodeIter != this->mNodeMap.end())
    {
    	rNodes.push_back(NodeIter->second);
        NodeIter++;
    }
}

// store all nodes of a structure in a vector
void NuTo::Structure::GetNodesTotal(std::vector<std::pair<int,NodeBase*> >& rNodes)
{
	rNodes.reserve(mNodeMap.size());
	rNodes.resize(0);
    boost::ptr_map<int,NodeBase>::iterator NodeIter = this->mNodeMap.begin();
    while (NodeIter != this->mNodeMap.end())
    {
    	rNodes.push_back(std::pair<int, NodeBase*>(NodeIter->first,NodeIter->second));
        NodeIter++;
    }
}

//! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
//! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still identical
void NuTo::Structure::NodeExchangePtr(int rId, NodeBase* rOldPtr, NodeBase* rNewPtr)
{
    //in node map
    //find it
    int nodeId = NodeGetId(rOldPtr);
    if (mNodeMap.erase(nodeId)!=1)
    {
        throw MechanicsException("[NuTo::Structure::NodeExchangePtr] Pointer to node (to exchange) does not exist.");
    }
    mNodeMap.insert(nodeId,rNewPtr);

    //in elements
    for (boost::ptr_map<int,ElementBase>::iterator itElement = mElementMap.begin(); itElement!= mElementMap.end(); itElement++)
    {
        itElement->second->ExchangeNodePtr(rOldPtr, rNewPtr);
    }

    //in groups
    for(boost::ptr_map<int,GroupBase>::iterator groupIt=mGroupMap.begin();groupIt!=mGroupMap.end(); ++groupIt)
    {
        if(groupIt->second->GetType()==NuTo::Groups::Nodes)
        {
        	if (groupIt->second->Contain(rId))
                groupIt->second->ExchangePtr(rId, rOldPtr, rNewPtr);
        }
    }

    //in constraints
    //in groups
    for(boost::ptr_map<int,ConstraintBase>::iterator constraintIt=mConstraintMap.begin();constraintIt!=mConstraintMap.end(); ++constraintIt)
    {
        constraintIt->second->ExchangeNodePtr(rOldPtr, rNewPtr);
    }


}

