#include <stdio.h>
#include <string.h>

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int main() {

printf("%s %d\n", get_filename_ext("test.tiff"), strlen(get_filename_ext("test.tiff")));
printf("%s %d\n", get_filename_ext("test.blah.tiff"), strlen(get_filename_ext("test.blah.tiff")));
printf("%s %d\n", get_filename_ext("test."), strlen(get_filename_ext("test.")));
printf("%s %d\n", get_filename_ext("test"), strlen(get_filename_ext("test")));
printf("%s %d\n", get_filename_ext("..."), strlen(get_filename_ext("...")));

}