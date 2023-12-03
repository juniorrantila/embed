#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
int asprintf(char**, char const*, ...);
#endif

typedef char const* c_string;
typedef enum {
    Type_Bin,
    Type_Text
} Type_;

typedef struct {
    c_string content;
    size_t size;
} String;
static int read_file_to_string(c_string, String*);

static int write_header(c_string output_path, c_string array_name, String content, Type_ type);
static int write_implementation(c_string output_path, c_string array_name, String content, Type_ type);

static int parse_type(c_string, Type_* type);

typedef struct {
    char* dest_dir;
    char* output_name;
    char* filename;
    char* type;
} Arguments;
static Arguments parse_arguments(int argc, char** argv);

static void usage(c_string);

int main(int argc, char* argv[])
{
    Arguments arguments = { 0 };
    String file = { 0 };
    Type_ type = Type_Bin;
    char* header_output_path = NULL;
    char* implementation_output_path = NULL;

    arguments = parse_arguments(argc, argv);
    if (arguments.type != NULL && parse_type(arguments.type, &type) < 0) {
        (void)fprintf(stderr, "Error: invalid type '%s'\n", argv[3]);
        usage(argv[0]);
    }

    if (read_file_to_string(arguments.filename, &file) < 0) {
        c_string error = strerror(errno); /* NOLINT(concurrency-mt-unsafe) */
        (void)fprintf(stderr, "Error: could not read '%s': %s", arguments.filename, error);
    }

    assert(asprintf(&header_output_path, "%s/%s.h", arguments.dest_dir, arguments.output_name) != -1);
    if (write_header(header_output_path, arguments.output_name, file, type) < 0) {
        perror("write_header");
    }

    assert(asprintf(&implementation_output_path, "%s/%s.c", arguments.dest_dir, arguments.output_name) != -1);
    if (write_implementation(implementation_output_path, arguments.output_name, file, type) < 0) {
        perror("write_header");
    }

    return 0;
}

static void usage(c_string program_name)
{
    (void)fprintf(stderr, "USAGE: %s <dest-dir> <output-name> <file> [type]\nType:\n  text\n  bin\n", program_name);
    exit(-1); /* NOLINT(concurrency-mt-unsafe) */
}

static Arguments parse_arguments(int argc, char** argv)
{
    Arguments arguments = { 0 };
    if (argc <= 3) {
        usage(argv[0]);
    }

    arguments.dest_dir = argv[1];
    arguments.output_name = argv[2];
    arguments.filename = argv[3];
    arguments.type = argv[4];
    return arguments;
}

static int parse_type(c_string name, Type_* type)
{
    if (name == NULL)
        return 0;
    if (strcmp("text", name) == 0)
        return *type = Type_Text, 0;
    if (strcmp("bin", name) == 0)
        return *type = Type_Bin, 0;
    return -1;
}

static int read_file_to_string(c_string filename, String* output)
{
   char* text = NULL;
   size_t len = 0;

   FILE *f = fopen(filename, "rb");
   if (f == 0)
       return -1;

   (void)fseek(f, 0, SEEK_END);
   len = (size_t) ftell(f);
   output->size = len;

   text = (char*)malloc(len+1);
   if (text == NULL)
       return -1;

   (void)fseek(f, 0, SEEK_SET);
   (void)fread(text, 1, len, f);
   (void)fclose(f);
   text[len] = 0;
   output->content = text;

   return 0;
}

static c_string type_to_c_type(Type_ type)
{
    switch(type) {
    case Type_Bin: return "unsigned char";
    case Type_Text: return "char";
    }
}

static int write_header(c_string output_path, c_string array_name, String content, Type_ type)
{
    int res = 0;
    size_t content_size = type == Type_Text ? content.size + 1 : content.size;
    c_string size_type = "unsigned long";
    FILE* f = fopen(output_path, "wb+");
    if (f == NULL)
        return -1;
    res = fprintf(f, "#pragma once\n"
               "extern const %s %s[%lu];\n"
               "const %s %s_size = %lu;\n",
               type_to_c_type(type), array_name, (unsigned long)content_size,
               size_type, array_name, (unsigned long)content_size);
    if (res < 0)
        return -1;
    (void)fclose(f);
    return 0;
}

static int write_implementation(c_string output_path, c_string array_name, String content, Type_ type)
{
    int result = -1;
    size_t i = 0;
    int res = 0;
    FILE* f = NULL;
    const size_t row_break = 60;
    size_t printed_chars = 0;

    (void)type;

    f = fopen(output_path, "wb+");
    if (f == NULL)
        return -1;
    res = fprintf(f, "const %s %s[%lu] = \n    \"", type_to_c_type(type), array_name, (unsigned long)(content.size + 1));
    if (res < 0)
        goto fi;

    for (i = 0; i < content.size; i++) {
        unsigned char c = (unsigned char)content.content[i];

        if (printed_chars >= row_break) {
            res = fprintf(f, "\"\n    \"");
            if (res < 0)
                goto fi;
            printed_chars = res;
        }

        if (c == '\"') {
            res = fprintf(f, "\\\"");
            if (res < 0)
                goto fi;
            printed_chars += res;
        } else if (c == '\n') {
            res = fprintf(f, "\\n");
            if (res < 0)
                goto fi;
            printed_chars += res;
        } else if (c == '\\') {
            res = fprintf(f, "\\\\");
            if (res < 0)
                goto fi;
            printed_chars += res;
        } else if (c == 0) {
            res = fprintf(f, "\\0");
            if (res < 0)
                goto fi;
            printed_chars += res;
        } else if (c < 128 && c > 31) {
            if (fputc(c, f) != c)
                goto fi;
            printed_chars += 1;
        } else {
            res = fprintf(f, "\\%o", c);
            if (res < 0)
                goto fi;
            printed_chars += res;
        }
    }
    res = fprintf(f, "\";\n");
    if (res < 0)
        goto fi;

    result = 0;
fi:
    fclose(f);
    return result;
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <stdarg.h>

int asprintf(char** dest, char const* fmt, ...)
{
    va_list args;
    int result = -1;
    int length = 0;
    char* intermidiate = NULL;

    va_start(args, fmt);
    length = vsnprintf(0, 0, fmt, args);
    if (length < 0)
        goto fi;

    intermidiate = (char*)calloc(length + 1, 1);
    if (intermidiate == NULL)
        goto fi;

    va_end(args);
    va_start(args, fmt);
    length = vsprintf(intermidiate, fmt, args);
    if (length < 0)
        goto fi;

    *dest = intermidiate;
    result = length;
fi:
    va_end(args);
    return result;
}
#endif
