
#include "ci_private.h"

/*
 * API functions.
 */

CI_DECLARE(int)
ci_parser_create(ci_parser_t *parser, ci_document_t *document);

/*
 * Error handling.
 */

static int
ci_parser_set_composer_error(ci_parser_t *parser,
        const char *problem, ci_mark_t problem_mark);

static int
ci_parser_set_composer_error_context(ci_parser_t *parser,
        const char *context, ci_mark_t context_mark,
        const char *problem, ci_mark_t problem_mark);


/*
 * Alias handling.
 */

static int
ci_parser_register_anchor(ci_parser_t *parser,
        int index, ci_char_t *anchor);

/*
 * Clean up functions.
 */

static void
ci_parser_delete_aliases(ci_parser_t *parser);

/*
 * Document loading context.
 */
struct loader_ctx {
    int *start;
    int *end;
    int *top;
};

/*
 * Composer functions.
 */
static int
ci_parser_create_nodes(ci_parser_t *parser, struct loader_ctx *ctx);

static int
ci_parser_create_document(ci_parser_t *parser, ci_event_t *event);

static int
ci_parser_create_alias(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

static int
ci_parser_create_scalar(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

static int
ci_parser_create_sequence(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

static int
ci_parser_create_mapping(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

static int
ci_parser_create_sequence_end(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

static int
ci_parser_create_mapping_end(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx);

/*
 * Load the next document of the stream.
 */

CI_DECLARE(int)
ci_parser_create(ci_parser_t *parser, ci_document_t *document)
{
    ci_event_t event;

    assert(parser);     /* Non-NULL parser object is expected. */
    assert(document);   /* Non-NULL document object is expected. */

    memset(document, 0, sizeof(ci_document_t));
    if (!STACK_INIT(parser, document->nodes, ci_node_t*))
        goto error;

    if (!parser->stream_start_produced) {
        if (!ci_parser_parse(parser, &event)) goto error;
        assert(event.type == CI_STREAM_START_EVENT);
                        /* STREAM-START is expected. */
    }

    if (parser->stream_end_produced) {
        return 1;
    }

    if (!ci_parser_parse(parser, &event)) goto error;
    if (event.type == CI_STREAM_END_EVENT) {
        return 1;
    }

    if (!STACK_INIT(parser, parser->aliases, ci_alias_data_t*))
        goto error;

    parser->document = document;

    if (!ci_parser_create_document(parser, &event)) goto error;

    ci_parser_delete_aliases(parser);
    parser->document = NULL;

    return 1;

error:

    ci_parser_delete_aliases(parser);
    ci_document_delete(document);
    parser->document = NULL;

    return 0;
}

/*
 * Set composer error.
 */

static int
ci_parser_set_composer_error(ci_parser_t *parser,
        const char *problem, ci_mark_t problem_mark)
{
    parser->error = CI_COMPOSER_ERROR;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}

/*
 * Set composer error with context.
 */

static int
ci_parser_set_composer_error_context(ci_parser_t *parser,
        const char *context, ci_mark_t context_mark,
        const char *problem, ci_mark_t problem_mark)
{
    parser->error = CI_COMPOSER_ERROR;
    parser->context = context;
    parser->context_mark = context_mark;
    parser->problem = problem;
    parser->problem_mark = problem_mark;

    return 0;
}

/*
 * Delete the stack of aliases.
 */

static void
ci_parser_delete_aliases(ci_parser_t *parser)
{
    while (!STACK_EMPTY(parser, parser->aliases)) {
        ci_free(POP(parser, parser->aliases).anchor);
    }
    STACK_DEL(parser, parser->aliases);
}

/*
 * Compose a document object.
 */

static int
ci_parser_create_document(ci_parser_t *parser, ci_event_t *event)
{
    struct loader_ctx ctx = { NULL, NULL, NULL };

    assert(event->type == CI_DOCUMENT_START_EVENT);
                        /* DOCUMENT-START is expected. */

    parser->document->version_directive
        = event->data.document_start.version_directive;
    parser->document->tag_directives.start
        = event->data.document_start.tag_directives.start;
    parser->document->tag_directives.end
        = event->data.document_start.tag_directives.end;
    parser->document->start_implicit
        = event->data.document_start.implicit;
    parser->document->start_mark = event->start_mark;

    if (!STACK_INIT(parser, ctx, int*)) return 0;
    if (!ci_parser_create_nodes(parser, &ctx)) {
        STACK_DEL(parser, ctx);
        return 0;
    }
    STACK_DEL(parser, ctx);

    return 1;
}

/*
 * Compose a node tree.
 */

static int
ci_parser_create_nodes(ci_parser_t *parser, struct loader_ctx *ctx)
{
    ci_event_t event;

    do {
        if (!ci_parser_parse(parser, &event)) return 0;

        switch (event.type) {
            case CI_ALIAS_EVENT:
                if (!ci_parser_create_alias(parser, &event, ctx)) return 0;
                break;
            case CI_SCALAR_EVENT:
                if (!ci_parser_create_scalar(parser, &event, ctx)) return 0;
                break;
            case CI_SEQUENCE_START_EVENT:
                if (!ci_parser_create_sequence(parser, &event, ctx)) return 0;
                break;
            case CI_SEQUENCE_END_EVENT:
                if (!ci_parser_create_sequence_end(parser, &event, ctx))
                    return 0;
                break;
            case CI_MAPPING_START_EVENT:
                if (!ci_parser_create_mapping(parser, &event, ctx)) return 0;
                break;
            case CI_MAPPING_END_EVENT:
                if (!ci_parser_create_mapping_end(parser, &event, ctx))
                    return 0;
                break;
            default:
                assert(0);  /* Could not happen. */
                return 0;
            case CI_DOCUMENT_END_EVENT:
                break;
        }
    } while (event.type != CI_DOCUMENT_END_EVENT);

    parser->document->end_implicit = event.data.document_end.implicit;
    parser->document->end_mark = event.end_mark;

    return 1;
}

/*
 * Add an anchor.
 */

static int
ci_parser_register_anchor(ci_parser_t *parser,
        int index, ci_char_t *anchor)
{
    ci_alias_data_t data;
    ci_alias_data_t *alias_data;

    if (!anchor) return 1;

    data.anchor = anchor;
    data.index = index;
    data.mark = parser->document->nodes.start[index-1].start_mark;

    for (alias_data = parser->aliases.start;
            alias_data != parser->aliases.top; alias_data ++) {
        if (strcmp((char *)alias_data->anchor, (char *)anchor) == 0) {
            ci_free(anchor);
            return ci_parser_set_composer_error_context(parser,
                    "found duplicate anchor; first occurrence",
                    alias_data->mark, "second occurrence", data.mark);
        }
    }

    if (!PUSH(parser, parser->aliases, data)) {
        ci_free(anchor);
        return 0;
    }

    return 1;
}

/*
 * Compose node into its parent in the stree.
 */

static int
ci_parser_create_node_add(ci_parser_t *parser, struct loader_ctx *ctx,
        int index)
{
    struct ci_node_s *parent;
    int parent_index;

    if (STACK_EMPTY(parser, *ctx)) {
        /* This is the root node, there's no tree to add it to. */
        return 1;
    }

    parent_index = *((*ctx).top - 1);
    parent = &parser->document->nodes.start[parent_index-1];

    switch (parent->type) {
        case CI_SEQUENCE_NODE:
            if (!STACK_LIMIT(parser, parent->data.sequence.items, INT_MAX-1))
                return 0;
            if (!PUSH(parser, parent->data.sequence.items, index))
                return 0;
            break;
        case CI_MAPPING_NODE: {
            ci_node_pair_t pair;
            if (!STACK_EMPTY(parser, parent->data.mapping.pairs)) {
                ci_node_pair_t *p = parent->data.mapping.pairs.top - 1;
                if (p->key != 0 && p->value == 0) {
                    p->value = index;
                    break;
                }
            }

            pair.key = index;
            pair.value = 0;
            if (!STACK_LIMIT(parser, parent->data.mapping.pairs, INT_MAX-1))
                return 0;
            if (!PUSH(parser, parent->data.mapping.pairs, pair))
                return 0;

            break;
        }
        default:
            assert(0); /* Could not happen. */
            return 0;
    }
    return 1;
}

/*
 * Compose a node corresponding to an alias.
 */

static int
ci_parser_create_alias(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    ci_char_t *anchor = event->data.alias.anchor;
    ci_alias_data_t *alias_data;

    for (alias_data = parser->aliases.start;
            alias_data != parser->aliases.top; alias_data ++) {
        if (strcmp((char *)alias_data->anchor, (char *)anchor) == 0) {
            ci_free(anchor);
            return ci_parser_create_node_add(parser, ctx, alias_data->index);
        }
    }

    ci_free(anchor);
    return ci_parser_set_composer_error(parser, "found undefined alias",
            event->start_mark);
}

/*
 * Compose a scalar node.
 */

static int
ci_parser_create_scalar(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    ci_node_t node;
    int index;
    ci_char_t *tag = event->data.scalar.tag;

    if (!STACK_LIMIT(parser, parser->document->nodes, INT_MAX-1)) goto error;

    if (!tag || strcmp((char *)tag, "!") == 0) {
        ci_free(tag);
        tag = ci_strdup((ci_char_t *)CI_DEFAULT_SCALAR_TAG);
        if (!tag) goto error;
    }

    SCALAR_NODE_INIT(node, tag, event->data.scalar.value,
            event->data.scalar.length, event->data.scalar.style,
            event->start_mark, event->end_mark);

    if (!PUSH(parser, parser->document->nodes, node)) goto error;

    index = parser->document->nodes.top - parser->document->nodes.start;

    if (!ci_parser_register_anchor(parser, index,
                event->data.scalar.anchor)) return 0;

    return ci_parser_create_node_add(parser, ctx, index);

error:
    ci_free(tag);
    ci_free(event->data.scalar.anchor);
    ci_free(event->data.scalar.value);
    return 0;
}

/*
 * Compose a sequence node.
 */

static int
ci_parser_create_sequence(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    ci_node_t node;
    struct {
        ci_node_item_t *start;
        ci_node_item_t *end;
        ci_node_item_t *top;
    } items = { NULL, NULL, NULL };
    int index;
    ci_char_t *tag = event->data.sequence_start.tag;

    if (!STACK_LIMIT(parser, parser->document->nodes, INT_MAX-1)) goto error;

    if (!tag || strcmp((char *)tag, "!") == 0) {
        ci_free(tag);
        tag = ci_strdup((ci_char_t *)CI_DEFAULT_SEQUENCE_TAG);
        if (!tag) goto error;
    }

    if (!STACK_INIT(parser, items, ci_node_item_t*)) goto error;

    SEQUENCE_NODE_INIT(node, tag, items.start, items.end,
            event->data.sequence_start.style,
            event->start_mark, event->end_mark);

    if (!PUSH(parser, parser->document->nodes, node)) goto error;

    index = parser->document->nodes.top - parser->document->nodes.start;

    if (!ci_parser_register_anchor(parser, index,
                event->data.sequence_start.anchor)) return 0;

    if (!ci_parser_create_node_add(parser, ctx, index)) return 0;

    if (!STACK_LIMIT(parser, *ctx, INT_MAX-1)) return 0;
    if (!PUSH(parser, *ctx, index)) return 0;

    return 1;

error:
    ci_free(tag);
    ci_free(event->data.sequence_start.anchor);
    return 0;
}

static int
ci_parser_create_sequence_end(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    int index;

    assert(((*ctx).top - (*ctx).start) > 0);

    index = *((*ctx).top - 1);
    assert(parser->document->nodes.start[index-1].type == CI_SEQUENCE_NODE);
    parser->document->nodes.start[index-1].end_mark = event->end_mark;

    (void)POP(parser, *ctx);

    return 1;
}

/*
 * Compose a mapping node.
 */

static int
ci_parser_create_mapping(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    ci_node_t node;
    struct {
        ci_node_pair_t *start;
        ci_node_pair_t *end;
        ci_node_pair_t *top;
    } pairs = { NULL, NULL, NULL };
    int index;
    ci_char_t *tag = event->data.mapping_start.tag;

    if (!STACK_LIMIT(parser, parser->document->nodes, INT_MAX-1)) goto error;

    if (!tag || strcmp((char *)tag, "!") == 0) {
        ci_free(tag);
        tag = ci_strdup((ci_char_t *)CI_DEFAULT_MAPPING_TAG);
        if (!tag) goto error;
    }

    if (!STACK_INIT(parser, pairs, ci_node_pair_t*)) goto error;

    MAPPING_NODE_INIT(node, tag, pairs.start, pairs.end,
            event->data.mapping_start.style,
            event->start_mark, event->end_mark);

    if (!PUSH(parser, parser->document->nodes, node)) goto error;

    index = parser->document->nodes.top - parser->document->nodes.start;

    if (!ci_parser_register_anchor(parser, index,
                event->data.mapping_start.anchor)) return 0;

    if (!ci_parser_create_node_add(parser, ctx, index)) return 0;

    if (!STACK_LIMIT(parser, *ctx, INT_MAX-1)) return 0;
    if (!PUSH(parser, *ctx, index)) return 0;

    return 1;

error:
    ci_free(tag);
    ci_free(event->data.mapping_start.anchor);
    return 0;
}

static int
ci_parser_create_mapping_end(ci_parser_t *parser, ci_event_t *event,
        struct loader_ctx *ctx)
{
    int index;

    assert(((*ctx).top - (*ctx).start) > 0);

    index = *((*ctx).top - 1);
    assert(parser->document->nodes.start[index-1].type == CI_MAPPING_NODE);
    parser->document->nodes.start[index-1].end_mark = event->end_mark;

    (void)POP(parser, *ctx);

    return 1;
}
