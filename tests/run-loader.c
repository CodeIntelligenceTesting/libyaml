#include <ci.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

int
main(int argc, char *argv[])
{
    int number;

    if (argc < 2) {
        printf("Usage: %s file1.ci ...\n", argv[0]);
        return 0;
    }

    for (number = 1; number < argc; number ++)
    {
        FILE *file;
        ci_parser_t parser;
        ci_document_t document;
        int done = 0;
        int count = 0;
        int error = 0;

        printf("[%d] Loading '%s': ", number, argv[number]);
        fflush(stdout);

        file = fopen(argv[number], "rb");
        assert(file);

        assert(ci_parser_initialize(&parser));

        ci_parser_set_input_file(&parser, file);

        while (!done)
        {
            if (!ci_parser_load(&parser, &document)) {
                error = 1;
                break;
            }

            done = (!ci_document_get_root_node(&document));

            ci_document_delete(&document);

            if (!done) count ++;
        }

        ci_parser_delete(&parser);

        assert(!fclose(file));

        printf("%s (%d documents)\n", (error ? "FAILURE" : "SUCCESS"), count);
    }

    return 0;
}

