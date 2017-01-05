#include "mechanics/timeIntegration/TimeIntegrationBase.h"
#include "mechanics/timeIntegration/NewmarkDirect.h"
#include "base/ErrorEnum.h"
#include "mechanics/structures/StructureBase.h"
#include "mechanics/MechanicsException.h"
#include "base/Timer.h"

#include "mechanics/structures/StructureBase.h"
#include "mechanics/structures/unstructured/StructureFETI.h"
#include "mechanics/structures/StructureBaseEnum.h"
#include "mechanics/structures/StructureOutputBlockMatrix.h"
#include "mechanics/structures/StructureOutputDummy.h"
#include "/usr/lib/openmpi/include/mpi.h"

#include "base/CallbackInterface.h"
#include "math/SparseMatrixCSRGeneral.h"
#include <eigen3/Eigen/Dense>
#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "mechanics/nodes/NodeEnum.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveCalculateStaticData.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveTimeStep.h"

#include <cmath>

namespace NuTo
{
class NewmarkFeti : public NewmarkDirect
{
public:

    using VectorXd      = Eigen::VectorXd;
    using MatrixXd      = Eigen::MatrixXd;
    using SparseMatrix  = Eigen::SparseMatrix<double>;

    ///
    /// \brief NewmarkFeti
    /// \param rStructure
    ///
    NewmarkFeti(StructureFETI* rStructure) : NewmarkDirect (rStructure)
    {

    }

    ///
    /// \brief GetTypeId
    /// \return
    ///
    std::string GetTypeId() const override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Not implemented!");
    }

    ///
    /// \brief HasCriticalTimeStep
    /// \return
    ///
    bool HasCriticalTimeStep() const override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Not implemented!");
    }

    ///
    /// \brief CalculateCriticalTimeStep
    /// \return
    ///
    double CalculateCriticalTimeStep() const override
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Not implemented!");
    }


    ///
    /// \brief GatherInterfaceRigidBodyModes
    /// \param interfaceRigidBodyModes
    /// \param numRigidBodyModesGlobal
    /// \return
    ///
    MatrixXd GatherInterfaceRigidBodyModes(Eigen::MatrixXd& interfaceRigidBodyModes, const int numRigidBodyModesGlobal);

    ///
    /// \brief GatherRigidBodyForceVector
    /// \param rigidBodyForceVectorLocal
    /// \param numRigidBodyModesGlobal
    /// \return
    ///
    VectorXd GatherRigidBodyForceVector(Eigen::VectorXd& rigidBodyForceVectorLocal, const int numRigidBodyModesGlobal);

    ///
    /// \brief MpiGatherRecvCountAndDispls
    /// \param recvCount
    /// \param displs
    /// \param numValues
    ///
    void MpiGatherRecvCountAndDispls(std::vector<int>& recvCount, std::vector<int>& displs, const int numValues);

    ///
    /// \brief CalculateNormResidual
    /// \param residual_mod
    /// \param activeDofSet
    /// \return
    ///
    BlockScalar CalculateNormResidual(BlockFullVector<double>& residual_mod, const std::set<Node::eDof>& activeDofSet);

    //! @brief Projected stabilized Bi-conjugate gradient method (BiCGStab)
    //!
    //! Iterative method to solve the interface problem.
    //! @param projection: Projection matrix that guarantees that the solution satisfies the constraint at every iteration
    int BiCgStab(const MatrixXd& projection, VectorXd& x, const VectorXd& rhs);

    //! @brief Conjugate projected gradient method (CG)
    //!
    //! Iterative method to solve the interface problem.
    //! @param projection: Projection matrix that guarantees that the solution satisfies the constraint at every iteration
    int CPG(const MatrixXd& projection, VectorXd& x, const VectorXd& rhs);

    //! @brief Solves the global and local problem
    //!
    //! Calculates the rigid body modes of the structure.
    //! Calculates the initial guess for the projected CG/BiCGStab method
    //! Solves for the Lagrange multipliers at the subdomain interfaces.
    //! Calculates the increment of the free degrees of freedom
    //!
    StructureOutputBlockVector FetiSolve(BlockFullVector<double> residual_mod, const std::set<Node::eDof>& activeDofSet, VectorXd& deltaLambda);

    //! @brief perform the time integration
    //! @param rTimeDelta ... length of the simulation
    NuTo::eError Solve(double rTimeDelta) override;

private:
    Eigen::SparseQR<Eigen::SparseMatrix<double>,Eigen::COLAMDOrdering<int>> mSolver;
    SparseMatrix mLocalPreconditioner;
    SparseMatrix mTangentStiffnessMatrix;
    const double    mCpgTolerance     = 1.0e-6;
    const int       mCpgMaxIterations = 1000;
};
}// namespace NuTo