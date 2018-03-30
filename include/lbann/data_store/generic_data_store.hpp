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
////////////////////////////////////////////////////////////////////////////////

#ifndef __GENERIC_DATA_STORE_HPP__
#define __GENERIC_DATA_STORE_HPP__


#include "lbann/base.hpp"
#include "lbann/comm.hpp"
#include <vector>
#include <unordered_set>

namespace lbann {

class generic_data_reader;
class lbann_comm;
class model;

/**
 * todo
 */

class generic_data_store {
 public:

  //! ctor
  generic_data_store(generic_data_reader *reader, model *m); 

  //! copy ctor
  generic_data_store(const generic_data_store&) = default;

  //! operator=
  generic_data_store& operator=(const generic_data_store&) = default;

  //! dtor
  virtual ~generic_data_store() {}

  virtual generic_data_store * copy() const = 0;

  /// called by generic_data_reader::setup_data_store
  virtual void setup();

  /// called by generic_data_reader::update
  void set_shuffled_indices(const std::vector<int> *indices);

  /// called by various image data readers 
  virtual void get_data_buf(int data_id, std::vector<unsigned char> *&buf, int multi_idx = 0) {}
  virtual void get_data_buf(int data_id, int tid, std::vector<double> *&buf) {}

  virtual void get_data_buf_DataType(int data_id, std::vector<DataType> *&buf) {}

  const std::string & get_name() const {
    return m_name;
  }

  void set_name(std::string name) {
    m_name = name;
  }

protected :

  bool m_use_two_sided_comms;

  generic_data_reader *m_reader;

  lbann_comm *m_comm;

  std::string m_name;

  /// returns the number of bytes in dir/fn; it's OK if dir = ""
  size_t get_file_size(std::string dir, std::string fn);

  /// number of indices that m_reader owns (in a global sense);
  /// equal to m_shuffled_indices->size()
  size_t m_num_global_indices;

  void set_num_global_indices() {
    m_num_global_indices = m_shuffled_indices->size();
  }

  /// the indices that will be used locally; the inner j-th vector
  /// contains indices referenced during the j-th call to
  /// generic_data_reader::fetch_data(...)
  const std::vector<std::vector<int> > *m_my_minibatch_indices;
  /// contains a concatenation of the indices in m_my_minibatch_indices
  std::vector<int> m_my_minibatch_indices_v;
  /// fills in m_my_minibatch_indices_v
  void get_minibatch_index_vector();

  /// m_mb_counts[j] contains the number of indices
  /// passed to data_reader::fetch_data in one epoch
  std::vector<int> m_mb_counts;
  /// fills in m_mb_counts
  void exchange_mb_counts();

  /// m_all_minibatch_indices[j] will contain all indices that
  /// will be passed to data_reader::fetch_data in one epoch,
  /// for all processors
  std::vector<std::vector<int>> m_all_minibatch_indices;
  /// fills in m_all_minibatch_indices
  void  exchange_mb_indices();

  /// the contents of these vectors change whenever m_shuffled_indices changes
  std::vector<std::unordered_set<int>> m_work_send;
  std::vector<std::unordered_set<int>> m_work_recv;
  /// fills in m_work_send and m_work_recv
  void compute_send_and_receive_lists();

  /// buffers and requests for data exchange using two way comms
  std::vector<std::vector<DataType>> m_sends; //@todo: move to cvs?
  std::vector<std::vector<DataType>> m_recvs; //@todo: move to cvs?
  std::vector<MPI_Request> m_sends_req;
  std::vector<MPI_Request> m_recv_req;
  /// fill in m_sends and m_recvs;
  virtual void start_sends_and_recvs() {}

  /// m_num_samples[j] contains the number of samples (datastore indices)
  /// that are owned by P_j
  std::vector<int> m_num_samples;

  /// the indices that this processor owns;
  std::unordered_set<int> m_my_datastore_indices;
  /// fills in m_my_datastore_indices and m_num_samples
  void get_my_datastore_indices();

  size_t m_num_readers;

  /// this processor's rank
  int  m_rank;

  /// number of procs in the model
  int  m_np;

  size_t m_epoch;

  bool m_in_memory;

  bool m_master;

  const std::vector<int> *m_shuffled_indices;

  model *m_model;

  /// base directory for data
  std::string m_dir;

  /// conduct extensive testing
  bool m_extended_testing;

  bool m_collect_minibatch_indices;

  virtual void exchange_data() = 0;

  /// returns the processor that owns the data associated
  /// with the index
  int get_index_owner(int idx) {
    return idx % m_np;
  }

  virtual void extended_testing() {}

  MPI_Comm m_mpi_comm;
};

}  // namespace lbann

#endif  // __GENERIC_DATA_STORE_HPP__
