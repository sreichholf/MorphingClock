/*
 * SmartMatrix Library - Multiplexed Panel Refresh Class
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SmartMatrixCoprocessorSend_h
#define SmartMatrixCoprocessorSend_h

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3CoprocessorSend {
public:
    struct timerpair {
        uint16_t timer_oe;
        uint16_t timer_period;
    };

#ifndef ADDX_UPDATE_ON_DATA_PINS
    struct addresspair {
        uint16_t bits_to_clear;
        uint16_t bits_to_set;
    };

    // gpiopair is 2x uint32_t to match size and spacing of values it is updating: GPIOx_PSOR and GPIOx_PCOR are 32-bit and adjacent to each other
    struct gpiopair {
        uint32_t  gpio_psor;
        uint32_t  gpio_pcor;
    };
#endif

#define PACKED_HUB75_FORMAT_PIXELS_PER_UNIT     4
#define PACKED_HUB75_FORMAT_BYTES_PER_UNIT      3

    struct rowBitStruct {
        uint8_t data[((((matrixWidth * matrixHeight) / CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType)) * PACKED_HUB75_FORMAT_BYTES_PER_UNIT)/PACKED_HUB75_FORMAT_PIXELS_PER_UNIT)];
    };

    struct rowDataStruct {
        uint8_t startOfFrameByte;
        uint8_t refreshRate;
        uint8_t brightness;
        uint8_t rowAddress;
        uint8_t latchesPerRow;
        uint8_t dataWidth;
        rowBitStruct rowbits[refreshDepth/COLOR_CHANNELS_PER_PIXEL];
        uint8_t endOfFrameByte;
    };

    typedef void (*matrix_underrun_callback)(void);
    typedef void (*matrix_calc_callback)(bool initial);

    // init
    SmartMatrix3CoprocessorSend(uint8_t bufferrows, rowDataStruct * rowDataBuffer);
    static void begin(void);

    // refresh API
    static rowDataStruct * getNextRowBufferPtr(void);
    static void writeRowBuffer(uint8_t currentRow);
    static void recoverFromDmaUnderrun(void);
    static bool isRowBufferFree(void);
    static void setRefreshRate(uint8_t newRefreshRate);
    static void setBrightness(uint8_t newBrightness);
    static void setMatrixCalculationsCallback(matrix_calc_callback f);
    static void setMatrixUnderrunCallback(matrix_underrun_callback f);

private:
    // enable ISR access to private member variables
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void coprocessorSendRowCalculationISR(void);
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void coprocessorSendRowShiftCompleteISR(void);

    // configuration helper functions
    static void calculateTimerLUT(void);

    static int dimmingFactor;
    static const int dimmingMaximum = 255;
    static uint16_t rowBitStructBytesToShift;
    static uint8_t refreshRate;
    static uint8_t dmaBufferNumRows;
    static rowDataStruct * matrixUpdateRows;

    static timerpair timerLUT[LATCHES_PER_ROW];
    static timerpair timerPairIdle;
#ifndef ADDX_UPDATE_ON_DATA_PINS
    static addresspair addressLUT[MATRIX_SCAN_MOD];
    static gpiopair gpiosync;
#endif
    static matrix_calc_callback matrixCalcCallback;
    static matrix_underrun_callback matrixUnderrunCallback;

    static CircularBuffer dmaBuffer;
};

#endif
