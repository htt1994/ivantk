/*=========================================================================

Image-based Vascular Analysis Toolkit (IVAN)

Copyright (c) 2012, Iván Macía Oliver
Vicomtech Foundation, San Sebastián - Donostia (Spain)
University of the Basque Country, San Sebastián - Donostia (Spain)

All rights reserved

See LICENSE file for license details

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

==========================================================================*/
// File: ivanGaussianDerivativeOperator.h
// Author: Iv�n Mac�a (imacia@vicomtech.org)
// Description: Modified version of itkGaussianDerivativeOperator, with support for gamma-normalized derivatives
// Date: 2010/05/19

#ifndef __ivanGaussianDerivativeOperator_h
#define __ivanGaussianDerivativeOperator_h

#include "itkNeighborhoodOperator.h"
#include "itkGaussianOperator.h"
#include "itkDerivativeOperator.h"

namespace ivan
{
/**
 * \class GaussianDerivativeOperator
 * \brief A NeighborhoodOperator whose coefficients are a one dimensional,
 * discrete derivative Gaussian kernel.
 *
 * GaussianDerivativeOperator can be used to calculate Gaussian derivatives
 * by taking its inner product with to a Neighborhood
 * (NeighborhooIterator) that is swept across an image region.
 * It is a directional operator.  N successive applications
 * oriented along each dimensional direction will calculate separable,
 * efficient, N-D Gaussian derivatives of an image region.
 *
 * GaussianDerivativeOperator takes three parameters:
 *
 * (1) The floating-point variance of the desired Gaussian function.
 *
 * (2) The order of the derivative to be calculated (zero order means
 *     it performs only smoothing as a standard itk::GaussianOperator)
 *
 * (3) The "maximum error" allowed in the discrete Gaussian
 * function.  "Maximum errror" is defined as the difference between the area
 * under the discrete Gaussian curve and the area under the continuous
 * Gaussian. Maximum error affects the Gaussian operator size. Care should
 * be taken not to make this value too small relative to the variance
 * lest the operator size become unreasonably large.
 *
 * References:
 * The Gaussian kernel contained in this operator was described
 * by Tony Lindeberg  (Discrete Scale-Space Theory and the Scale-Space
 * Primal Sketch. Dissertation. Royal Institute of Technology, Stockholm,
 * Sweden. May 1991.).
 *
 * \author Ivan Macia, VICOMTech, Spain, http://www.vicomtech.org
 *
 * This implementation is derived from the Insight Journal paper:
 * http://hdl.handle.net/1926/1290
 *
 * Modifications in IVAN library
 * - Support for gamma-normalized derivatives
 *
 * \sa GaussianOperator
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * \sa Neighborhood
 */
template< class TPixel, unsigned int VDimension = 2,
  class TAllocator = itk::NeighborhoodAllocator< TPixel > >
class ITK_EXPORT GaussianDerivativeOperator :
  public itk::NeighborhoodOperator< TPixel, VDimension, TAllocator >
{
public:
  /** Standard class typedefs. */
  typedef GaussianDerivativeOperator                             Self;
  typedef itk::NeighborhoodOperator< TPixel, VDimension, TAllocator > Superclass;

  /** Neighborhood operator types. */
  typedef itk::GaussianOperator< TPixel, VDimension, TAllocator >   GaussianOperatorType;
  typedef itk::DerivativeOperator< TPixel, VDimension, TAllocator > DerivativeOperatorType;

  /** Constructor. */
  GaussianDerivativeOperator();

  /** Copy constructor */
  GaussianDerivativeOperator(const Self & other);

  /** Assignment operator */
  Self & operator=(const Self & other);


  /** Set/Get the flag for calculating scale-space normalized
   * derivatives.
   *
   * Normalized derivatives are obtained multiplying by the scale
   * parameter $t^1/order$. This use useful for scale-space selection
   * algorithms such as blob detection. The scaling results in the
   * value of the derivatives being independent of the size of an
   * object. */
  void SetNormalizeAcrossScale(bool flag) { m_NormalizeAcrossScale = flag; }
  bool GetNormalizeAcrossScale() const { return m_NormalizeAcrossScale; }
  itkBooleanMacro(NormalizeAcrossScale);
  
  /** Set/Get the normalization factor for derivatives. */
  void SetGamma(const double gamma) { m_Gamma = gamma; }
  double GetGamma() const { return m_Gamma; }

  /** Set/Get the variance of the Gaussian kernel.
   *
   */
  void SetVariance(const double variance) { m_Variance = variance; }
  double GetVariance() const { return m_Variance; }

  /** Set/Get the spacing for the direction of this kernel. */
  void SetSpacing(const double spacing) { m_Spacing = spacing; }
  double GetSpacing() const { return m_Spacing; }

  /** Set/Get the desired maximum error of the gaussian approximation.  Maximum
   * error is the difference between the area under the discrete Gaussian curve
   * and the area under the continuous Gaussian. Maximum error affects the
   * Gaussian operator size. The value is clamped between 0.00001 and 0.99999. */
  void SetMaximumError(const double maxerror)
  {
    const double Min = 0.00001;
    const double Max = 1.0 - Min;

    m_MaximumError = std::max( Min, std::min( Max, maxerror ) );
  }
  double GetMaximumError() { return m_MaximumError; }

  /** Sets/Get a limit for growth of the kernel.  Small maximum error values with
   *  large variances will yield very large kernel sizes.  This value can be
   *  used to truncate a kernel in such instances.  A warning will be given on
   *  truncation of the kernel. */
  void SetMaximumKernelWidth(unsigned int n)
  {
    m_MaximumKernelWidth = n;
  }

  /** Sets/Get the order of the derivative. */
  void SetOrder(const unsigned int order) { m_Order = order;}
  unsigned int GetOrder() const { return m_Order; }

  /** Prints member variables */
  virtual void PrintSelf(std::ostream & os, itk::Indent i) const;

protected:

  typedef typename Superclass::CoefficientVector CoefficientVector;

  /** Returns the value of the modified Bessel function I0(x) at a point x >= 0.
    */
  static double ModifiedBesselI0(double);

  /** Returns the value of the modified Bessel function I1(x) at a point x,
   * x real.  */
  static double ModifiedBesselI1(double);

  /** Returns the value of the modified Bessel function Ik(x) at a point x>=0,
   * where k>=2. */
  static double ModifiedBesselI(int, double);

  /** Calculates operator coefficients. */
  CoefficientVector GenerateCoefficients();

  /** Arranges coefficients spatially in the memory buffer. */
  void Fill(const CoefficientVector & coeff)
  { this->FillCenteredDirectional(coeff); }
private:

  /* methods for generations of the coeeficients for a gaussian
   * operator of 0-order respecting the remaining parameters */
  CoefficientVector GenerateGaussianCoefficients() const;

  /** For compatibility with itkWarningMacro */
  const char * GetNameOfClass() const
  {
    return "itkGaussianDerivativeOperator";
  }
  
private:

  /** Normalize derivatives across scale space */
  bool           m_NormalizeAcrossScale;
  
  /** Gamma normalization factor for derivatives (typically between 0.0 and 1.0). */
  double         m_Gamma;

  /** Desired variance of the discrete Gaussian function. */
  double         m_Variance;

  /** Difference between the areas under the curves of the continuous and
   * discrete Gaussian functions. */
  double         m_MaximumError;

  /** Maximum kernel size allowed.  This value is used to truncate a kernel
   *  that has grown too large.  A warning is given when the specified maximum
   *  error causes the kernel to exceed this size. */
  unsigned int   m_MaximumKernelWidth;

  /** Order of the derivative. */
  unsigned int   m_Order;

  /** Spacing in the direction of this kernel. */
  double         m_Spacing;
};

} // end namespace ivan

#if 0 //HACK: Not yet implemented
// Define instantiation macro for this template.
#define ITK_TEMPLATE_GaussianDerivativeOperator(_, EXPORT, TypeX, TypeY)     \
  namespace itk                                                              \
  {                                                                          \
  _( 2 ( class EXPORT GaussianDerivativeOperator< ITK_TEMPLATE_2 TypeX > ) ) \
  namespace Templates                                                        \
  {                                                                          \
  typedef GaussianDerivativeOperator< ITK_TEMPLATE_2 TypeX >                 \
  GaussianDerivativeOperator##TypeY;                                       \
  }                                                                          \
  }

#if ITK_TEMPLATE_EXPLICIT
#include "Templates/ivanGaussianDerivativeOperator+-.h"
#endif
#endif

#if ITK_TEMPLATE_TXX
#include "ivanGaussianDerivativeOperator.hxx"
#endif

#endif
