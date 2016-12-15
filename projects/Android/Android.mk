# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPPFLAGS := -DNOMINMAX -D__ANDROID__
LOCAL_MODULE    := rxcpp
LOCAL_LDLIBS	:= -llog

ifeq ($(TARGET_ARCH),armeabi-v7a)
    LOCAL_CPPFLAGS  += -D_XM_ARM_NEON_INTRINSICS_ -D_M_ARM -D__ARM_NEON__
    LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/../../Rx

LOCAL_SRC_FILES := \
   ../../Rx/v2/src/rxcpp/operators/rx-all.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-amb.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-any.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-buffer_count.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-buffer_time.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-buffer_time_count.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-combine_latest.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-concat.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-concat_map.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-connect_forever.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-debounce.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-delay.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-distinct.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-distinct_until_changed.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-element_at.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-filter.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-finally.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-flat_map.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-group_by.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-ignore_elements.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-lift.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-map.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-merge.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-multicast.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-observe_on.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-on_error_resume_next.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-pairwise.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-publish.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-reduce.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-ref_count.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-repeat.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-replay.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-retry.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-sample_time.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-scan.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-sequence_equal.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-skip.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-skip_last.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-skip_until.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-start_with.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-subscribe.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-subscribe_on.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-switch_if_empty.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-switch_on_next.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-take.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-take_last.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-take_until.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-tap.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-time_interval.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-timeout.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-timestamp.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-with_latest_from.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-window.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-window_time.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-window_time_count.hpp \
   ../../Rx/v2/src/rxcpp/operators/rx-zip.hpp \
   ../../Rx/v2/src/rxcpp/rx-connectable_observable.hpp \
   ../../Rx/v2/src/rxcpp/rx-coordination.hpp \
   ../../Rx/v2/src/rxcpp/rx-grouped_observable.hpp \
   ../../Rx/v2/src/rxcpp/rx-includes.hpp \
   ../../Rx/v2/src/rxcpp/rx-notification.hpp \
   ../../Rx/v2/src/rxcpp/rx-observable.hpp \
   ../../Rx/v2/src/rxcpp/rx-observer.hpp \
   ../../Rx/v2/src/rxcpp/rx-operators.hpp \
   ../../Rx/v2/src/rxcpp/rx-predef.hpp \
   ../../Rx/v2/src/rxcpp/rx-scheduler.hpp \
   ../../Rx/v2/src/rxcpp/rx-sources.hpp \
   ../../Rx/v2/src/rxcpp/rx-subjects.hpp \
   ../../Rx/v2/src/rxcpp/rx-subscriber.hpp \
   ../../Rx/v2/src/rxcpp/rx-subscription.hpp \
   ../../Rx/v2/src/rxcpp/rx-test.hpp \
   ../../Rx/v2/src/rxcpp/rx-trace.hpp \
   ../../Rx/v2/src/rxcpp/rx-util.hpp \
   ../../Rx/v2/src/rxcpp/rx.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-currentthread.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-eventloop.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-immediate.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-newthread.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-runloop.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-sameworker.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-test.hpp \
   ../../Rx/v2/src/rxcpp/schedulers/rx-virtualtime.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-create.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-defer.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-error.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-interval.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-iterate.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-never.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-range.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-scope.hpp \
   ../../Rx/v2/src/rxcpp/sources/rx-timer.hpp \
   ../../Rx/v2/src/rxcpp/subjects/rx-behavior.hpp \
   ../../Rx/v2/src/rxcpp/subjects/rx-replaysubject.hpp \
   ../../Rx/v2/src/rxcpp/subjects/rx-subject.hpp \
   ../../Rx/v2/src/rxcpp/subjects/rx-synchronize.hpp
include $(BUILD_STATIC_LIBRARY)
