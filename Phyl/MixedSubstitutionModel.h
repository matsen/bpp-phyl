//
// File: MixedSubstitutionModel.h
// Created by: Davud Fournier, Laurent Gueguen
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

#ifndef _MIXEDSUBSTITUTIONMODEL_H_
#define _MIXEDSUBSTITUTIONMODEL_H_

#include "NumCalc/AbstractDiscreteDistribution.h"
#include "NumCalc/ConstantDistribution.h"
#include "NumCalc/SimpleDiscreteDistribution.h"
#include "NumCalc/VectorTools.h"
#include "AbstractSubstitutionModel.h"
#include "PhylogeneticsApplicationTools.h"
#include "NucleotideSubstitutionModel.h"
#include "Seq/alphabets"

#include <vector>
#include <string>
#include <map>
#include <cstring> // C lib for string copy

namespace bpp
{

/**
 * @brief Substitution models defined as a mixture of "simple"
 * substitution models.
 * @author Laurent Guéguen
 *
 * All the models are of the same type (for example T92 or GY94),
 * and their parameter values can follow discrete distributions.
 *
 * In this kind of model, there is no generator or transition
 * probabilities.
 *
 * There is a map with connection from parameter names to discrete
 * distributions, and then a related vector of "simple" substitution
 * models for all the combinations of parameter values.
 *
 * For example:
 * HKY85(kappa=Gamma(n=3,alpha=2,beta=5),
 *       theta=TruncExponential(n=4,lambda=0.2,tp=1),
 *       theta1=0.4,
 *       theta2=TruncExponential(n=5,lambda=0.6,tp=1))
 *
 * defines 60 different HKY85 models.
 *
 * If a distribution parameter does not respect the constraints of
 * this parameter, there is an Exception at the creation of the
 * wrong mdel, if any.
 *
 * When used through a MixedTreeLikelihood objetc, all the models
 * are equiprobable and then the comptuing of the likelihoods and
 * probabilities are the average of the "simple" models values.
 *
 */
class MixedSubstitutionModel :
  public AbstractSubstitutionModel
{
private:
  std::map<std::string, DiscreteDistribution*> distributionMap_;

  std::vector<SubstitutionModel*> modelsContainer_;

public:
  MixedSubstitutionModel(const Alphabet* alpha,
                         SubstitutionModel* model,
                         std::map<std::string, DiscreteDistribution*> parametersDistributionsList);

  MixedSubstitutionModel(const MixedSubstitutionModel&);
  
  ~MixedSubstitutionModel();

  MixedSubstitutionModel* clone() const { return new MixedSubstitutionModel(*this); }

public:
  /**
   * @brief Returns a specific model from the mixture
   */

  SubstitutionModel* getNModel(unsigned int i)
  {
    return modelsContainer_[i];
  }

  unsigned int getNumberOfModels() const
  {
    return modelsContainer_.size();
  }

  std::string getName() const { return "MixedSubstitutionModel"; }

  void updateMatrices();

  /**
   * @brief This function can not be applied here, so it is defined
   * to prevent wrong usage.
   */
  void setFreq(std::map<int,double>&);

  unsigned int getNumberOfStates() const;
};
} //end of namespace bpp.

#endif  //_MIXEDSUBSTITUTIONMODEL_H_