//
// File: AbstractSubstitutionModel.cpp
// Created by: Julien Dutheil
// Created on: Tue May 27 10:31:49 2003
//

/*
   Copyright or © or Copr. CNRS, (November 16, 2004)

   This software is a computer program whose purpose is to provide classes
   for phylogenetic data analysis.

   This software is governed by the CeCILL  license under French law and
   abiding by the rules of distribution of free software.  You can  use,
   modify and/ or redistribute the software under the terms of the CeCILL
   license as circulated by CEA, CNRS and INRIA at the following URL
   "http://www.cecill.info".

   As a counterpart to the access to the source code and  rights to copy,
   modify and redistribute granted by the license, users are provided only
   with a limited warranty  and the software's author,  the holder of the
   economic rights,  and the successive licensors  have only  limited
   liability.

   In this respect, the user's attention is drawn to the risks associated
   with loading,  using,  modifying and/or developing or reproducing the
   software by the user in light of its specific status of free software,
   that may mean  that it is complicated to manipulate,  and  that  also
   therefore means  that it is reserved for developers  and  experienced
   professionals having in-depth computer knowledge. Users are therefore
   encouraged to load and test the software's suitability as regards their
   requirements in conditions enabling the security of their systems and/or
   data to be ensured and,  more generally, to use and operate it in the
   same conditions as regards security.

   The fact that you are presently reading this means that you have had
   knowledge of the CeCILL license and that you accept its terms.
 */

#include "AbstractSubstitutionModel.h"

#include <Bpp/Text/TextTools.h>
#include <Bpp/Numeric/VectorTools.h>
#include <Bpp/Numeric/Matrix/MatrixTools.h>
#include <Bpp/Numeric/Matrix/EigenValue.h>

// From SeqLib:
#include <Bpp/Seq/Container/SequenceContainerTools.h>

using namespace bpp;
using namespace std;

/******************************************************************************/

AbstractSubstitutionModel::AbstractSubstitutionModel(const Alphabet* alpha, const std::string& prefix) :
  AbstractParameterAliasable(prefix),
  alphabet_(alpha),
  size_(alpha->getSize()),
  rate_(1),
  chars_(size_),
  generator_(size_, size_),
  pijt_(size_, size_),
  dpijt_(size_, size_),
  d2pijt_(size_, size_),
  leftEigenVectors_(size_, size_),
  rightEigenVectors_(size_, size_),
  eigenValues_(size_),
  freq_(size_),
  eigenDecompose_(true)
{
  for (unsigned int i = 0; i < size_; i++)
  {
    freq_[i]=1.0/size_;
    chars_[i] = static_cast<int>(i);
  }

}

/******************************************************************************/

void AbstractSubstitutionModel::updateMatrices()
{
  // Compute eigen values and vectors:
  EigenValue<double> ev(generator_);
  rightEigenVectors_ = ev.getV();
  MatrixTools::inv(rightEigenVectors_, leftEigenVectors_);
  eigenValues_ = ev.getRealEigenValues();
}

/******************************************************************************/

const Matrix<double>& AbstractSubstitutionModel::getPij_t(double t) const
{
  if (t == 0)
  {
    MatrixTools::getId(size_, pijt_);
  }
  else
  {
    MatrixTools::mult<double>(rightEigenVectors_, VectorTools::exp(rate_ * eigenValues_ * t), leftEigenVectors_, pijt_);
  }
  return pijt_;
}

const Matrix<double>& AbstractSubstitutionModel::getdPij_dt(double t) const
{
  MatrixTools::mult(rightEigenVectors_, rate_ * eigenValues_ * VectorTools::exp(rate_ * eigenValues_ * t), leftEigenVectors_, dpijt_);
  return dpijt_;
}

const Matrix<double>& AbstractSubstitutionModel::getd2Pij_dt2(double t) const
{
  MatrixTools::mult(rightEigenVectors_, NumTools::sqr(rate_ * eigenValues_) * VectorTools::exp(rate_ * eigenValues_ * t), leftEigenVectors_, d2pijt_);
  return d2pijt_;
}

/******************************************************************************/

double AbstractSubstitutionModel::getInitValue(unsigned int i, int state) const throw (BadIntException)
{
  if (i >= size_) throw BadIntException(i, "AbstractSubstitutionModel::getInitValue");
  if (state < 0 || !alphabet_->isIntInAlphabet(state)) throw BadIntException(state, "AbstractSubstitutionModel::getInitValue. Character " + alphabet_->intToChar(state) + " is not allowed in model.");
  vector<int> states = alphabet_->getAlias(state);
  for (unsigned int j = 0; j < states.size(); j++)
  {
    if (getAlphabetChar(i) == states[j]) return 1.;
  }
  return 0.;
}

/******************************************************************************/

void AbstractSubstitutionModel::setFreqFromData(const SequenceContainer& data, unsigned int pseudoCount)
{
  map<int, int> counts;
  SequenceContainerTools::getCounts(data, counts);
  int t = 0;
  map<int, double> freqs;

  for (unsigned int i = 0; i < size_; i++)
  {
    t += counts[i] + pseudoCount;
  }
  for (unsigned int i = 0; i < size_; i++)
  {
    freqs[i] = ((double)counts[i] + pseudoCount) / t;
  }

  //Re-compute generator and eigen values:
  setFreq(freqs);
}

/******************************************************************************/

void AbstractSubstitutionModel::setFreq(map<int, double>& freqs)
{
  for (unsigned int i = 0; i < size_; i++)
  {
    freq_[i] = freqs[i];
  }
  //Re-compute generator and eigen values:
  updateMatrices();
}

/******************************************************************************/
double AbstractSubstitutionModel::getScale() const
{
  vector<double> _v;
  MatrixTools::diag(generator_, _v);
  return -VectorTools::scalar<double, double>(_v, freq_);
}

/******************************************************************************/
double AbstractSubstitutionModel::getRate() const
{
  return rate_;
}

/******************************************************************************/
void AbstractSubstitutionModel::setRate(double rate)
{
  if (rate<=0)
    throw Exception("Bad value for rate: " + TextTools::toString(rate));

  if (hasParameter("rate"))
    setParameterValue("rate",rate_);

  rate_=rate;
}

void AbstractSubstitutionModel::addRateParameter()
{
  addParameter_(Parameter(getNamespace()+"rate", rate_, &Parameter::R_PLUS_STAR));
}

/******************************************************************************/

void AbstractReversibleSubstitutionModel::updateMatrices()
{
  RowMatrix<double> Pi;
  MatrixTools::diag(freq_, Pi);
  MatrixTools::mult(exchangeability_, Pi, generator_); //Diagonal elements of the exchangability matrix will be ignored.
  // Compute diagonal elements of the generator:
  for (unsigned int i = 0; i < size_; i++)
  {
    double lambda = 0;
    for (unsigned int j = 0; j < size_; j++)
    {
      if (j != i) lambda += generator_(i,j);
    }
    generator_(i,i) = -lambda;
  }
  // Normalization:
  double scale = getScale();
  MatrixTools::scale(generator_, 1. / scale);

  // Normalize exchangeability matrix too:
  MatrixTools::scale(exchangeability_, 1. / scale);
  // Compute diagonal elements of the exchangeability matrix:
  for (unsigned int i = 0; i < size_; i++)
  {
    exchangeability_(i,i) = generator_(i,i) / freq_[i];
  }
  AbstractSubstitutionModel::updateMatrices();
}

/******************************************************************************/
