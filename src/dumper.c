
#include "ci_private.h"

/*
 * API functions.
 */

CI_DECLARE(int)
ci_emitter_open(ci_emitter_t *emitter);

CI_DECLARE(int)
ci_emitter_close(ci_emitter_t *emitter);

CI_DECLARE(int)
ci_emitter_dump(ci_emitter_t *emitter, ci_document_t *document);

/*
 * Clean up functions.
 */

static void
ci_emitter_delete_document_and_anchors(ci_emitter_t *emitter);

/*
 * Anchor functions.
 */

static void
ci_emitter_anchor_node(ci_emitter_t *emitter, int index);

static ci_char_t *
ci_emitter_generate_anchor(ci_emitter_t *emitter, int anchor_id);


/*
 * Serialize functions.
 */

static int
ci_emitter_dump_node(ci_emitter_t *emitter, int index);

static int
ci_emitter_dump_alias(ci_emitter_t *emitter, ci_char_t *anchor);

static int
ci_emitter_dump_scalar(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor);

static int
ci_emitter_dump_sequence(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor);

static int
ci_emitter_dump_mapping(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor);

/*
 * Issue a STREAM-START event.
 */

CI_DECLARE(int)
ci_emitter_open(ci_emitter_t *emitter)
{
    ci_event_t event;
    ci_mark_t mark = { 0, 0, 0 };

    assert(emitter);            /* Non-NULL emitter object is required. */
    assert(!emitter->opened);   /* Emitter should not be opened yet. */

    STREAM_START_EVENT_INIT(event, CI_ANY_ENCODING, mark, mark);

    if (!ci_emitter_emit(emitter, &event)) {
        return 0;
    }

    emitter->opened = 1;

    return 1;
}

/*
 * Issue a STREAM-END event.
 */

CI_DECLARE(int)
ci_emitter_close(ci_emitter_t *emitter)
{
    ci_event_t event;
    ci_mark_t mark = { 0, 0, 0 };

    assert(emitter);            /* Non-NULL emitter object is required. */
    assert(emitter->opened);    /* Emitter should be opened. */

    if (emitter->closed) return 1;

    STREAM_END_EVENT_INIT(event, mark, mark);

    if (!ci_emitter_emit(emitter, &event)) {
        return 0;
    }

    emitter->closed = 1;

    return 1;
}

/*
 * Dump a CI document.
 */

CI_DECLARE(int)
ci_emitter_dump(ci_emitter_t *emitter, ci_document_t *document)
{
    ci_event_t event;
    ci_mark_t mark = { 0, 0, 0 };

    assert(emitter);            /* Non-NULL emitter object is required. */
    assert(document);           /* Non-NULL emitter object is expected. */

    emitter->document = document;

    if (!emitter->opened) {
        if (!ci_emitter_open(emitter)) goto error;
    }

    if (STACK_EMPTY(emitter, document->nodes)) {
        if (!ci_emitter_close(emitter)) goto error;
        ci_emitter_delete_document_and_anchors(emitter);
        return 1;
    }

    assert(emitter->opened);    /* Emitter should be opened. */

    emitter->anchors = (ci_anchors_t*)ci_malloc(sizeof(*(emitter->anchors))
            * (document->nodes.top - document->nodes.start));
    if (!emitter->anchors) goto error;
    memset(emitter->anchors, 0, sizeof(*(emitter->anchors))
            * (document->nodes.top - document->nodes.start));

    DOCUMENT_START_EVENT_INIT(event, document->version_directive,
            document->tag_directives.start, document->tag_directives.end,
            document->start_implicit, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) goto error;

    ci_emitter_anchor_node(emitter, 1);
    if (!ci_emitter_dump_node(emitter, 1)) goto error;

    DOCUMENT_END_EVENT_INIT(event, document->end_implicit, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) goto error;

    ci_emitter_delete_document_and_anchors(emitter);

    return 1;

error:

    ci_emitter_delete_document_and_anchors(emitter);

    return 0;
}

/*
 * Clean up the emitter object after a document is dumped.
 */

static void
ci_emitter_delete_document_and_anchors(ci_emitter_t *emitter)
{
    int index;

    if (!emitter->anchors) {
        ci_document_delete(emitter->document);
        emitter->document = NULL;
        return;
    }

    for (index = 0; emitter->document->nodes.start + index
            < emitter->document->nodes.top; index ++) {
        ci_node_t node = emitter->document->nodes.start[index];
        if (!emitter->anchors[index].serialized) {
            ci_free(node.tag);
            if (node.type == CI_SCALAR_NODE) {
                ci_free(node.data.scalar.value);
            }
        }
        if (node.type == CI_SEQUENCE_NODE) {
            STACK_DEL(emitter, node.data.sequence.items);
        }
        if (node.type == CI_MAPPING_NODE) {
            STACK_DEL(emitter, node.data.mapping.pairs);
        }
    }

    STACK_DEL(emitter, emitter->document->nodes);
    ci_free(emitter->anchors);

    emitter->anchors = NULL;
    emitter->last_anchor_id = 0;
    emitter->document = NULL;
}

/*
 * Check the references of a node and assign the anchor id if needed.
 */

static void
ci_emitter_anchor_node(ci_emitter_t *emitter, int index)
{
    ci_node_t *node = emitter->document->nodes.start + index - 1;
    ci_node_item_t *item;
    ci_node_pair_t *pair;

    emitter->anchors[index-1].references ++;

    if (emitter->anchors[index-1].references == 1) {
        switch (node->type) {
            case CI_SEQUENCE_NODE:
                for (item = node->data.sequence.items.start;
                        item < node->data.sequence.items.top; item ++) {
                    ci_emitter_anchor_node(emitter, *item);
                }
                break;
            case CI_MAPPING_NODE:
                for (pair = node->data.mapping.pairs.start;
                        pair < node->data.mapping.pairs.top; pair ++) {
                    ci_emitter_anchor_node(emitter, pair->key);
                    ci_emitter_anchor_node(emitter, pair->value);
                }
                break;
            default:
                break;
        }
    }

    else if (emitter->anchors[index-1].references == 2) {
        emitter->anchors[index-1].anchor = (++ emitter->last_anchor_id);
    }
}

/*
 * Generate a textual representation for an anchor.
 */

#define ANCHOR_TEMPLATE         "id%03d"
#define ANCHOR_TEMPLATE_LENGTH  16

static ci_char_t *
ci_emitter_generate_anchor(SHIM(ci_emitter_t *emitter), int anchor_id)
{
    ci_char_t *anchor = CI_MALLOC(ANCHOR_TEMPLATE_LENGTH);

    if (!anchor) return NULL;

    sprintf((char *)anchor, ANCHOR_TEMPLATE, anchor_id);

    return anchor;
}

/*
 * Serialize a node.
 */

static int
ci_emitter_dump_node(ci_emitter_t *emitter, int index)
{
    ci_node_t *node = emitter->document->nodes.start + index - 1;
    int anchor_id = emitter->anchors[index-1].anchor;
    ci_char_t *anchor = NULL;

    if (anchor_id) {
        anchor = ci_emitter_generate_anchor(emitter, anchor_id);
        if (!anchor) return 0;
    }

    if (emitter->anchors[index-1].serialized) {
        return ci_emitter_dump_alias(emitter, anchor);
    }

    emitter->anchors[index-1].serialized = 1;

    switch (node->type) {
        case CI_SCALAR_NODE:
            return ci_emitter_dump_scalar(emitter, node, anchor);
        case CI_SEQUENCE_NODE:
            return ci_emitter_dump_sequence(emitter, node, anchor);
        case CI_MAPPING_NODE:
            return ci_emitter_dump_mapping(emitter, node, anchor);
        default:
            assert(0);      /* Could not happen. */
            break;
    }

    return 0;       /* Could not happen. */
}

/*
 * Serialize an alias.
 */

static int
ci_emitter_dump_alias(ci_emitter_t *emitter, ci_char_t *anchor)
{
    ci_event_t event;
    ci_mark_t mark  = { 0, 0, 0 };

    ALIAS_EVENT_INIT(event, anchor, mark, mark);

    return ci_emitter_emit(emitter, &event);
}

/*
 * Serialize a scalar.
 */

static int
ci_emitter_dump_scalar(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor)
{
    ci_event_t event;
    ci_mark_t mark  = { 0, 0, 0 };

    int plain_implicit = (strcmp((char *)node->tag,
                CI_DEFAULT_SCALAR_TAG) == 0);
    int quoted_implicit = (strcmp((char *)node->tag,
                CI_DEFAULT_SCALAR_TAG) == 0);

    SCALAR_EVENT_INIT(event, anchor, node->tag, node->data.scalar.value,
            node->data.scalar.length, plain_implicit, quoted_implicit,
            node->data.scalar.style, mark, mark);

    return ci_emitter_emit(emitter, &event);
}

/*
 * Serialize a sequence.
 */

static int
ci_emitter_dump_sequence(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor)
{
    ci_event_t event;
    ci_mark_t mark  = { 0, 0, 0 };

    int implicit = (strcmp((char *)node->tag, CI_DEFAULT_SEQUENCE_TAG) == 0);

    ci_node_item_t *item;

    SEQUENCE_START_EVENT_INIT(event, anchor, node->tag, implicit,
            node->data.sequence.style, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) return 0;

    for (item = node->data.sequence.items.start;
            item < node->data.sequence.items.top; item ++) {
        if (!ci_emitter_dump_node(emitter, *item)) return 0;
    }

    SEQUENCE_END_EVENT_INIT(event, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) return 0;

    return 1;
}

/*
 * Serialize a mapping.
 */

static int
ci_emitter_dump_mapping(ci_emitter_t *emitter, ci_node_t *node,
        ci_char_t *anchor)
{
    ci_event_t event;
    ci_mark_t mark  = { 0, 0, 0 };

    int implicit = (strcmp((char *)node->tag, CI_DEFAULT_MAPPING_TAG) == 0);

    ci_node_pair_t *pair;

    MAPPING_START_EVENT_INIT(event, anchor, node->tag, implicit,
            node->data.mapping.style, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) return 0;

    for (pair = node->data.mapping.pairs.start;
            pair < node->data.mapping.pairs.top; pair ++) {
        if (!ci_emitter_dump_node(emitter, pair->key)) return 0;
        if (!ci_emitter_dump_node(emitter, pair->value)) return 0;
    }

    MAPPING_END_EVENT_INIT(event, mark, mark);
    if (!ci_emitter_emit(emitter, &event)) return 0;

    return 1;
}

