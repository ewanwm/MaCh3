#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>
#include <string>

#include "plottingUtils.h"
#include "plottingManager.h"
#include "styleManager.h"
#include "inputManager.h"

namespace py = pybind11;


void initPlotting(py::module &m){

    auto m_plotting = m.def_submodule("plotting");
    m_plotting.doc() = "This is a Python binding of MaCh3s C++ based plotting library Library";

    py::class_<MaCh3Plotting::PlottingManager>(m_plotting, "PlottingManager")
        .def(py::init<const std::string &>()) // <- constructor setting non-default constructor
        .def(py::init()) // <- default constructor
        .def("initialise", &MaCh3Plotting::PlottingManager::initialise, "initalise this PlottingManager")
        .def("usage", &MaCh3Plotting::PlottingManager::usage, "Print a usage message for the current executable")
        .def("parse_inputs", &MaCh3Plotting::PlottingManager::parseInputsVec, "Parse command line variables")
        .def("set_exec", &MaCh3Plotting::PlottingManager::setExec, "Set the name of the current executable, which will be used when getting executable specific options from the plotting config file" )
        .def("get_file_name", &MaCh3Plotting::PlottingManager::getFileName, "Get the path to a particular file")
        .def("get_file_label", &MaCh3Plotting::PlottingManager::getFileLabel, "Get the specified label of a particular input file")
        .def("get_draw_options", &MaCh3Plotting::PlottingManager::getDrawOptions, "Get any additional root drawing options specified by the user")
        .def("get_output_name", py::overload_cast<const std::string &>(&MaCh3Plotting::PlottingManager::getOutputName), "Get the output name specified by the user")
        .def("get_file_names", &MaCh3Plotting::PlottingManager::getFileNames, "Get the list of all file names")
        .def("get_file_labels", &MaCh3Plotting::PlottingManager::getFileLabels, "Get the list of all file labels")
        .def("get_n_files", &MaCh3Plotting::PlottingManager::getNFiles, "Get the number of specified files")
        .def("get_split_by_sample", &MaCh3Plotting::PlottingManager::getSplitBySample, "Get whether or not the user has set the 'split by sample' (-s) option")
        .def("get_plot_ratios", &MaCh3Plotting::PlottingManager::getPlotRatios, "Get whether or not the user specified the 'plot ratios' (-r) option")
        .def("get_draw_grid", &MaCh3Plotting::PlottingManager::getDrawGrid, "Get wheter or not the user has specified the 'draw grid' (-g) option")
        .def("style", &MaCh3Plotting::PlottingManager::style, "Get the StyleManager associated with this PlottingManager")
        .def("input", &MaCh3Plotting::PlottingManager::input, "Get the InputManager associated with this PlottingManager")

        // EM: I can't figure out how to add the getOption methods as theres not really a way to deduce the return type from yaml so leaving them out for now :/
        //     I think one solution would be to extend the PlottingManager class inside of python (add a pyPlottingManager (or something like that) that derives 
        //     from this one and add the functions to that) and then fold that python code into the module somehow. But that is currently beyond my pybinding abilities
        ;

    py::class_<MaCh3Plotting::InputManager>(m_plotting, "InputManager")
        .def("print", &MaCh3Plotting::InputManager::print, "Print a summary of everything this manager knows")
        .def("get_llh_scan", &MaCh3Plotting::InputManager::getLLHScan, "Get the LLH scan for a particular parameter from a particular file")
        .def("get_llh_scan_by_sample", &MaCh3Plotting::InputManager::getSampleSpecificLLHScan, "Get the LLH scan for a particular parameter from a particular file for a particular sample")
        .def("get_enabled_llh", &MaCh3Plotting::InputManager::getEnabledLLH, "Get whether a particular file has LLH scans for a particular parameter")
        .def("get_enabled_llh_by_sample", &MaCh3Plotting::InputManager::getEnabledLLHBySample, "Get whether a particular file has LLH scans for a particular parameter for a particular sample")
        .def("get_post_fit_error", &MaCh3Plotting::InputManager::getPostFitError, "Get the post fit error for a parameter from a particular file")
        .def("get_post_fit_value", &MaCh3Plotting::InputManager::getPostFitValue, "Get the post fit value for a parameter from a particular file")
        .def("get_known_parameters", &MaCh3Plotting::InputManager::getKnownParameters, "Get all the parameters that this manager knows about. Useful for iterating over")
        .def("get_known_samples", &MaCh3Plotting::InputManager::getKnownSamples, "Get all the samples that this manager knows about. Useful for iterating over")
        .def("get_tagged_parameters", &MaCh3Plotting::InputManager::getTaggedParameters, "Get all the parameters whose tags match some specified list")
        .def("get_tagged_samples", &MaCh3Plotting::InputManager::getTaggedSamples, "Get all the samples whose tags match some specified list")
        .def("get_n_input_files", &MaCh3Plotting::InputManager::getNInputFiles, "Get the number of input files registered with this manager")
        .def("get_known_llh_parameters", &MaCh3Plotting::InputManager::getKnownLLHParameters, "Get all the parameters that a file has LLH scans for")
        .def("get_known_llh_samples", &MaCh3Plotting::InputManager::getKnownLLHSamples, "Get all the samples that a file has individual LLH scans for")
        .def("get_known_post_fit_parameters", &MaCh3Plotting::InputManager::getKnownPostFitParameters, "Get all the parameters that a file has post fit values and errors for")
        ;

    py::class_<MaCh3Plotting::StyleManager>(m_plotting, "StyleManager")
        .def("prettify_parameter_name", &MaCh3Plotting::StyleManager::prettifyParamName, "Convert internally used parameter name to a nice pretty name that can be used in plots")
        .def("prettify_sample_name", &MaCh3Plotting::StyleManager::prettifyParamName, "Convert internally used sample name to a nice pretty name that can be used in plots")
        ;

}