#pragma once

#ifndef _BAD_SPLINE_
#define _BAD_SPLINE_ 123456789
#endif

/// KS: We store coefficients {y,b,c,d} in one array one by one, this is only to define it once rather then insert "4" all over the code
#define _nCoeff_ 4
/// KS: For TF1 we store at most 5 coefficients, we could make it more flexible but for now define it here to make future changes easier to track
#define _nTF1Coeff_ 2

/// HW: Coefficients for grabbing items from manycoeff_arr (rather than having y=manycoeffarray[index+0])
enum SplineSegmentCoeffs
{
  kCoeffY = 0,
  kCoeffB = 1,
  kCoeffC = 2,
  kCoeffD = 3
};


// *******************
/// @brief KS: Struct storing information for spline monolith
struct SplineMonoStruct {
// *******************
  /// KS: CPU arrays to hold X coefficient
  std::vector<float> coeff_x;

  /// CPU arrays to hold other coefficients
  std::vector<float> coeff_many;

  /// KS: CPU Number of knots per spline
  std::vector<unsigned int> nKnots_arr;

  /// CW: CPU array with the number of points per spline (not per spline point!)
  std::vector<short int> paramNo_arr;
};


// WARNING KS: Please add stuff here with super caution. This header is being added to gpuSplineUtils.cu. Right now we support most of CUDA even super old. If you add some header with fancy templates it will not compile for older CUDA. This header is a way to use common macros or Enum in CPU and GPU code. For more sophisticated structs please use SplineStructs.h