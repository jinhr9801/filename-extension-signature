/*
 MIT License

Copyright (c) 2020 HenryKing

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef ff_file_formats_h
#define ff_file_formats_h

#include <stdio.h>
#include <string.h>

typedef enum _FFType {
    FFTypeUnknown = 0,
    
    // DOCUMENT
    FFTypePDF,
    FFTypeDOC, // Microsoft Office document [before 2007]
    FFTypeDOCX, // Microsoft Office document [after 2007]
    
    // IMAGE 1
    FFTypeJPEG,
    FFTypePNG,
    FFTypeWEBP,
    FFTypeGIF,
    FFTypeTIFF,
    FFTypeBMP,
    FFTypeICO,
    FFTypeJ2K, // J2K, JPC, JPX
    FFTypeJP2,
    FFTypeEPS, // EPI, EPSF, EPSI
    FFTypePSD,
    FFTypePSB,
    
    // AUDIO & VIDEO 1
    FFTypeM4A, // AAC [Apple Audio]
    FFTypeM4B, // [Apple Audio Book]
    FFTypeM4P, // [Apple AES Protected Audio]
    FFTypeM4V, // [Apple Video] [Apple TV .M4V (M4VH)] [Apple iPhone .M4V (M4VP)]
    FFTypeMOV, // [Apple QuickTime .MOV, .QT]
    
    FFTypeMP4,
    FFTypeMP3,
    FFTypeMP2,
    
    FFTypeWAV,
    FFTypeAVI,
    FFTypeAIFF,
    FFTypeMID,
    
    FFTypeFLAC,
    FFTypeAPE,
    
    FFTypeCount,
    
    // IMAGE 2
    FFTypeSVG,
    FFTypeDIB,
    FFTypeTGA,
    
    // AUDIO & VIDEO 2
    FFTypeWMA,
    FFTypeWMV,
    FFTypeASF,
    
    FFTypeXCount,
}FFType;

#ifdef __cplusplus
extern "C" {
#endif

FFType ff_get_type_from_file(const char* file_path_and_name);

/*
 read 100 bytes from file with offset 0, as the params to invoke this function
 */
FFType ff_get_type_from_data(unsigned char* binary_data, size_t data_len);

const char* ff_get_ext_name_by_type(FFType type);

#ifdef __cplusplus
}
#endif

#endif /* ff_file_formats_h */
