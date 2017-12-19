#include "BoostUnitTest.h"
#include "mechanics/dofs/DofVector.h"
#include <sstream>

using namespace NuTo;

struct TestVectors
{
    DofType dof0 = DofType("foo", 1);
    DofType dof1 = DofType("bar", 1);

    DofVector<double> v0;
    DofVector<double> v1;

    TestVectors()
    {
        v0[dof0] = Eigen::Vector3d({1, 2, 3});
        v0[dof1] = Eigen::Vector2d({8, 9});

        v1[dof0] = Eigen::Vector3d({10, 20, 30});
        v1[dof1] = Eigen::Vector2d({80, 90});
    }
};

BOOST_FIXTURE_TEST_CASE(DofVectorAddition, TestVectors)
{
    v0 += v1;
    BoostUnitTest::CheckEigenMatrix(v0[dof0], Eigen::Vector3d(11, 22, 33));
    BoostUnitTest::CheckEigenMatrix(v0[dof1], Eigen::Vector2d(88, 99));
    BoostUnitTest::CheckEigenMatrix(v1[dof0], Eigen::Vector3d(10, 20, 30));
    BoostUnitTest::CheckEigenMatrix(v1[dof1], Eigen::Vector2d(80, 90));
}

BOOST_FIXTURE_TEST_CASE(DofVectorScalarMultiplication, TestVectors)
{
    v0 *= 2.;
    BoostUnitTest::CheckEigenMatrix(v0[dof0], Eigen::Vector3d(2, 4, 6));
    BoostUnitTest::CheckEigenMatrix(v0[dof1], Eigen::Vector2d(16, 18));

    DofVector<double> v = v0 * 0.5;
    BoostUnitTest::CheckEigenMatrix(v[dof0], Eigen::Vector3d(1, 2, 3));
    BoostUnitTest::CheckEigenMatrix(v[dof1], Eigen::Vector2d(8, 9));
}

BOOST_FIXTURE_TEST_CASE(DofVectorUninitializedAddition, TestVectors)
{
    DofVector<double> v;
    v += v0 + v1;
    BoostUnitTest::CheckEigenMatrix(v[dof0], Eigen::Vector3d(11, 22, 33));
    BoostUnitTest::CheckEigenMatrix(v[dof1], Eigen::Vector2d(88, 99));
}

BOOST_FIXTURE_TEST_CASE(DofVectorExport, TestVectors)
{
    Eigen::VectorXd vExportD1 = ToEigen(v0, {dof1});
    BoostUnitTest::CheckVector(vExportD1, std::vector<double>{8, 9}, 2);

    Eigen::VectorXd vExportD0D1 = ToEigen(v0, {dof0, dof1});
    BoostUnitTest::CheckVector(vExportD0D1, std::vector<double>{1, 2, 3, 8, 9}, 5);

    Eigen::VectorXd vExportD1D0 = ToEigen(v0, {dof1, dof0});
    BoostUnitTest::CheckVector(vExportD1D0, std::vector<double>{8, 9, 1, 2, 3}, 5);
}

BOOST_FIXTURE_TEST_CASE(DofVectorImport, TestVectors)
{
    Eigen::VectorXd v(5);
    v << 0, 1, 2, 3, 4;

    FromEigen(v0, v, {dof1, dof0});
    BoostUnitTest::CheckEigenMatrix(v0[dof0], Eigen::Vector3d(0, 1, 2));
    BoostUnitTest::CheckEigenMatrix(v0[dof1], Eigen::Vector2d(3, 4));
}


BOOST_FIXTURE_TEST_CASE(DofVectorStream, TestVectors)
{
    std::stringstream ss;
    ss << v0;
}
