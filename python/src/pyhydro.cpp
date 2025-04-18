// torch
#include <torch/extension.h>
#include <torch/nn/modules/container/any.h>

// snap
#include <snap/hydro/hydro.hpp>
#include <snap/hydro/hydro_formatter.hpp>
#include <snap/input/parameter_input.hpp>

// python
#include "pyoptions.hpp"

namespace py = pybind11;

void bind_hydro(py::module &m) {
  py::class_<snap::HydroOptions>(m, "HydroOptions")
      .def(py::init<>())
      .def(py::init<snap::ParameterInput>())
      .def("__repr__",
           [](const snap::HydroOptions &a) {
             return fmt::format("HydroOptions{}", a);
           })
      .ADD_OPTION(snap::ConstGravityOptions, snap::HydroOptions, grav)
      .ADD_OPTION(snap::CoriolisOptions, snap::HydroOptions, coriolis)
      .ADD_OPTION(snap::EquationOfStateOptions, snap::HydroOptions, eos)
      .ADD_OPTION(snap::CoordinateOptions, snap::HydroOptions, coord)
      .ADD_OPTION(snap::RiemannSolverOptions, snap::HydroOptions, riemann)
      .ADD_OPTION(snap::PrimitiveProjectorOptions, snap::HydroOptions, proj)
      .ADD_OPTION(snap::ReconstructOptions, snap::HydroOptions, recon1)
      .ADD_OPTION(snap::ReconstructOptions, snap::HydroOptions, recon23)
      .ADD_OPTION(snap::InternalBoundaryOptions, snap::HydroOptions, ib)
      .ADD_OPTION(snap::VerticalImplicitOptions, snap::HydroOptions, vic);

  py::class_<snap::PrimitiveProjectorOptions>(m, "PrimitiveProjectorOptions")
      .def(py::init<>())
      .def("__repr__",
           [](const snap::PrimitiveProjectorOptions &a) {
             return fmt::format("PrimitiveProjectorOptions{}", a);
           })
      .ADD_OPTION(std::string, snap::PrimitiveProjectorOptions, type)
      .ADD_OPTION(double, snap::PrimitiveProjectorOptions, margin);

  ADD_SNAP_MODULE(Hydro, HydroOptions)
      .def("nvar", &snap::HydroImpl::nvar)
      .def("max_time_step", &snap::HydroImpl::max_time_step)
      .def("forward", &snap::HydroImpl::forward);
}
