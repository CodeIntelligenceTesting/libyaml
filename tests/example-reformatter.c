
#include <ci.h>

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    int help = 0;
    int canonical = 0;
    int unicode = 0;
    int k;
    int done = 0;

    ci_parser_t parser;
    ci_emitter_t emitter;
    ci_event_t event;

    /* Clear the objects. */

    memset(&parser, 0, sizeof(parser));
    memset(&emitter, 0, sizeof(emitter));
    memset(&event, 0, sizeof(event));

    /* Analyze command line options. */

    for (k = 1; k < argc; k ++)
    {
        if (strcmp(argv[k], "-h") == 0
                || strcmp(argv[k], "--help") == 0) {
            help = 1;
        }

        else if (strcmp(argv[k], "-c") == 0
                || strcmp(argv[k], "--canonical") == 0) {
            canonical = 1;
        }

        else if (strcmp(argv[k], "-u") == 0
                || strcmp(argv[k], "--unicode") == 0) {
            unicode = 1;
        }

        else {
            fprintf(stderr, "Unrecognized option: %s\n"
                    "Try `%s --help` for more information.\n",
                    argv[k], argv[0]);
            return 1;
        }
    }

    /* Display the help string. */

    if (help)
    {
        printf("%s [--canonical] [--unicode] <input >output\n"
                "or\n%s -h | --help\nReformat a CI stream\n\nOptions:\n"
                "-h, --help\t\tdisplay this help and exit\n"
                "-c, --canonical\t\toutput in the canonical CI format\n"
                "-u, --unicode\t\toutput unescaped non-ASCII characters\n",
                argv[0], argv[0]);
        return 0;
    }

    /* Initialize the parser and emitter objects. */

    if (!ci_parser_initialize(&parser))
        goto parser_error;

    if (!ci_emitter_initialize(&emitter))
        goto emitter_error;

    /* Set the parser parameters. */

    ci_parser_set_input_file(&parser, stdin);

    /* Set the emitter parameters. */

    ci_emitter_set_output_file(&emitter, stdout);

    ci_emitter_set_canonical(&emitter, canonical);
    ci_emitter_set_unicode(&emitter, unicode);

    /* The main loop. */

    while (!done)
    {
        /* Get the next event. */

        if (!ci_parser_parse(&parser, &event))
            goto parser_error;

        /* Check if this is the stream end. */

        if (event.type == CI_STREAM_END_EVENT) {
            done = 1;
        }

        /* Emit the event. */

        if (!ci_emitter_emit(&emitter, &event))
            goto emitter_error;
    }

    ci_parser_delete(&parser);
    ci_emitter_delete(&emitter);

    return 0;

parser_error:

    /* Display a parser error message. */

    switch (parser.error)
    {
        case CI_MEMORY_ERROR:
            fprintf(stderr, "Memory error: Not enough memory for parsing\n");
            break;

        case CI_READER_ERROR:
            if (parser.problem_value != -1) {
                fprintf(stderr, "Reader error: %s: #%X at %ld\n", parser.problem,
                        parser.problem_value, (long)parser.problem_offset);
            }
            else {
                fprintf(stderr, "Reader error: %s at %ld\n", parser.problem,
                        (long)parser.problem_offset);
            }
            break;

        case CI_SCANNER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Scanner error: %s at line %d, column %d\n"
                        "%s at line %d, column %d\n", parser.context,
                        (int)parser.context_mark.line+1, (int)parser.context_mark.column+1,
                        parser.problem, (int)parser.problem_mark.line+1,
                        (int)parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Scanner error: %s at line %d, column %d\n",
                        parser.problem, (int)parser.problem_mark.line+1,
                        (int)parser.problem_mark.column+1);
            }
            break;

        case CI_PARSER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Parser error: %s at line %d, column %d\n"
                        "%s at line %d, column %d\n", parser.context,
                        (int)parser.context_mark.line+1, (int)parser.context_mark.column+1,
                        parser.problem, (int)parser.problem_mark.line+1,
                        (int)parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Parser error: %s at line %d, column %d\n",
                        parser.problem, (int)parser.problem_mark.line+1,
                        (int)parser.problem_mark.column+1);
            }
            break;

        default:
            /* Couldn't happen. */
            fprintf(stderr, "Internal error\n");
            break;
    }

    ci_parser_delete(&parser);
    ci_emitter_delete(&emitter);

    return 1;

emitter_error:

    /* Display an emitter error message. */

    switch (emitter.error)
    {
        case CI_MEMORY_ERROR:
            fprintf(stderr, "Memory error: Not enough memory for emitting\n");
            break;

        case CI_WRITER_ERROR:
            fprintf(stderr, "Writer error: %s\n", emitter.problem);
            break;

        case CI_EMITTER_ERROR:
            fprintf(stderr, "Emitter error: %s\n", emitter.problem);
            break;

        default:
            /* Couldn't happen. */
            fprintf(stderr, "Internal error\n");
            break;
    }

    ci_parser_delete(&parser);
    ci_emitter_delete(&emitter);

    return 1;
}

