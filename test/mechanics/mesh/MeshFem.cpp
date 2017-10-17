#include "BoostUnitTest.h"
#include "mechanics/mesh/MeshFem.h"
#include "mechanics/interpolation/InterpolationTriangleLinear.h"

NuTo::MeshFem DummyMesh(NuTo::DofType dofType)
{
    NuTo::MeshFem mesh;
    auto& interpolationCoords = mesh.CreateInterpolation(NuTo::InterpolationTriangleLinear(2));
    auto& interpolationDof = mesh.CreateInterpolation(NuTo::InterpolationTriangleLinear(dofType.GetNum()));

    auto& n0 = mesh.Nodes.Add(Eigen::Vector2d({1, 0}));
    auto& n1 = mesh.Nodes.Add(Eigen::Vector2d({2, 0}));
    auto& n2 = mesh.Nodes.Add(Eigen::Vector2d({0, 3}));

    auto& nd0 = mesh.Nodes.Add(Eigen::VectorXd::Constant(1, 1));
    auto& nd1 = mesh.Nodes.Add(Eigen::VectorXd::Constant(1, 2));
    auto& nd2 = mesh.Nodes.Add(Eigen::VectorXd::Constant(1, 3));

    auto& e0 = mesh.Elements.Add({{{n0, n1, n2}, interpolationCoords}});
    e0.AddDofElement(dofType, {{nd0, nd1, nd2}, interpolationDof});
    return mesh;
}

BOOST_AUTO_TEST_CASE(MeshAddStuff)
{
    NuTo::DofType d("Dof", 1);
    NuTo::MeshFem mesh = DummyMesh(d);

    auto& e0 = mesh.Elements.front();
    BoostUnitTest::CheckVector(e0.CoordinateElement().ExtractNodeValues(), std::vector<double>({1, 0, 2, 0, 0, 3}), 6);
}

BOOST_AUTO_TEST_CASE(MeshNodeSelection)
{
    NuTo::DofType d("Dof", 1);
    NuTo::MeshFem mesh = DummyMesh(d);

    const auto& n = NuTo::GetNodeAt(mesh.Elements, Eigen::Vector2d(0, 3), d);
    BoostUnitTest::CheckEigenMatrix(n.GetValues(), Eigen::VectorXd::Constant(1, 3));

    BOOST_CHECK_THROW(NuTo::GetNodeAt(mesh.Elements, Eigen::Vector2d(0, 0), d), NuTo::Exception);
}
