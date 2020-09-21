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

#include "ff_file_formats.h"

#define FF_MAX_EXT_LEN  8
#define FF_TYPES_COUNT  10

extern const char g_ff_types[FF_TYPES_COUNT][FF_MAX_EXT_LEN];

typedef struct _FFFeature {
    size_t  offset;
    unsigned char need;
    unsigned char value;
}FFFeature;

typedef struct _FFFormat {
    FFType type;
    size_t feature_count;
    const FFFeature features[];
}FFFormat;

extern const FFFormat g_ff_formats[FF_TYPES_COUNT];

//------------------------------------------------------------------------------------------------------

FFType ff_get_type_from_data(unsigned char* binary_data, size_t data_len)
{
    size_t feature_count = 0;
    
    const FFFormat* cur_format = g_ff_formats;
    const FFFeature* cur_feature = NULL;
    
    for (size_t i = 0; i < FF_TYPES_COUNT; i++) {
        feature_count = cur_format->feature_count;
        cur_feature = cur_format->features;
        
        for (size_t j = 0; j < feature_count; j++) {
            if (data_len < cur_feature->offset + 1) {
                goto end;
            }
            
            if (binary_data[cur_feature->offset] != cur_feature->value) {
                goto end;
            }
           
            continue;
            
        end:
            if (cur_feature->need == 1) {
                break;
            }
        }
    }
    
    return FFTypeUnknown;
}

const char* ff_get_ext_name_by_type(FFType type)
{
    if ((int)type < 0 || (int)type > FF_TYPES_COUNT) {
        return g_ff_types[0];
    }
    return g_ff_types[(int)type];
}

//------------------------------------------------------------------------------------------------------

const char g_ff_types[FF_TYPES_COUNT][FF_MAX_EXT_LEN] = {
    "",
    "JPEG",
    "PNG",
};

const FFFormat g_ff_formats[FF_TYPES_COUNT] = {
    
};
