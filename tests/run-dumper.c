#include <ci.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#define BUFFER_SIZE 65536
#define MAX_DOCUMENTS  16

int copy_document(ci_document_t *document_to, ci_document_t *document_from)
{
    ci_node_t *node;
    ci_node_item_t *item;
    ci_node_pair_t *pair;

    if (!ci_document_initialize(document_to, document_from->version_directive,
                document_from->tag_directives.start,
                document_from->tag_directives.end,
                document_from->start_implicit, document_from->end_implicit))
        return 0;

    for (node = document_from->nodes.start;
            node < document_from->nodes.top; node ++) {
        switch (node->type) {
            case CI_SCALAR_NODE:
                if (!ci_document_add_scalar(document_to, node->tag,
                            node->data.scalar.value, node->data.scalar.length,
                            node->data.scalar.style)) goto error;
                break;
            case CI_SEQUENCE_NODE:
                if (!ci_document_add_sequence(document_to, node->tag,
                            node->data.sequence.style)) goto error;
                break;
            case CI_MAPPING_NODE:
                if (!ci_document_add_mapping(document_to, node->tag,
                            node->data.mapping.style)) goto error;
                break;
            default:
                assert(0);
                break;
        }
    }

    for (node = document_from->nodes.start;
            node < document_from->nodes.top; node ++) {
        switch (node->type) {
            case CI_SEQUENCE_NODE:
                for (item = node->data.sequence.items.start;
                        item < node->data.sequence.items.top; item ++) {
                    if (!ci_document_append_sequence_item(document_to,
                                node - document_from->nodes.start + 1,
                                *item)) goto error;
                }
                break;
            case CI_MAPPING_NODE:
                for (pair = node->data.mapping.pairs.start;
                        pair < node->data.mapping.pairs.top; pair ++) {
                    if (!ci_document_append_mapping_pair(document_to,
                                node - document_from->nodes.start + 1,
                                pair->key, pair->value)) goto error;
                }
                break;
            default:
                break;
        }
    }
    return 1;

error:
    ci_document_delete(document_to);
    return 0;
}

int compare_nodes(ci_document_t *document1, int index1,
        ci_document_t *document2, int index2, int level)
{
    int k;
    ci_node_t *node1;
    ci_node_t *node2;
    if (level++ > 1000) return 0;
    node1 = ci_document_get_node(document1, index1);
    node2 = ci_document_get_node(document2, index2);

    assert(node1);
    assert(node2);

    if (node1->type != node2->type)
        return 0;

    if (strcmp((char *)node1->tag, (char *)node2->tag) != 0) return 0;

    switch (node1->type) {
        case CI_SCALAR_NODE:
            if (node1->data.scalar.length != node2->data.scalar.length)
                return 0;
            if (strncmp((char *)node1->data.scalar.value, (char *)node2->data.scalar.value,
                        node1->data.scalar.length) != 0) return 0;
            break;
        case CI_SEQUENCE_NODE:
            if ((node1->data.sequence.items.top - node1->data.sequence.items.start) !=
                    (node2->data.sequence.items.top - node2->data.sequence.items.start))
                return 0;
            for (k = 0; k < (node1->data.sequence.items.top - node1->data.sequence.items.start); k ++) {
                if (!compare_nodes(document1, node1->data.sequence.items.start[k],
                            document2, node2->data.sequence.items.start[k], level)) return 0;
            }
            break;
        case CI_MAPPING_NODE:
            if ((node1->data.mapping.pairs.top - node1->data.mapping.pairs.start) !=
                    (node2->data.mapping.pairs.top - node2->data.mapping.pairs.start))
                return 0;
            for (k = 0; k < (node1->data.mapping.pairs.top - node1->data.mapping.pairs.start); k ++) {
                if (!compare_nodes(document1, node1->data.mapping.pairs.start[k].key,
                            document2, node2->data.mapping.pairs.start[k].key, level)) return 0;
                if (!compare_nodes(document1, node1->data.mapping.pairs.start[k].value,
                            document2, node2->data.mapping.pairs.start[k].value, level)) return 0;
            }
            break;
        default:
            assert(0);
            break;
    }
    return 1;
}

int compare_documents(ci_document_t *document1, ci_document_t *document2)
{
    int k;

    if ((document1->version_directive && !document2->version_directive)
            || (!document1->version_directive && document2->version_directive)
            || (document1->version_directive && document2->version_directive
                && (document1->version_directive->major != document2->version_directive->major
                    || document1->version_directive->minor != document2->version_directive->minor)))
        return 0;

    if ((document1->tag_directives.end - document1->tag_directives.start) !=
            (document2->tag_directives.end - document2->tag_directives.start))
        return 0;
    for (k = 0; k < (document1->tag_directives.end - document1->tag_directives.start); k ++) {
        if ((strcmp((char *)document1->tag_directives.start[k].handle,
                        (char *)document2->tag_directives.start[k].handle) != 0)
                || (strcmp((char *)document1->tag_directives.start[k].prefix,
                    (char *)document2->tag_directives.start[k].prefix) != 0))
            return 0;
    }

    if ((document1->nodes.top - document1->nodes.start) !=
            (document2->nodes.top - document2->nodes.start))
        return 0;

    if (document1->nodes.top != document1->nodes.start) {
        if (!compare_nodes(document1, 1, document2, 1, 0))
            return 0;
    }

    return 1;
}

int print_output(char *name, unsigned char *buffer, size_t size, int count)
{
    FILE *file;
    char data[BUFFER_SIZE];
    size_t data_size = 1;
    size_t total_size = 0;
    if (count >= 0) {
        printf("FAILED (at the document #%d)\nSOURCE:\n", count+1);
    }
    file = fopen(name, "rb");
    assert(file);
    while (data_size > 0) {
        data_size = fread(data, 1, BUFFER_SIZE, file);
        assert(!ferror(file));
        if (!data_size) break;
        assert(fwrite(data, 1, data_size, stdout) == data_size);
        total_size += data_size;
        if (feof(file)) break;
    }
    fclose(file);
    printf("#### (length: %ld)\n", (long)total_size);
    printf("OUTPUT:\n%s#### (length: %ld)\n", buffer, (long)size);
    return 0;
}

int
main(int argc, char *argv[])
{
    int number;
    int canonical = 0;
    int unicode = 0;

    number = 1;
    while (number < argc) {
        if (strcmp(argv[number], "-c") == 0) {
            canonical = 1;
        }
        else if (strcmp(argv[number], "-u") == 0) {
            unicode = 1;
        }
        else if (argv[number][0] == '-') {
            printf("Unknown option: '%s'\n", argv[number]);
            return 0;
        }
        if (argv[number][0] == '-') {
            if (number < argc-1) {
                memmove(argv+number, argv+number+1, (argc-number-1)*sizeof(char *));
            }
            argc --;
        }
        else {
            number ++;
        }
    }

    if (argc < 2) {
        printf("Usage: %s [-c] [-u] file1.ci ...\n", argv[0]);
        return 0;
    }

    for (number = 1; number < argc; number ++)
    {
        FILE *file;
        ci_parser_t parser;
        ci_emitter_t emitter;

        ci_document_t document;
        unsigned char buffer[BUFFER_SIZE+1];
        size_t written = 0;
        ci_document_t documents[MAX_DOCUMENTS];
        size_t document_number = 0;
        int done = 0;
        int count = 0;
        int error = 0;
        int k;
        memset(buffer, 0, BUFFER_SIZE+1);
        memset(documents, 0, MAX_DOCUMENTS*sizeof(ci_document_t));

        printf("[%d] Loading, dumping, and loading again '%s': ", number, argv[number]);
        fflush(stdout);

        file = fopen(argv[number], "rb");
        assert(file);

        assert(ci_parser_init(&parser));
        ci_parser_set_input_file(&parser, file);
        assert(ci_emitter_initialize(&emitter));
        if (canonical) {
            ci_emitter_set_canonical(&emitter, 1);
        }
        if (unicode) {
            ci_emitter_set_unicode(&emitter, 1);
        }
        ci_emitter_set_output_string(&emitter, buffer, BUFFER_SIZE, &written);
        ci_emitter_open(&emitter);

        while (!done)
        {
            if (!ci_parser_create(&parser, &document)) {
                error = 1;
                break;
            }

            done = (!ci_document_get_root_node(&document));
            if (!done) {
                assert(document_number < MAX_DOCUMENTS);
                assert(copy_document(&(documents[document_number++]), &document));
                assert(ci_emitter_dump(&emitter, &document) || 
                        (ci_emitter_flush(&emitter) && print_output(argv[number], buffer, written, count)));
                count ++;
            }
            else {
                ci_document_delete(&document);
            }
        }

        ci_parser_delete(&parser);
        assert(!fclose(file));
        ci_emitter_close(&emitter);
        ci_emitter_delete(&emitter);

        if (!error)
        {
            count = done = 0;
            assert(ci_parser_init(&parser));
            ci_parser_set_input_string(&parser, buffer, written);

            while (!done)
            {
                assert(ci_parser_create(&parser, &document) || print_output(argv[number], buffer, written, count));
                done = (!ci_document_get_root_node(&document));
                if (!done) {
                    assert(compare_documents(documents+count, &document) || print_output(argv[number], buffer, written, count));
                    count ++;
                }
                ci_document_delete(&document);
            }
            ci_parser_delete(&parser);
        }

        for (k = 0; k < document_number; k ++) {
            ci_document_delete(documents+k);
        }

        printf("PASSED (length: %ld)\n", (long)written);
        print_output(argv[number], buffer, written, -1);
    }

    return 0;
}
