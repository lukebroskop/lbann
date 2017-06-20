////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2016, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
// Written by the LBANN Research Team (B. Van Essen, et al.) listed in
// the CONTRIBUTORS file. <lbann-dev@llnl.gov>
//
// LLNL-CODE-697807.
// All rights reserved.
//
// This file is part of LBANN: Livermore Big Artificial Neural Network
// Toolkit. For details, see http://software.llnl.gov/LBANN or
// https://github.com/LLNL/LBANN.
//
// Licensed under the Apache License, Version 2.0 (the "Licensee"); you
// may not use this file except in compliance with the License.  You may
// obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the license.
//
// lbann_callback_dump_activations .hpp .cpp - Callbacks to dump activations
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_CALLBACKS_CALLBACK_DUMP_ACTIVATIONS_HPP_INCLUDED
#define LBANN_CALLBACKS_CALLBACK_DUMP_ACTIVATIONS_HPP_INCLUDED

#include "lbann/callbacks/lbann_callback.hpp"

namespace lbann {

/**
 * Dump activations matrices to files.
 * This will dump each hidden layer's weight/bias matrix after each epoch.
 * The matrices are written to files using Elemental's simple ASCII format. This
 * is not meant for checkpointing, but for exporting weight matrices for
 * analysis that isn't easily done in LBANN.
 * Note this dumps matrices during each mini-batch. This will be slow and
 * produce a lot of output.
 */
class lbann_callback_dump_activations : public lbann_callback {
 public:
  /**
   * @param basename The basename for writing files.
   */
  lbann_callback_dump_activations(std::string basename, int batch_interval = 1) :
    lbann_callback(batch_interval), m_basename(basename) {
    set_name("dump_activations");
  }
  void on_forward_prop_end(model *m, Layer *l);
 private:
  /** Basename for writing files. */
  std::string m_basename;
};

}  // namespace lbann

#endif  // LBANN_CALLBACKS_CALLBACK_DUMP_ACTIVATIONS_HPP_INCLUDED
