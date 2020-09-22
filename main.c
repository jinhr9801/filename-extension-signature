//

#include "ff_file_formats.h"

int main(int argc, const char* argv[]) {
    if (argc < 2 ) {
#ifdef DEBUG
        const char file_name[] = "/Users/henry/Downloads/download.pdf";
        
        FFType type = ff_get_type_from_file(file_name);
        if (type == FFTypeUnknown) {
            printf("Fail to get the file type!\n");
        } else {
            const char* ext = ff_get_ext_name_by_type(type);
            printf("The file type is: %s!\n", ext);
        }
#else
        printf("Please supply the file path and name as the first argument!\n");
#endif
        return 0;
    }
    
    const char* file_name = argv[1];
    
    FFType type = ff_get_type_from_file(file_name);
    if (type == FFTypeUnknown) {
        printf("Fail to get the file type!\n");
    } else {
        const char* ext = ff_get_ext_name_by_type(type);
        printf("The file type is: %s!\n", ext);
    }
    return 0;
}
