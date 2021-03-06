/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "OpenGLRenderer"

#include "Debug.h"
#include "GammaFontRenderer.h"
#include "Properties.h"

namespace android {
namespace uirenderer {

///////////////////////////////////////////////////////////////////////////////
// Constructors/destructor
///////////////////////////////////////////////////////////////////////////////

GammaFontRenderer::GammaFontRenderer() {
    INIT_LOGD("Creating gamma font renderer");

    // Get the renderer properties
    char property[PROPERTY_VALUE_MAX];

    // Get the gamma
    float gamma = DEFAULT_TEXT_GAMMA;
    if (property_get(PROPERTY_TEXT_GAMMA, property, NULL) > 0) {
        INIT_LOGD("  Setting text gamma to %s", property);
        gamma = atof(property);
    } else {
        INIT_LOGD("  Using default text gamma of %.2f", DEFAULT_TEXT_GAMMA);
    }

    // Get the black gamma threshold
    mBlackThreshold = DEFAULT_TEXT_BLACK_GAMMA_THRESHOLD;
    if (property_get(PROPERTY_TEXT_BLACK_GAMMA_THRESHOLD, property, NULL) > 0) {
        INIT_LOGD("  Setting text black gamma threshold to %s", property);
        mBlackThreshold = atoi(property);
    } else {
        INIT_LOGD("  Using default text black gamma threshold of %d",
                DEFAULT_TEXT_BLACK_GAMMA_THRESHOLD);
    }

    // Get the white gamma threshold
    mWhiteThreshold = DEFAULT_TEXT_WHITE_GAMMA_THRESHOLD;
    if (property_get(PROPERTY_TEXT_WHITE_GAMMA_THRESHOLD, property, NULL) > 0) {
        INIT_LOGD("  Setting text white gamma threshold to %s", property);
        mWhiteThreshold = atoi(property);
    } else {
        INIT_LOGD("  Using default white black gamma threshold of %d",
                DEFAULT_TEXT_WHITE_GAMMA_THRESHOLD);
    }

    // Compute the gamma tables
    const float blackGamma = gamma;
    const float whiteGamma = 1.0f / gamma;

    for (uint32_t i = 0; i <= 255; i++) {
        mDefault[i] = i;

        const float v = i / 255.0f;
        const float black = pow(v, blackGamma);
        const float white = pow(v, whiteGamma);

        mBlackGamma[i] = uint8_t((float)::floor(black * 255.0f + 0.5f));
        mWhiteGamma[i] = uint8_t((float)::floor(white * 255.0f + 0.5f));
    }

    // Configure the font renderers
    mDefaultRenderer.setGammaTable(&mDefault[0]);
    mBlackGammaRenderer.setGammaTable(&mBlackGamma[0]);
    mWhiteGammaRenderer.setGammaTable(&mWhiteGamma[0]);
}

FontRenderer& GammaFontRenderer::getFontRenderer(const SkPaint* paint) {
    if (paint->getShader() == NULL) {
        uint32_t c = paint->getColor();
        const int r = (c >> 16) & 0xFF;
        const int g = (c >>  8) & 0xFF;
        const int b = (c      ) & 0xFF;
        const int luminance = (r * 2 + g * 5 + b) >> 3;

        if (luminance <= mBlackThreshold) {
            return mBlackGammaRenderer;
        } else if (luminance >= mWhiteThreshold) {
            return mWhiteGammaRenderer;
        }
    }
    return mDefaultRenderer;
}

}; // namespace uirenderer
}; // namespace android
