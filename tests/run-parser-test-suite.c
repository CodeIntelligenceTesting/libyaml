#include <ci.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void print_escaped(ci_char_t * str, size_t length);
int usage(int ret);

int main(int argc, char *argv[])
{
    FILE *input;
    ci_parser_t parser;
    ci_event_t event;
    int flow = -1; /** default no flow style collections */
    int i = 0;
    int foundfile = 0;
    int max_level;
    char *output;

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--max-level", 11) == 0) {
            i++;
            max_level = strtol(argv[i], &output, 10);
            ci_set_max_nest_level(max_level);
        }
        else if (strncmp(argv[i], "--flow", 6) == 0) {
            if (i+1 == argc)
                return usage(1);
            i++;
            if (strncmp(argv[i], "keep", 4) == 0)
                flow = 0;
            else if (strncmp(argv[i], "on", 2) == 0)
                flow = 1;
            else if (strncmp(argv[i], "off", 3) == 0)
                flow = -1;
            else
                return usage(1);
        }
        else if (strncmp(argv[i], "--help", 6) == 0)
            return usage(0);
        else if (strncmp(argv[i], "-h", 2) == 0)
            return usage(0);
        else if (!foundfile) {
            input = fopen(argv[i], "rb");
            foundfile = 1;
        }
        else
            return usage(1);
    }
    if (!foundfile) {
        input = stdin;
    }
    assert(input);

    if (!ci_parser_initialize(&parser)) {
        fprintf(stderr, "Could not initialize the parser object\n");
        return 1;
    }
    ci_parser_set_input_file(&parser, input);

    while (1) {
        ci_event_type_t type;
        if (!ci_parser_parse(&parser, &event)) {
            if ( parser.problem_mark.line || parser.problem_mark.column ) {
                fprintf(stderr, "Parse error: %s\nLine: %lu Column: %lu\n",
                    parser.problem,
                    (unsigned long)parser.problem_mark.line + 1,
                    (unsigned long)parser.problem_mark.column + 1);
            }
            else {
                fprintf(stderr, "Parse error: %s\n", parser.problem);
            }
            return 1;
        }
        type = event.type;

        if (type == CI_NO_EVENT)
            printf("???\n");
        else if (type == CI_STREAM_START_EVENT)
            printf("+STR\n");
        else if (type == CI_STREAM_END_EVENT)
            printf("-STR\n");
        else if (type == CI_DOCUMENT_START_EVENT) {
            printf("+DOC");
            if (!event.data.document_start.implicit)
                printf(" ---");
            printf("\n");
        }
        else if (type == CI_DOCUMENT_END_EVENT) {
            printf("-DOC");
            if (!event.data.document_end.implicit)
                printf(" ...");
            printf("\n");
        }
        else if (type == CI_MAPPING_START_EVENT) {
            printf("+MAP");
            if (flow == 0 && event.data.mapping_start.style == CI_FLOW_MAPPING_STYLE)
                printf(" {}");
            else if (flow == 1)
                printf(" {}");
            if (event.data.mapping_start.anchor)
                printf(" &%s", event.data.mapping_start.anchor);
            if (event.data.mapping_start.tag)
                printf(" <%s>", event.data.mapping_start.tag);
            printf("\n");
        }
        else if (type == CI_MAPPING_END_EVENT)
            printf("-MAP\n");
        else if (type == CI_SEQUENCE_START_EVENT) {
            printf("+SEQ");
            if (flow == 0 && event.data.sequence_start.style == CI_FLOW_SEQUENCE_STYLE)
                printf(" []");
            else if (flow == 1)
                printf(" []");
            if (event.data.sequence_start.anchor)
                printf(" &%s", event.data.sequence_start.anchor);
            if (event.data.sequence_start.tag)
                printf(" <%s>", event.data.sequence_start.tag);
            printf("\n");
        }
        else if (type == CI_SEQUENCE_END_EVENT)
            printf("-SEQ\n");
        else if (type == CI_SCALAR_EVENT) {
            printf("=VAL");
            if (event.data.scalar.anchor)
                printf(" &%s", event.data.scalar.anchor);
            if (event.data.scalar.tag)
                printf(" <%s>", event.data.scalar.tag);
            switch (event.data.scalar.style) {
            case CI_PLAIN_SCALAR_STYLE:
                printf(" :");
                break;
            case CI_SINGLE_QUOTED_SCALAR_STYLE:
                printf(" '");
                break;
            case CI_DOUBLE_QUOTED_SCALAR_STYLE:
                printf(" \"");
                break;
            case CI_LITERAL_SCALAR_STYLE:
                printf(" |");
                break;
            case CI_FOLDED_SCALAR_STYLE:
                printf(" >");
                break;
            case CI_ANY_SCALAR_STYLE:
                abort();
            }
            print_escaped(event.data.scalar.value, event.data.scalar.length);
            printf("\n");
        }
        else if (type == CI_ALIAS_EVENT)
            printf("=ALI *%s\n", event.data.alias.anchor);
        else
            abort();

        ci_event_delete(&event);

        if (type == CI_STREAM_END_EVENT)
            break;
    }

    assert(!fclose(input));
    ci_parser_delete(&parser);
    fflush(stdout);

    return 0;
}

void print_escaped(ci_char_t * str, size_t length)
{
    int i;
    char c;

    for (i = 0; i < length; i++) {
        c = *(str + i);
        if (c == '\\')
            printf("\\\\");
        else if (c == '\0')
            printf("\\0");
        else if (c == '\b')
            printf("\\b");
        else if (c == '\n')
            printf("\\n");
        else if (c == '\r')
            printf("\\r");
        else if (c == '\t')
            printf("\\t");
        else
            printf("%c", c);
    }
}

int usage(int ret) {
    fprintf(stderr, "Usage: libci-parser [--flow (on|off|keep)] [<input-file>]\n");
    return ret;
}
