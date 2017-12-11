#include "mechanics/mesh/UnitMeshFem.h"
#include "BoostUnitTest.h"

void Check2DMesh(NuTo::MeshFem& mesh)
{
    BOOST_CHECK_EQUAL(mesh.Nodes.Size(), 3 * 8);

    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(0., 0.)));
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1., 1.)));

    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1. / 2., 1. / 7.)));
    BOOST_CHECK_NO_THROW(mesh.NodeAtCoordinate(Eigen::Vector2d(1. / 2., 5. / 7.)));

    auto f = [](Eigen::VectorXd coords) // transforms mesh to (42,4) -- (44, 11)
    {
        Eigen::VectorXd newCoords(2);
        newCoords[0] = 42 + coords[0] * 2;
        newCoords[1] = 4 + coords[1] * 7;
        return newCoords;
    };

    NuTo::MeshFem transformedMesh = NuTo::UnitMeshFem::Transform(std::move(mesh), f);

    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(42., 4.)));
    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(44., 11.)));

    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(43., 5.)));
    BOOST_CHECK_NO_THROW(transformedMesh.NodeAtCoordinate(Eigen::Vector2d(43., 9.)));
}

BOOST_AUTO_TEST_CASE(MeshTrusses)
{
    constexpr int numElements = 15;
    auto mesh = NuTo::UnitMeshFem::CreateTrusses(numElements);
    BOOST_CHECK_EQUAL(mesh.Elements.Size(), numElements);
    BOOST_CHECK_EQUAL(mesh.Nodes.Size(), numElements + 1);

    for (const auto& node : mesh.Nodes)
    {
        BOOST_CHECK(
                std::abs((node.GetValues()[0] * numElements) - std::round(node.GetValues()[0] * numElements) < 1e-6));
        BOOST_CHECK(node.GetValues()[0] <= 1.0 && node.GetValues()[0] >= 0.0);
    }

    for (const auto& element : mesh.Elements)
    {
        BOOST_CHECK(element.CoordinateElement().GetNode(0).GetValues()[0] <
                    element.CoordinateElement().GetNode(1).GetValues()[0]);
    }
}

BOOST_AUTO_TEST_CASE(MeshQuad)
{
    auto mesh = NuTo::UnitMeshFem::CreateQuads(2, 7);
    BOOST_CHECK_EQUAL(mesh.Elements.Size(), 2 * 7);
    Check2DMesh(mesh);
}

BOOST_AUTO_TEST_CASE(MeshTriangle)
{
    auto mesh = NuTo::UnitMeshFem::CreateTriangles(2, 7);
    BOOST_CHECK_EQUAL(mesh.Elements.Size(), 2 * 7 * 2);
    Check2DMesh(mesh);
}

BOOST_AUTO_TEST_CASE(MeshValidAfterTransform)
{
    auto mesh = NuTo::UnitMeshFem::CreateQuads(1, 1);
    Eigen::VectorXd expected(8);
    expected << 0, 0, 1, 0, 1, 1, 0, 1;

    auto& coordinateElement = mesh.Elements[0].CoordinateElement();
    BoostUnitTest::CheckEigenMatrix(coordinateElement.ExtractNodeValues(), expected);

    auto f = [](Eigen::VectorXd coords) { return Eigen::Vector2d(coords[0] * 4, coords[1] * 42); };

    NuTo::MeshFem transformedMesh = NuTo::UnitMeshFem::Transform(std::move(mesh), f);
    auto& transformedCoordinateElement = transformedMesh.Elements[0].CoordinateElement();
    expected << 0, 0, 4, 0, 4, 42, 0, 42;
    BoostUnitTest::CheckEigenMatrix(transformedCoordinateElement.ExtractNodeValues(), expected);

    transformedMesh.Nodes[0].SetValue(0, 6174);
    expected << 6174, 0, 4, 0, 4, 42, 0, 42;
    BoostUnitTest::CheckEigenMatrix(transformedCoordinateElement.ExtractNodeValues(), expected);
}
