/*
 * This file is part of hipSYCL, a SYCL implementation based on CUDA/HIP
 *
 * Copyright (c) 2020 Aksel Alpay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "hipSYCL/sycl/backend.hpp"
#include "hipSYCL/sycl/device.hpp"

#ifdef SYCL_EXT_HIPSYCL_BACKEND_HIP
#include "hipSYCL/runtime/hip/hip_queue.hpp"
#include "hipSYCL/runtime/error.hpp"

#ifndef HIPSYCL_GLUE_HIP_BACKEND_INTEROP_HPP
#define HIPSYCL_GLUE_HIP_BACKEND_INTEROP_HPP

namespace hipsycl {
namespace glue {


template <> struct backend_interop<sycl::backend::hip> {
  
  using error_type = hipError_t;

  using native_mem_type = void *;
  using native_device_type = int;
  using native_queue_type = hipStream_t;

  template <class Accessor_type>
  static native_mem_type get_native_mem(const Accessor_type &a) {
    return a.get_pointer();
  }

  static native_device_type get_native_device(const sycl::device &d) {
    return sycl::detail::extract_rt_device(d).get_id();
  }

#ifdef __HIPSYCL_ENABLE_HIP_TARGET__
  static native_queue_type
  get_native_queue(void *launcher_params) {

    if (!launcher_params) {
      rt::register_error(
          __hipsycl_here(),
          rt::error_info{"Invalid argument to get_native_queue()"});
      
      return native_queue_type{};
    }

    return static_cast<rt::hip_queue*>(launcher_params)->get_stream();
  }

  static native_queue_type
  get_native_queue(rt::device_id dev, rt::backend_executor *executor) {
    rt::multi_queue_executor *mqe =
        dynamic_cast<rt::multi_queue_executor *>(executor);

    if (!mqe) {
      rt::register_error(
          __hipsycl_here(),
          rt::error_info{"Invalid argument to get_native_queue()"});
      return native_queue_type{};
    }

    rt::inorder_queue *q = nullptr;
    mqe->for_each_queue(
        dev, [&](rt::inorder_queue *current_queue) { q = current_queue; });
    assert(q);

    rt::hip_queue *backend_queue = dynamic_cast<rt::hip_queue *>(q);
    assert(backend_queue);

    return backend_queue->get_stream();
  }
#endif

  static sycl::device make_sycl_device(int device_id) {
    return sycl::device{
        rt::device_id{rt::backend_descriptor{rt::hardware_platform::rocm,
                                             rt::api_platform::hip},
                      device_id}};
  }

  static constexpr bool can_make_platform = false;
  static constexpr bool can_make_device = true;
  static constexpr bool can_make_context = false;
  static constexpr bool can_make_queue = false;
  static constexpr bool can_make_event = false;
  static constexpr bool can_make_buffer = false;
  static constexpr bool can_make_sampled_image = false;
  static constexpr bool can_make_image_sampler = false;
  static constexpr bool can_make_stream = false;
  static constexpr bool can_make_kernel = false;
  static constexpr bool can_make_module = false;

  static constexpr bool can_extract_native_platform = false;
  static constexpr bool can_extract_native_device = true;
  static constexpr bool can_extract_native_context = false;
  static constexpr bool can_extract_native_queue = false;
  static constexpr bool can_extract_native_event = false;
  static constexpr bool can_extract_native_buffer = false;
  static constexpr bool can_extract_native_sampled_image = false;
  static constexpr bool can_extract_native_image_sampler = false;
  static constexpr bool can_extract_native_stream = false;
  static constexpr bool can_extract_native_kernel = false;
  static constexpr bool can_extract_native_module = false;
  static constexpr bool can_extract_native_device_event = false;
  static constexpr bool can_extract_native_mem = true;
};

}
}

#endif
#endif
