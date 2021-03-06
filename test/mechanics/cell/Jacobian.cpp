#include "BoostUnitTest.h"
#include "nuto/mechanics/cell/Jacobian.h"

#include "nuto/mechanics/interpolation/InterpolationTrussLinear.h"
#include "nuto/mechanics/interpolation/InterpolationTrussQuadratic.h"
#include "nuto/mechanics/interpolation/InterpolationTriangleLinear.h"
#include "nuto/mechanics/interpolation/InterpolationQuadLinear.h"
#include "nuto/mechanics/interpolation/InterpolationBrickLinear.h"
#include "nuto/mechanics/interpolation/InterpolationTetrahedronLinear.h"

using namespace NuTo;

BOOST_AUTO_TEST_CASE(Jacobian1DDet)
{
    Eigen::MatrixXd B = InterpolationTrussLinear::DerivativeShapeFunctions();
    Eigen::VectorXd coordinates = Eigen::Vector2d({12., 17});
    Jacobian jacobian(coordinates, B);
    BOOST_CHECK_CLOSE(jacobian.Det(), 2.5, 1.e-10);
}

BOOST_AUTO_TEST_CASE(Jacobian2DDet)
{
    Eigen::MatrixXd B = InterpolationQuadLinear::DerivativeShapeFunctions(Eigen::Vector2d({0, 0}));
    Eigen::VectorXd coordinates = Eigen::VectorXd(8);
    coordinates << 0, 0, 2, 0, 2, 8, 0, 8; // rectangle from (0,0) to (2,8)


    Jacobian jacobian(coordinates, B);
    BOOST_CHECK_CLOSE(jacobian.Det(), 4, 1.e-10); // reference area = 4, this area = 16;
}

BOOST_AUTO_TEST_CASE(Jacobian3DDet)
{
    Eigen::MatrixXd B = InterpolationTetrahedronLinear::DerivativeShapeFunctions();
    Eigen::VectorXd coordinates = Eigen::VectorXd(12);
    coordinates << 0, 0, 0, 10, 0, 0, 0, 5, 0, 0, 0, 42;

    Jacobian jacobian(coordinates, B);
    BOOST_CHECK_CLOSE(jacobian.Det(), 10 * 5 * 42, 1.e-10);
}

BOOST_AUTO_TEST_CASE(Jacobian1Din2D)
{
    {
        Eigen::MatrixXd B = InterpolationTrussLinear::DerivativeShapeFunctions();
        Eigen::VectorXd coordinates = Eigen::VectorXd(4);
        coordinates << 0, 0, 3, 4;

        Jacobian jacobian(coordinates, B);
        BOOST_CHECK_CLOSE(jacobian.Det() * 2, 5, 1.e-10);
        //
        // total length of the element is 5 = sqrt(3*3 + 4*4)
        // factor 2 in Det() comes from the integration point weigth
    }
    {
        Eigen::VectorXd coordinates = Eigen::VectorXd(6);
        coordinates << 0, 0, 1.5, 2, 3, 4;

        Eigen::MatrixXd B0 = InterpolationTrussQuadratic::DerivativeShapeFunctions(
                Eigen::VectorXd::Constant(1, -std::sqrt(1. / 3.)));
        Eigen::MatrixXd B1 =
                InterpolationTrussQuadratic::DerivativeShapeFunctions(Eigen::VectorXd::Constant(1, std::sqrt(1. / 3.)));
        Jacobian jacobian0(coordinates, B0);
        Jacobian jacobian1(coordinates, B1);
        BOOST_CHECK_CLOSE(jacobian0.Det() + jacobian1.Det(), 5, 1.e-10);
        //
        // total length of the element is 5 = sqrt(3*3 + 4*4)
        // each integration point has weight 1
    }
}

//! @brief calculates the area of the triangle (a,b,c) via the jacobian to a reference unit triangle
double TriangleAreaViaJacobian(Eigen::Vector3d a, Eigen::Vector3d b, Eigen::Vector3d c)
{
    Eigen::MatrixXd B = InterpolationTriangleLinear::DerivativeShapeFunctions(Eigen::Vector2d::Zero());
    Eigen::VectorXd coordinates = Eigen::VectorXd(9);
    coordinates << a.x(), a.y(), a.z(), b.x(), b.y(), b.z(), c.x(), c.y(), c.z();

    Jacobian jacobian(coordinates, B);

    double ratioAreaRealToAreaNatural = jacobian.Det();
    double areaNatural = 0.5; // triangle (0,0) ; (1,0) ; (0,1)
    return ratioAreaRealToAreaNatural * areaNatural;
}

BOOST_AUTO_TEST_CASE(Jacobian2Din3D)
{
    // make the 3d vector construction shorter.
    auto V = [](double x, double y, double z) { return Eigen::Vector3d(x, y, z); };

    BOOST_CHECK_CLOSE(TriangleAreaViaJacobian(V(0, 0, 0), V(1, 0, 0), V(0, 1, 0)), 0.5, 1.e-10);
    BOOST_CHECK_CLOSE(TriangleAreaViaJacobian(V(0, 0, 0), V(5, 0, 0), V(0, 0, 7)), 5. * 7 / 2, 1.e-10);
    BOOST_CHECK_CLOSE(TriangleAreaViaJacobian(V(0, 4, 0), V(0, 0, 3), V(0, 4, 3)), 3. * 4 / 2, 1.e-10);
}

BOOST_AUTO_TEST_CASE(Jacobian2Din3DComputeNormal)
{
    Eigen::Vector3d p1(1., 0., 0.);
    Eigen::Vector3d p2(0., 1., 0.);
    Eigen::Vector3d p3(0., 0., 1.);

    Eigen::MatrixXd B = InterpolationTriangleLinear::DerivativeShapeFunctions(Eigen::Vector2d::Zero());
    Eigen::VectorXd coordinates = Eigen::VectorXd(9);
    coordinates << p1, p2, p3;

    Jacobian jacobian(coordinates, B);

    Eigen::Vector3d normal = jacobian.Normal();
    double expectedNormalComponents = 1. / sqrt(3.);
    BOOST_CHECK_CLOSE(normal[0], expectedNormalComponents, 1.e-8);
    BOOST_CHECK_CLOSE(normal[1], expectedNormalComponents, 1.e-8);
    BOOST_CHECK_CLOSE(normal[2], expectedNormalComponents, 1.e-8);
}

BOOST_AUTO_TEST_CASE(Jacobian1Din2DComputeNormal)
{
    Eigen::Vector2d p1(1., 0.);
    Eigen::Vector2d p2(0., 1.);

    Eigen::MatrixXd B = InterpolationTrussLinear::DerivativeShapeFunctions();
    Eigen::VectorXd coordinates = Eigen::VectorXd(4);
    coordinates << p1, p2;

    Jacobian jacobian(coordinates, B);

    Eigen::Vector2d normal = jacobian.Normal();
    double expectedNormalComponents = 1. / sqrt(2.);
    BOOST_CHECK_CLOSE(normal[0], expectedNormalComponents, 1.e-8);
    BOOST_CHECK_CLOSE(normal[1], expectedNormalComponents, 1.e-8);
}

BOOST_AUTO_TEST_CASE(JacobianTransform)
{
    // TODO! How?
}

BOOST_AUTO_TEST_CASE(JacobianExceptions)
{
    Eigen::MatrixXd B = InterpolationBrickLinear::DerivativeShapeFunctions(Eigen::Vector3d(0., 0., 0.));
    Eigen::Vector3d coordinates(1., 2., 3.);
    BOOST_CHECK_THROW(Jacobian jacobian(coordinates, B), Exception);
}
