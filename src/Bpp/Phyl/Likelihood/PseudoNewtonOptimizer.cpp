//
// File: PseudoNewtonOptimizer.cpp
// Created by: Julien Dutheil
// Created on: Tue Nov 16 12:33 2004
//

/*
Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

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

/**************************************************************************/

#include "../TreeTemplateTools.h"
#include "PseudoNewtonOptimizer.h"
#include "DRHomogeneousTreeLikelihood.h"

#include <Bpp/Numeric/VectorTools.h>
#include <Bpp/Text/TextTools.h>
#include <Bpp/App/ApplicationTools.h>

using namespace bpp;

/**************************************************************************/
           
bool PseudoNewtonOptimizer::PNStopCondition::isToleranceReached() const
{
  return NumTools::abs<double>(
      dynamic_cast<const PseudoNewtonOptimizer*>(optimizer_)->currentValue_ -
      dynamic_cast<const PseudoNewtonOptimizer *>(optimizer_)->previousValue_) < tolerance_; 
}
   
/**************************************************************************/
  
PseudoNewtonOptimizer::PseudoNewtonOptimizer(DerivableSecondOrder* function) :
  AbstractOptimizer(function),
  previousPoint_(),
  previousValue_(0),
  n_(0),
  params_(),
  maxCorrection_(10)
{
  setDefaultStopCondition_(new FunctionStopCondition(this));
  setStopCondition(*getDefaultStopCondition());
}

/**************************************************************************/

void PseudoNewtonOptimizer::doInit(const ParameterList& params) throw (Exception)
{
  n_ = getParameters().size();
  params_ = getParameters().getParameterNames();
  getFunction()->enableSecondOrderDerivatives(true);
  getFunction()->setParameters(getParameters());
}

/**************************************************************************/

double PseudoNewtonOptimizer::doStep() throw (Exception)
{
  ParameterList* bckPoint = 0;
  if (updateParameters()) bckPoint = new ParameterList(getFunction()->getParameters());
  double newValue = 0;
  // Compute derivative at current point:
  std::vector<double> movements(n_);
  ParameterList newPoint = getParameters();
  for (unsigned int i = 0; i < n_; i++)
  {
    double  firstOrderDerivative = getFunction()->getFirstOrderDerivative(params_[i]);
    double secondOrderDerivative = getFunction()->getSecondOrderDerivative(params_[i]);
    if (secondOrderDerivative == 0)
    {
      movements[i] = 0;
    }
    else if (secondOrderDerivative < 0)
    {
      printMessage("!!! Second order derivative is negative for parameter " + params_[i] + "(" + TextTools::toString(getParameters()[i].getValue()) + "). No move performed.");
      //movements[i] = 0;  // We want to reach a minimum, not a maximum!
      // My personnal improvement:
      movements[i] = -firstOrderDerivative / secondOrderDerivative;
    }
    else movements[i] = firstOrderDerivative / secondOrderDerivative;
    if (std::isnan(movements[i]))
    {
      printMessage("!!! Non derivable point at " + params_[i] + ". No move performed. (f=" + TextTools::toString(currentValue_) + ", d1=" + TextTools::toString(firstOrderDerivative) + ", d2=" + TextTools::toString(secondOrderDerivative) + ").");
      movements[i] = 0; // Either first or second order derivative is infinity. This may happen when the function == inf at this point.
    }
    //DEBUG:
    //cout << "PN[" << i << "]=" << _parameters.getParameter(params_[i])->getValue() << "\t" << movements[i] << "\t " << firstOrderDerivative << "\t" << secondOrderDerivative << endl;
    newPoint[i].setValue(getParameters()[i].getValue() - movements[i]);
  }
  newValue = getFunction()->f(newPoint);

  // Check newValue:
  unsigned int count = 0;
  while (newValue > currentValue_)
  {
    //Restore previous point (all parameters in case of global constraint):
    if (updateParameters()) getFunction()->setParameters(*bckPoint);

    count++;
    if (count >= maxCorrection_)
    {
      printMessage("!!! Felsenstein-Churchill correction applied too much time. Stopping here. Convergence probably not reached.");
      tolIsReached_ = true;
      if (!updateParameters()) getFunction()->setParameters(getParameters());
      return currentValue_;
      //throw Exception("PseudoNewtonOptimizer::step(). Felsenstein-Churchill correction applied more than 10 times.");
    }

    printMessage("!!! Function at new point is greater than at current point: " + TextTools::toString(newValue) + ">" + TextTools::toString(currentValue_) + ". Applying Felsenstein-Churchill correction.");
    if (getMessageHandler())
      getParameters().printParameters(*getMessageHandler());
    for (unsigned int i = 0; i < movements.size(); i++)
    {
      movements[i] = movements[i] / 2;
      newPoint[i].setValue(getParameters()[i].getValue() - movements[i]);
    }
    newValue = getFunction()->f(newPoint);
  }
  
  previousPoint_ = getParameters();
  previousValue_ = currentValue_;
  getParameters_() = newPoint;

  if (updateParameters()) delete bckPoint;
  return newValue;
}

/**************************************************************************/
