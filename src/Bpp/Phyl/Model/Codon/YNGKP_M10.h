//
// File: YNGKP_M10.h
// Created by: Laurent Gueguen
// Created on: May 2010
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

#ifndef _YNGKP_M10_H_
#define _YNGKP_M10_H_

#include "../AbstractBiblioMixedSubstitutionModel.h"
#include "../MixtureOfASubstitutionModel.h"
#include "../FrequenciesSet/CodonFrequenciesSet.h"

#include <Bpp/Seq/GeneticCode/GeneticCode.h>

namespace bpp
{

/**
 * @brief The Yang et al (2000) M10 substitution model for codons.
 * @author Laurent Guéguen
 *
 * This model is a mixture of models as described in YN98 class, the
 * mixture being defined on the selection parameter oomega to allow it
 * to vary among sites, following a mixture of a Beta distribution and
 * of a Gamma distribution.
 *
 * This model includes 5 parameters (@f$\kappa@f$, @f$ p @f$ and
 * @f$q@f$ of the @f$ Beta(p,q) @f$ distribution, @f$ \alpha @f$ and
 * @f$\beta@f$ of the @f$ 1 + Gamma(\alpha,\beta) @f$
 * distribution,@f$p0@f$ the weight of the Beta distribution. The
 * codon frequencies @f$ \pi_j @f$ are either observed or infered.
 *
 * References:
 *
 * Yang, Z., R. Nielsen, N. Goldman, and A.-M. K. Pedersen (2000)
 * Genetics 155:431-449.
 * 
 */
class YNGKP_M10:
    public AbstractBiblioMixedSubstitutionModel,
    virtual public ReversibleSubstitutionModel
{
private:
  std::auto_ptr<MixtureOfASubstitutionModel> pmixmodel_;

  /**
   * @brief indexes of 2 codons between which the substitution is
   * synonymous, to set a basis to the homogeneization of the rates.
   */
  size_t synfrom_, synto_;

  unsigned int nBeta_, nGamma_;
  
public:
  /*
   *@brief Constructor that requires the number of classes of the
   * BetaDiscreteDistribution and the GammaDiscreteDistribution.
   *
   */
  
  YNGKP_M10(const GeneticCode* gc, FrequenciesSet* codonFreqs, unsigned int nbBeta, unsigned int nbGamma);

  ~YNGKP_M10();
  
  YNGKP_M10* clone() const { return new YNGKP_M10(*this); }

  YNGKP_M10(const YNGKP_M10&);

  YNGKP_M10& operator=(const YNGKP_M10&);

protected:
  void updateMatrices();

public:
  const SubstitutionModel& getModel() const { return *pmixmodel_.get(); }

  const MixedSubstitutionModel& getMixedModel() const { return *pmixmodel_.get(); }

  std::string getName() const { return "YNGKP_M10"; }

  unsigned int getNBeta() const 
  {
    return nBeta_;
  }
  
  unsigned int getNGamma() const 
  {
    return nGamma_;
  }
  
private:
  SubstitutionModel& getModel() { return *pmixmodel_.get(); }

  MixedSubstitutionModel& getMixedModel() { return *pmixmodel_.get(); }

  const FrequenciesSet* getFrequenciesSet() const {return pmixmodel_->getNModel(1)->getFrequenciesSet();}
};

} //end of namespace bpp.

#endif	//_YNGKP_M10_H_

