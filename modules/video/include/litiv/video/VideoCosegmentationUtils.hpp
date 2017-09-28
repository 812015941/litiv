
// This file is part of the LITIV framework; visit the original repository at
// https://github.com/plstcharles/litiv for more information.
//
// Copyright 2015 Pierre-Luc St-Charles; pierre-luc.st-charles<at>polymtl.ca
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "litiv/imgproc/CosegmentationUtils.hpp"

/// super-interface for video cosegmentation algos which exposes common interface functions
template<lv::ParallelAlgoType eImpl, typename TLabel, size_t nInputArraySize, size_t nOutputArraySize=nInputArraySize>
struct IVideoCosegmentor_ : public ICosegmentor_<eImpl,TLabel,nInputArraySize,nOutputArraySize> {
    /// shortcut to input matrix array type
    using FrameArrayIn = std::array<cv::Mat,nInputArraySize>;
    /// shortcut to output matrix array type
    using FrameArrayOut = std::array<cv::Mat_<TLabel>,nOutputArraySize>;
    /// shortcut to ROI matrix array type
    using FrameArrayROI = std::array<cv::Mat_<uint8_t>,nInputArraySize>;
    /// (re)initiaization method; needs to be called before starting cosegmentation (assumes no specific ROI)
    void initialize(const FrameArrayIn& aImages);
    /// (re)initiaization method; needs to be called before starting cosegmentation
    virtual void initialize(const FrameArrayIn& aImages, const FrameArrayROI& aROIs) = 0;
    /// returns the default learning rate value used in 'apply'
    virtual double getDefaultLearningRate() const = 0;
    /// segments the input images based on common visual data and on the temporal model, simultanously updating the latter based on 'dLearningRate'
    virtual void apply(const FrameArrayIn& aImages, FrameArrayOut& aMasks, double dLearningRate=-1) = 0;
    /// turns automatic model reset on or off
    virtual void setAutomaticModelReset(bool);
    /// modifies the given ROIs so they will not cause lookup errors near borders when used in the processing step
    virtual void validateROIs(FrameArrayROI& aROIs) const;
    /// sets the ROIs to be used for input analysis (calls validateROIs internally -- derived method may need to also reset its model)
    virtual void setROIs(FrameArrayROI& aROIs);
    /// returns a copy of the ROIs used for input analysis
    virtual FrameArrayROI getROIsCopy() const;
    /// required for derived class destruction from this interface
    virtual ~IVideoCosegmentor_() = default;

protected:
    /// default impl constructor (for common parameters only -- none must be const to avoid constructor hell when deriving)
    IVideoCosegmentor_();
    /// common (re)initiaization method for all impl types (should be called in impl-specific initialize func)
    virtual void initialize_common(const FrameArrayIn& aImages, const FrameArrayROI& aROIs);
    /// specifies whether the algorithm parameters are fully initialized or not (must be handled by derived class)
    bool m_bInitialized;
    /// specifies whether the model has been fully initialized or not (must be handled by derived class)
    bool m_bModelInitialized;
    /// specifies whether automatic model resets are enabled or not
    bool m_bAutoModelResetEnabled;
    /// minimum image ROI grid border size required by internal cliques & descriptors
    size_t m_nROIBorderSize;
    /// current frame index, frame count since last model reset & model reset cooldown counters
    size_t m_nFrameIdx, m_nFramesSinceLastReset, m_nModelResetCooldown;
    /// model ROI used for input analysis (specific to the input image size)
    std::array<cv::Mat_<uint8_t>,nInputArraySize> m_aROIs;
    /// total number of pixels, and ROI pixels before & after border cleanup for all input images
    std::array<size_t,nInputArraySize> m_anTotPxCounts, m_anOrigROIPxCounts, m_anFinalROIPxCounts;
    /// the segmentation masks generated by the method at [t-1]
    std::array<cv::Mat_<TLabel>,nOutputArraySize> m_aLastMasks;
    /// the input frames processed by the method at [t-1]
    std::array<cv::Mat,nInputArraySize> m_aLastInputs;
};

#if HAVE_CUDA

/// typename shortcut for cuda video cosegm algo impls
template<typename TLabel, size_t nInputArraySize, size_t nOutputArraySize=nInputArraySize>
using IVideoCosegmentor_CUDA = IVideoCosegmentor_<lv::CUDA,TLabel,nInputArraySize,nOutputArraySize>;

#endif //HAVE_CUDA

/// typename shortcut for non-parallel (default) cosegm algo impls
template<typename TLabel, size_t nInputArraySize, size_t nOutputArraySize=nInputArraySize>
using IVideoCosegmentor = IVideoCosegmentor_<lv::NonParallel,TLabel,nInputArraySize,nOutputArraySize>;

#define __LITIV_VIDEOCOSEGM_HPP__
#include "litiv/video/VideoCosegmentationUtils.inl.hpp"
#undef __LITIV_VIDEOCOSEGM_HPP__