
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
    ci_event_t input_event;
    ci_document_t output_document;

    int root;

    /* Clear the objects. */

    memset(&parser, 0, sizeof(parser));
    memset(&emitter, 0, sizeof(emitter));
    memset(&input_event, 0, sizeof(input_event));
    memset(&output_document, 0, sizeof(output_document));

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
        printf("%s <input\n"
                "or\n%s -h | --help\nDeconstruct a CI stream\n\nOptions:\n"
                "-h, --help\t\tdisplay this help and exit\n"
                "-c, --canonical\t\toutput in the canonical CI format\n"
                "-u, --unicode\t\toutput unescaped non-ASCII characters\n",
                argv[0], argv[0]);
        return 0;
    }

    /* Initialize the parser and emitter objects. */

    if (!ci_parser_init(&parser)) {
        fprintf(stderr, "Could not initialize the parser object\n");
        return 1;
    }

    if (!ci_emitter_initialize(&emitter)) {
        ci_parser_delete(&parser);
        fprintf(stderr, "Could not inialize the emitter object\n");
        return 1;
    }

    /* Set the parser parameters. */

    ci_parser_set_input_file(&parser, stdin);

    /* Set the emitter parameters. */

    ci_emitter_set_output_file(&emitter, stdout);

    ci_emitter_set_canonical(&emitter, canonical);
    ci_emitter_set_unicode(&emitter, unicode);

    /* Create and emit the STREAM-START event. */

    if (!ci_emitter_open(&emitter))
        goto emitter_error;

    /* Create a output_document object. */

    if (!ci_document_initialize(&output_document, NULL, NULL, NULL, 0, 0))
        goto document_error;

    /* Create the root sequence. */

    root = ci_document_add_sequence(&output_document, NULL,
            CI_BLOCK_SEQUENCE_STYLE);
    if (!root) goto document_error;

    /* Loop through the input events. */

    while (!done)
    {
        int properties, key, value, map, seq;

        /* Get the next event. */

        if (!ci_parser_parse(&parser, &input_event))
            goto parser_error;

        /* Check if this is the stream end. */

        if (input_event.type == CI_STREAM_END_EVENT) {
            done = 1;
        }

        /* Create a mapping node and attach it to the root sequence. */

        properties = ci_document_add_mapping(&output_document, NULL,
                CI_BLOCK_MAPPING_STYLE);
        if (!properties) goto document_error;
        if (!ci_document_append_sequence_item(&output_document,
                    root, properties)) goto document_error;

        /* Analyze the event. */

        switch (input_event.type)
        {
            case CI_STREAM_START_EVENT:

                /* Add 'type': 'STREAM-START'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"STREAM-START", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'encoding': <encoding>. */

                if (input_event.data.stream_start.encoding)
                {
                    ci_encoding_t encoding
                        = input_event.data.stream_start.encoding;

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"encoding", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)(encoding == CI_UTF8_ENCODING ? "utf-8" :
                             encoding == CI_UTF16LE_ENCODING ? "utf-16-le" :
                             encoding == CI_UTF16BE_ENCODING ? "utf-16-be" :
                             "unknown"), -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }
                    
                break;

            case CI_STREAM_END_EVENT:

                /* Add 'type': 'STREAM-END'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"STREAM-END", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            case CI_DOCUMENT_START_EVENT:

                /* Add 'type': 'DOCUMENT-START'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"DOCUMENT-START", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Display the output_document version numbers. */

                if (input_event.data.document_start.version_directive)
                {
                    ci_version_directive_t *version
                        = input_event.data.document_start.version_directive;
                    char number[64];

                    /* Add 'version': {}. */
                    
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"version", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    map = ci_document_add_mapping(&output_document, NULL,
                            CI_FLOW_MAPPING_STYLE);
                    if (!map) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, map)) goto document_error;

                    /* Add 'major': <number>. */

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"major", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    sprintf(number, "%d", version->major);
                    value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_INT_TAG,
                        (ci_char_t *)number, -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                map, key, value)) goto document_error;

                    /* Add 'minor': <number>. */

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"minor", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    sprintf(number, "%d", version->minor);
                    value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_INT_TAG,
                        (ci_char_t *)number, -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                map, key, value)) goto document_error;
                }

                /* Display the output_document tag directives. */

                if (input_event.data.document_start.tag_directives.start
                        != input_event.data.document_start.tag_directives.end)
                {
                    ci_tag_directive_t *tag;

                    /* Add 'tags': []. */
                    
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"tags", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    seq = ci_document_add_sequence(&output_document, NULL,
                            CI_BLOCK_SEQUENCE_STYLE);
                    if (!seq) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, seq)) goto document_error;

                    for (tag = input_event.data.document_start.tag_directives.start;
                            tag != input_event.data.document_start.tag_directives.end;
                            tag ++)
                    {
                        /* Add {}. */

                        map = ci_document_add_mapping(&output_document, NULL,
                                CI_FLOW_MAPPING_STYLE);
                        if (!map) goto document_error;
                        if (!ci_document_append_sequence_item(&output_document,
                                    seq, map)) goto document_error;

                        /* Add 'handle': <handle>. */

                        key = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)"handle", -1, CI_PLAIN_SCALAR_STYLE);
                        if (!key) goto document_error;
                        value = ci_document_add_scalar(&output_document, NULL,
                            tag->handle, -1, CI_DOUBLE_QUOTED_SCALAR_STYLE);
                        if (!value) goto document_error;
                        if (!ci_document_append_mapping_pair(&output_document,
                                    map, key, value)) goto document_error;

                        /* Add 'prefix': <prefix>. */

                        key = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)"prefix", -1, CI_PLAIN_SCALAR_STYLE);
                        if (!key) goto document_error;
                        value = ci_document_add_scalar(&output_document, NULL,
                            tag->prefix, -1, CI_DOUBLE_QUOTED_SCALAR_STYLE);
                        if (!value) goto document_error;
                        if (!ci_document_append_mapping_pair(&output_document,
                                    map, key, value)) goto document_error;
                    }
                }

                /* Add 'implicit': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"implicit", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.document_start.implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            case CI_DOCUMENT_END_EVENT:

                /* Add 'type': 'DOCUMENT-END'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"DOCUMENT-END", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'implicit': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"implicit", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.document_end.implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            case CI_ALIAS_EVENT:

                /* Add 'type': 'ALIAS'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"ALIAS", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'anchor': <anchor>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"anchor", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                        input_event.data.alias.anchor, -1,
                        CI_DOUBLE_QUOTED_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            case CI_SCALAR_EVENT:

                /* Add 'type': 'SCALAR'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"SCALAR", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'anchor': <anchor>. */

                if (input_event.data.scalar.anchor)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"anchor", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.scalar.anchor, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'tag': <tag>. */

                if (input_event.data.scalar.tag)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"tag", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.scalar.tag, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'value': <value>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"value", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                        input_event.data.scalar.value,
                        input_event.data.scalar.length,
                        CI_DOUBLE_QUOTED_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Display if the scalar tag is implicit. */

                /* Add 'implicit': {} */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"version", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                map = ci_document_add_mapping(&output_document, NULL,
                        CI_FLOW_MAPPING_STYLE);
                if (!map) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, map)) goto document_error;

                /* Add 'plain': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"plain", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.scalar.plain_implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            map, key, value)) goto document_error;

                /* Add 'quoted': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"quoted", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.scalar.quoted_implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            map, key, value)) goto document_error;

                /* Display the style information. */

                if (input_event.data.scalar.style)
                {
                    ci_scalar_style_t style = input_event.data.scalar.style;

                    /* Add 'style': <style>. */

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"style", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)(style == CI_PLAIN_SCALAR_STYLE ? "plain" :
                             style == CI_SINGLE_QUOTED_SCALAR_STYLE ?
                                    "single-quoted" :
                             style == CI_DOUBLE_QUOTED_SCALAR_STYLE ?
                                    "double-quoted" :
                             style == CI_LITERAL_SCALAR_STYLE ? "literal" :
                             style == CI_FOLDED_SCALAR_STYLE ? "folded" :
                             "unknown"), -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                break;

            case CI_SEQUENCE_START_EVENT:

                /* Add 'type': 'SEQUENCE-START'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"SEQUENCE-START", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'anchor': <anchor>. */

                if (input_event.data.sequence_start.anchor)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"anchor", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.sequence_start.anchor, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'tag': <tag>. */

                if (input_event.data.sequence_start.tag)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"tag", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.sequence_start.tag, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'implicit': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"implicit", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.sequence_start.implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Display the style information. */

                if (input_event.data.sequence_start.style)
                {
                    ci_sequence_style_t style
                        = input_event.data.sequence_start.style;

                    /* Add 'style': <style>. */

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"style", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)(style == CI_BLOCK_SEQUENCE_STYLE ? "block" :
                             style == CI_FLOW_SEQUENCE_STYLE ? "flow" :
                             "unknown"), -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                break;

            case CI_SEQUENCE_END_EVENT:

                /* Add 'type': 'SEQUENCE-END'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"SEQUENCE-END", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            case CI_MAPPING_START_EVENT:

                /* Add 'type': 'MAPPING-START'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"MAPPING-START", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Add 'anchor': <anchor>. */

                if (input_event.data.mapping_start.anchor)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"anchor", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.mapping_start.anchor, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'tag': <tag>. */

                if (input_event.data.mapping_start.tag)
                {
                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"tag", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            input_event.data.mapping_start.tag, -1,
                            CI_DOUBLE_QUOTED_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                /* Add 'implicit': <flag>. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"implicit", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, (ci_char_t *)CI_BOOL_TAG,
                        (ci_char_t *)(input_event.data.mapping_start.implicit ?
                         "true" : "false"), -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                /* Display the style information. */

                if (input_event.data.mapping_start.style)
                {
                    ci_mapping_style_t style
                        = input_event.data.mapping_start.style;

                    /* Add 'style': <style>. */

                    key = ci_document_add_scalar(&output_document, NULL,
                        (ci_char_t *)"style", -1, CI_PLAIN_SCALAR_STYLE);
                    if (!key) goto document_error;
                    value = ci_document_add_scalar(&output_document, NULL,
                            (ci_char_t *)(style == CI_BLOCK_MAPPING_STYLE ? "block" :
                             style == CI_FLOW_MAPPING_STYLE ? "flow" :
                             "unknown"), -1, CI_PLAIN_SCALAR_STYLE);
                    if (!value) goto document_error;
                    if (!ci_document_append_mapping_pair(&output_document,
                                properties, key, value)) goto document_error;
                }

                break;

            case CI_MAPPING_END_EVENT:

                /* Add 'type': 'MAPPING-END'. */

                key = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"type", -1, CI_PLAIN_SCALAR_STYLE);
                if (!key) goto document_error;
                value = ci_document_add_scalar(&output_document, NULL,
                    (ci_char_t *)"MAPPING-END", -1, CI_PLAIN_SCALAR_STYLE);
                if (!value) goto document_error;
                if (!ci_document_append_mapping_pair(&output_document,
                            properties, key, value)) goto document_error;

                break;

            default:
                /* It couldn't really happen. */
                break;
        }

        /* Delete the event object. */

        ci_event_delete(&input_event);
    }

    if (!ci_emitter_dump(&emitter, &output_document))
        goto emitter_error;
    if (!ci_emitter_close(&emitter))
        goto emitter_error;

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
                fprintf(stderr, "Reader error: %s: #%X at %zd\n", parser.problem,
                        parser.problem_value, parser.problem_offset);
            }
            else {
                fprintf(stderr, "Reader error: %s at %zd\n", parser.problem,
                        parser.problem_offset);
            }
            break;

        case CI_SCANNER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Scanner error: %s at line %lu, column %lu\n"
                        "%s at line %lu, column %lu\n", parser.context,
                        parser.context_mark.line+1, parser.context_mark.column+1,
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Scanner error: %s at line %lu, column %lu\n",
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            break;

        case CI_PARSER_ERROR:
            if (parser.context) {
                fprintf(stderr, "Parser error: %s at line %lu, column %lu\n"
                        "%s at line %lu, column %lu\n", parser.context,
                        parser.context_mark.line+1, parser.context_mark.column+1,
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            else {
                fprintf(stderr, "Parser error: %s at line %lu, column %lu\n",
                        parser.problem, parser.problem_mark.line+1,
                        parser.problem_mark.column+1);
            }
            break;

        default:
            /* Couldn't happen. */
            fprintf(stderr, "Internal error\n");
            break;
    }

    ci_event_delete(&input_event);
    ci_document_delete(&output_document);
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

    ci_event_delete(&input_event);
    ci_document_delete(&output_document);
    ci_parser_delete(&parser);
    ci_emitter_delete(&emitter);

    return 1;

document_error:

    fprintf(stderr, "Memory error: Not enough memory for creating a document\n");

    ci_event_delete(&input_event);
    ci_document_delete(&output_document);
    ci_parser_delete(&parser);
    ci_emitter_delete(&emitter);

    return 1;
}

