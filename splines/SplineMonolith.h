#pragma once

// C++ includes
#include <cstdlib>
#include <iomanip>

// ROOT include
#include "TFile.h"
#include "TH1F.h"
#include "TKey.h"
#include "TString.h"
#include "TIterator.h"
#include "TStopwatch.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TTree.h"

#ifdef MULTITHREAD
#include "omp.h"
#endif

// MaCh3  includes
#include "samplePDF/Structs.h"
#include "manager/MaCh3Logger.h"

//KS: We store coefficients {y,b,c,d} in one array one by one, this is only to define it once rather then insert "4" all over the code
#define _nCoeff_ 4
//KS: For TF1 we store at most 5 coefficients, we could make it more flexible but for now define it here to make future changes easier to track
#define _nTF1Coeff_ 5


#ifdef CUDA
extern void SynchroniseSplines();
#endif

// Make template class so we can use TF1 and TSpline3
class SMonolith {
  public:
    /// @brief Constructor
    /// @param MasterSpline Vector of TSpline3 pointers which we strip back
    SMonolith(std::vector< std::vector<TSpline3*> > &MasterSpline);
    /// @brief Constructor
    /// @param MasterSpline Vector of TSpline3_red pointers which we strip back
    SMonolith(std::vector< std::vector<TSpline3_red*> > &MasterSpline);
    /// @brief Constructor
    /// @param MasterSpline Vector of TF1 pointers which we strip back
    SMonolith(std::vector< std::vector<TF1*> > &MasterSpline);
    /// @brief Constructor
    /// @param MasterSpline Vector of TF1_red pointers which we strip back
    SMonolith(std::vector< std::vector<TF1_red*> > &MasterSpline);
    /// @brief Constructor where you pass path to preprocessed root FileName
    /// @param FileName path to pre-processed root file containing stripped monolith info
    SMonolith(std::string FileName);
    /// @brief Destructor for SMonolith class.
    ~SMonolith();

    /// @brief  CW: This Eval should be used when using two separate x,{y,a,b,c,d} arrays to store the weights; probably the best one here! Same thing but pass parameter spline segments instead of variations
    void Evaluate();

    /// @brief CW: Evaluate weights on the CPU/GPU
    void Evaluate_TF1();

    /// @brief KS: After calculations are done on GPU we copy memory to CPU. This operation is asynchronous meaning while memory is being copied some operations are being carried. Memory must be copied before actual reweight. This function make sure all has been copied.
    inline void SynchroniseMemTransfer()
    {
      #ifdef CUDA
      SynchroniseSplines();
      #endif
      return;
    };

    /// @brief KS: Get pointer to total weight to make fit faster wrooom!
    /// @param event Name event number in used MC
    /// @return Pointer to the total weight
    inline const float* retPointer(const int event) {return &cpu_total_weights[event];}
    
    /// @brief KS: Set pointers to spline params
    /// @param spline_ParsPointers Vector of pointers to spline params
    inline void setSplinePointers(std::vector< const double* > spline_ParsPointers) {
      splineParsPointer = spline_ParsPointers;
      for (__int__ i = 0; i < nParams; ++i) SplineInfoArray[i].splineParsPointer = spline_ParsPointers[i];
    };
    
    /// The returned gpu weights, read by the GPU
    float* cpu_weights;
    /// KS: This holds the total CPU weights that gets read in samplePDFND
    float *cpu_total_weights;

  private:
    /// @brief KS: Set everything to null etc.
    inline void Initialise();
    /// @brief CW: Function to scan through the MasterSpline of TSpline3
    inline void ScanMasterSpline(std::vector<std::vector<TSpline3_red*> > &MasterSpline, unsigned int &NEvents, int &MaxPoints, short int &nParams, int &nSplines, unsigned int &nKnots);
    /// @brief CW: Function to scan through the MasterSpline of TF1
    inline void ScanMasterSpline(std::vector<std::vector<TF1_red*> > &MasterSpline, unsigned int &NEvents, int &MaxPoints, short int &nParams);
    /// @brief CW: Prepare the TSpline3_red objects for the GPU
    inline void PrepareForGPU(std::vector<std::vector<TSpline3_red*> > &MasterSpline);
    inline void PrepareForGPU_TSpline3();

    /// @brief CW: Prepare the TF1_red objects for the GPU
    inline void PrepareForGPU(std::vector<std::vector<TF1_red*> > &MasterSpline);
    inline void PrepareForGPU_TF1();
        
    /// @brief CW: Reduced the TSpline3 to TSpline3_red
    inline std::vector<std::vector<TSpline3_red*> > ReduceTSpline3(std::vector<std::vector<TSpline3*> > &MasterSpline);
    /// @brief CW: Reduced the TF1 to TF1_red
    inline std::vector<std::vector<TF1_red*> > ReduceTF1(std::vector<std::vector<TF1*> > &MasterSpline);
    
    /// @brief CW: This loads up coefficients into two arrays: one x array and one yabcd array
    /// @brief CW: This should maximize our cache hits!
    inline void getSplineCoeff_SepMany(TSpline3_red* &spl, int &nPoints, float *&xArray, float *&manyArray);
    /// @brief CW: Helper function used in the constructor, tests to see if the spline is flat
    inline bool isFlat(TSpline3_red* &spl);
    /// @brief CW: Gets the polynomial coefficients for TF1
    inline void getTF1Coeff(TF1_red* &spl, int &nPoints, float *&coeffs);
    
    /// @brief CW:Code used in step by step reweighting, Find Spline Segment for each param
    inline void FindSplineSegment();
    /// @brief CPU based code which eval weight for each spline
    inline void CalcSplineWeights();
    /// @brief Same but TF1
    inline void CalcSplineWeights_TF1();
    /// @brief Calc total event weight
    inline void ModifyWeights();
    /// @brief Conversion from valid splines to all
    inline void ModifyWeights_GPU();
    
    /// @brief KS: Prepare spline file that can be used for fast loading
    inline void PrepareSplineFile();
    /// @brief KS: Load preprocessed spline file
    inline void LoadSplineFile(std::string FileName);

    /// Array of FastSplineInfo structs: keeps information on each xsec spline for fast evaluation
    /// Method identical to TSpline3::Eval(double) but faster because less operations
    FastSplineInfo *SplineInfoArray;
    // Segments store currently found segment while vals parameter values, they are not in FastSplineInfo as in case of GPU we need to copy paste it to GPU
    short int *segments;
    float *vals;
    /// This holds pointer to parameter position which we later copy paste it to GPU
    std::vector< const double* > splineParsPointer;

    /// Number of events
    unsigned int NEvents;
    /// Number of NIWG parameters that have splines
    short int nParams;
    /// Max knots for production
    int _max_knots;
    /// holds the index for good splines; don't do unsigned since starts with negative value!
    int *index_cpu;

    /// Number of valid splines
    unsigned int NSplines_valid;
    /// Number of total splines we can maximally have, if each event had the maximum number of splines found across all events
    unsigned int NSplines_total;

    /// Number of total splines if each event had every parameter's spline
    unsigned int NSplines_total_large;

    /// Sum of all knots over all splines
    unsigned int nKnots;
    
    // Just some pointers to memory that doesn't get allocated so we can access the GPU
    // GPU arrays to hold monolith and weights
    float *gpu_weights;
    float *gpu_total_weights;
    // CPU arrays to hold monolith and weights
    float *cpu_weights_var;
    
    /// KS: Map keeping track how many parameters applies to each event, we keep two numbers here {number of splines per event, index where splines start for a given event}
    std::vector<unsigned int> cpu_nParamPerEvent;
    unsigned int *gpu_nParamPerEvent;

    // GPU arrays to hold number of points
    std::vector<short int> cpu_nPoints_arr;
    short int *gpu_nPoints_arr;
    //KS: Consider merging paramNo and nKnots into one consecutive array
    std::vector<short int> cpu_paramNo_arr;
    short int *gpu_paramNo_arr;
    //KS: Number of knots per spline
    std::vector<unsigned int> cpu_nKnots_arr;
    unsigned int *gpu_nKnots_arr;
    //KS: GPU arrays to hold X coefficient
    std::vector<float> cpu_coeff_x;
    float *gpu_coeff_x;
    // GPU arrays to hold other coefficients
    std::vector<float> cpu_coeff_many;
    float *gpu_coeff_many;

    bool SaveSplineFile;
};