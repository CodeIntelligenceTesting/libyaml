
#include "ci_private.h"

/*
 * Get the library version.
 */

CI_DECLARE(const char *)
ci_get_version_string(void)
{
    return "";
}

/*
 * Get the library version numbers.
 */

CI_DECLARE(void)
ci_get_version(int *major, int *minor, int *patch)
{
    *major = 0;
    *minor = 0;
    *patch = 0;
}

/*
 * Allocate a dynamic memory block.
 */

CI_DECLARE(void *)
ci_malloc(size_t size)
{
    return malloc(size ? size : 1);
}

/*
 * Reallocate a dynamic memory block.
 */

CI_DECLARE(void *)
ci_realloc(void *ptr, size_t size)
{
    return ptr ? realloc(ptr, size ? size : 1) : malloc(size ? size : 1);
}

/*
 * Free a dynamic memory block.
 */

CI_DECLARE(void)
ci_free(void *ptr)
{
    if (ptr) free(ptr);
}

/*
 * Duplicate a string.
 */

CI_DECLARE(ci_char_t *)
ci_strdup(const ci_char_t *str)
{
    if (!str)
        return NULL;

    return (ci_char_t *)strdup((char *)str);
}

/*
 * Extend a string.
 */

CI_DECLARE(int)
ci_string_extend(ci_char_t **start,
        ci_char_t **pointer, ci_char_t **end)
{
    ci_char_t *new_start = (ci_char_t *)ci_realloc((void*)*start, (*end - *start)*2);

    if (!new_start) return 0;

    memset(new_start + (*end - *start), 0, *end - *start);

    *pointer = new_start + (*pointer - *start);
    *end = new_start + (*end - *start)*2;
    *start = new_start;

    return 1;
}

/*
 * Append a string B to a string A.
 */

CI_DECLARE(int)
ci_string_join(
        ci_char_t **a_start, ci_char_t **a_pointer, ci_char_t **a_end,
        ci_char_t **b_start, ci_char_t **b_pointer, SHIM(ci_char_t **b_end))
{
    UNUSED_PARAM(b_end)
    if (*b_start == *b_pointer)
        return 1;

    while (*a_end - *a_pointer <= *b_pointer - *b_start) {
        if (!ci_string_extend(a_start, a_pointer, a_end))
            return 0;
    }

    memcpy(*a_pointer, *b_start, *b_pointer - *b_start);
    *a_pointer += *b_pointer - *b_start;

    return 1;
}

/*
 * Extend a stack.
 */

CI_DECLARE(int)
ci_stack_extend(void **start, void **top, void **end)
{
    void *new_start;

    if ((char *)*end - (char *)*start >= INT_MAX / 2)
	return 0;

    new_start = ci_realloc(*start, ((char *)*end - (char *)*start)*2);

    if (!new_start) return 0;

    *top = (char *)new_start + ((char *)*top - (char *)*start);
    *end = (char *)new_start + ((char *)*end - (char *)*start)*2;
    *start = new_start;

    return 1;
}

/*
 * Extend or move a queue.
 */

CI_DECLARE(int)
ci_queue_extend(void **start, void **head, void **tail, void **end)
{
    /* Check if we need to resize the queue. */

    if (*start == *head && *tail == *end) {
        void *new_start = ci_realloc(*start,
                ((char *)*end - (char *)*start)*2);

        if (!new_start) return 0;

        *head = (char *)new_start + ((char *)*head - (char *)*start);
        *tail = (char *)new_start + ((char *)*tail - (char *)*start);
        *end = (char *)new_start + ((char *)*end - (char *)*start)*2;
        *start = new_start;
    }

    /* Check if we need to move the queue at the beginning of the buffer. */

    if (*tail == *end) {
        if (*head != *tail) {
            memmove(*start, *head, (char *)*tail - (char *)*head);
        }
        *tail = (char *)*tail - (char *)*head + (char *)*start;
        *head = *start;
    }

    return 1;
}


/*
 * Create a new parser object.
 */

CI_DECLARE(int)
ci_parser_init(ci_parser_t *parser)
{
    assert(parser);     /* Non-NULL parser object expected. */

    memset(parser, 0, sizeof(ci_parser_t));
    if (!BUFFER_INIT(parser, parser->raw_buffer, INPUT_RAW_BUFFER_SIZE))
        goto error;
    if (!BUFFER_INIT(parser, parser->buffer, INPUT_BUFFER_SIZE))
        goto error;
    if (!QUEUE_INIT(parser, parser->tokens, INITIAL_QUEUE_SIZE, ci_token_t*))
        goto error;
    if (!STACK_INIT(parser, parser->indents, int*))
        goto error;
    if (!STACK_INIT(parser, parser->simple_keys, ci_simple_key_t*))
        goto error;
    if (!STACK_INIT(parser, parser->states, ci_parser_state_t*))
        goto error;
    if (!STACK_INIT(parser, parser->marks, ci_mark_t*))
        goto error;
    if (!STACK_INIT(parser, parser->tag_directives, ci_tag_directive_t*))
        goto error;

    return 1;

error:

    BUFFER_DEL(parser, parser->raw_buffer);
    BUFFER_DEL(parser, parser->buffer);
    QUEUE_DEL(parser, parser->tokens);
    STACK_DEL(parser, parser->indents);
    STACK_DEL(parser, parser->simple_keys);
    STACK_DEL(parser, parser->states);
    STACK_DEL(parser, parser->marks);
    STACK_DEL(parser, parser->tag_directives);

    return 0;
}

/*
 * Destroy a parser object.
 */

CI_DECLARE(void)
ci_parser_delete(ci_parser_t *parser)
{
    assert(parser); /* Non-NULL parser object expected. */

    BUFFER_DEL(parser, parser->raw_buffer);
    BUFFER_DEL(parser, parser->buffer);
    while (!QUEUE_EMPTY(parser, parser->tokens)) {
        ci_token_delete(&DEQUEUE(parser, parser->tokens));
    }
    QUEUE_DEL(parser, parser->tokens);
    STACK_DEL(parser, parser->indents);
    STACK_DEL(parser, parser->simple_keys);
    STACK_DEL(parser, parser->states);
    STACK_DEL(parser, parser->marks);
    while (!STACK_EMPTY(parser, parser->tag_directives)) {
        ci_tag_directive_t tag_directive = POP(parser, parser->tag_directives);
        ci_free(tag_directive.handle);
        ci_free(tag_directive.prefix);
    }
    STACK_DEL(parser, parser->tag_directives);

    memset(parser, 0, sizeof(ci_parser_t));
}

/*
 * String read handler.
 */

static int
ci_string_read_handler(void *data, unsigned char *buffer, size_t size,
        size_t *size_read)
{
    ci_parser_t *parser = (ci_parser_t *)data;

    if (parser->input.string.current == parser->input.string.end) {
        *size_read = 0;
        return 1;
    }

    if (size > (size_t)(parser->input.string.end
                - parser->input.string.current)) {
        size = parser->input.string.end - parser->input.string.current;
    }

    memcpy(buffer, parser->input.string.current, size);
    parser->input.string.current += size;
    *size_read = size;
    return 1;
}

/*
 * File read handler.
 */

static int
ci_file_read_handler(void *data, unsigned char *buffer, size_t size,
        size_t *size_read)
{
    ci_parser_t *parser = (ci_parser_t *)data;

    *size_read = fread(buffer, 1, size, parser->input.file);
    return !ferror(parser->input.file);
}

/*
 * Set a string input.
 */

CI_DECLARE(void)
ci_parser_set_input_string(ci_parser_t *parser,
        const unsigned char *input, size_t size)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(input);  /* Non-NULL input string expected. */

    parser->read_handler = ci_string_read_handler;
    parser->read_handler_data = parser;

    parser->input.string.start = input;
    parser->input.string.current = input;
    parser->input.string.end = input+size;
}

/*
 * Set a file input.
 */

CI_DECLARE(void)
ci_parser_set_input_file(ci_parser_t *parser, FILE *file)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(file);   /* Non-NULL file object expected. */

    parser->read_handler = ci_file_read_handler;
    parser->read_handler_data = parser;

    parser->input.file = file;
}

/*
 * Set a generic input.
 */

CI_DECLARE(void)
ci_parser_set_input(ci_parser_t *parser,
        ci_read_handler_t *handler, void *data)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->read_handler);  /* You can set the source only once. */
    assert(handler);    /* Non-NULL read handler expected. */

    parser->read_handler = handler;
    parser->read_handler_data = data;
}

/*
 * Set the source encoding.
 */

CI_DECLARE(void)
ci_parser_set_encoding(ci_parser_t *parser, ci_encoding_t encoding)
{
    assert(parser); /* Non-NULL parser object expected. */
    assert(!parser->encoding); /* Encoding is already set or detected. */

    parser->encoding = encoding;
}

/*
 * Create a new emitter object.
 */

CI_DECLARE(int)
ci_emitter_initialize(ci_emitter_t *emitter)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    memset(emitter, 0, sizeof(ci_emitter_t));
    if (!BUFFER_INIT(emitter, emitter->buffer, OUTPUT_BUFFER_SIZE))
        goto error;
    if (!BUFFER_INIT(emitter, emitter->raw_buffer, OUTPUT_RAW_BUFFER_SIZE))
        goto error;
    if (!STACK_INIT(emitter, emitter->states, ci_emitter_state_t*))
        goto error;
    if (!QUEUE_INIT(emitter, emitter->events, INITIAL_QUEUE_SIZE, ci_event_t*))
        goto error;
    if (!STACK_INIT(emitter, emitter->indents, int*))
        goto error;
    if (!STACK_INIT(emitter, emitter->tag_directives, ci_tag_directive_t*))
        goto error;

    return 1;

error:

    BUFFER_DEL(emitter, emitter->buffer);
    BUFFER_DEL(emitter, emitter->raw_buffer);
    STACK_DEL(emitter, emitter->states);
    QUEUE_DEL(emitter, emitter->events);
    STACK_DEL(emitter, emitter->indents);
    STACK_DEL(emitter, emitter->tag_directives);

    return 0;
}

/*
 * Destroy an emitter object.
 */

CI_DECLARE(void)
ci_emitter_delete(ci_emitter_t *emitter)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    BUFFER_DEL(emitter, emitter->buffer);
    BUFFER_DEL(emitter, emitter->raw_buffer);
    STACK_DEL(emitter, emitter->states);
    while (!QUEUE_EMPTY(emitter, emitter->events)) {
        ci_event_delete(&DEQUEUE(emitter, emitter->events));
    }
    QUEUE_DEL(emitter, emitter->events);
    STACK_DEL(emitter, emitter->indents);
    while (!STACK_EMPTY(empty, emitter->tag_directives)) {
        ci_tag_directive_t tag_directive = POP(emitter, emitter->tag_directives);
        ci_free(tag_directive.handle);
        ci_free(tag_directive.prefix);
    }
    STACK_DEL(emitter, emitter->tag_directives);
    ci_free(emitter->anchors);

    memset(emitter, 0, sizeof(ci_emitter_t));
}

/*
 * String write handler.
 */

static int
ci_string_write_handler(void *data, unsigned char *buffer, size_t size)
{
  ci_emitter_t *emitter = (ci_emitter_t *)data;

    if (emitter->output.string.size - *emitter->output.string.size_written
            < size) {
        memcpy(emitter->output.string.buffer
                + *emitter->output.string.size_written,
                buffer,
                emitter->output.string.size
                - *emitter->output.string.size_written);
        *emitter->output.string.size_written = emitter->output.string.size;
        return 0;
    }

    memcpy(emitter->output.string.buffer
            + *emitter->output.string.size_written, buffer, size);
    *emitter->output.string.size_written += size;
    return 1;
}

/*
 * File write handler.
 */

static int
ci_file_write_handler(void *data, unsigned char *buffer, size_t size)
{
    ci_emitter_t *emitter = (ci_emitter_t *)data;

    return (fwrite(buffer, 1, size, emitter->output.file) == size);
}
/*
 * Set a string output.
 */

CI_DECLARE(void)
ci_emitter_set_output_string(ci_emitter_t *emitter,
        unsigned char *output, size_t size, size_t *size_written)
{
    assert(emitter);    /* Non-NULL emitter object expected. */
    assert(!emitter->write_handler);    /* You can set the output only once. */
    assert(output);     /* Non-NULL output string expected. */

    emitter->write_handler = ci_string_write_handler;
    emitter->write_handler_data = emitter;

    emitter->output.string.buffer = output;
    emitter->output.string.size = size;
    emitter->output.string.size_written = size_written;
    *size_written = 0;
}

/*
 * Set a file output.
 */

CI_DECLARE(void)
ci_emitter_set_output_file(ci_emitter_t *emitter, FILE *file)
{
    assert(emitter);    /* Non-NULL emitter object expected. */
    assert(!emitter->write_handler);    /* You can set the output only once. */
    assert(file);       /* Non-NULL file object expected. */

    emitter->write_handler = ci_file_write_handler;
    emitter->write_handler_data = emitter;

    emitter->output.file = file;
}

/*
 * Set a generic output handler.
 */

CI_DECLARE(void)
ci_emitter_set_output(ci_emitter_t *emitter,
        ci_write_handler_t *handler, void *data)
{
    assert(emitter);    /* Non-NULL emitter object expected. */
    assert(!emitter->write_handler);    /* You can set the output only once. */
    assert(handler);    /* Non-NULL handler object expected. */

    emitter->write_handler = handler;
    emitter->write_handler_data = data;
}

/*
 * Set the output encoding.
 */

CI_DECLARE(void)
ci_emitter_set_encoding(ci_emitter_t *emitter, ci_encoding_t encoding)
{
    assert(emitter);    /* Non-NULL emitter object expected. */
    assert(!emitter->encoding);     /* You can set encoding only once. */

    emitter->encoding = encoding;
}

/*
 * Set the canonical output style.
 */

CI_DECLARE(void)
ci_emitter_set_canonical(ci_emitter_t *emitter, int canonical)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    emitter->canonical = (canonical != 0);
}

/*
 * Set the indentation increment.
 */

CI_DECLARE(void)
ci_emitter_set_indent(ci_emitter_t *emitter, int indent)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    emitter->best_indent = (1 < indent && indent < 10) ? indent : 2;
}

/*
 * Set the preferred line width.
 */

CI_DECLARE(void)
ci_emitter_set_width(ci_emitter_t *emitter, int width)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    emitter->best_width = (width >= 0) ? width : -1;
}

/*
 * Set if unescaped non-ASCII characters are allowed.
 */

CI_DECLARE(void)
ci_emitter_set_unicode(ci_emitter_t *emitter, int unicode)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    emitter->unicode = (unicode != 0);
}

/*
 * Set the preferred line break character.
 */

CI_DECLARE(void)
ci_emitter_set_break(ci_emitter_t *emitter, ci_break_t line_break)
{
    assert(emitter);    /* Non-NULL emitter object expected. */

    emitter->line_break = line_break;
}

/*
 * Destroy a token object.
 */

CI_DECLARE(void)
ci_token_delete(ci_token_t *token)
{
    assert(token);  /* Non-NULL token object expected. */

    switch (token->type)
    {
        case CI_TAG_DIRECTIVE_TOKEN:
            ci_free(token->data.tag_directive.handle);
            ci_free(token->data.tag_directive.prefix);
            break;

        case CI_ALIAS_TOKEN:
            ci_free(token->data.alias.value);
            break;

        case CI_ANCHOR_TOKEN:
            ci_free(token->data.anchor.value);
            break;

        case CI_TAG_TOKEN:
            ci_free(token->data.tag.handle);
            ci_free(token->data.tag.suffix);
            break;

        case CI_SCALAR_TOKEN:
            ci_free(token->data.scalar.value);
            break;

        default:
            break;
    }

    memset(token, 0, sizeof(ci_token_t));
}

/*
 * Check if a string is a valid UTF-8 sequence.
 *
 * Check 'reader.c' for more details on UTF-8 encoding.
 */

static int
ci_check_utf8(const ci_char_t *start, size_t length)
{
    const ci_char_t *end = start+length;
    const ci_char_t *pointer = start;

    while (pointer < end) {
        unsigned char octet;
        unsigned int width;
        unsigned int value;
        size_t k;

        octet = pointer[0];
        width = (octet & 0x80) == 0x00 ? 1 :
                (octet & 0xE0) == 0xC0 ? 2 :
                (octet & 0xF0) == 0xE0 ? 3 :
                (octet & 0xF8) == 0xF0 ? 4 : 0;
        value = (octet & 0x80) == 0x00 ? octet & 0x7F :
                (octet & 0xE0) == 0xC0 ? octet & 0x1F :
                (octet & 0xF0) == 0xE0 ? octet & 0x0F :
                (octet & 0xF8) == 0xF0 ? octet & 0x07 : 0;
        if (!width) return 0;
        if (pointer+width > end) return 0;
        for (k = 1; k < width; k ++) {
            octet = pointer[k];
            if ((octet & 0xC0) != 0x80) return 0;
            value = (value << 6) + (octet & 0x3F);
        }
        if (!((width == 1) ||
            (width == 2 && value >= 0x80) ||
            (width == 3 && value >= 0x800) ||
            (width == 4 && value >= 0x10000))) return 0;

        pointer += width;
    }

    return 1;
}

/*
 * Create STREAM-START.
 */

CI_DECLARE(int)
ci_stream_start_event_initialize(ci_event_t *event,
        ci_encoding_t encoding)
{
    ci_mark_t mark = { 0, 0, 0 };

    assert(event);  /* Non-NULL event object is expected. */

    STREAM_START_EVENT_INIT(*event, encoding, mark, mark);

    return 1;
}

/*
 * Create STREAM-END.
 */

CI_DECLARE(int)
ci_stream_end_event_initialize(ci_event_t *event)
{
    ci_mark_t mark = { 0, 0, 0 };

    assert(event);  /* Non-NULL event object is expected. */

    STREAM_END_EVENT_INIT(*event, mark, mark);

    return 1;
}

/*
 * Create DOCUMENT-START.
 */

CI_DECLARE(int)
ci_document_start_event_initialize(ci_event_t *event,
        ci_version_directive_t *version_directive,
        ci_tag_directive_t *tag_directives_start,
        ci_tag_directive_t *tag_directives_end,
        int implicit)
{
    struct {
        ci_error_type_t error;
    } context;
    ci_mark_t mark = { 0, 0, 0 };
    ci_version_directive_t *version_directive_copy = NULL;
    struct {
        ci_tag_directive_t *start;
        ci_tag_directive_t *end;
        ci_tag_directive_t *top;
    } tag_directives_copy = { NULL, NULL, NULL };
    ci_tag_directive_t value = { NULL, NULL };

    assert(event);          /* Non-NULL event object is expected. */
    assert((tag_directives_start && tag_directives_end) ||
            (tag_directives_start == tag_directives_end));
                            /* Valid tag directives are expected. */

    if (version_directive) {
        version_directive_copy = CI_MALLOC_STATIC(ci_version_directive_t);
        if (!version_directive_copy) goto error;
        version_directive_copy->major = version_directive->major;
        version_directive_copy->minor = version_directive->minor;
    }

    if (tag_directives_start != tag_directives_end) {
        ci_tag_directive_t *tag_directive;
        if (!STACK_INIT(&context, tag_directives_copy, ci_tag_directive_t*))
            goto error;
        for (tag_directive = tag_directives_start;
                tag_directive != tag_directives_end; tag_directive ++) {
            assert(tag_directive->handle);
            assert(tag_directive->prefix);
            if (!ci_check_utf8(tag_directive->handle,
                        strlen((char *)tag_directive->handle)))
                goto error;
            if (!ci_check_utf8(tag_directive->prefix,
                        strlen((char *)tag_directive->prefix)))
                goto error;
            value.handle = ci_strdup(tag_directive->handle);
            value.prefix = ci_strdup(tag_directive->prefix);
            if (!value.handle || !value.prefix) goto error;
            if (!PUSH(&context, tag_directives_copy, value))
                goto error;
            value.handle = NULL;
            value.prefix = NULL;
        }
    }

    DOCUMENT_START_EVENT_INIT(*event, version_directive_copy,
            tag_directives_copy.start, tag_directives_copy.top,
            implicit, mark, mark);

    return 1;

error:
    ci_free(version_directive_copy);
    while (!STACK_EMPTY(context, tag_directives_copy)) {
        ci_tag_directive_t value = POP(context, tag_directives_copy);
        ci_free(value.handle);
        ci_free(value.prefix);
    }
    STACK_DEL(context, tag_directives_copy);
    ci_free(value.handle);
    ci_free(value.prefix);

    return 0;
}

/*
 * Create DOCUMENT-END.
 */

CI_DECLARE(int)
ci_document_end_event_initialize(ci_event_t *event, int implicit)
{
    ci_mark_t mark = { 0, 0, 0 };

    assert(event);      /* Non-NULL emitter object is expected. */

    DOCUMENT_END_EVENT_INIT(*event, implicit, mark, mark);

    return 1;
}

/*
 * Create ALIAS.
 */

CI_DECLARE(int)
ci_alias_event_initialize(ci_event_t *event, const ci_char_t *anchor)
{
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *anchor_copy = NULL;

    assert(event);      /* Non-NULL event object is expected. */
    assert(anchor);     /* Non-NULL anchor is expected. */

    if (!ci_check_utf8(anchor, strlen((char *)anchor))) return 0;

    anchor_copy = ci_strdup(anchor);
    if (!anchor_copy)
        return 0;

    ALIAS_EVENT_INIT(*event, anchor_copy, mark, mark);

    return 1;
}

/*
 * Create SCALAR.
 */

CI_DECLARE(int)
ci_scalar_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag,
        const ci_char_t *value, int length,
        int plain_implicit, int quoted_implicit,
        ci_scalar_style_t style)
{
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *anchor_copy = NULL;
    ci_char_t *tag_copy = NULL;
    ci_char_t *value_copy = NULL;

    assert(event);      /* Non-NULL event object is expected. */
    assert(value);      /* Non-NULL anchor is expected. */

    if (anchor) {
        if (!ci_check_utf8(anchor, strlen((char *)anchor))) goto error;
        anchor_copy = ci_strdup(anchor);
        if (!anchor_copy) goto error;
    }

    if (tag) {
        if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
        tag_copy = ci_strdup(tag);
        if (!tag_copy) goto error;
    }

    if (length < 0) {
        length = strlen((char *)value);
    }

    if (!ci_check_utf8(value, length)) goto error;
    value_copy = CI_MALLOC(length+1);
    if (!value_copy) goto error;
    memcpy(value_copy, value, length);
    value_copy[length] = '\0';

    SCALAR_EVENT_INIT(*event, anchor_copy, tag_copy, value_copy, length,
            plain_implicit, quoted_implicit, style, mark, mark);

    return 1;

error:
    ci_free(anchor_copy);
    ci_free(tag_copy);
    ci_free(value_copy);

    return 0;
}

/*
 * Create SEQUENCE-START.
 */

CI_DECLARE(int)
ci_sequence_start_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag, int implicit,
        ci_sequence_style_t style)
{
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *anchor_copy = NULL;
    ci_char_t *tag_copy = NULL;

    assert(event);      /* Non-NULL event object is expected. */

    if (anchor) {
        if (!ci_check_utf8(anchor, strlen((char *)anchor))) goto error;
        anchor_copy = ci_strdup(anchor);
        if (!anchor_copy) goto error;
    }

    if (tag) {
        if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
        tag_copy = ci_strdup(tag);
        if (!tag_copy) goto error;
    }

    SEQUENCE_START_EVENT_INIT(*event, anchor_copy, tag_copy,
            implicit, style, mark, mark);

    return 1;

error:
    ci_free(anchor_copy);
    ci_free(tag_copy);

    return 0;
}

/*
 * Create SEQUENCE-END.
 */

CI_DECLARE(int)
ci_sequence_end_event_initialize(ci_event_t *event)
{
    ci_mark_t mark = { 0, 0, 0 };

    assert(event);      /* Non-NULL event object is expected. */

    SEQUENCE_END_EVENT_INIT(*event, mark, mark);

    return 1;
}

/*
 * Create MAPPING-START.
 */

CI_DECLARE(int)
ci_mapping_start_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag, int implicit,
        ci_mapping_style_t style)
{
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *anchor_copy = NULL;
    ci_char_t *tag_copy = NULL;

    assert(event);      /* Non-NULL event object is expected. */

    if (anchor) {
        if (!ci_check_utf8(anchor, strlen((char *)anchor))) goto error;
        anchor_copy = ci_strdup(anchor);
        if (!anchor_copy) goto error;
    }

    if (tag) {
        if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
        tag_copy = ci_strdup(tag);
        if (!tag_copy) goto error;
    }

    MAPPING_START_EVENT_INIT(*event, anchor_copy, tag_copy,
            implicit, style, mark, mark);

    return 1;

error:
    ci_free(anchor_copy);
    ci_free(tag_copy);

    return 0;
}

/*
 * Create MAPPING-END.
 */

CI_DECLARE(int)
ci_mapping_end_event_initialize(ci_event_t *event)
{
    ci_mark_t mark = { 0, 0, 0 };

    assert(event);      /* Non-NULL event object is expected. */

    MAPPING_END_EVENT_INIT(*event, mark, mark);

    return 1;
}

/*
 * Destroy an event object.
 */

CI_DECLARE(void)
ci_event_delete(ci_event_t *event)
{
    ci_tag_directive_t *tag_directive;

    assert(event);  /* Non-NULL event object expected. */

    switch (event->type)
    {
        case CI_DOCUMENT_START_EVENT:
            ci_free(event->data.document_start.version_directive);
            for (tag_directive = event->data.document_start.tag_directives.start;
                    tag_directive != event->data.document_start.tag_directives.end;
                    tag_directive++) {
                ci_free(tag_directive->handle);
                ci_free(tag_directive->prefix);
            }
            ci_free(event->data.document_start.tag_directives.start);
            break;

        case CI_ALIAS_EVENT:
            ci_free(event->data.alias.anchor);
            break;

        case CI_SCALAR_EVENT:
            ci_free(event->data.scalar.anchor);
            ci_free(event->data.scalar.tag);
            ci_free(event->data.scalar.value);
            break;

        case CI_SEQUENCE_START_EVENT:
            ci_free(event->data.sequence_start.anchor);
            ci_free(event->data.sequence_start.tag);
            break;

        case CI_MAPPING_START_EVENT:
            ci_free(event->data.mapping_start.anchor);
            ci_free(event->data.mapping_start.tag);
            break;

        default:
            break;
    }

    memset(event, 0, sizeof(ci_event_t));
}

/*
 * Create a document object.
 */

CI_DECLARE(int)
ci_document_initialize(ci_document_t *document,
        ci_version_directive_t *version_directive,
        ci_tag_directive_t *tag_directives_start,
        ci_tag_directive_t *tag_directives_end,
        int start_implicit, int end_implicit)
{
    struct {
        ci_error_type_t error;
    } context;
    struct {
        ci_node_t *start;
        ci_node_t *end;
        ci_node_t *top;
    } nodes = { NULL, NULL, NULL };
    ci_version_directive_t *version_directive_copy = NULL;
    struct {
        ci_tag_directive_t *start;
        ci_tag_directive_t *end;
        ci_tag_directive_t *top;
    } tag_directives_copy = { NULL, NULL, NULL };
    ci_tag_directive_t value = { NULL, NULL };
    ci_mark_t mark = { 0, 0, 0 };

    assert(document);       /* Non-NULL document object is expected. */
    assert((tag_directives_start && tag_directives_end) ||
            (tag_directives_start == tag_directives_end));
                            /* Valid tag directives are expected. */

    if (!STACK_INIT(&context, nodes, ci_node_t*)) goto error;

    if (version_directive) {
        version_directive_copy = CI_MALLOC_STATIC(ci_version_directive_t);
        if (!version_directive_copy) goto error;
        version_directive_copy->major = version_directive->major;
        version_directive_copy->minor = version_directive->minor;
    }

    if (tag_directives_start != tag_directives_end) {
        ci_tag_directive_t *tag_directive;
        if (!STACK_INIT(&context, tag_directives_copy, ci_tag_directive_t*))
            goto error;
        for (tag_directive = tag_directives_start;
                tag_directive != tag_directives_end; tag_directive ++) {
            assert(tag_directive->handle);
            assert(tag_directive->prefix);
            if (!ci_check_utf8(tag_directive->handle,
                        strlen((char *)tag_directive->handle)))
                goto error;
            if (!ci_check_utf8(tag_directive->prefix,
                        strlen((char *)tag_directive->prefix)))
                goto error;
            value.handle = ci_strdup(tag_directive->handle);
            value.prefix = ci_strdup(tag_directive->prefix);
            if (!value.handle || !value.prefix) goto error;
            if (!PUSH(&context, tag_directives_copy, value))
                goto error;
            value.handle = NULL;
            value.prefix = NULL;
        }
    }

    DOCUMENT_INIT(*document, nodes.start, nodes.end, version_directive_copy,
            tag_directives_copy.start, tag_directives_copy.top,
            start_implicit, end_implicit, mark, mark);

    return 1;

error:
    STACK_DEL(&context, nodes);
    ci_free(version_directive_copy);
    while (!STACK_EMPTY(&context, tag_directives_copy)) {
        ci_tag_directive_t value = POP(&context, tag_directives_copy);
        ci_free(value.handle);
        ci_free(value.prefix);
    }
    STACK_DEL(&context, tag_directives_copy);
    ci_free(value.handle);
    ci_free(value.prefix);

    return 0;
}

/*
 * Destroy a document object.
 */

CI_DECLARE(void)
ci_document_delete(ci_document_t *document)
{
    ci_tag_directive_t *tag_directive;

    assert(document);   /* Non-NULL document object is expected. */

    while (!STACK_EMPTY(&context, document->nodes)) {
        ci_node_t node = POP(&context, document->nodes);
        ci_free(node.tag);
        switch (node.type) {
            case CI_SCALAR_NODE:
                ci_free(node.data.scalar.value);
                break;
            case CI_SEQUENCE_NODE:
                STACK_DEL(&context, node.data.sequence.items);
                break;
            case CI_MAPPING_NODE:
                STACK_DEL(&context, node.data.mapping.pairs);
                break;
            default:
                assert(0);  /* Should not happen. */
        }
    }
    STACK_DEL(&context, document->nodes);

    ci_free(document->version_directive);
    for (tag_directive = document->tag_directives.start;
            tag_directive != document->tag_directives.end;
            tag_directive++) {
        ci_free(tag_directive->handle);
        ci_free(tag_directive->prefix);
    }
    ci_free(document->tag_directives.start);

    memset(document, 0, sizeof(ci_document_t));
}

/**
 * Get a document node.
 */

CI_DECLARE(ci_node_t *)
ci_document_get_node(ci_document_t *document, int index)
{
    assert(document);   /* Non-NULL document object is expected. */

    if (index > 0 && document->nodes.start + index <= document->nodes.top) {
        return document->nodes.start + index - 1;
    }
    return NULL;
}

/**
 * Get the root object.
 */

CI_DECLARE(ci_node_t *)
ci_document_get_root_node(ci_document_t *document)
{
    assert(document);   /* Non-NULL document object is expected. */

    if (document->nodes.top != document->nodes.start) {
        return document->nodes.start;
    }
    return NULL;
}

/*
 * Add a scalar node to a document.
 */

CI_DECLARE(int)
ci_document_add_scalar(ci_document_t *document,
        const ci_char_t *tag, const ci_char_t *value, int length,
        ci_scalar_style_t style)
{
    struct {
        ci_error_type_t error;
    } context;
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *tag_copy = NULL;
    ci_char_t *value_copy = NULL;
    ci_node_t node;

    assert(document);   /* Non-NULL document object is expected. */
    assert(value);      /* Non-NULL value is expected. */

    if (!tag) {
        tag = (ci_char_t *)CI_DEFAULT_SCALAR_TAG;
    }

    if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
    tag_copy = ci_strdup(tag);
    if (!tag_copy) goto error;

    if (length < 0) {
        length = strlen((char *)value);
    }

    if (!ci_check_utf8(value, length)) goto error;
    value_copy = CI_MALLOC(length+1);
    if (!value_copy) goto error;
    memcpy(value_copy, value, length);
    value_copy[length] = '\0';

    SCALAR_NODE_INIT(node, tag_copy, value_copy, length, style, mark, mark);
    if (!PUSH(&context, document->nodes, node)) goto error;

    return document->nodes.top - document->nodes.start;

error:
    ci_free(tag_copy);
    ci_free(value_copy);

    return 0;
}

/*
 * Add a sequence node to a document.
 */

CI_DECLARE(int)
ci_document_add_sequence(ci_document_t *document,
        const ci_char_t *tag, ci_sequence_style_t style)
{
    struct {
        ci_error_type_t error;
    } context;
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *tag_copy = NULL;
    struct {
        ci_node_item_t *start;
        ci_node_item_t *end;
        ci_node_item_t *top;
    } items = { NULL, NULL, NULL };
    ci_node_t node;

    assert(document);   /* Non-NULL document object is expected. */

    if (!tag) {
        tag = (ci_char_t *)CI_DEFAULT_SEQUENCE_TAG;
    }

    if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
    tag_copy = ci_strdup(tag);
    if (!tag_copy) goto error;

    if (!STACK_INIT(&context, items, ci_node_item_t*)) goto error;

    SEQUENCE_NODE_INIT(node, tag_copy, items.start, items.end,
            style, mark, mark);
    if (!PUSH(&context, document->nodes, node)) goto error;

    return document->nodes.top - document->nodes.start;

error:
    STACK_DEL(&context, items);
    ci_free(tag_copy);

    return 0;
}

/*
 * Add a mapping node to a document.
 */

CI_DECLARE(int)
ci_document_add_mapping(ci_document_t *document,
        const ci_char_t *tag, ci_mapping_style_t style)
{
    struct {
        ci_error_type_t error;
    } context;
    ci_mark_t mark = { 0, 0, 0 };
    ci_char_t *tag_copy = NULL;
    struct {
        ci_node_pair_t *start;
        ci_node_pair_t *end;
        ci_node_pair_t *top;
    } pairs = { NULL, NULL, NULL };
    ci_node_t node;

    assert(document);   /* Non-NULL document object is expected. */

    if (!tag) {
        tag = (ci_char_t *)CI_DEFAULT_MAPPING_TAG;
    }

    if (!ci_check_utf8(tag, strlen((char *)tag))) goto error;
    tag_copy = ci_strdup(tag);
    if (!tag_copy) goto error;

    if (!STACK_INIT(&context, pairs, ci_node_pair_t*)) goto error;

    MAPPING_NODE_INIT(node, tag_copy, pairs.start, pairs.end,
            style, mark, mark);
    if (!PUSH(&context, document->nodes, node)) goto error;

    return document->nodes.top - document->nodes.start;

error:
    STACK_DEL(&context, pairs);
    ci_free(tag_copy);

    return 0;
}

/*
 * Append an item to a sequence node.
 */

CI_DECLARE(int)
ci_document_append_sequence_item(ci_document_t *document,
        int sequence, int item)
{
    struct {
        ci_error_type_t error;
    } context;

    assert(document);       /* Non-NULL document is required. */
    assert(sequence > 0
            && document->nodes.start + sequence <= document->nodes.top);
                            /* Valid sequence id is required. */
    assert(document->nodes.start[sequence-1].type == CI_SEQUENCE_NODE);
                            /* A sequence node is required. */
    assert(item > 0 && document->nodes.start + item <= document->nodes.top);
                            /* Valid item id is required. */

    if (!PUSH(&context,
                document->nodes.start[sequence-1].data.sequence.items, item))
        return 0;

    return 1;
}

/*
 * Append a pair of a key and a value to a mapping node.
 */

CI_DECLARE(int)
ci_document_append_mapping_pair(ci_document_t *document,
        int mapping, int key, int value)
{
    struct {
        ci_error_type_t error;
    } context;

    ci_node_pair_t pair;

    assert(document);       /* Non-NULL document is required. */
    assert(mapping > 0
            && document->nodes.start + mapping <= document->nodes.top);
                            /* Valid mapping id is required. */
    assert(document->nodes.start[mapping-1].type == CI_MAPPING_NODE);
                            /* A mapping node is required. */
    assert(key > 0 && document->nodes.start + key <= document->nodes.top);
                            /* Valid key id is required. */
    assert(value > 0 && document->nodes.start + value <= document->nodes.top);
                            /* Valid value id is required. */

    pair.key = key;
    pair.value = value;

    if (!PUSH(&context,
                document->nodes.start[mapping-1].data.mapping.pairs, pair))
        return 0;

    return 1;
}


