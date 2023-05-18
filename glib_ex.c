#include <glib.h>
#include <stdio.h>

int main() {
    GString* str = g_string_new("Hello, GLib!");
    printf("%s\n", str->str);
    g_string_free(str, TRUE);
    return 0;
}