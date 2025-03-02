////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2022, Lawrence Livermore National Security, LLC.
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
////////////////////////////////////////////////////////////////////////////////
// MUST include this
#include "Catch2BasicSupport.hpp"

// File being tested
#include <lbann/transforms/vision/random_resized_crop_with_fixed_aspect_ratio.hpp>
#include <lbann/transforms/vision/resize.hpp>
#include <lbann/transforms/vision/random_crop.hpp>
#include <lbann/utils/random_number_generators.hpp>
#include "helper.hpp"

TEST_CASE("Testing random resized crop with fixed aspect ratio preprocessing", "[preproc]") {
  lbann::utils::type_erased_matrix mat = lbann::utils::type_erased_matrix(El::Matrix<uint8_t>());
  // Grab the necessary I/O RNG and lock it
  lbann::locked_io_rng_ref io_rng = lbann::set_io_generators_local_index(0);

  SECTION("matrix with one channel") {
    ones(mat.template get<uint8_t>(), 5, 5, 1);
    std::vector<size_t> dims = {1, 5, 5};

    SECTION("resizing larger and cropping") {
      auto resize_cropper = lbann::transform::random_resized_crop_with_fixed_aspect_ratio(7, 7, 3, 3);

      SECTION("applying the resize/crop") {
        REQUIRE_NOTHROW(resize_cropper.apply(mat, dims));

        SECTION("resizing/cropping changes dims correctly") {
          REQUIRE(dims[0] == 1);
          REQUIRE(dims[1] == 3);
          REQUIRE(dims[2] == 3);
        }
        SECTION("resizing/cropping does not change matrix type") {
          REQUIRE_NOTHROW(mat.template get<uint8_t>());
        }
        SECTION("resizing/cropping produces correct values") {
          auto& real_mat = mat.template get<uint8_t>();
          apply_elementwise(
            real_mat, 3, 3, 1,
            [](uint8_t& x, El::Int row, El::Int col, El::Int) {
              REQUIRE(x == 1);
            });
        }

        SECTION("compare with resize then crop") {
          lbann::utils::type_erased_matrix mat2 =
            lbann::utils::type_erased_matrix(El::Matrix<uint8_t>());
          ones(mat2.template get<uint8_t>(), 5, 5, 1);
          std::vector<size_t> dims2 = {1, 5, 5};
          auto resizer = lbann::transform::resize(7, 7);
          auto cropper = lbann::transform::random_crop(3, 3);
          REQUIRE_NOTHROW(resizer.apply(mat2, dims2));
          REQUIRE_NOTHROW(cropper.apply(mat2, dims2));
          REQUIRE(dims == dims2);
          const uint8_t* buf = mat.template get<uint8_t>().LockedBuffer();
          const uint8_t* buf2 = mat2.template get<uint8_t>().LockedBuffer();
          for (size_t i = 0; i < dims2[1]*dims2[2]; ++i) {
            REQUIRE(buf[i] == buf2[i]);
          }
        }
      }
    }
    SECTION("resizing smaller and cropping") {
      auto resize_cropper = lbann::transform::random_resized_crop_with_fixed_aspect_ratio(3, 3, 1, 1);

      SECTION("applying the resize/crop") {
        REQUIRE_NOTHROW(resize_cropper.apply(mat, dims));

        SECTION("resizing/cropping changes dims correctly") {
          REQUIRE(dims[0] == 1);
          REQUIRE(dims[1] == 1);
          REQUIRE(dims[2] == 1);
        }
        SECTION("resizing/cropping does not change matrix type") {
          REQUIRE_NOTHROW(mat.template get<uint8_t>());
        }
        SECTION("resizing/cropping produces correct values") {
          auto& real_mat = mat.template get<uint8_t>();
          apply_elementwise(
            real_mat, 1, 1, 1,
            [](uint8_t& x, El::Int row, El::Int col, El::Int) {
              REQUIRE(x == 1);
            });
        }

        SECTION("compare with resize then crop") {
          lbann::utils::type_erased_matrix mat2 =
            lbann::utils::type_erased_matrix(El::Matrix<uint8_t>());
          ones(mat2.template get<uint8_t>(), 5, 5, 1);
          std::vector<size_t> dims2 = {1, 5, 5};
          auto resizer = lbann::transform::resize(3, 3);
          auto cropper = lbann::transform::random_crop(1, 1);
          REQUIRE_NOTHROW(resizer.apply(mat2, dims2));
          REQUIRE_NOTHROW(cropper.apply(mat2, dims2));
          REQUIRE(dims == dims2);
          const uint8_t* buf = mat.template get<uint8_t>().LockedBuffer();
          const uint8_t* buf2 = mat2.template get<uint8_t>().LockedBuffer();
          for (size_t i = 0; i < dims2[1]*dims2[2]; ++i) {
            REQUIRE(buf[i] == buf2[i]);
          }
        }
      }
    }
  }

  SECTION("matrix with three channels") {
    ones(mat.template get<uint8_t>(), 5, 5, 3);
    std::vector<size_t> dims = {3, 5, 5};

    SECTION("resizing larger and cropping") {
      auto resize_cropper = lbann::transform::random_resized_crop_with_fixed_aspect_ratio(7, 7, 3, 3);

      SECTION("applying the resize/crop") {
        REQUIRE_NOTHROW(resize_cropper.apply(mat, dims));

        SECTION("resizing/cropping changes dims correctly") {
          REQUIRE(dims[0] == 3);
          REQUIRE(dims[1] == 3);
          REQUIRE(dims[2] == 3);
        }
        SECTION("resizing/cropping does not change matrix type") {
          REQUIRE_NOTHROW(mat.template get<uint8_t>());
        }
        SECTION("resizing/cropping produces correct values") {
          auto& real_mat = mat.template get<uint8_t>();
          apply_elementwise(
            real_mat, 3, 3, 3,
            [](uint8_t& x, El::Int row, El::Int col, El::Int) {
              REQUIRE(x == 1);
            });
        }

        SECTION("compare with resize then crop") {
          lbann::utils::type_erased_matrix mat2 =
            lbann::utils::type_erased_matrix(El::Matrix<uint8_t>());
          ones(mat2.template get<uint8_t>(), 5, 5, 3);
          std::vector<size_t> dims2 = {3, 5, 5};
          auto resizer = lbann::transform::resize(7, 7);
          auto cropper = lbann::transform::random_crop(3, 3);
          REQUIRE_NOTHROW(resizer.apply(mat2, dims2));
          REQUIRE_NOTHROW(cropper.apply(mat2, dims2));
          REQUIRE(dims == dims2);
          const uint8_t* buf = mat.template get<uint8_t>().LockedBuffer();
          const uint8_t* buf2 = mat2.template get<uint8_t>().LockedBuffer();
          for (size_t i = 0; i < dims2[1]*dims2[2]; ++i) {
            REQUIRE(buf[i] == buf2[i]);
          }
        }
      }
    }
    SECTION("resizing smaller and cropping") {
      auto resize_cropper = lbann::transform::random_resized_crop_with_fixed_aspect_ratio(3, 3, 1, 1);

      SECTION("applying the resize/crop") {
        REQUIRE_NOTHROW(resize_cropper.apply(mat, dims));

        SECTION("resizing/cropping changes dims correctly") {
          REQUIRE(dims[0] == 3);
          REQUIRE(dims[1] == 1);
          REQUIRE(dims[2] == 1);
        }
        SECTION("resizing/cropping does not change matrix type") {
          REQUIRE_NOTHROW(mat.template get<uint8_t>());
        }
        SECTION("resizing/cropping produces correct values") {
          auto& real_mat = mat.template get<uint8_t>();
          apply_elementwise(
            real_mat, 1, 1, 3,
            [](uint8_t& x, El::Int row, El::Int col, El::Int) {
              REQUIRE(x == 1);
            });
        }

        SECTION("compare with resize then crop") {
          lbann::utils::type_erased_matrix mat2 =
            lbann::utils::type_erased_matrix(El::Matrix<uint8_t>());
          ones(mat2.template get<uint8_t>(), 5, 5, 3);
          std::vector<size_t> dims2 = {3, 5, 5};
          auto resizer = lbann::transform::resize(3, 3);
          auto cropper = lbann::transform::random_crop(1, 1);
          REQUIRE_NOTHROW(resizer.apply(mat2, dims2));
          REQUIRE_NOTHROW(cropper.apply(mat2, dims2));
          REQUIRE(dims == dims2);
          const uint8_t* buf = mat.template get<uint8_t>().LockedBuffer();
          const uint8_t* buf2 = mat2.template get<uint8_t>().LockedBuffer();
          for (size_t i = 0; i < dims2[1]*dims2[2]; ++i) {
            REQUIRE(buf[i] == buf2[i]);
          }
        }
      }
    }
  }
}
