#pragma once

//C++ includes
#include <list>

//ROOT includes
#include "THStack.h"
#include "TLegend.h"

//MaCh3 includes
#include "Oscillator/OscillatorBase.h"

#include "splines/splineFDBase.h"

#include "covariance/covarianceXsec.h"
#include "covariance/covarianceOsc.h"

#include "samplePDF/samplePDFBase.h"
#include "samplePDF/FDMCStruct.h"
#include "samplePDF/ShiftFunctors.h"

/// @brief Class responsible for handling implementation of samples used in analysis, reweighting and returning LLH
class samplePDFFDBase :  public samplePDFBase
{
public:
  //######################################### Functions #########################################

  samplePDFFDBase(){};
  samplePDFFDBase(double pot, std::string mc_version, covarianceXsec* xsec_cov);
  virtual ~samplePDFFDBase();

  const int GetNDim(){return nDimensions;} //DB Function to differentiate 1D or 2D binning
  std::string GetName(){return samplename;}
  inline int GetBinningOpt(){return BinningOpt;}

  //===============================================================================
  // DB Reweighting and Likelihood functions

  //ETA - abstract these to samplePDFFDBase
  //DB Require these four functions to allow conversion from TH1(2)D to array for multi-threaded GetLikelihood
  void addData(TH1D* Data);
  void addData(TH2D* Data);
  void addData(std::vector<double> &data);
  void addData(std::vector< std::vector <double> > &data);
  /// @brief DB Multi-threaded GetLikelihood
  double GetLikelihood();
  //===============================================================================

  void reweight();
  double GetEventWeight(int iSample, int iEntry);

  // Setup and config functions
  void UseNonDoubledAngles(bool ans) {doubled_angle = ans;};
  void UseBinnedOscReweighting(bool ans);
  void UseBinnedOscReweighting(bool ans, int nbins, double *osc_bins);
  
  const double **oscpars;
  void SetXsecCov(covarianceXsec* xsec_cov);
  void SetOscCov(covarianceOsc* osc_cov);
  ///  @brief including Dan's magic NuOscillator
  void SetupNuOscillator(std::string OscillatorYaml);

  /// @deprecated The `DumpWeights` function is deprecated and should not be used.
  /// It was kept for backwards compatibility in compiling but has no effect.
  ///
  /// @note This function was marked for deprecation as of 14/01/2015 by KD.
  ///       - DB (27/08/2020): The function is incredibly hardcoded.
  ///       - DB Consider using 'LetsPrintSomeWeights' to achieve the same functionality.
  ///
  /// @param outname The name of the output file.
  virtual void DumpWeights(std::string outname) {(void)outname; return; };
  //================================================================================

  virtual void setupSplines(fdmc_base *skobj, const char *splineFile, int nutype, int signal){};
  void FindEventOscBin();

 protected:
  /// @todo - I think this will be tricky to abstract. fdmc_base will have to contain the pointers to the appropriate weights, can probably pass the number of these weights to constructor?
  /// @brief DB Function to determine which weights apply to which types of samples pure virtual!!
  virtual void SetupWeightPointers() = 0;
  
  splineFDBase *splineFile;
  //===============================================================================
  //DB Functions relating to sample and exec setup  
  //ETA - abstracting these core functions
  //init will setup all the specific variables 
  //void init(double pot, std::string mc_version, covarianceXsec *xsec_cov){return;};
  //void setupMC(manager* sample_manager, const char *sampleInputFile, const char *splineFile, fdmc_base *fdobj, double pot, int nutype, int oscnutype, bool signal, int iSample, bool hasfloats=false){std::cout << "SAMPLEPDFFDBase::setupMC " << std::endl; return;};
  //virtual void setupSplines(fdmc_base *skobj, const char *splineFile, int nutype, int signal);

  void fillSplineBins();

  //Functions which find the nominal bin and bin edges
  void FindNominalBinAndEdges1D();
  void FindNominalBinAndEdges2D();

  //DB Overrided functions of base class which calculate erec bin and boundaries for reweighting speedup in beam samples
  //ETA - this can be done using the core info stored in the new fdmc_struct
  void set1DBinning(int nbins, double* boundaries);
  void set1DBinning(int nbins, double low, double high);
  void set2DBinning(int nbins1, double* boundaries1, int nbins2, double* boundaries2);
  void set2DBinning(int nbins1, double low1, double high1, int nbins2, double low2, double high2);
  void set1DBinning(std::vector<double> &XVec);
  void set2DBinning(std::vector<double> &XVec, std::vector<double> &YVec);
  void SetupSampleBinning();
  std::string XVarStr, YVarStr;
  unsigned int SampleNXBins, SampleNYBins;
  std::vector<double> SampleXBins;
  std::vector<double> SampleYBins;
  //===============================================================================

  /// @brief ETA - a function to setup and pass values to functional parameters where you need to pass a value to some custom reweight calc or engine
  virtual void PrepFunctionalParameters(){};
  /// @brief ETA - generic function applying shifts
  virtual void applyShifts(int iSample, int iEvent){(void) iSample; (void) iEvent;};
  /// @brief DB Function which determines if an event is selected, where Selection double looks like {{ND280KinematicTypes Var1, douuble LowBound}
  bool IsEventSelected(const int iSample, const int iEvent);
  bool IsEventSelected(const std::vector<std::string>& ParameterStr, const int iSample, const int iEvent);
  bool IsEventSelected(const std::vector<std::string>& ParameterStr, const std::vector<std::vector<double>> &SelectionCuts, const int iSample, const int iEvent);

  void CalcXsecNormsBins(int iSample);
  /// @brief This just gets read in from a yaml file
  bool GetIsRHC() {return IsRHC;}
  /// @brief Calculate the spline weight for a given event
  double CalcXsecWeightSpline(const int iSample, const int iEvent);
  /// @brief Calculate the norm weight for a given event
  double CalcXsecWeightNorm(const int iSample, const int iEvent);
  /// @brief Virtual so this can be over-riden in an experiment derived class
  virtual double CalcXsecWeightFunc(int iSample, int iEvent){(void)iSample; (void)iEvent; return 1.0;};

  virtual double ReturnKinematicParameter(std::string KinematicParamter, int iSample, int iEvent) = 0;
  virtual double ReturnKinematicParameter(double KinematicVariable, int iSample, int iEvent) = 0;
  virtual std::vector<double> ReturnKinematicParameterBinning(std::string KinematicParameter) = 0; //Returns binning for parameter Var
  virtual const double* ReturnKinematicParameterByReference(std::string KinematicParamter, int iSample, int iEvent) = 0;
  virtual const double* ReturnKinematicParameterByReference(double KinematicVariable, int iSample, int iEvent) = 0;
  //ETA - new function to generically convert a string from xsec cov to a kinematic type
  //virtual double StringToKinematicVar(std::string kinematic_str) = 0;

  // Function to setup Functional and shift parameters. This isn't idea but
  // do this in your experiment specific code for now as we don't have a 
  // generic treatment for func and shift pars yet
  virtual void SetupFuncParameters(){return;};
  void SetupNormParameters();

  //===============================================================================
  //DB Functions required for reweighting functions  
  //DB Replace previous implementation with reading bin contents from samplePDF_array
  void fill1DHist();
  void fill2DHist();

  /// @brief DB Nice new multi-threaded function which calculates the event weights and fills the relevant bins of an array
#ifdef MULTITHREAD
  void fillArray_MP();
#endif
  void fillArray();

  /// @brief Helper function to reset histograms
  inline void ResetHistograms();
  
  //===============================================================================
  //DB Variables required for GetLikelihood
  //
  /// DB Vectors to hold bin edges
  std::vector<double> XBinEdges;
  std::vector<double> YBinEdges;

  /// DB Array to be filled after reweighting
  double** samplePDFFD_array;
  /// KS Array used for MC stat
  double** samplePDFFD_array_w2;
  /// DB Array to be filled in AddData
  double** samplePDFFD_data;
  //===============================================================================

  //===============================================================================
  //MC variables
  std::vector<struct fdmc_base> MCSamples;
  TFile *_sampleFile;
  TTree *_data;
  //===============================================================================

  //===============================================================================
  /// DB Variables required for oscillation
  OscillatorBase* NuOscillator;
  
  /// An axis to set binned oscillation weights
  TAxis *osc_binned_axis ;
  //===============================================================================
  
  //Variables controlling oscillation parameters
  bool doubled_angle;
  bool osc_binned;

  //===============================================================================
  //DB Covariance Objects
  //ETA - All experiments will need an xsec, det and osc cov
  //these should be added to samplePDFBase to be honest
  covarianceXsec *XsecCov;
  covarianceOsc *OscCov;
  //=============================================================================== 

  //ETA - binning opt can probably go soon...
  int BinningOpt;
  /// keep track of the dimensions of the sample binning
  int nDimensions;
  int SampleDetID;
  bool IsRHC;
  /// holds "TrueNeutrinoEnergy" and the strings used for the sample binning.
  std::vector<std::string> SplineBinnedVars;

  std::string samplename;

  /// Information to store for normalisation pars
  std::vector<XsecNorms4> xsec_norms;
  int nFuncParams;
  std::vector<std::string> funcParsNames;
  std::vector<int> funcParsIndex;

  //===========================================================================
  //DB Vectors to store which kinematic cuts we apply
  //like in XsecNorms but for events in sample. Read in from sample yaml file 
  std::vector< std::string > SelectionStr; 
  std::vector< std::vector<double> > Selection; //The enum, then the bounds corresponding to the string

  // like in XsecNorms but for events in sample. Read in from sample yaml file
  // in samplePDFExperimentBase.cpp
  std::vector< std::vector<double> > SelectionBounds;

  //What gets used in IsEventSelected, which gets set equal to user input plus 
  //all the vectors in StoreSelection
  //std::vector< std::vector<double> > Selection;
  int NSelections;
  //What gets pulled from config options
  std::vector< std::vector<double> > StoredSelection; 
  //===========================================================================
};
