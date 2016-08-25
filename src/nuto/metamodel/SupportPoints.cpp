// $Id$

/*******************************************************************************
 Bauhaus-University Weimar
 Author: Joerg F. Unger ,  September 2009
*******************************************************************************/

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/ptr_container/serialize_ptr_list.hpp>
#endif //ENABLE_SERIALIZATION

#include <boost/math/distributions/normal.hpp>

#include "nuto/metamodel/MetamodelException.h"
#include "nuto/metamodel/SupportPoints.h"
#include "nuto/metamodel/Transformation.h"

NuTo::SupportPoints::SupportPoints()
{
    mTransformationBuild = false;
}

NuTo::SupportPoints::~SupportPoints()
{
    ClearTransformations();
}

//! @brief clear support points
void NuTo::SupportPoints::Clear()
{
    mSPOrigInput.Resize(0,0);
    mSPTransInput.Resize(0,0);
    mSPTransOutput.Resize(0,0);
    mWeight.resize(0);

    mlTransformationInput.clear();
    mlTransformationOutput.clear();

    mTransformationBuild = false;
}

//! @brief info about support points
void NuTo::SupportPoints::Info()const
{
    throw MetamodelException("[SupportPoints::Info()] not yet implemented.");
}

void NuTo::SupportPoints::BuildTransformation()
{
	mSPTransInput = mSPOrigInput;
	for (boost::ptr_list<Transformation>::iterator it = mlTransformationInput.begin(); it!=mlTransformationInput.end();it++)
	{
		it->Build(mSPTransInput);
        it->TransformForward(mSPTransInput);
    }
    
	mSPTransOutput = mSPOrigOutput;
	for (boost::ptr_list<Transformation>::iterator it = mlTransformationOutput.begin(); it!=mlTransformationOutput.end();it++)
	{
		it->Build(mSPTransOutput);
        it->TransformForward(mSPTransOutput);
    }
	mTransformationBuild = true;
}


void NuTo::SupportPoints::AppendTransformationInput(Transformation* rTransformation)
{
    mlTransformationInput.push_back(rTransformation);
}

void NuTo::SupportPoints::AppendTransformationOutput(Transformation* rTransformation)
{
    mlTransformationOutput.push_back(rTransformation);
}

void NuTo::SupportPoints::SetSupportPoints(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSPOrigInput, const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSPOrigOutput)
{
    if (rSPOrigInput.GetNumColumns()!=rSPOrigOutput.GetNumColumns())
    {
        throw MetamodelException("[NuTo::SupportPoints::SetSupportPoints] Number of columns for input and output must be identical (=number of samples).");   
    }
    
    mSPOrigInput = rSPOrigInput;
    mSPOrigOutput  = rSPOrigOutput;

	mWeight.resize(GetNumSupportPoints(),1);
    
    // clear all transformations, since it is no longer for sure that the dimensions remain identical
    mTransformationBuild = false;
    ClearTransformations();
}

//! @brief perform forward transformation for inputs (from orig to transformed)
void NuTo::SupportPoints::TransformForwardInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const
{
    for (boost::ptr_list<Transformation>::const_iterator it=mlTransformationInput.begin();it!=mlTransformationInput.end(); it++)
        it->TransformForward(rCoordinates);
}

//! @brief perform backward transformation for inputs  (from transformed to orig)
void NuTo::SupportPoints::TransformBackwardInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const
{
    for (boost::ptr_list<Transformation>::const_reverse_iterator it=mlTransformationInput.rbegin();it!=mlTransformationInput.rend(); it++)
        it->TransformBackward(rCoordinates);
}

//! @brief perform forward transformation for outputs (from transformed to orig)
//! @brief attention, this is exactly the backwards order, since transformations for outputs are given in revers order
//! @brief orig->trans1->trans2-transformed
void NuTo::SupportPoints::TransformForwardOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const
{
    for (boost::ptr_list<Transformation>::const_reverse_iterator it=mlTransformationOutput.rbegin();it!=mlTransformationOutput.rend(); it++)
        it->TransformBackward(rCoordinates);
}    

//! @brief perform backward transformation for outputs
//! @brief attention, this is exactly the backwards order, since transformations for given in revers order
void NuTo::SupportPoints::TransformBackwardOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const
{
    for (boost::ptr_list<Transformation>::const_iterator it=mlTransformationOutput.begin();it!=mlTransformationOutput.end(); it++)
        it->TransformForward(rCoordinates);
}

//! @brief Clears all the transformations for input and output
void NuTo::SupportPoints::ClearTransformations()
{
    mlTransformationInput.clear();
    mlTransformationOutput.clear();
}

// calculate mean values of original inputs
void NuTo::SupportPoints::GetMeanValueOriginalInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMean)const
{
	this->CalculateMeanValues(this->mSPOrigInput, rMean);
}

// calculate mean values of transformed inputs
void NuTo::SupportPoints::GetMeanValueTransformedInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMean)const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetMeanValueOTransformedInput] transformation needs to be built first.");
	}
	this->CalculateMeanValues(this->mSPTransInput, rMean);
}

// calculate mean values of original outputs
void NuTo::SupportPoints::GetMeanValueOriginalOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMean)const
{
	this->CalculateMeanValues(this->mSPOrigOutput, rMean);
}

// calculate mean values of transformed outputs
void NuTo::SupportPoints::GetMeanValueTransformedOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMean)const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetMeanValueOTransformedOutput] transformation needs to be built first.");
	}
	this->CalculateMeanValues(this->mSPTransOutput, rMean);
}

// calculate variance of original inputs
void NuTo::SupportPoints::GetVarianceOriginalInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rVariance)const
{
	this->CalculateVariance(this->mSPOrigInput, rVariance);
}

// calculate variance of transformed inputs
void NuTo::SupportPoints::GetVarianceTransformedInput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rVariance)const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetVarianceTransformedInput] transformation needs to be built first.");
	}
	this->CalculateVariance(this->mSPTransInput, rVariance);
}

// calculate variance of original outputs
void NuTo::SupportPoints::GetVarianceOriginalOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rVariance)const
{
	this->CalculateVariance(this->mSPOrigOutput, rVariance);
}

// calculate variance of transformed outputs
void NuTo::SupportPoints::GetVarianceTransformedOutput(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rVariance)const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetVarianceTransformedOutput] transformation needs to be built first.");
	}
	this->CalculateVariance(this->mSPTransOutput, rVariance);
}

// calculate covariance matrix
void NuTo::SupportPoints::GetCovarianceMatrixOriginal(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCovarianceMatrix) const
{
	this->CalculateCovarianceMatrix(this->mSPOrigInput, this->mSPOrigOutput, rCovarianceMatrix);
}

// calculate covariance matrix
void NuTo::SupportPoints::GetCovarianceMatrixTransformed(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCovarianceMatrix) const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetCovarianceMatrixTransformed] transformation needs to be built first.");
	}
	this->CalculateCovarianceMatrix(this->mSPTransInput, this->mSPTransOutput, rCovarianceMatrix);
}

// calculate Pearson's correlation matrix using original support point coordinates
void NuTo::SupportPoints::GetPearsonCorrelationMatrixOriginal(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix) const
{
	this->CalculatePearsonCorrelationMatrix(this->mSPOrigInput, this->mSPOrigOutput, rCorrelationMatrix);
}

// calculate Pearson's correlation matrix using transformed support point coordinates
void NuTo::SupportPoints::GetPearsonCorrelationMatrixTransformed(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix) const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetPearsonCorrelationMatrixTransformed] transformation needs to be built first.");
	}
	this->CalculatePearsonCorrelationMatrix(this->mSPTransInput, this->mSPTransOutput, rCorrelationMatrix);
}

// calculate the confidence interval on Pearson's correlation coefficient using original support point coordinates
void NuTo::SupportPoints::GetPearsonCorrelationMatrixConfidenceIntervalsOriginal(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMinCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMaxCorrelationMatrix, double rAlpha) const
{
	try
	{
		this->CalculatePearsonCorrelationMatrixConfidenceIntervals(this->mSPOrigInput, this->mSPOrigOutput, rCorrelationMatrix, rMinCorrelationMatrix, rMaxCorrelationMatrix, rAlpha);
	}
	catch(NuTo::Exception& e)
	{
		NuTo::MetamodelException myException(e.ErrorMessage());
		myException.AddMessage("[NuTo::SupportPoints::GetPearsonCorrelationMatrixConfidenceIntervalsOriginal] error calculating confidence intervals on Person's correlation coefficient.");
		throw myException;
	}

}

// calculate the confidence interval on Pearson's correlation coefficient using transformed support point coordinates
void NuTo::SupportPoints::GetPearsonCorrelationMatrixConfidenceIntervalsTransformed(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMinCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMaxCorrelationMatrix, double rAlpha) const
{
	if(!this->mTransformationBuild)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::GetPearsonCorrelationMatrixConfidenceIntervalsTransformed] transformation needs to be built first.");
	}
	try
	{
		this->CalculatePearsonCorrelationMatrixConfidenceIntervals(this->mSPTransInput, this->mSPTransOutput, rCorrelationMatrix, rMinCorrelationMatrix, rMaxCorrelationMatrix, rAlpha);
	}
	catch(NuTo::Exception& e)
	{
		NuTo::MetamodelException myException(e.ErrorMessage());
		myException.AddMessage("[NuTo::SupportPoints::GetPearsonCorrelationMatrixConfidenceIntervalsTransformed] error calculating confidence intervals on Person's correlation coefficient.");
		throw myException;
	}

}

// calculate mean values
/*!
 * The sample mean is separately calculated for each point coordinate \f$x_{i\ldots}\f$.
 * For the \f$i\f$-th coordinate the sample mean \f$\bar{x}_i\f$ is defined as
 * \f[
 *  \bar{x}_i = \dfrac{1}{n} \sum_{j=1}^n x_{ij}, \qquad i = 1 \ldots p,
 * \f]
 * where \f$p\f$ is the number of point coordinates, \f$n\f$ is the number of samples.
 */
void NuTo::SupportPoints::CalculateMeanValues(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rData, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMean) const
{
	int numRows = rData.GetNumRows();
	int numSamples = rData.GetNumColumns();
	if(numSamples < 1)
	{
		throw MetamodelException("[NuTo::SupportPoints::CalculateMeanValues] number of samples must be larger than zero.");
	}
	double factor = 1.0/static_cast<double>(numSamples);
	rMean.Resize(numRows,1);

	// calculate mean value
	for(int rowCount = 0; rowCount < numRows; rowCount++)
	{
		double mean=0.0;
	    const double *dataPtr = &rData.data()[rowCount];
	    for (int sample=0; sample<numSamples; sample++)
		{
	        mean += factor * (*dataPtr);
	        dataPtr+=numRows;
		}
	    rMean.SetValue(rowCount,0,mean);
	}
}

// calculate variances
/*!
 * The sample variance is separately calculated for each point coordinate \f$x_{i\ldots}\f$.
 * For the \f$i\f$-th coordinate the sample variance \f$\sigma_{{x}_i}^2\f$ is defined as
 * \f[
 *  \sigma_{{x}_i}^2 = \dfrac{1}{n - 1} \sum_{j=1}^n \left[x_{ij} - \bar{x}_i\right]^2, \qquad i = 1 \ldots p,
 * \f]
 * where \f$p\f$ is the number of point coordinates, \f$n\f$ is the number of samples, and \f$\bar{x}_i\f$ is the sample mean of the
 * corresponding coordinate.
 */
void NuTo::SupportPoints::CalculateVariance(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rData, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rVariance) const
{
	int numRows = rData.GetNumRows();
	int numSamples = rData.GetNumColumns();
	if(numSamples < 2)
	{
		throw MetamodelException("[NuTo::SupportPoints::CalculateVariance] number of samples must be larger than one.");
	}
	rVariance.Resize(numRows,1);
	double factor = 1.0/static_cast<double>(numSamples-1);

	// calculate mean values
	FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> meanVector;
	this->CalculateMeanValues(rData, meanVector);

	// calculate variance
	for(int rowCount = 0; rowCount < numRows; rowCount++)
	{
		double variance=0.0;
		double mean=meanVector.GetValue(rowCount,0);
	    const double *dataPtr = &rData.data()[rowCount];
	    for (int sample=0; sample<numSamples; sample++)
		{
	    	double delta = *dataPtr - mean;
	        variance += factor * delta * delta;
	        dataPtr+=numRows;
		}
	    rVariance.SetValue(rowCount,0,variance);
	}
}

// calculate covariance matrix
/*!
 * The sample covariance matrix \f$\boldsymbol{COV}\f$ of the support points is defined as
 * \f[
 *  \boldsymbol{COV} = \begin{bmatrix}
 *    cov(x_1,x_1) & cov(x_1, x_2) & \cdots & cov(x_1, x_{p_i}) & cov(x_1, y_1) & cov(x_1, y_2) & \cdots & cov(x_1, y_{p_o})\\
 *    cov(x_2,x_1) & cov(x_2, x_2) & \cdots & cov(x_2, x_{p_i}) & cov(x_2, y_1) & cov(x_2, y_2) & \cdots & cov(x_2, y_{p_o})\\
 *    \vdots & \vdots & \ddots & \vdots & \vdots & \vdots & \ddots & \vdots\\
 *    cov(x_{p_i},x_1) & cov(x_{p_i}, x_2) & \cdots & cov(x_{p_i}, x_{p_i}) & cov(x_{p_i}, y_1) & cov(x_{p_i}, y_2) & \cdots & cov(x_{p_i}, y_{p_o})\\
 *    cov(y_1,x_1) & cov(y_1, x_2) & \cdots & cov(y_1, x_{p_i}) & cov(y_1, y_1) & cov(y_1, y_2) & \cdots & cov(y_1, y_{p_o})\\
 *    cov(y_2,x_1) & cov(y_2, x_2) & \cdots & cov(y_2, x_{p_i}) & cov(y_2, y_1) & cov(y_2, y_2) & \cdots & cov(y_2, y_{p_o})\\
 *    \vdots & \vdots & \ddots & \vdots & \vdots & \vdots & \ddots & \vdots\\
 *    cov(y_{p_o},x_1) & cov(y_{p_o}, x_2) & \cdots & cov(y_{p_o}, x_{p_i}) & cov(y_{p_o}, y_1) & cov(y_{p_o}, y_2) & \cdots & cov(y_{p_o}, y_{p_o})
 *  \end{bmatrix},
 * \f]
 * with
 * \f[
 *   cov(z_i,z_j) = \dfrac{1}{n-1} \sum_{k=1}^n (z_{ik} - \bar{z}_i)(z_{jk} - \bar{z}_j),
 * \f]
 * where \f$\bar{z}_i\f$ is the sample mean of the \f$i\f$-th coordinate, \f$n\f$ is the number of samples, \f$p_i\f$ is the number of input coordinates,
 * and \f$p_o\f$ is the number of output coordinates. Since \f$cov(z_i,z_j) = cov(z_j,z_i)\f$ the covariance matrix is symmetric.
 */
void NuTo::SupportPoints::CalculateCovarianceMatrix(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rInputData, const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rOutputData, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCovarianceMatrix) const
{
	// get data
	int numInputData = rInputData.GetNumRows();
	int numOutputData = rOutputData.GetNumRows();
	int numSamples = rInputData.GetNumColumns();
	if(numSamples != rOutputData.GetNumColumns())
	{
		throw MetamodelException("[NuTo::SupportPoints::CalculateCovarianceMatrix] number of samples in input data and number of samples in output data must be equal.");
	}
	if(numSamples < 2)
	{
		throw MetamodelException("[NuTo::SupportPoints::CalculateCovarianceMatrix] number of samples must be larger than one.");
	}

	// calculate mean values
	FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> inputDataMeanVector;
	this->CalculateMeanValues(rInputData, inputDataMeanVector);
	FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> outputDataMeanVector;
	this->CalculateMeanValues(rOutputData, outputDataMeanVector);

	// calculate covariance matrix
	rCovarianceMatrix.Resize(numInputData+numOutputData,numInputData+numOutputData);
	for(int row = 0; row < numInputData; row++)
	{
		double meanRow = inputDataMeanVector.GetValue(row,0);
		// covariance input - input
		for(int column = row; column < numInputData; column++)
		{
			double cov = 0;
			double meanColumn = inputDataMeanVector.GetValue(column,0);
			const double *dataRowPtr = &rInputData.data()[row];
			const double *dataColumnPtr = &rInputData.data()[column];
			for(int sample = 0; sample < numSamples; sample++)
			{
				cov += (*dataRowPtr - meanRow) * (*dataColumnPtr - meanColumn);
				dataRowPtr += numInputData;
				dataColumnPtr += numInputData;
			}
			cov /= static_cast<double>(numSamples-1);
			rCovarianceMatrix.SetValue(row,column, cov);
			if(row != column)
			{
				rCovarianceMatrix.SetValue(column, row, cov);
			}
		}
		// covariance input - output / output - input
		for(int column = numInputData; column < numInputData + numOutputData; column++)
		{
			double cov = 0;
			double meanColumn = outputDataMeanVector.GetValue(column - numInputData,0);
			const double *dataRowPtr = &rInputData.data()[row];
			const double *dataColumnPtr = &rOutputData.data()[column - numInputData];
			for(int sample = 0; sample < numSamples; sample++)
			{
				cov += (*dataRowPtr - meanRow) * (*dataColumnPtr - meanColumn);
				dataRowPtr += numInputData;
				dataColumnPtr += numOutputData;
			}
			cov /= static_cast<double>(numSamples-1);
			rCovarianceMatrix.SetValue(row,column, cov);
			rCovarianceMatrix.SetValue(column, row, cov);
		}
	}
	// covariance output - output
	for(int row = numInputData; row < numInputData + numOutputData; row++)
	{
		double meanRow = outputDataMeanVector.GetValue(row - numInputData,0);
		for(int column = row; column < numInputData + numOutputData; column++)
		{
			double cov = 0;
			double meanColumn = outputDataMeanVector.GetValue(column - numInputData,0);
			const double *dataRowPtr = &rOutputData.data()[row - numInputData];
			const double *dataColumnPtr = &rOutputData.data()[column - numInputData];
			for(int sample = 0; sample < numSamples; sample++)
			{
				cov += (*dataRowPtr - meanRow) * (*dataColumnPtr - meanColumn);
				dataRowPtr += numOutputData;
				dataColumnPtr += numOutputData;
			}
			cov /= static_cast<double>(numSamples-1);
			rCovarianceMatrix.SetValue(row,column, cov);
			if(row != column)
			{
				rCovarianceMatrix.SetValue(column, row, cov);
			}
		}
	}
}

// calculate Pearson's sample correlation matrix
/*!
 * Pearson's sample correlation matrix \f$\boldsymbol{R}\f$ is defined as
 * \f[
 *  \boldsymbol{R} = \begin{bmatrix}
 *    \rho_{x_1,x_1} & \rho_{x_1, x_2} & \cdots & \rho_{x_1, x_{p_i}} & \rho_{x_1, y_1} & \rho_{x_1, y_2} & \cdots & \rho_{x_1, y_{p_o}}\\
 *    \rho_{x_2,x_1} & \rho_{x_2, x_2} & \cdots & \rho_{x_2, x_{p_i}} & \rho_{x_2, y_1} & \rho_{x_2, y_2} & \cdots & \rho_{x_2, y_{p_o}}\\
 *    \vdots & \vdots & \ddots & \vdots & \vdots & \vdots & \ddots & \vdots\\
 *    \rho_{x_{p_i},x_1} & \rho_{x_{p_i}, x_2} & \cdots & \rho_{x_{p_i}, x_{p_i}} & \rho_{x_{p_i}, y_1} & \rho_{x_{p_i}, y_2} & \cdots & \rho_{x_{p_i}, y_{p_o}}\\
 *    \rho_{y_1,x_1} & \rho_{y_1, x_2} & \cdots & \rho_{y_1, x_{p_i}} & \rho_{y_1, y_1} & \rho_{y_1, y_2} & \cdots & \rho_{y_1, y_{p_o}}\\
 *    \rho_{y_2,x_1} & \rho_{y_2, x_2} & \cdots & \rho_{y_2, x_{p_i}} & \rho_{y_2, y_1} & \rho_{y_2, y_2} & \cdots & \rho_{y_2, y_{p_o}}\\
 *    \vdots & \vdots & \ddots & \vdots & \vdots & \vdots & \ddots & \vdots\\
 *    \rho_{y_{p_o},x_1} & \rho_{y_{p_o}, x_2} & \cdots & \rho_{y_{p_o}, x_{p_i}} & \rho_{y_{p_o}, y_1} & \rho_{y_{p_o}, y_2} & \cdots & \rho_{y_{p_o}, y_{p_o}}
 *  \end{bmatrix}
 * \f]
 * with the sample correlation coefficient \f$\rho_{z_i,z_j}\f$ given by
 * \f[
 * \rho_{z_i,z_j} = \dfrac{cov(z_i,z_j)}{\sigma_{z_i} \sigma_{z_j}} = \dfrac{1}{n-1} \sum_{k=1}^n \left[\dfrac{z_{ik} - \bar{z}_i}{\sigma_{z_i}} \right]\left[\dfrac{z_{jk} - \bar{z}_j}{\sigma_{z_j}} \right],
 * \f]
 * where \f$cov(z_i,z_j)\f$ is the covariance of the variables \f$z_i\f$ and \f$z_j\f$, \f$\sigma_{z_i}, \sigma_{z_j}\f$ are the corresponding sample standard deviations,
 * \f$\bar{z}_i, \bar{z}_j\f$ are the corresponding sample means, \f$n\f$ is the number of samples, \f$p_i\f$ is the number of input coordinates,
 * and \f$p_o\f$ is the number of output coordinates.
 */
void NuTo::SupportPoints::CalculatePearsonCorrelationMatrix(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rInputData, const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rOutputData, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix) const
{
	// calculate covariance matrix
	this->CalculateCovarianceMatrix(rInputData, rOutputData, rCorrelationMatrix);

	// calculate standard deviation of diagonal values
	int dim = rCorrelationMatrix.GetNumRows();
	std::vector<double> stddev(dim);
	assert(dim == rCorrelationMatrix.GetNumColumns());
	assert(dim == rInputData.GetNumRows() + rOutputData.GetNumRows());
	for(int dimCount = 0; dimCount < dim; dimCount++)
	{
		assert(rCorrelationMatrix.GetValue(dimCount,dimCount) > 0.0);
		stddev[dimCount] = sqrt(rCorrelationMatrix.GetValue(dimCount,dimCount));
	}

	// scale covariance matrix
	for(int row = 0; row < dim; row++)
	{
		for(int column = 0; column < dim; column++)
		{
			rCorrelationMatrix.SetValue(row,column, rCorrelationMatrix.GetValue(row,column)/(stddev[row]*stddev[column]));
		}
	}
}

// calculate the confidence interval on the coefficients of Pearson's correlation matrix
/*!
 * The (1-\f$\alpha\f$) confidence interval on Pearson's correlation coefficient \f$\rho\f$ is defined as
 * \f[
 *  \tanh \left( \text{arctanh} (\rho) - \dfrac{Z_{0.5\alpha}}{\sqrt{n-3}} \right) \leq \rho \leq \tanh \left( \text{arctanh} (\rho) + \dfrac{Z_{0.5\alpha}}{\sqrt{n-3}} \right),
 * \f]
 * where \f$Z_{0.5\alpha}\f$ is the \f$(1-0.5\alpha)\f$-quantil of the standard normal distribution, and \f$n\f$ is the number of support points.
 */
void NuTo::SupportPoints::CalculatePearsonCorrelationMatrixConfidenceIntervals(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rInputData, const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rOutputData, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMinCorrelationMatrix, FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rMaxCorrelationMatrix, double rAlpha ) const
{
	// check number of samples
	if(this->GetNumSupportPoints() < 4)
	{
		throw NuTo::MetamodelException("[NuTo::SupportPoints::CalculatePearsonCorrelationMatrixConfidenceIntervals] at least 4 support points are required for the calculation of the confidence interval on correlation coefficients.");
	}

	try
	{
		// calculate Pearson's correlation matrix
		this->CalculatePearsonCorrelationMatrix(rInputData, rOutputData, rCorrelationMatrix);

		// calculate the (1-0.5 * rAlpha)-quantile of the standard normal distribution
		boost::math::normal stdNormalDistribution;
		double quantile = boost::math::quantile(boost::math::complement(stdNormalDistribution,0.5 * rAlpha));
		double delta = quantile/sqrt(static_cast<double>(this->GetNumSupportPoints() - 3));

		// copy correlation matrix
		rMinCorrelationMatrix = rCorrelationMatrix;
		rMaxCorrelationMatrix = rCorrelationMatrix;

		// calculate confidence interval
		for(int row = 0; row < this->GetDimInput() + this->GetDimOutput(); row++)
		{
			for(int col = row + 1; col < this->GetDimInput() + this->GetDimOutput(); col++)
			{
				rMinCorrelationMatrix(row,col) = tanh(atanh(rCorrelationMatrix(row,col)) - delta);
				rMinCorrelationMatrix(col,row) = rMinCorrelationMatrix(row,col);
				rMaxCorrelationMatrix(row,col) = tanh(atanh(rCorrelationMatrix(row,col)) + delta);
				rMaxCorrelationMatrix(col,row) = rMaxCorrelationMatrix(row,col);
			}
		}
	}
	catch(NuTo::Exception& e)
	{
		NuTo::MetamodelException myException(e.ErrorMessage());
		myException.AddMessage("[NuTo::SupportPoints::CalculatePearsonCorrelationMatrixConfidenceIntervals] error calculating confidence intervals on correlation coefficients.");
		throw myException;
	}
}


#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::SupportPoints::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::SupportPoints::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::SupportPoints::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::SupportPoints::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::SupportPoints::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::SupportPoints::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::SupportPoints::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize SupportPoints" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_NVP(mSPOrigInput)
       & BOOST_SERIALIZATION_NVP(mSPOrigOutput)
       & BOOST_SERIALIZATION_NVP(mSPTransInput)
       & BOOST_SERIALIZATION_NVP(mSPTransOutput)
       & BOOST_SERIALIZATION_NVP(mWeight)
       & BOOST_SERIALIZATION_NVP(mlTransformationInput)
       & BOOST_SERIALIZATION_NVP(mlTransformationOutput)
       & BOOST_SERIALIZATION_NVP(mTransformationBuild);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize SupportPoints" << std::endl;
#endif
}
#endif // ENABLE_SERIALIZATION
