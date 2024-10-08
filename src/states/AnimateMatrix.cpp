// MIT License
//
// Copyright 2024 Eurofurence e.V. 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @author Honigeintopf
 */

#include <EFLed.h>
#include <EFLogging.h>
#include <EFPrideFlags.h>

#include "FSMState.h"


const char* AnimateMatrix::getName() {
    return "AnimateMatrix";
}

bool AnimateMatrix::shouldBeRemembered() {
    return true;
}

const unsigned int AnimateMatrix::getTickRateMs() {
    return 100;
}

void AnimateMatrix::entry() {
    this->tick = 0;
}

void AnimateMatrix::run() {
    // Prepare base pattern
    std::vector<CRGB> dragon = {
        CRGB::Black,
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black
    };

    std::vector<CRGB> bar = {
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black,
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
    };

    // Calculate current pattern based on tick
    std::rotate(dragon.begin(), dragon.begin() + this->tick % EFLED_DRAGON_NUM, dragon.end());
    std::rotate(bar.rbegin(), bar.rbegin() + this->tick % EFLED_EFBAR_NUM, bar.rend());

    dragon.insert(dragon.end(), bar.begin(), bar.end());
    EFLed.setAll(dragon.data());

    // Prepare next tick
    this->tick++;
}

std::unique_ptr<FSMState> AnimateMatrix::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}
