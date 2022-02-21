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

#pragma once

#include "rng.h"
#include "rpu_pulsed_device.h"
#include "utility_functions.h"

namespace RPU {

template <typename T> class SelfDefineRPUDevice;

BUILD_PULSED_DEVICE_META_PARAMETER(
    SelfDefine,
    /*implements*/
    DeviceUpdateType::SelfDefine,
    
    /*parameter def*/    
    std::vector<T> sd_up_pulse;
    std::vector<T> sd_up_weight;
    std::vector<T> sd_down_pulse;
    std::vector<T> sd_down_weight;
    T sd_n_points = (T)0.00;
    ,
    /*print body*/
    ss << "\t(dtod=" << this->dw_min_dtod << ")" << std::endl;
    ss << "\t up_down:\t" << this->up_down << "\t(dtod=" << this->up_down_dtod << ")"
       << std::endl;
    ,
    /* calc weight granularity body */
    return this->dw_min;
    ,
    /*Add*/
    bool implementsWriteNoise() const override { return true; };);

template <typename T> class SelfDefineRPUDevice : public PulsedRPUDevice<T> {

  BUILD_PULSED_DEVICE_CONSTRUCTORS(
      SelfDefineRPUDevice,
      /* ctor*/
      int x_sz = this->x_size_;
      int d_sz = this->d_size_;

      w_gamma_down_ = Array_2D_Get<T>(d_sz, x_sz);
      w_gamma_up_ = Array_2D_Get<T>(d_sz, x_sz);

      for (int j = 0; j < x_sz; ++j) {
        for (int i = 0; i < d_sz; ++i) {
          w_gamma_up_[i][j] = (T)0.0;
          w_gamma_down_[i][j] = (T)0.0;
        }
      },
      /* dtor*/
      Array_2D_Free<T>(w_gamma_down_);
      Array_2D_Free<T>(w_gamma_up_);
      ,
      /* copy */
      for (int j = 0; j < other.x_size_; ++j) {
        for (int i = 0; i < other.d_size_; ++i) {
          w_gamma_down_[i][j] = other.w_gamma_down_[i][j];
          w_gamma_up_[i][j] = other.w_gamma_up_[i][j];
        }
      },
      /* move assignment */
      w_gamma_down_ = other.w_gamma_down_;
      w_gamma_up_ = other.w_gamma_up_;

      other.w_gamma_down_ = nullptr;
      other.w_gamma_up_ = nullptr;
      ,
      /* swap*/
      swap(a.w_gamma_up_, b.w_gamma_up_);
      swap(a.w_gamma_down_, b.w_gamma_down_);
      ,
      /* dp names*/
      names.push_back(std::string("gamma_up"));
      names.push_back(std::string("gamma_down"));
      ,
      /* dp2vec body*/
      int n_prev = (int)names.size();
      int size = this->x_size_ * this->d_size_;

      for (int i = 0; i < size; ++i) {
        data_ptrs[n_prev][i] = w_gamma_up_[0][i];
        data_ptrs[n_prev + 1][i] = w_gamma_down_[0][i];
      },
      /* vec2dp body*/
      int n_prev = (int)names.size();
      int size = this->x_size_ * this->d_size_;

      for (int i = 0; i < size; ++i) {
        w_gamma_up_[0][i] = data_ptrs[n_prev][i];
        w_gamma_down_[0][i] = data_ptrs[n_prev + 1][i];
      },
      /*invert copy DP */
      T **gamma_down = rpu->getGammaDown();
      T **gamma_up = rpu->getGammaUp();
      for (int j = 0; j < this->x_size_; ++j) {
        for (int i = 0; i < this->d_size_; ++i) {
          w_gamma_down_[i][j] = gamma_up[i][j];
          w_gamma_up_[i][j] = gamma_down[i][j];
        }
      }

  );

  void printDP(int x_count, int d_count) const override;

  inline T **getGammaUp() const { return w_gamma_up_; };
  inline T **getGammaDown() const { return w_gamma_down_; };

  void doSparseUpdate(
      T **weights, int i, const int *x_signed_indices, int x_count, int d_sign, RNG<T> *rng)
      override;
  void doDenseUpdate(T **weights, int *coincidences, RNG<T> *rng) override;

private:
  T **w_gamma_up_ = nullptr;
  T **w_gamma_down_ = nullptr;
};
} // namespace RPU
