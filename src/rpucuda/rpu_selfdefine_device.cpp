/**
 * (C) Copyright 2020, 2021 IBM. All Rights Reserved.
 *
 * This code is licensed under the Apache License, Version 2.0. You may
 * obtain a copy of this license in the LICENSE.txt file in the root directory
 * of this source tree or at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * Any modifications or derivative works of this code must retain this
 * copyright notice, and modified files need to carry a notice indicating
 * that they have been altered from the originals.
 */

#include "rpu_selfdefine_device.h"
#include "utility_functions.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>

namespace RPU {

/********************************************************************************
 * Linear Step RPU Device
 *********************************************************************************/

// delete populate, use parent populate
// template <typename T>
// void SelfDefineRPUDevice<T>::populate(
//     const SelfDefineRPUDeviceMetaParameter<T> &p, RealWorldRNG<T> *rng) {

//   PulsedRPUDevice<T>::populate(p, rng); // will clone par
//   auto &par = getPar();

//   T gain_std = par.sd_gamma_dtod;
//   T up_down_std = par.sd_gamma_up_down_dtod;
//   T up_down = par.sd_gamma_up_down;

//   T up_bias = up_down > 0 ? (T)0.0 : up_down;
//   T down_bias = up_down > 0 ? -up_down : (T)0.0;

//   // use scale_up/down instead of gamma_up/down
//   // scale_up/down are dtod in this case (for every point)
//   for (int i = 0; i < this->d_size_; ++i) {
//     for (int j = 0; j < this->x_size_; ++j) {

//       T gain = (T)1.0 + gain_std * rng->sampleGauss();
//       T r = up_down_std * rng->sampleGauss();

//       w_gamma_up_[i][j] = (up_bias + gain + r);
//       w_gamma_down_[i][j] = (down_bias + gain - r);

//       if (par.enforce_consistency) {
//         w_gamma_up_[i][j] = fabs(w_gamma_up_[i][j]);
//         w_gamma_down_[i][j] = fabs(w_gamma_down_[i][j]);
//       }
//     }
//   }
// }

template <typename T> void SelfDefineRPUDevice<T>::printDP(int x_count, int d_count) const {

  if (x_count < 0 || x_count > this->x_size_) {
    x_count = this->x_size_;
  }

  if (d_count < 0 || d_count > this->d_size_) {
    d_count = this->d_size_;
  }
  bool persist_if = getPar().usesPersistentWeight();

  for (int i = 0; i < d_count; ++i) {
    for (int j = 0; j < x_count; ++j) {
      std::cout.precision(5);
      std::cout << i << "," << j << ": ";
      std::cout << "[<" << this->w_max_bound_[i][j] << ",";
      std::cout << this->w_min_bound_[i][j] << ">,<";
      std::cout << this->w_scale_up_[i][j] << ",";
      std::cout << this->w_scale_down_[i][j] << ">]";
      std::cout.precision(10);
      std::cout << this->w_decay_scale_[i][j] << ", ";
      std::cout.precision(6);
      std::cout << this->w_diffusion_rate_[i][j] << ", ";
      std::cout << this->w_reset_bias_[i][j];
      if (persist_if) {
        std::cout << ", " << this->w_persistent_[i][j];
      }
      std::cout << "]";
    }
    std::cout << std::endl;
  }
}

namespace {
template <typename T>
inline void update_once(
    T &w,
    T &w_apparent,
    int &sign,
    T &scale_down,
    T &scale_up,
    T &min_bound,
    T &max_bound,
    T &interpolated,
    const T &dw_min_std,
    const T &write_noise_std,
    RNG<T> *rng) {

  if (sign > 0){
    w -= interpolated * scale_down * ((T)1.0 + dw_min_std * rng->sampleGauss());
  } else {
    w += interpolated * scale_up * ((T)1.0 + dw_min_std * rng->sampleGauss());
  }
  w = MAX(w, min_bound);
  w = MIN(w, max_bound);

  if (write_noise_std > (T)0.0) {
    w_apparent = w + write_noise_std * ((T)1.0 + dw_min_std * rng->sampleGauss());
  }
}

} // namespace

template <typename T>
void SelfDefineRPUDevice<T>::doSparseUpdate(
    T **weights, int i, const int *x_signed_indices, int x_count, int d_sign, RNG<T> *rng) {

  const auto &par = getPar();

  T *scale_down = this->w_scale_down_[i];
  T *scale_up = this->w_scale_up_[i];
  T *w = par.usesPersistentWeight() ? this->w_persistent_[i] : weights[i];
  T *w_apparent = weights[i];
  T *min_bound = this->w_min_bound_[i];
  T *max_bound = this->w_max_bound_[i];
  
  std::vector<T> sd_up_pulse = par.sd_up_pulse;
  std::vector<T> sd_up_weight = par.sd_up_weight;
  std::vector<T> sd_down_pulse = par.sd_down_pulse;
  std::vector<T> sd_down_weight = par.sd_down_weight;
  T n_points = par.sd_n_points;

  // interpolate here?
  // array of j length?
  // i row idx
  // j col idx
  for(int n = 0; n < n_points; i++) {
    
  }

  T write_noise_std = par.getScaledWriteNoise();
  PULSED_UPDATE_W_LOOP(update_once(
                           w[j], w_apparent[j], sign, scale_down[j], scale_up[j], min_bound[j], max_bound[j], interpolated, par.dw_min_std, write_noise_std,
                           rng););
}

template <typename T>
void SelfDefineRPUDevice<T>::doDenseUpdate(T **weights, int *coincidences, RNG<T> *rng) {

  const auto &par = getPar();

  T *scale_down = this->w_scale_down_[0];
  T *scale_up = this->w_scale_up_[0];
  T *w = par.usesPersistentWeight() ? this->w_persistent_[0] : weights[0];
  T *w_apparent = weights[0];
  T *min_bound = this->w_min_bound_[0];
  T *max_bound = this->w_max_bound_[0];
  T write_noise_std = par.getScaledWriteNoise();

  PULSED_UPDATE_W_LOOP_DENSE(update_once(
                                 w[j], w_apparent[j], sign, scale_down[j], scale_up[j],
                                 min_bound[j], max_bound[j],
                                 par.dw_min_std, write_noise_std, rng););
}

template class SelfDefineRPUDevice<float>;
#ifdef RPU_USE_DOUBLE
template class SelfDefineRPUDevice<double>;
#endif

} // namespace RPU
