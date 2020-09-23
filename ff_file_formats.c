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

#define FF_MAX_EXT_LEN  5
#define FF_MAX_OPTIONAL_COUNT 10   // max 255
#define FF_NEED     (FF_MAX_OPTIONAL_COUNT + 1)

typedef struct _FFFeature {
    size_t  offset;
    unsigned char need;
    unsigned char value;
}FFFeature;

typedef struct _FFFormat {
    const char ext[FF_MAX_EXT_LEN];

    size_t feature_count;
    const FFFeature* features;
}FFFormat;

extern const FFFormat g_ff_formats[FFTypeXCount];

//------------------------------------------------------------------------------------------------------

// return 0 : false; 1 : true
static int _ff_check_features(unsigned char* binary_data, size_t data_len, const FFFormat* cur_format)
{
    int max_index = -1;
    int fail_index = -1;
    int optional_count[FF_MAX_OPTIONAL_COUNT] = { 0 };
    
    size_t feature_count = cur_format->feature_count;
    const FFFeature* cur_feature = NULL;
    
    for (size_t j = 0; j < feature_count; j++) {
        cur_feature = cur_format->features + j;
        
        // store optional key
        if (cur_feature->need != FF_NEED) {
            if (cur_feature->need == fail_index) {
                continue;
            }
            
            if (cur_feature->need > max_index) {
                max_index = cur_feature->need;
            }
            
//            assert(max_index < FF_MAX_OPTIONAL_COUNT);
            optional_count[cur_feature->need]++;
        }
        //---------------------
        
        if (data_len < cur_feature->offset + 1) {
            goto end;
        }
        
        if (binary_data[cur_feature->offset] != cur_feature->value) {
            goto end;
        }
       
        if (cur_feature->need != FF_NEED) {
            optional_count[cur_feature->need]--;
        }
        continue;
        
    end:
        if (cur_feature->need == FF_NEED) {
            return 0;
        } else {
            fail_index = cur_feature->need;
        }
    } // features
    
    if (max_index >= 0) {
        for (int i = 0; i < max_index + 1; i++) {
            if (optional_count[i] == 0) {
                return 1;
            }
        }
        return 0;
    }
    return 1;
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
    {4, FF_NEED, 0x2D},
    
    {0, FF_NEED, 0x25},
    {1, FF_NEED, 0x50},
    {2, FF_NEED, 0x44},
    {3, FF_NEED, 0x46},
};

const FFFeature g_ff_doc[] = {
    {0, FF_NEED, 0xD0},
    {1, FF_NEED, 0xCF},
    {2, FF_NEED, 0x11},
    {3, FF_NEED, 0xE0},
    
    {4, FF_NEED, 0xA1},
    {5, FF_NEED, 0xB1},
    {6, FF_NEED, 0x1A},
    {7, FF_NEED, 0xE1},
};

const FFFeature g_ff_docx[] = {
    {4, FF_NEED, 0x14},
    {5, FF_NEED, 0x00},
    {6, 0, 0x06},
    {6, 1, 0x00},
    {7, FF_NEED, 0x00},
    
    {0, FF_NEED, 0x50},
    {1, FF_NEED, 0x4B},
    {2, FF_NEED, 0x03},
    {3, FF_NEED, 0x04},
};

//------------------------------------------------------------------------------------------------------
// IMAGE 1
const FFFeature g_ff_jpeg[] = {
    {0, FF_NEED, 0xFF},
    {1, FF_NEED, 0xD8},
    {2, FF_NEED, 0xFF},
    {3, 0, 0xE0},
    {3, 1, 0xE1},
    {3, 2, 0xE2},
    {3, 3, 0xE3},
    {3, 8, 0xE8},
};

const FFFeature g_ff_png[] = {
    {0, FF_NEED, 0x89},
    {1, FF_NEED, 0x50},
    {2, FF_NEED, 0x4E},
    {3, FF_NEED, 0x47},
    {4, FF_NEED, 0x0D},
    {5, FF_NEED, 0x0A},
    {6, FF_NEED, 0x1A},
    {7, FF_NEED, 0x0A},
};

const FFFeature g_ff_webp[] = {
    {0, FF_NEED, 0x52},
    {1, FF_NEED, 0x49},
    {2, FF_NEED, 0x46},
    {3, FF_NEED, 0x46},
    
    {8, FF_NEED, 0x57},
    {9, FF_NEED, 0x45},
    {10, FF_NEED, 0x42},
    {11, FF_NEED, 0x50},
};

const FFFeature g_ff_gif[] = {
    {0, FF_NEED, 0x47},
    {1, FF_NEED, 0x49},
    {2, FF_NEED, 0x46},
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
    {0, FF_NEED, 0x42},
    {1, FF_NEED, 0x4D},
};

const FFFeature g_ff_ico[] = {
    {0, FF_NEED, 0x00},
    {1, FF_NEED, 0x00},
    {2, 0, 0x01},
    {2, 1, 0x02},
    {3, FF_NEED, 0x00},
};

const FFFeature g_ff_j2k[] = {
    {0, FF_NEED, 0xFF},
    {1, FF_NEED, 0x4F},
    {2, FF_NEED, 0xFF},
    {3, FF_NEED, 0x51},
};

const FFFeature g_ff_jp2[] = {
    // standard 1
    {4, 0, 0x6A},
    {5, 0, 0x50},
    
    {7, 0, 0x20},
    {8, 0, 0x0D},
    {9, 0, 0x0A},
    {10, 0, 0x87},
    {11, 0, 0x0A},
    
    // ftyp
    {4, 1, 0x66},
    {5, 1, 0x74},
    {6, 1, 0x79},
    {7, 1, 0x70},
    
    {8, 1, 0x4A},
    {9, 1, 0x50},
    {10, 1, 0x32},
};

const FFFeature g_ff_eps[] = {
    {0, FF_NEED, 0x25},
    {1, FF_NEED, 0x21},
    {2, FF_NEED, 0x50},
    {3, FF_NEED, 0x53},
    {4, FF_NEED, 0x2D},
    {5, FF_NEED, 0x41},
    {6, FF_NEED, 0x64},
    {7, FF_NEED, 0x6F},
};

const FFFeature g_ff_psd[] = {
    {0, FF_NEED, 0x38},
    {1, FF_NEED, 0x42},
    {2, FF_NEED, 0x50},
    {3, FF_NEED, 0x53},
    {5, FF_NEED, 0x01},
};

const FFFeature g_ff_psb[] = {
    {0, FF_NEED, 0x38},
    {1, FF_NEED, 0x42},
    {2, FF_NEED, 0x50},
    {3, FF_NEED, 0x53},
    {5, FF_NEED, 0x02},
};

//------------------------------------------------------------------------------------------------------
// AUDIO & VIDEO
const FFFeature g_ff_m4a[] = {
    {10, FF_NEED, 0x41},
    
    {8, FF_NEED, 0x4D},
    {9, FF_NEED, 0x34},
    
    {4, FF_NEED, 0x66},
    {5, FF_NEED, 0x74},
    {6, FF_NEED, 0x79},
    {7, FF_NEED, 0x70},
};

const FFFeature g_ff_m4b[] = {
    {10, FF_NEED, 0x42},
    
    {8, FF_NEED, 0x4D},
    {9, FF_NEED, 0x34},
    
    {4, FF_NEED, 0x66},
    {5, FF_NEED, 0x74},
    {6, FF_NEED, 0x79},
    {7, FF_NEED, 0x70},
};

const FFFeature g_ff_m4p[] = {
    {10, FF_NEED, 0x50},
    
    {8, FF_NEED, 0x4D},
    {9, FF_NEED, 0x34},
    
    {4, FF_NEED, 0x66},
    {5, FF_NEED, 0x74},
    {6, FF_NEED, 0x79},
    {7, FF_NEED, 0x70},
};

const FFFeature g_ff_m4v[] = {
    {10, FF_NEED, 0x56},
    
    {8, FF_NEED, 0x4D},
    {9, FF_NEED, 0x34},
    
    {4, FF_NEED, 0x66},
    {5, FF_NEED, 0x74},
    {6, FF_NEED, 0x79},
    {7, FF_NEED, 0x70},
};

const FFFeature g_ff_mov[] = {
    // ftyp qt
    {8, 0, 0x71},
    {9, 0, 0x74},
    
    {4, 0, 0x66},
    {5, 0, 0x74},
    {6, 0, 0x79},
    {7, 0, 0x70},
    
    // QuickTime Movie
    {4, 1, 0x6D},
    {5, 1, 0x6F},
    {6, 1, 0x6F},
    {7, 1, 0x76},
    
    {4, 2, 0x66},
    {5, 2, 0x72},
    {6, 2, 0x65},
    {7, 2, 0x65},
    
    {4, 3, 0x6D},
    {5, 3, 0x64},
    {6, 3, 0x61},
    {7, 3, 0x74},
    
    {4, 4, 0x77},
    {5, 4, 0x69},
    {6, 4, 0x64},
    {7, 4, 0x65},
    
    {4, 5, 0x70},
    {5, 5, 0x6E},
    {6, 5, 0x6F},
    {7, 5, 0x74},
    
    {4, 6, 0x73},
    {5, 6, 0x6B},
    {6, 6, 0x69},
    {7, 6, 0x70},
};

const FFFeature g_ff_mp4[] = {
    {4, FF_NEED, 0x66},
    {5, FF_NEED, 0x74},
    {6, FF_NEED, 0x79},
    {7, FF_NEED, 0x70},
    
    // mp4
    {8, 0, 0x6d},
    {9, 0, 0x70},
    {10, 0, 0x34},
    
    // iso2 & isom
    {8, 1, 0x69},
    {9, 1, 0x73},
    {10, 1, 0x6f},
    
    // MSNV [Sony PSP]
    {8, 2, 0x4D},
    {9, 2, 0x53},
    {10, 2, 0x4E},
    
    // NDAS [Nero Digital AAC Audio]
    // NDSC [Nero Cinema Profile]
    // NDSH [Nero HDTV Profile]
    // NDSM [Nero Mobile Profile]
    // NDSP [Nero Portable Profile]
    // NDSS [Nero Standard Profile]
    // NDXC [Nero Cinema Profile]
    // NDXH [Nero HDTV Profile]
    // NDXM [Nero Mobile Profile]
    // NDXP [Nero Portable Profile]
    // NDXS [Nero Standard Profile]
    {8, 3, 0x4E},
    {9, 3, 0x44},
    
    // mp7
    {8, 20, 0x6d},
    {9, 20, 0x70},
    {10, 20, 0x37},
    
    // avc1
    {8, 21, 0x61},
    {9, 21, 0x76},
    {10, 21, 0x63},
    
    // drc1
    {8, 22, 0x64},
    {9, 22, 0x72},
    {10, 22, 0x63},
};

const FFFeature g_ff_mp3[] = {
    {0, 0, 0x49},
    {1, 0, 0x44},
    {2, 0, 0x33},
    
    {0, 1, 0xFF},
    {1, 1, 0xFB},
    
    {0, 2, 0xFF},
    {1, 2, 0xF3},
    
    {0, 3, 0xFF},
    {1, 3, 0xFA},
    
    {0, 4, 0xFF},
    {1, 4, 0xF2},
    
    {0, 4, 0xFF},
    {1, 4, 0xE3},
};

const FFFeature g_ff_mp2[] = {
    {0, FF_NEED, 0xFF},
    
    {1, 0, 0xFD},
    {1, 1, 0xF4},
    {1, 2, 0xF5},
    {1, 3, 0xFC},
};

const FFFeature g_ff_wav[] = {
    {8, FF_NEED, 0x57},
    {9, FF_NEED, 0x41},
    {10, FF_NEED, 0x56},
    {11, FF_NEED, 0x45},
    
    {0, FF_NEED, 0x52},
    {1, FF_NEED, 0x49},
    {2, FF_NEED, 0x46},
    {3, FF_NEED, 0x46},
};

const FFFeature g_ff_avi[] = {
    {8, FF_NEED, 0x41},
    {9, FF_NEED, 0x56},
    {10, FF_NEED, 0x49},
    
    {0, FF_NEED, 0x52},
    {1, FF_NEED, 0x49},
    {2, FF_NEED, 0x46},
    {3, FF_NEED, 0x46},
};

const FFFeature g_ff_aiff[] = {
    {0, FF_NEED, 0x46},
    {1, FF_NEED, 0x4F},
    {2, FF_NEED, 0x52},
    {3, FF_NEED, 0x4D},
    {4, FF_NEED, 0x00},
};

const FFFeature g_ff_asf[] = {
    {0, FF_NEED, 0x30},
    {1, FF_NEED, 0x26},
    {2, FF_NEED, 0xB2},
    {3, FF_NEED, 0x75},
    {4, FF_NEED, 0x8E},
    {5, FF_NEED, 0x66},
    {6, FF_NEED, 0xCF},
    {7, FF_NEED, 0x11},
};

const FFFeature g_ff_mid[] = {
    {0, FF_NEED, 0x4D},
    {1, FF_NEED, 0x54},
    {2, FF_NEED, 0x68},
    {3, FF_NEED, 0x64},
};

const FFFeature g_ff_flac[] = {
    {0, FF_NEED, 0x66},
    {1, FF_NEED, 0x4C},
    {2, FF_NEED, 0x61},
    {3, FF_NEED, 0x43},
};

const FFFeature g_ff_ape[] = {
    {0, FF_NEED, 0x4D},
    {1, FF_NEED, 0x41},
    {2, FF_NEED, 0x43},
    {3, FF_NEED, 0x20},
};

//------------------------------------------------------------------------------------------------------
// IMAGE 2
const FFFeature g_ff_svg[] = {
    {0, FF_NEED, 0x3C},
    {1, FF_NEED, 0x3F},
    {2, FF_NEED, 0x78},
    {3, FF_NEED, 0x6D},
    {4, FF_NEED, 0x6C},
    {5, FF_NEED, 0x20},
};

//------------------------------------------------------------------------------------------------------

const FFFormat g_ff_formats[FFTypeXCount] = {
    {"", 0, NULL}, // Unknown
    
    // DOCUMENT
    {"PDF", sizeof(g_ff_pdf)/sizeof(FFFeature), g_ff_pdf},
    {"DOC", sizeof(g_ff_doc)/sizeof(FFFeature), g_ff_doc},
    {"DOCX", sizeof(g_ff_docx)/sizeof(FFFeature), g_ff_docx},
    
    // IMAGE 1
    {"JPEG", sizeof(g_ff_jpeg)/sizeof(FFFeature), g_ff_jpeg},
    {"PNG",  sizeof(g_ff_png)/sizeof(FFFeature), g_ff_png},
    {"WEBP", sizeof(g_ff_webp)/sizeof(FFFeature), g_ff_webp},
    {"GIF",  sizeof(g_ff_gif)/sizeof(FFFeature), g_ff_gif},
    {"TIFF", sizeof(g_ff_tiff)/sizeof(FFFeature), g_ff_tiff},
    {"BMP",  sizeof(g_ff_bmp)/sizeof(FFFeature), g_ff_bmp},
    {"ICO",  sizeof(g_ff_ico)/sizeof(FFFeature), g_ff_ico},
    {"J2K",  sizeof(g_ff_j2k)/sizeof(FFFeature), g_ff_j2k},
    {"JP2",  sizeof(g_ff_jp2)/sizeof(FFFeature), g_ff_jp2},
    {"EPS",  sizeof(g_ff_eps)/sizeof(FFFeature), g_ff_eps},
    {"PSD",  sizeof(g_ff_psd)/sizeof(FFFeature), g_ff_psd},
    {"PSB",  sizeof(g_ff_psb)/sizeof(FFFeature), g_ff_psb},
    
    // AUDIO
    {"M4A",  sizeof(g_ff_m4a)/sizeof(FFFeature), g_ff_m4a},
    {"M4B",  sizeof(g_ff_m4b)/sizeof(FFFeature), g_ff_m4b},
    {"M4P",  sizeof(g_ff_m4p)/sizeof(FFFeature), g_ff_m4p},
    {"M4V",  sizeof(g_ff_m4v)/sizeof(FFFeature), g_ff_m4v},
    {"MOV",  sizeof(g_ff_mov)/sizeof(FFFeature), g_ff_mov},
    
    {"MP4",  sizeof(g_ff_mp4)/sizeof(FFFeature), g_ff_mp4},
    {"MP3",  sizeof(g_ff_mp3)/sizeof(FFFeature), g_ff_mp3},
    {"MP2",  sizeof(g_ff_mp2)/sizeof(FFFeature), g_ff_mp2},
    
    {"WAV",  sizeof(g_ff_wav)/sizeof(FFFeature), g_ff_wav},
    {"AVI",  sizeof(g_ff_avi)/sizeof(FFFeature), g_ff_avi},
    {"AIFF",  sizeof(g_ff_aiff)/sizeof(FFFeature), g_ff_aiff},
    {"MID",  sizeof(g_ff_mid)/sizeof(FFFeature), g_ff_mid},
    {"FLAC",  sizeof(g_ff_flac)/sizeof(FFFeature), g_ff_flac},
    {"APE",  sizeof(g_ff_ape)/sizeof(FFFeature), g_ff_ape},
    
    {"", 0,  NULL}, // padding
    
    // IMAGE 2
    {"SVG",  sizeof(g_ff_svg)/sizeof(FFFeature), g_ff_svg},
    {"DIB",  0, NULL},
    {"TGA",  0, NULL},
    
    // AUDIO & VIDEO 2
    
    {"WMA",  sizeof(g_ff_asf)/sizeof(FFFeature), g_ff_asf},
    {"WMV",  sizeof(g_ff_asf)/sizeof(FFFeature), g_ff_asf},
    {"ASF",  sizeof(g_ff_asf)/sizeof(FFFeature), g_ff_asf},
};

//------------------------------------------------------------------------------------------------------
