// torch
#include <torch/torch.h>

// base
#include <configure.h>

// snap
#include <snap/mesh/mesh_formatter.hpp>

#include "coordinate.hpp"

namespace snap {
CoordinateOptions::CoordinateOptions(ParameterInput pin) {
  auto nghost = pin->GetOrAddInteger("meshblock", "nghost", 2);

  x1min(pin->GetOrAddReal("meshblock", "x1min", 0.0));
  x1max(pin->GetOrAddReal("meshblock", "x1max", 1.0));

  x2min(pin->GetOrAddReal("meshblock", "x2min", 0.0));
  x2max(pin->GetOrAddReal("meshblock", "x2max", 1.0));

  x3min(pin->GetOrAddReal("meshblock", "x3min", 0.0));
  x3max(pin->GetOrAddReal("meshblock", "x3max", 1.0));

  if (pin->DoesParameterExist("meshblock", "nx1")) {
    nx1(pin->GetInteger("meshblock", "nx1"));
  } else {
    nx1(pin->GetOrAddInteger("mesh", "nx1", 1));
  }

  if (pin->DoesParameterExist("meshblock", "nx2")) {
    nx2(pin->GetInteger("meshblock", "nx2"));
  } else {
    nx2(pin->GetOrAddInteger("mesh", "nx2", 1));
  }

  if (pin->DoesParameterExist("meshblock", "nx3")) {
    nx3(pin->GetInteger("meshblock", "nx3"));
  } else {
    nx3(pin->GetOrAddInteger("mesh", "nx3", 1));
  }
}

CoordinateImpl::CoordinateImpl(const CoordinateOptions& options_)
    : options(options_) {
  auto const& op = options;

  auto dx = (op.x1max() - op.x1min()) / op.nx1();
  auto x1min = op.nx1() > 1 ? op.x1min() - op.nghost() * dx : op.x1min();
  auto x1max = op.nx1() > 1 ? op.x1max() + op.nghost() * dx : op.x1max();
  x1f = torch::linspace(x1min, x1max, op.nc1() + 1, torch::kFloat64);

  dx = (op.x2max() - op.x2min()) / op.nx2();
  auto x2min = op.nx2() > 1 ? op.x2min() - op.nghost() * dx : op.x2min();
  auto x2max = op.nx2() > 1 ? op.x2max() + op.nghost() * dx : op.x2max();
  x2f = torch::linspace(x2min, x2max, op.nc2() + 1, torch::kFloat64);

  dx = (op.x3max() - op.x3min()) / op.nx3();
  auto x3min = op.nx3() > 1 ? op.x3min() - op.nghost() * dx : op.x3min();
  auto x3max = op.nx3() > 1 ? op.x3max() + op.nghost() * dx : op.x3max();
  x3f = torch::linspace(x3min, x3max, op.nc3() + 1, torch::kFloat64);
}

void CoordinateImpl::reset_coordinates(std::vector<MeshGenerator> meshgens) {
  auto const& op = options;
  TORCH_CHECK(meshgens.size() == 3, "requires exactly three mesh generators");

  if (meshgens[0] != nullptr) {
    int nx1f = x1f.size(0);
    auto rx = compute_logical_position(
        torch::linspace(0, nx1f, nx1f, torch::kFloat64), nx1f, true);
    x1f.copy_(meshgens[0](rx, op.x1min(), op.x1max()));
  }

  if (meshgens[1] != nullptr) {
    int nx2f = x2f.size(0);
    auto rx = compute_logical_position(
        torch::linspace(0, nx2f, nx2f, torch::kFloat64), nx2f, true);
    x2f.copy_(meshgens[1](rx, op.x2min(), op.x2max()));
  }

  if (meshgens[2] != nullptr) {
    int nx3f = x3f.size(0);
    auto rx = compute_logical_position(
        torch::linspace(0, nx3f, nx3f, torch::kFloat64), nx3f, true);
    x3f.copy_(meshgens[2](rx, op.x3min(), op.x3max()));
  }
}

void CoordinateImpl::print(std::ostream& stream) const {
  stream << "x1f = [";
  for (int i = 0; i < x1f.size(0); ++i) {
    stream << x1f[i].item<float>();
    if (i < x1f.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl << "x1v = [";
  for (int i = 0; i < x1v.size(0); ++i) {
    stream << x1v[i].item<float>();
    if (i < x1v.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl;

  stream << "x2f = [";
  for (int i = 0; i < x2f.size(0); ++i) {
    stream << x2f[i].item<float>();
    if (i < x2f.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl << "x2v = [";
  for (int i = 0; i < x2v.size(0); ++i) {
    stream << x2v[i].item<float>();
    if (i < x2v.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl;

  stream << "x3f = [";
  for (int i = 0; i < x3f.size(0); ++i) {
    stream << x3f[i].item<float>();
    if (i < x3f.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl << "x3v = [";
  for (int i = 0; i < x3v.size(0); ++i) {
    stream << x3v[i].item<float>();
    if (i < x3v.size(0) - 1) {
      stream << ", ";
    }
  }
  stream << "]" << std::endl;
}

torch::Tensor CoordinateImpl::center_width1() const {
  // return dx1f.unsqueeze(0).unsqueeze(0).expand({x3v.size(0), x2v.size(0),
  // -1});
  return dx1f.unsqueeze(0).unsqueeze(1);
}

torch::Tensor CoordinateImpl::center_width2() const {
  // return dx2f.unsqueeze(0).unsqueeze(1).expand({x3v.size(0), -1,
  // x1v.size(0)});
  return dx2f.unsqueeze(0).unsqueeze(2);
}

torch::Tensor CoordinateImpl::center_width3() const {
  // return dx3f.unsqueeze(1).unsqueeze(2).expand({-1, x2v.size(0),
  // x1v.size(0)});
  return dx3f.unsqueeze(1).unsqueeze(2);
}

torch::Tensor CoordinateImpl::face_area1() const {
  return dx3f.outer(dx2f).unsqueeze(2).expand({-1, -1, x1f.size(0)});
}

torch::Tensor CoordinateImpl::face_area2() const {
  return dx3f.outer(dx1f).unsqueeze(1).expand({-1, x2f.size(0), -1});
}

torch::Tensor CoordinateImpl::face_area3() const {
  return dx2f.outer(dx1f).unsqueeze(0).expand({x3f.size(0), -1, -1});
}

torch::Tensor CoordinateImpl::cell_volume() const {
  // return dx1f.outer(dx2f).unsqueeze(0).outer(dx3f).unsqueeze(0);
  return torch::einsum("km,mji->kji",
                       {dx3f.unsqueeze(1), dx2f.outer(dx1f).unsqueeze(0)});
}

torch::Tensor CoordinateImpl::find_cell_index(
    torch::Tensor const& coords) const {
  torch::Tensor index = torch::zeros_like(coords, torch::dtype(torch::kInt64));

  // x1dir
  index.slice(1, 0, 1) = torch::searchsorted(x1f, coords.slice(1, 0, 1));

  // x2dir
  if (coords.size(1) > 1) {
    index.slice(1, 1, 2) = torch::searchsorted(x2f, coords.slice(1, 1, 2));
  }

  // x3dir
  if (coords.size(1) > 2) {
    index.slice(1, 2, 3) = torch::searchsorted(x3f, coords.slice(1, 2, 3));
  }
  return index;
}

torch::Tensor CoordinateImpl::vec_lower(torch::Tensor prim, int type) const {
  if (type == kPrimitive) {
    return prim.narrow(0, Index::IVX, 3);
  } else if (type == kDPMassLR) {
    return prim.narrow(1, Index::IVX, 3);
  } else {
    std::stringstream msg;
    msg << fmt::format("{}::vec_lower:unknown type id: {}", name_(), type);
    throw std::runtime_error(msg.str());
  }
}

torch::Tensor CoordinateImpl::vec_raise(torch::Tensor prim, int type) const {
  if (type == kPrimitive) {
    return prim.narrow(1, Index::IVX, 3);
  } else if (type == kDPMassLR) {
    return prim.narrow(1, Index::IVX, 3);
  } else {
    std::stringstream msg;
    msg << fmt::format("{}::vec_raise:unknown type id: {}", name_(), type);
    throw std::runtime_error(msg.str());
  }
}

std::array<torch::Tensor, 3> CoordinateImpl::vec_from_cartesian(
    std::array<double, 3> vec) const {
  auto omega1 = torch::tensor(vec[0]).expand({1, 1, 1});
  auto omega2 = torch::tensor(vec[1]).expand({1, 1, 1});
  auto omega3 = torch::tensor(vec[2]).expand({1, 1, 1});
  return {omega1, omega2, omega3};
}

torch::Tensor CoordinateImpl::forward(torch::Tensor flux1, torch::Tensor flux2,
                                      torch::Tensor flux3) {
  torch::NoGradGuard no_grad;

  enum { DIM1 = 3, DIM2 = 2, DIM3 = 1, DIMC = 0 };

  auto vol = cell_volume().unsqueeze(0);
  auto dflx = torch::zeros_like(flux1);

  int is, ie, js, je, ks, ke;

  if (vol.size(DIM1) > 1) {
    is = 1;
    ie = vol.size(DIM1) - 1;
  } else {
    is = 0;
    ie = 1;
  }

  if (vol.size(DIM2) > 1) {
    js = 1;
    je = vol.size(DIM2) - 1;
  } else {
    js = 0;
    je = 1;
  }

  if (vol.size(DIM3) > 1) {
    ks = 1;
    ke = vol.size(DIM3) - 1;
  } else {
    ks = 0;
    ke = 1;
  }

  if (flux1.numel() > 0) {
    dflx.slice(DIM1, is, ie) +=
        face_area1(is + 1, ie + 1).unsqueeze(0) *
            flux1.slice(DIM1, is + 1, ie + 1) -
        face_area1(is, ie).unsqueeze(0) * flux1.slice(DIM1, is, ie);
  }

  if (flux2.numel() > 0) {
    dflx.slice(DIM2, js, je) +=
        face_area2(js + 1, je + 1).unsqueeze(0) *
            flux2.slice(DIM2, js + 1, je + 1) -
        face_area2(js, je).unsqueeze(0) * flux2.slice(DIM2, js, je);
  }

  if (flux3.numel() > 0) {
    dflx.slice(DIM3, ks, ke) +=
        face_area3(ks + 1, ke + 1).unsqueeze(0) *
            flux3.slice(DIM3, ks + 1, ke + 1) -
        face_area3(ks, ke).unsqueeze(0) * flux3.slice(DIM3, ks, ke);
  }

  return dflx / vol;
}

IndexRange get_interior(torch::IntArrayRef const& shape, int nghost,
                        int extend_x1, int extend_x2, int extend_x3) {
  int len = shape.size();
  int nc1 = shape[len - 1];
  int nc2 = shape[len - 2];
  int nc3 = shape[len - 3];
  int start1, len1, start2, len2, start3, len3;

  if (nc1 > 1) {
    start1 = nghost;
    len1 = nc1 - 2 * nghost;
  } else {
    start1 = 0;
    len1 = 1;
  }

  if (nc2 > 1) {
    start2 = nghost;
    len2 = nc2 - 2 * nghost;
  } else {
    start2 = 0;
    len2 = 1;
  }

  if (nc3 > 1) {
    start3 = nghost;
    len3 = nc3 - 2 * nghost;
  } else {
    start3 = 0;
    len3 = 1;
  }

  IndexRange result;
  result.push_back(torch::indexing::Slice(start3, start3 + len3 + extend_x3));
  result.push_back(torch::indexing::Slice(start2, start2 + len2 + extend_x2));
  result.push_back(torch::indexing::Slice(start1, start1 + len1 + extend_x1));

  for (int n = 3; n < shape.size(); ++n) {
    result.insert(result.begin(), torch::indexing::Slice());
  }

  return result;
}
}  // namespace snap
