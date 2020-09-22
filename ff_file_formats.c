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

typedef struct _FFFeature {
    size_t  offset;
    unsigned char need;
    unsigned char value;
}FFFeature;

typedef struct _FFFormat {
    const char ext[FF_MAX_EXT_LEN];
    size_t optional_need;
    
    size_t feature_count;
    const FFFeature* features;
}FFFormat;

extern const FFFormat g_ff_formats[FFTypeXCount];

//------------------------------------------------------------------------------------------------------

static int _ff_check_features(unsigned char* binary_data, size_t data_len, const FFFormat* cur_format)
{
    size_t match_optional_count = 0;
    
    size_t feature_count = cur_format->feature_count;
    const FFFeature* cur_feature = NULL;
    
    for (size_t j = 0; j < feature_count; j++) {
        cur_feature = cur_format->features + j;
        
        if (data_len < cur_feature->offset + 1) {
            goto end;
        }
        
        if (binary_data[cur_feature->offset] != cur_feature->value) {
            goto end;
        }
       
        if (cur_feature->need != 1) {
            match_optional_count++;
        }
        continue;
        
    end:
        if (cur_feature->need == 1) {
            return 0;
        }
    } // features
    
    if (match_optional_count >= cur_format->optional_need) {
        return 1;
    }
    
    return 0;
}

FFType ff_get_type_from_file(const char* file_path_and_name)
{
    size_t len = strlen(file_path_and_name);
    if (len < 1) {
        return FFTypeUnknown;
    }
    
    size_t min_i = len > FF_MAX_EXT_LEN ? len - FF_MAX_EXT_LEN - 1 : 0;
    
    char ext[FF_MAX_EXT_LEN] = { 0 };
    for (size_t i = len; i > min_i; i--) {
        if (file_path_and_name[i] == '.') {
            strcpy(ext, file_path_and_name + i + 1);
            break;
        }
    }
    
    FFType type = FFTypeUnknown;
    const FFFormat* cur_format = NULL;
    
    if (ext[0] != '\0') {
        char c = 0;
        for (size_t i = 0; i < FF_MAX_EXT_LEN && ext[0] != '\0'; i++) {
            c = ext[i];
            if (c >= 'a' && c <= 'z') {
                ext[i] = c - ('a' - 'A');
            }
        }
        
        for (size_t i = 1; i < FFTypeXCount; i++) {
            cur_format = g_ff_formats + i;
            if (0 == strcmp(ext, cur_format->ext)) {
                if (i > FFTypeCount && cur_format->feature_count == 0) {
                    return (FFType)i;
                }
                
                type = (FFType)i;
                break;
            }
            cur_format = NULL;
        }
    }
    
    FILE* file = fopen(file_path_and_name, "r");
    if (file == NULL) {
        printf("Fail to open the file: %s!\n", file_path_and_name);
        return 0;
    }

    unsigned char binary_data[100] = { 0 };
    size_t sz = fread(binary_data, 1, sizeof(binary_data), file);
    if (sz > 0) {
        if (cur_format == NULL || 1 != _ff_check_features(binary_data, sz, cur_format)) {
            type = ff_get_type_from_data(binary_data, sz);
        }
    }
    
    fclose(file);
    return type;
}

FFType ff_get_type_from_data(unsigned char* binary_data, size_t data_len)
{
    const FFFormat* cur_format = NULL;
    for (size_t i = 1; i < FFTypeCount; i++) {
        cur_format = g_ff_formats + i;
        
        if (1 == _ff_check_features(binary_data, data_len, cur_format)) {
            return (FFType)i;
        }
    }
    
    return FFTypeUnknown;
}

const char* ff_get_ext_name_by_type(FFType type)
{
    if ((int)type < 0 || (int)type >= FFTypeXCount) {
        return g_ff_formats[0].ext;
    }
    return g_ff_formats[(int)type].ext;
}

//------------------------------------------------------------------------------------------------------
// DOCUMENT
const FFFeature g_ff_pdf[] = {
    {0, 1, 0x25},
    {1, 1, 0x50},
    {2, 1, 0x44},
    {3, 1, 0x46},
    {4, 1, 0x2D},
};

//------------------------------------------------------------------------------------------------------
// IMAGE 1
const FFFeature g_ff_jpeg[] = {
    {0, 1, 0xFF},
    {1, 1, 0xD8},
    {2, 1, 0xFF},
    {3, 1, 0xE0},
};

const FFFeature g_ff_png[] = {
    {0, 1, 0x89},
    {1, 1, 0x50},
    {2, 1, 0x4E},
    {3, 1, 0x47},
    {4, 1, 0x0D},
    {5, 1, 0x0A},
    {6, 1, 0x1A},
    {7, 1, 0x0A},
};

const FFFeature g_ff_webp[] = {
    {0, 1, 0x52},
    {1, 1, 0x49},
    {2, 1, 0x46},
    {3, 1, 0x46},
    {8, 1, 0x57},
    {9, 1, 0x45},
    {10, 1, 0x42},
    {11, 1, 0x50},
};

const FFFeature g_ff_gif[] = {
    {0, 1, 0x47},
    {1, 1, 0x49},
    {2, 1, 0x46},
};

const FFFeature g_ff_tiff[] = {
    {0, 0, 0x4D},
    {1, 0, 0x4D},
    {2, 0, 0x00},
    {3, 0, 0x2A},
    
    {0, 0, 0x49},
    {1, 0, 0x49},
    {2, 0, 0x2A},
    {3, 0, 0x00},
};

const FFFeature g_ff_bmp[] = {
    {0, 1, 0x42},
    {1, 1, 0x4D},
};

const FFFeature g_ff_ico[] = {
    {0, 1, 0x00},
    {1, 1, 0x00},
    {2, 0, 0x01},
    {3, 1, 0x00},
    
    {2, 0, 0x02},
};

const FFFeature g_ff_j2k[] = {
    {0, 1, 0xFF},
    {1, 1, 0x4F},
    {2, 1, 0xFF},
    {3, 1, 0x51},
};

const FFFeature g_ff_jp2[] = {
    {4, 1, 0x6A},
    {5, 1, 0x50},
    {7, 1, 0x20},
    {8, 1, 0x0D},
    {9, 1, 0x0A},
    {10, 1, 0x87},
    {11, 1, 0x0A},
};

const FFFeature g_ff_eps[] = {
    {0, 1, 0x25},
    {1, 1, 0x21},
    {2, 1, 0x50},
    {3, 1, 0x53},
    {4, 1, 0x2D},
    {5, 1, 0x41},
    {6, 1, 0x64},
    {7, 1, 0x6F},
};

const FFFeature g_ff_psd[] = {
    {0, 1, 0x38},
    {1, 1, 0x42},
    {2, 1, 0x50},
    {3, 1, 0x53},
    {5, 1, 0x01},
};

const FFFeature g_ff_psb[] = {
    {0, 1, 0x38},
    {1, 1, 0x42},
    {2, 1, 0x50},
    {3, 1, 0x53},
    {5, 1, 0x02},
};

//------------------------------------------------------------------------------------------------------
// IMAGE 2
const FFFeature g_ff_svg[] = {
    {0, 1, 0x3C},
    {1, 1, 0x3F},
    {2, 1, 0x78},
    {3, 1, 0x6D},
    {4, 1, 0x6C},
    {5, 1, 0x20},
};

//------------------------------------------------------------------------------------------------------

const FFFormat g_ff_formats[FFTypeXCount] = {
    {"", 0, 0, NULL}, // Unknown
    
    // DOCUMENT
    {"PDF", 0, sizeof(g_ff_pdf)/sizeof(FFFeature), g_ff_pdf},
    
    // IMAGE 1
    {"JPEG", 0, sizeof(g_ff_jpeg)/sizeof(FFFeature), g_ff_jpeg},
    {"PNG", 0, sizeof(g_ff_png)/sizeof(FFFeature), g_ff_png},
    {"WEBP", 0, sizeof(g_ff_webp)/sizeof(FFFeature), g_ff_webp},
    {"GIF", 0, sizeof(g_ff_gif)/sizeof(FFFeature), g_ff_gif},
    {"TIFF", 4, sizeof(g_ff_tiff)/sizeof(FFFeature), g_ff_tiff},
    {"BMP", 0, sizeof(g_ff_bmp)/sizeof(FFFeature), g_ff_bmp},
    {"ICO", 1, sizeof(g_ff_ico)/sizeof(FFFeature), g_ff_ico},
    {"J2K", 0, sizeof(g_ff_j2k)/sizeof(FFFeature), g_ff_j2k},
    {"JP2", 0, sizeof(g_ff_jp2)/sizeof(FFFeature), g_ff_jp2},
    {"EPS", 0, sizeof(g_ff_eps)/sizeof(FFFeature), g_ff_eps},
    {"PSD", 0, sizeof(g_ff_psd)/sizeof(FFFeature), g_ff_psd},
    {"PSB", 0, sizeof(g_ff_psb)/sizeof(FFFeature), g_ff_psb},
    
    {"", 0, 0, NULL}, // pading
    
    // IMAGE 2
    {"SVG", 0, sizeof(g_ff_svg)/sizeof(FFFeature), g_ff_svg},
    {"DIB", 0, 0, NULL},
    {"TGA", 0, 0, NULL},
};

//------------------------------------------------------------------------------------------------------
