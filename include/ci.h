/**
 * @file ci.h
 * @brief Public interface for libci.
 * 
 * Include the header file with the code:
 * @code
 * #include <ci.h>
 * @endcode
 */

#ifndef CI_H
#define CI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @defgroup export Export Definitions
 * @{
 */

/** The public API declaration. */

#if defined(__MINGW32__)
#   define  CI_DECLARE(type)  type
#elif defined(_WIN32)
#   if defined(CI_DECLARE_STATIC)
#       define  CI_DECLARE(type)  type
#   elif defined(CI_DECLARE_EXPORT)
#       define  CI_DECLARE(type)  __declspec(dllexport) type
#   else
#       define  CI_DECLARE(type)  __declspec(dllimport) type
#   endif
#else
#   define  CI_DECLARE(type)  type
#endif

/** @} */

/**
 * @defgroup version Version Information
 * @{
 */

/**
 * Get the library version as a string.
 *
 * @returns The function returns the pointer to a static string of the form
 * @c "X.Y.Z", where @c X is the major version number, @c Y is a minor version
 * number, and @c Z is the patch version number.
 */

CI_DECLARE(const char *)
ci_get_version_string(void);

/**
 * Get the library version numbers.
 *
 * @param[out]      major   Major version number.
 * @param[out]      minor   Minor version number.
 * @param[out]      patch   Patch version number.
 */

CI_DECLARE(void)
ci_get_version(int *major, int *minor, int *patch);

/** @} */

/**
 * @defgroup basic Basic Types
 * @{
 */

/** The character type (UTF-8 octet). */
typedef unsigned char ci_char_t;

/** The version directive data. */
typedef struct ci_version_directive_s {
    /** The major version number. */
    int major;
    /** The minor version number. */
    int minor;
} ci_version_directive_t;

/** The tag directive data. */
typedef struct ci_tag_directive_s {
    /** The tag handle. */
    ci_char_t *handle;
    /** The tag prefix. */
    ci_char_t *prefix;
} ci_tag_directive_t;

/** The stream encoding. */
typedef enum ci_encoding_e {
    /** Let the parser choose the encoding. */
    CI_ANY_ENCODING,
    /** The default UTF-8 encoding. */
    CI_UTF8_ENCODING,
    /** The UTF-16-LE encoding with BOM. */
    CI_UTF16LE_ENCODING,
    /** The UTF-16-BE encoding with BOM. */
    CI_UTF16BE_ENCODING
} ci_encoding_t;

/** Line break types. */

typedef enum ci_break_e {
    /** Let the parser choose the break type. */
    CI_ANY_BREAK,
    /** Use CR for line breaks (Mac style). */
    CI_CR_BREAK,
    /** Use LN for line breaks (Unix style). */
    CI_LN_BREAK,
    /** Use CR LN for line breaks (DOS style). */
    CI_CRLN_BREAK
} ci_break_t;

/** Many bad things could happen with the parser and emitter. */
typedef enum ci_error_type_e {
    /** No error is produced. */
    CI_NO_ERROR,

    /** Cannot allocate or reallocate a block of memory. */
    CI_MEMORY_ERROR,

    /** Cannot read or decode the input stream. */
    CI_READER_ERROR,
    /** Cannot scan the input stream. */
    CI_SCANNER_ERROR,
    /** Cannot parse the input stream. */
    CI_PARSER_ERROR,
    /** Cannot compose a CI document. */
    CI_COMPOSER_ERROR,

    /** Cannot write to the output stream. */
    CI_WRITER_ERROR,
    /** Cannot emit a CI stream. */
    CI_EMITTER_ERROR
} ci_error_type_t;

/** The pointer position. */
typedef struct ci_mark_s {
    /** The position index. */
    size_t index;

    /** The position line. */
    size_t line;

    /** The position column. */
    size_t column;
} ci_mark_t;

/** @} */

/**
 * @defgroup styles Node Styles
 * @{
 */

/** Scalar styles. */
typedef enum ci_scalar_style_e {
    /** Let the emitter choose the style. */
    CI_ANY_SCALAR_STYLE,

    /** The plain scalar style. */
    CI_PLAIN_SCALAR_STYLE,

    /** The single-quoted scalar style. */
    CI_SINGLE_QUOTED_SCALAR_STYLE,
    /** The double-quoted scalar style. */
    CI_DOUBLE_QUOTED_SCALAR_STYLE,

    /** The literal scalar style. */
    CI_LITERAL_SCALAR_STYLE,
    /** The folded scalar style. */
    CI_FOLDED_SCALAR_STYLE
} ci_scalar_style_t;

/** Sequence styles. */
typedef enum ci_sequence_style_e {
    /** Let the emitter choose the style. */
    CI_ANY_SEQUENCE_STYLE,

    /** The block sequence style. */
    CI_BLOCK_SEQUENCE_STYLE,
    /** The flow sequence style. */
    CI_FLOW_SEQUENCE_STYLE
} ci_sequence_style_t;

/** Mapping styles. */
typedef enum ci_mapping_style_e {
    /** Let the emitter choose the style. */
    CI_ANY_MAPPING_STYLE,

    /** The block mapping style. */
    CI_BLOCK_MAPPING_STYLE,
    /** The flow mapping style. */
    CI_FLOW_MAPPING_STYLE
/*    CI_FLOW_SET_MAPPING_STYLE   */
} ci_mapping_style_t;

/** @} */

/**
 * @defgroup tokens Tokens
 * @{
 */

/** Token types. */
typedef enum ci_token_type_e {
    /** An empty token. */
    CI_NO_TOKEN,

    /** A STREAM-START token. */
    CI_STREAM_START_TOKEN,
    /** A STREAM-END token. */
    CI_STREAM_END_TOKEN,

    /** A VERSION-DIRECTIVE token. */
    CI_VERSION_DIRECTIVE_TOKEN,
    /** A TAG-DIRECTIVE token. */
    CI_TAG_DIRECTIVE_TOKEN,
    /** A DOCUMENT-START token. */
    CI_DOCUMENT_START_TOKEN,
    /** A DOCUMENT-END token. */
    CI_DOCUMENT_END_TOKEN,

    /** A BLOCK-SEQUENCE-START token. */
    CI_BLOCK_SEQUENCE_START_TOKEN,
    /** A BLOCK-MAPPING-START token. */
    CI_BLOCK_MAPPING_START_TOKEN,
    /** A BLOCK-END token. */
    CI_BLOCK_END_TOKEN,

    /** A FLOW-SEQUENCE-START token. */
    CI_FLOW_SEQUENCE_START_TOKEN,
    /** A FLOW-SEQUENCE-END token. */
    CI_FLOW_SEQUENCE_END_TOKEN,
    /** A FLOW-MAPPING-START token. */
    CI_FLOW_MAPPING_START_TOKEN,
    /** A FLOW-MAPPING-END token. */
    CI_FLOW_MAPPING_END_TOKEN,

    /** A BLOCK-ENTRY token. */
    CI_BLOCK_ENTRY_TOKEN,
    /** A FLOW-ENTRY token. */
    CI_FLOW_ENTRY_TOKEN,
    /** A KEY token. */
    CI_KEY_TOKEN,
    /** A VALUE token. */
    CI_VALUE_TOKEN,

    /** An ALIAS token. */
    CI_ALIAS_TOKEN,
    /** An ANCHOR token. */
    CI_ANCHOR_TOKEN,
    /** A TAG token. */
    CI_TAG_TOKEN,
    /** A SCALAR token. */
    CI_SCALAR_TOKEN
} ci_token_type_t;

/** The token structure. */
typedef struct ci_token_s {

    /** The token type. */
    ci_token_type_t type;

    /** The token data. */
    union {

        /** The stream start (for @c CI_STREAM_START_TOKEN). */
        struct {
            /** The stream encoding. */
            ci_encoding_t encoding;
        } stream_start;

        /** The alias (for @c CI_ALIAS_TOKEN). */
        struct {
            /** The alias value. */
            ci_char_t *value;
        } alias;

        /** The anchor (for @c CI_ANCHOR_TOKEN). */
        struct {
            /** The anchor value. */
            ci_char_t *value;
        } anchor;

        /** The tag (for @c CI_TAG_TOKEN). */
        struct {
            /** The tag handle. */
            ci_char_t *handle;
            /** The tag suffix. */
            ci_char_t *suffix;
        } tag;

        /** The scalar value (for @c CI_SCALAR_TOKEN). */
        struct {
            /** The scalar value. */
            ci_char_t *value;
            /** The length of the scalar value. */
            size_t length;
            /** The scalar style. */
            ci_scalar_style_t style;
        } scalar;

        /** The version directive (for @c CI_VERSION_DIRECTIVE_TOKEN). */
        struct {
            /** The major version number. */
            int major;
            /** The minor version number. */
            int minor;
        } version_directive;

        /** The tag directive (for @c CI_TAG_DIRECTIVE_TOKEN). */
        struct {
            /** The tag handle. */
            ci_char_t *handle;
            /** The tag prefix. */
            ci_char_t *prefix;
        } tag_directive;

    } data;

    /** The beginning of the token. */
    ci_mark_t start_mark;
    /** The end of the token. */
    ci_mark_t end_mark;

} ci_token_t;

/**
 * Free any memory allocated for a token object.
 *
 * @param[in,out]   token   A token object.
 */

CI_DECLARE(void)
ci_token_delete(ci_token_t *token);

/** @} */

/**
 * @defgroup events Events
 * @{
 */

/** Event types. */
typedef enum ci_event_type_e {
    /** An empty event. */
    CI_NO_EVENT,

    /** A STREAM-START event. */
    CI_STREAM_START_EVENT,
    /** A STREAM-END event. */
    CI_STREAM_END_EVENT,

    /** A DOCUMENT-START event. */
    CI_DOCUMENT_START_EVENT,
    /** A DOCUMENT-END event. */
    CI_DOCUMENT_END_EVENT,

    /** An ALIAS event. */
    CI_ALIAS_EVENT,
    /** A SCALAR event. */
    CI_SCALAR_EVENT,

    /** A SEQUENCE-START event. */
    CI_SEQUENCE_START_EVENT,
    /** A SEQUENCE-END event. */
    CI_SEQUENCE_END_EVENT,

    /** A MAPPING-START event. */
    CI_MAPPING_START_EVENT,
    /** A MAPPING-END event. */
    CI_MAPPING_END_EVENT
} ci_event_type_t;

/** The event structure. */
typedef struct ci_event_s {

    /** The event type. */
    ci_event_type_t type;

    /** The event data. */
    union {
        
        /** The stream parameters (for @c CI_STREAM_START_EVENT). */
        struct {
            /** The document encoding. */
            ci_encoding_t encoding;
        } stream_start;

        /** The document parameters (for @c CI_DOCUMENT_START_EVENT). */
        struct {
            /** The version directive. */
            ci_version_directive_t *version_directive;

            /** The list of tag directives. */
            struct {
                /** The beginning of the tag directives list. */
                ci_tag_directive_t *start;
                /** The end of the tag directives list. */
                ci_tag_directive_t *end;
            } tag_directives;

            /** Is the document indicator implicit? */
            int implicit;
        } document_start;

        /** The document end parameters (for @c CI_DOCUMENT_END_EVENT). */
        struct {
            /** Is the document end indicator implicit? */
            int implicit;
        } document_end;

        /** The alias parameters (for @c CI_ALIAS_EVENT). */
        struct {
            /** The anchor. */
            ci_char_t *anchor;
        } alias;

        /** The scalar parameters (for @c CI_SCALAR_EVENT). */
        struct {
            /** The anchor. */
            ci_char_t *anchor;
            /** The tag. */
            ci_char_t *tag;
            /** The scalar value. */
            ci_char_t *value;
            /** The length of the scalar value. */
            size_t length;
            /** Is the tag optional for the plain style? */
            int plain_implicit;
            /** Is the tag optional for any non-plain style? */
            int quoted_implicit;
            /** The scalar style. */
            ci_scalar_style_t style;
        } scalar;

        /** The sequence parameters (for @c CI_SEQUENCE_START_EVENT). */
        struct {
            /** The anchor. */
            ci_char_t *anchor;
            /** The tag. */
            ci_char_t *tag;
            /** Is the tag optional? */
            int implicit;
            /** The sequence style. */
            ci_sequence_style_t style;
        } sequence_start;

        /** The mapping parameters (for @c CI_MAPPING_START_EVENT). */
        struct {
            /** The anchor. */
            ci_char_t *anchor;
            /** The tag. */
            ci_char_t *tag;
            /** Is the tag optional? */
            int implicit;
            /** The mapping style. */
            ci_mapping_style_t style;
        } mapping_start;

    } data;

    /** The beginning of the event. */
    ci_mark_t start_mark;
    /** The end of the event. */
    ci_mark_t end_mark;

} ci_event_t;

/**
 * Create the STREAM-START event.
 *
 * @param[out]      event       An empty event object.
 * @param[in]       encoding    The stream encoding.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_stream_start_event_initialize(ci_event_t *event,
        ci_encoding_t encoding);

/**
 * Create the STREAM-END event.
 *
 * @param[out]      event       An empty event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_stream_end_event_initialize(ci_event_t *event);

/**
 * Create the DOCUMENT-START event.
 *
 * The @a implicit argument is considered as a stylistic parameter and may be
 * ignored by the emitter.
 *
 * @param[out]      event                   An empty event object.
 * @param[in]       version_directive       The %CI directive value or
 *                                          @c NULL.
 * @param[in]       tag_directives_start    The beginning of the %TAG
 *                                          directives list.
 * @param[in]       tag_directives_end      The end of the %TAG directives
 *                                          list.
 * @param[in]       implicit                If the document start indicator is
 *                                          implicit.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_document_start_event_initialize(ci_event_t *event,
        ci_version_directive_t *version_directive,
        ci_tag_directive_t *tag_directives_start,
        ci_tag_directive_t *tag_directives_end,
        int implicit);

/**
 * Create the DOCUMENT-END event.
 *
 * The @a implicit argument is considered as a stylistic parameter and may be
 * ignored by the emitter.
 *
 * @param[out]      event       An empty event object.
 * @param[in]       implicit    If the document end indicator is implicit.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_document_end_event_initialize(ci_event_t *event, int implicit);

/**
 * Create an ALIAS event.
 *
 * @param[out]      event       An empty event object.
 * @param[in]       anchor      The anchor value.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_alias_event_initialize(ci_event_t *event, const ci_char_t *anchor);

/**
 * Create a SCALAR event.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * Either the @a tag attribute or one of the @a plain_implicit and
 * @a quoted_implicit flags must be set.
 *
 * @param[out]      event           An empty event object.
 * @param[in]       anchor          The scalar anchor or @c NULL.
 * @param[in]       tag             The scalar tag or @c NULL.
 * @param[in]       value           The scalar value.
 * @param[in]       length          The length of the scalar value.
 * @param[in]       plain_implicit  If the tag may be omitted for the plain
 *                                  style.
 * @param[in]       quoted_implicit If the tag may be omitted for any
 *                                  non-plain style.
 * @param[in]       style           The scalar style.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_scalar_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag,
        const ci_char_t *value, int length,
        int plain_implicit, int quoted_implicit,
        ci_scalar_style_t style);

/**
 * Create a SEQUENCE-START event.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * Either the @a tag attribute or the @a implicit flag must be set.
 *
 * @param[out]      event       An empty event object.
 * @param[in]       anchor      The sequence anchor or @c NULL.
 * @param[in]       tag         The sequence tag or @c NULL.
 * @param[in]       implicit    If the tag may be omitted.
 * @param[in]       style       The sequence style.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_sequence_start_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag, int implicit,
        ci_sequence_style_t style);

/**
 * Create a SEQUENCE-END event.
 *
 * @param[out]      event       An empty event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_sequence_end_event_initialize(ci_event_t *event);

/**
 * Create a MAPPING-START event.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * Either the @a tag attribute or the @a implicit flag must be set.
 *
 * @param[out]      event       An empty event object.
 * @param[in]       anchor      The mapping anchor or @c NULL.
 * @param[in]       tag         The mapping tag or @c NULL.
 * @param[in]       implicit    If the tag may be omitted.
 * @param[in]       style       The mapping style.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_mapping_start_event_initialize(ci_event_t *event,
        const ci_char_t *anchor, const ci_char_t *tag, int implicit,
        ci_mapping_style_t style);

/**
 * Create a MAPPING-END event.
 *
 * @param[out]      event       An empty event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_mapping_end_event_initialize(ci_event_t *event);

/**
 * Free any memory allocated for an event object.
 *
 * @param[in,out]   event   An event object.
 */

CI_DECLARE(void)
ci_event_delete(ci_event_t *event);

/** @} */

/**
 * @defgroup nodes Nodes
 * @{
 */

/** The tag @c !!null with the only possible value: @c null. */
#define CI_NULL_TAG       "tag:ci.org,2002:null"
/** The tag @c !!bool with the values: @c true and @c false. */
#define CI_BOOL_TAG       "tag:ci.org,2002:bool"
/** The tag @c !!str for string values. */
#define CI_STR_TAG        "tag:ci.org,2002:str"
/** The tag @c !!int for integer values. */
#define CI_INT_TAG        "tag:ci.org,2002:int"
/** The tag @c !!float for float values. */
#define CI_FLOAT_TAG      "tag:ci.org,2002:float"
/** The tag @c !!timestamp for date and time values. */
#define CI_TIMESTAMP_TAG  "tag:ci.org,2002:timestamp"

/** The tag @c !!seq is used to denote sequences. */
#define CI_SEQ_TAG        "tag:ci.org,2002:seq"
/** The tag @c !!map is used to denote mapping. */
#define CI_MAP_TAG        "tag:ci.org,2002:map"

/** The default scalar tag is @c !!str. */
#define CI_DEFAULT_SCALAR_TAG     CI_STR_TAG
/** The default sequence tag is @c !!seq. */
#define CI_DEFAULT_SEQUENCE_TAG   CI_SEQ_TAG
/** The default mapping tag is @c !!map. */
#define CI_DEFAULT_MAPPING_TAG    CI_MAP_TAG

/** Node types. */
typedef enum ci_node_type_e {
    /** An empty node. */
    CI_NO_NODE,

    /** A scalar node. */
    CI_SCALAR_NODE,
    /** A sequence node. */
    CI_SEQUENCE_NODE,
    /** A mapping node. */
    CI_MAPPING_NODE
} ci_node_type_t;

/** The forward definition of a document node structure. */
typedef struct ci_node_s ci_node_t;

/** An element of a sequence node. */
typedef int ci_node_item_t;

/** An element of a mapping node. */
typedef struct ci_node_pair_s {
    /** The key of the element. */
    int key;
    /** The value of the element. */
    int value;
} ci_node_pair_t;

/** The node structure. */
struct ci_node_s {

    /** The node type. */
    ci_node_type_t type;

    /** The node tag. */
    ci_char_t *tag;

    /** The node data. */
    union {
        
        /** The scalar parameters (for @c CI_SCALAR_NODE). */
        struct {
            /** The scalar value. */
            ci_char_t *value;
            /** The length of the scalar value. */
            size_t length;
            /** The scalar style. */
            ci_scalar_style_t style;
        } scalar;

        /** The sequence parameters (for @c CI_SEQUENCE_NODE). */
        struct {
            /** The stack of sequence items. */
            struct {
                /** The beginning of the stack. */
                ci_node_item_t *start;
                /** The end of the stack. */
                ci_node_item_t *end;
                /** The top of the stack. */
                ci_node_item_t *top;
            } items;
            /** The sequence style. */
            ci_sequence_style_t style;
        } sequence;

        /** The mapping parameters (for @c CI_MAPPING_NODE). */
        struct {
            /** The stack of mapping pairs (key, value). */
            struct {
                /** The beginning of the stack. */
                ci_node_pair_t *start;
                /** The end of the stack. */
                ci_node_pair_t *end;
                /** The top of the stack. */
                ci_node_pair_t *top;
            } pairs;
            /** The mapping style. */
            ci_mapping_style_t style;
        } mapping;

    } data;

    /** The beginning of the node. */
    ci_mark_t start_mark;
    /** The end of the node. */
    ci_mark_t end_mark;

};

/** The document structure. */
typedef struct ci_document_s {

    /** The document nodes. */
    struct {
        /** The beginning of the stack. */
        ci_node_t *start;
        /** The end of the stack. */
        ci_node_t *end;
        /** The top of the stack. */
        ci_node_t *top;
    } nodes;

    /** The version directive. */
    ci_version_directive_t *version_directive;

    /** The list of tag directives. */
    struct {
        /** The beginning of the tag directives list. */
        ci_tag_directive_t *start;
        /** The end of the tag directives list. */
        ci_tag_directive_t *end;
    } tag_directives;

    /** Is the document start indicator implicit? */
    int start_implicit;
    /** Is the document end indicator implicit? */
    int end_implicit;

    /** The beginning of the document. */
    ci_mark_t start_mark;
    /** The end of the document. */
    ci_mark_t end_mark;

} ci_document_t;

/**
 * Create a CI document.
 *
 * @param[out]      document                An empty document object.
 * @param[in]       version_directive       The %CI directive value or
 *                                          @c NULL.
 * @param[in]       tag_directives_start    The beginning of the %TAG
 *                                          directives list.
 * @param[in]       tag_directives_end      The end of the %TAG directives
 *                                          list.
 * @param[in]       start_implicit          If the document start indicator is
 *                                          implicit.
 * @param[in]       end_implicit            If the document end indicator is
 *                                          implicit.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_document_initialize(ci_document_t *document,
        ci_version_directive_t *version_directive,
        ci_tag_directive_t *tag_directives_start,
        ci_tag_directive_t *tag_directives_end,
        int start_implicit, int end_implicit);

/**
 * Delete a CI document and all its nodes.
 *
 * @param[in,out]   document        A document object.
 */

CI_DECLARE(void)
ci_document_delete(ci_document_t *document);

/**
 * Get a node of a CI document.
 *
 * The pointer returned by this function is valid until any of the functions
 * modifying the documents are called.
 *
 * @param[in]       document        A document object.
 * @param[in]       index           The node id.
 *
 * @returns the node objct or @c NULL if @c node_id is out of range.
 */

CI_DECLARE(ci_node_t *)
ci_document_get_node(ci_document_t *document, int index);

/**
 * Get the root of a CI document node.
 *
 * The root object is the first object added to the document.
 *
 * The pointer returned by this function is valid until any of the functions
 * modifying the documents are called.
 *
 * An empty document produced by the parser signifies the end of a CI
 * stream.
 *
 * @param[in]       document        A document object.
 *
 * @returns the node object or @c NULL if the document is empty.
 */

CI_DECLARE(ci_node_t *)
ci_document_get_root_node(ci_document_t *document);

/**
 * Create a SCALAR node and attach it to the document.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * @param[in,out]   document        A document object.
 * @param[in]       tag             The scalar tag.
 * @param[in]       value           The scalar value.
 * @param[in]       length          The length of the scalar value.
 * @param[in]       style           The scalar style.
 *
 * @returns the node id or @c 0 on error.
 */

CI_DECLARE(int)
ci_document_add_scalar(ci_document_t *document,
        const ci_char_t *tag, const ci_char_t *value, int length,
        ci_scalar_style_t style);

/**
 * Create a SEQUENCE node and attach it to the document.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * @param[in,out]   document    A document object.
 * @param[in]       tag         The sequence tag.
 * @param[in]       style       The sequence style.
 *
 * @returns the node id or @c 0 on error.
 */

CI_DECLARE(int)
ci_document_add_sequence(ci_document_t *document,
        const ci_char_t *tag, ci_sequence_style_t style);

/**
 * Create a MAPPING node and attach it to the document.
 *
 * The @a style argument may be ignored by the emitter.
 *
 * @param[in,out]   document    A document object.
 * @param[in]       tag         The sequence tag.
 * @param[in]       style       The sequence style.
 *
 * @returns the node id or @c 0 on error.
 */

CI_DECLARE(int)
ci_document_add_mapping(ci_document_t *document,
        const ci_char_t *tag, ci_mapping_style_t style);

/**
 * Add an item to a SEQUENCE node.
 *
 * @param[in,out]   document    A document object.
 * @param[in]       sequence    The sequence node id.
 * @param[in]       item        The item node id.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_document_append_sequence_item(ci_document_t *document,
        int sequence, int item);

/**
 * Add a pair of a key and a value to a MAPPING node.
 *
 * @param[in,out]   document    A document object.
 * @param[in]       mapping     The mapping node id.
 * @param[in]       key         The key node id.
 * @param[in]       value       The value node id.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_document_append_mapping_pair(ci_document_t *document,
        int mapping, int key, int value);

/** @} */

/**
 * @defgroup parser Parser Definitions
 * @{
 */

/**
 * The prototype of a read handler.
 *
 * The read handler is called when the parser needs to read more bytes from the
 * source.  The handler should write not more than @a size bytes to the @a
 * buffer.  The number of written bytes should be set to the @a length variable.
 *
 * @param[in,out]   data        A pointer to an application data specified by
 *                              ci_parser_set_input().
 * @param[out]      buffer      The buffer to write the data from the source.
 * @param[in]       size        The size of the buffer.
 * @param[out]      size_read   The actual number of bytes read from the source.
 *
 * @returns On success, the handler should return @c 1.  If the handler failed,
 * the returned value should be @c 0.  On EOF, the handler should set the
 * @a size_read to @c 0 and return @c 1.
 */

typedef int ci_read_handler_t(void *data, unsigned char *buffer, size_t size,
        size_t *size_read);

/**
 * This structure holds information about a potential simple key.
 */

typedef struct ci_simple_key_s {
    /** Is a simple key possible? */
    int possible;

    /** Is a simple key required? */
    int required;

    /** The number of the token. */
    size_t token_number;

    /** The position mark. */
    ci_mark_t mark;
} ci_simple_key_t;

/**
 * The states of the parser.
 */
typedef enum ci_parser_state_e {
    /** Expect STREAM-START. */
    CI_PARSE_STREAM_START_STATE,
    /** Expect the beginning of an implicit document. */
    CI_PARSE_IMPLICIT_DOCUMENT_START_STATE,
    /** Expect DOCUMENT-START. */
    CI_PARSE_DOCUMENT_START_STATE,
    /** Expect the content of a document. */
    CI_PARSE_DOCUMENT_CONTENT_STATE,
    /** Expect DOCUMENT-END. */
    CI_PARSE_DOCUMENT_END_STATE,

    /** Expect a block node. */
    CI_PARSE_BLOCK_NODE_STATE,
    /** Expect a block node or indentless sequence. */
    CI_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE,
    /** Expect a flow node. */
    CI_PARSE_FLOW_NODE_STATE,
    /** Expect the first entry of a block sequence. */
    CI_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE,
    /** Expect an entry of a block sequence. */
    CI_PARSE_BLOCK_SEQUENCE_ENTRY_STATE,

    /** Expect an entry of an indentless sequence. */
    CI_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE,
    /** Expect the first key of a block mapping. */
    CI_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE,
    /** Expect a block mapping key. */
    CI_PARSE_BLOCK_MAPPING_KEY_STATE,
    /** Expect a block mapping value. */
    CI_PARSE_BLOCK_MAPPING_VALUE_STATE,
    /** Expect the first entry of a flow sequence. */
    CI_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE,

    /** Expect an entry of a flow sequence. */
    CI_PARSE_FLOW_SEQUENCE_ENTRY_STATE,
    /** Expect a key of an ordered mapping. */
    CI_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE,
    /** Expect a value of an ordered mapping. */
    CI_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE,
    /** Expect the and of an ordered mapping entry. */
    CI_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE,
    /** Expect the first key of a flow mapping. */
    CI_PARSE_FLOW_MAPPING_FIRST_KEY_STATE,
    /** Expect a key of a flow mapping. */

    CI_PARSE_FLOW_MAPPING_KEY_STATE,
    /** Expect a value of a flow mapping. */
    CI_PARSE_FLOW_MAPPING_VALUE_STATE,
    /** Expect an empty value of a flow mapping. */
    CI_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE,
    /** Expect nothing. */
    CI_PARSE_END_STATE
} ci_parser_state_t;

/**
 * This structure holds aliases data.
 */

typedef struct ci_alias_data_s {
    /** The anchor. */
    ci_char_t *anchor;
    /** The node id. */
    int index;
    /** The anchor mark. */
    ci_mark_t mark;
} ci_alias_data_t;

/**
 * The parser structure.
 *
 * All members are internal.  Manage the structure using the @c ci_parser_
 * family of functions.
 */

typedef struct ci_parser_s {

    /**
     * @name Error handling
     * @{
     */

    /** Error type. */
    ci_error_type_t error;
    /** Error description. */
    const char *problem;
    /** The byte about which the problem occurred. */
    size_t problem_offset;
    /** The problematic value (@c -1 is none). */
    int problem_value;
    /** The problem position. */
    ci_mark_t problem_mark;
    /** The error context. */
    const char *context;
    /** The context position. */
    ci_mark_t context_mark;

    /**
     * @}
     */

    /**
     * @name Reader stuff
     * @{
     */

    /** Read handler. */
    ci_read_handler_t *read_handler;

    /** A pointer for passing to the read handler. */
    void *read_handler_data;

    /** Standard (string or file) input data. */
    union {
        /** String input data. */
        struct {
            /** The string start pointer. */
            const unsigned char *start;
            /** The string end pointer. */
            const unsigned char *end;
            /** The string current position. */
            const unsigned char *current;
        } string;

        /** File input data. */
        FILE *file;
    } input;

    /** EOF flag */
    int eof;

    /** The working buffer. */
    struct {
        /** The beginning of the buffer. */
        ci_char_t *start;
        /** The end of the buffer. */
        ci_char_t *end;
        /** The current position of the buffer. */
        ci_char_t *pointer;
        /** The last filled position of the buffer. */
        ci_char_t *last;
    } buffer;

    /* The number of unread characters in the buffer. */
    size_t unread;

    /** The raw buffer. */
    struct {
        /** The beginning of the buffer. */
        unsigned char *start;
        /** The end of the buffer. */
        unsigned char *end;
        /** The current position of the buffer. */
        unsigned char *pointer;
        /** The last filled position of the buffer. */
        unsigned char *last;
    } raw_buffer;

    /** The input encoding. */
    ci_encoding_t encoding;

    /** The offset of the current position (in bytes). */
    size_t offset;

    /** The mark of the current position. */
    ci_mark_t mark;

    /**
     * @}
     */

    /**
     * @name Scanner stuff
     * @{
     */

    /** Have we started to scan the input stream? */
    int stream_start_produced;

    /** Have we reached the end of the input stream? */
    int stream_end_produced;

    /** The number of unclosed '[' and '{' indicators. */
    int flow_level;

    /** The tokens queue. */
    struct {
        /** The beginning of the tokens queue. */
        ci_token_t *start;
        /** The end of the tokens queue. */
        ci_token_t *end;
        /** The head of the tokens queue. */
        ci_token_t *head;
        /** The tail of the tokens queue. */
        ci_token_t *tail;
    } tokens;

    /** The number of tokens fetched from the queue. */
    size_t tokens_parsed;

    /** Does the tokens queue contain a token ready for dequeueing. */
    int token_available;

    /** The indentation levels stack. */
    struct {
        /** The beginning of the stack. */
        int *start;
        /** The end of the stack. */
        int *end;
        /** The top of the stack. */
        int *top;
    } indents;

    /** The current indentation level. */
    int indent;

    /** May a simple key occur at the current position? */
    int simple_key_allowed;

    /** The stack of simple keys. */
    struct {
        /** The beginning of the stack. */
        ci_simple_key_t *start;
        /** The end of the stack. */
        ci_simple_key_t *end;
        /** The top of the stack. */
        ci_simple_key_t *top;
    } simple_keys;

    /**
     * @}
     */

    /**
     * @name Parser stuff
     * @{
     */

    /** The parser states stack. */
    struct {
        /** The beginning of the stack. */
        ci_parser_state_t *start;
        /** The end of the stack. */
        ci_parser_state_t *end;
        /** The top of the stack. */
        ci_parser_state_t *top;
    } states;

    /** The current parser state. */
    ci_parser_state_t state;

    /** The stack of marks. */
    struct {
        /** The beginning of the stack. */
        ci_mark_t *start;
        /** The end of the stack. */
        ci_mark_t *end;
        /** The top of the stack. */
        ci_mark_t *top;
    } marks;

    /** The list of TAG directives. */
    struct {
        /** The beginning of the list. */
        ci_tag_directive_t *start;
        /** The end of the list. */
        ci_tag_directive_t *end;
        /** The top of the list. */
        ci_tag_directive_t *top;
    } tag_directives;

    /**
     * @}
     */

    /**
     * @name Dumper stuff
     * @{
     */

    /** The alias data. */
    struct {
        /** The beginning of the list. */
        ci_alias_data_t *start;
        /** The end of the list. */
        ci_alias_data_t *end;
        /** The top of the list. */
        ci_alias_data_t *top;
    } aliases;

    /** The currently parsed document. */
    ci_document_t *document;

    /**
     * @}
     */

} ci_parser_t;

/**
 * Initialize a parser.
 *
 * This function creates a new parser object.  An application is responsible
 * for destroying the object using the ci_parser_delete() function.
 *
 * @param[out]      parser  An empty parser object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_parser_initialize(ci_parser_t *parser);

/**
 * Destroy a parser.
 *
 * @param[in,out]   parser  A parser object.
 */

CI_DECLARE(void)
ci_parser_delete(ci_parser_t *parser);

/**
 * Set a string input.
 *
 * Note that the @a input pointer must be valid while the @a parser object
 * exists.  The application is responsible for destroying @a input after
 * destroying the @a parser.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       input   A source data.
 * @param[in]       size    The length of the source data in bytes.
 */

CI_DECLARE(void)
ci_parser_set_input_string(ci_parser_t *parser,
        const unsigned char *input, size_t size);

/**
 * Set a file input.
 *
 * @a file should be a file object open for reading.  The application is
 * responsible for closing the @a file.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       file    An open file.
 */

CI_DECLARE(void)
ci_parser_set_input_file(ci_parser_t *parser, FILE *file);

/**
 * Set a generic input handler.
 *
 * @param[in,out]   parser  A parser object.
 * @param[in]       handler A read handler.
 * @param[in]       data    Any application data for passing to the read
 *                          handler.
 */

CI_DECLARE(void)
ci_parser_set_input(ci_parser_t *parser,
        ci_read_handler_t *handler, void *data);

/**
 * Set the source encoding.
 *
 * @param[in,out]   parser      A parser object.
 * @param[in]       encoding    The source encoding.
 */

CI_DECLARE(void)
ci_parser_set_encoding(ci_parser_t *parser, ci_encoding_t encoding);

/**
 * Scan the input stream and produce the next token.
 *
 * Call the function subsequently to produce a sequence of tokens corresponding
 * to the input stream.  The initial token has the type
 * @c CI_STREAM_START_TOKEN while the ending token has the type
 * @c CI_STREAM_END_TOKEN.
 *
 * An application is responsible for freeing any buffers associated with the
 * produced token object using the @c ci_token_delete function.
 *
 * An application must not alternate the calls of ci_parser_scan() with the
 * calls of ci_parser_parse() or ci_parser_load(). Doing this will break
 * the parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      token       An empty token object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_parser_scan(ci_parser_t *parser, ci_token_t *token);

/**
 * Parse the input stream and produce the next parsing event.
 *
 * Call the function subsequently to produce a sequence of events corresponding
 * to the input stream.  The initial event has the type
 * @c CI_STREAM_START_EVENT while the ending event has the type
 * @c CI_STREAM_END_EVENT.
 *
 * An application is responsible for freeing any buffers associated with the
 * produced event object using the ci_event_delete() function.
 *
 * An application must not alternate the calls of ci_parser_parse() with the
 * calls of ci_parser_scan() or ci_parser_load(). Doing this will break the
 * parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      event       An empty event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_parser_parse(ci_parser_t *parser, ci_event_t *event);

/**
 * Parse the input stream and produce the next CI document.
 *
 * Call this function subsequently to produce a sequence of documents
 * constituting the input stream.
 *
 * If the produced document has no root node, it means that the document
 * end has been reached.
 *
 * An application is responsible for freeing any data associated with the
 * produced document object using the ci_document_delete() function.
 *
 * An application must not alternate the calls of ci_parser_load() with the
 * calls of ci_parser_scan() or ci_parser_parse(). Doing this will break
 * the parser.
 *
 * @param[in,out]   parser      A parser object.
 * @param[out]      document    An empty document object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_parser_load(ci_parser_t *parser, ci_document_t *document);

/**
 * Set the maximum depth of nesting.
 *
 * Default: 1000
 *
 * Each nesting level increases the stack and the number of previous
 * starting events that the parser has to check.
 *
 * @param[in]       max         The maximum number of allowed nested events
 */

CI_DECLARE(void)
ci_set_max_nest_level(int max);

/** @} */

/**
 * @defgroup emitter Emitter Definitions
 * @{
 */

/**
 * The prototype of a write handler.
 *
 * The write handler is called when the emitter needs to flush the accumulated
 * characters to the output.  The handler should write @a size bytes of the
 * @a buffer to the output.
 *
 * @param[in,out]   data        A pointer to an application data specified by
 *                              ci_emitter_set_output().
 * @param[in]       buffer      The buffer with bytes to be written.
 * @param[in]       size        The size of the buffer.
 *
 * @returns On success, the handler should return @c 1.  If the handler failed,
 * the returned value should be @c 0.
 */

typedef int ci_write_handler_t(void *data, unsigned char *buffer, size_t size);

/** The emitter states. */
typedef enum ci_emitter_state_e {
    /** Expect STREAM-START. */
    CI_EMIT_STREAM_START_STATE,
    /** Expect the first DOCUMENT-START or STREAM-END. */
    CI_EMIT_FIRST_DOCUMENT_START_STATE,
    /** Expect DOCUMENT-START or STREAM-END. */
    CI_EMIT_DOCUMENT_START_STATE,
    /** Expect the content of a document. */
    CI_EMIT_DOCUMENT_CONTENT_STATE,
    /** Expect DOCUMENT-END. */
    CI_EMIT_DOCUMENT_END_STATE,

    /** Expect the first item of a flow sequence. */
    CI_EMIT_FLOW_SEQUENCE_FIRST_ITEM_STATE,
    /** Expect an item of a flow sequence. */
    CI_EMIT_FLOW_SEQUENCE_ITEM_STATE,
    /** Expect the first key of a flow mapping. */
    CI_EMIT_FLOW_MAPPING_FIRST_KEY_STATE,
    /** Expect a key of a flow mapping. */
    CI_EMIT_FLOW_MAPPING_KEY_STATE,
    /** Expect a value for a simple key of a flow mapping. */
    CI_EMIT_FLOW_MAPPING_SIMPLE_VALUE_STATE,

    /** Expect a value of a flow mapping. */
    CI_EMIT_FLOW_MAPPING_VALUE_STATE,
    /** Expect the first item of a block sequence. */
    CI_EMIT_BLOCK_SEQUENCE_FIRST_ITEM_STATE,
    /** Expect an item of a block sequence. */
    CI_EMIT_BLOCK_SEQUENCE_ITEM_STATE,
    /** Expect the first key of a block mapping. */
    CI_EMIT_BLOCK_MAPPING_FIRST_KEY_STATE,
    /** Expect the key of a block mapping. */
    CI_EMIT_BLOCK_MAPPING_KEY_STATE,

    /** Expect a value for a simple key of a block mapping. */
    CI_EMIT_BLOCK_MAPPING_SIMPLE_VALUE_STATE,
    /** Expect a value of a block mapping. */
    CI_EMIT_BLOCK_MAPPING_VALUE_STATE,
    /** Expect nothing. */
    CI_EMIT_END_STATE
} ci_emitter_state_t;


/* This is needed for C++ */

typedef struct ci_anchors_s {
    /** The number of references. */
    int references;
    /** The anchor id. */
    int anchor;
    /** If the node has been emitted? */
    int serialized;
} ci_anchors_t;

/**
 * The emitter structure.
 *
 * All members are internal.  Manage the structure using the @c ci_emitter_
 * family of functions.
 */

typedef struct ci_emitter_s {

    /**
     * @name Error handling
     * @{
     */

    /** Error type. */
    ci_error_type_t error;
    /** Error description. */
    const char *problem;

    /**
     * @}
     */

    /**
     * @name Writer stuff
     * @{
     */

    /** Write handler. */
    ci_write_handler_t *write_handler;

    /** A pointer for passing to the write handler. */
    void *write_handler_data;

    /** Standard (string or file) output data. */
    union {
        /** String output data. */
        struct {
            /** The buffer pointer. */
            unsigned char *buffer;
            /** The buffer size. */
            size_t size;
            /** The number of written bytes. */
            size_t *size_written;
        } string;

        /** File output data. */
        FILE *file;
    } output;

    /** The working buffer. */
    struct {
        /** The beginning of the buffer. */
        ci_char_t *start;
        /** The end of the buffer. */
        ci_char_t *end;
        /** The current position of the buffer. */
        ci_char_t *pointer;
        /** The last filled position of the buffer. */
        ci_char_t *last;
    } buffer;

    /** The raw buffer. */
    struct {
        /** The beginning of the buffer. */
        unsigned char *start;
        /** The end of the buffer. */
        unsigned char *end;
        /** The current position of the buffer. */
        unsigned char *pointer;
        /** The last filled position of the buffer. */
        unsigned char *last;
    } raw_buffer;

    /** The stream encoding. */
    ci_encoding_t encoding;

    /**
     * @}
     */

    /**
     * @name Emitter stuff
     * @{
     */

    /** If the output is in the canonical style? */
    int canonical;
    /** The number of indentation spaces. */
    int best_indent;
    /** The preferred width of the output lines. */
    int best_width;
    /** Allow unescaped non-ASCII characters? */
    int unicode;
    /** The preferred line break. */
    ci_break_t line_break;

    /** The stack of states. */
    struct {
        /** The beginning of the stack. */
        ci_emitter_state_t *start;
        /** The end of the stack. */
        ci_emitter_state_t *end;
        /** The top of the stack. */
        ci_emitter_state_t *top;
    } states;

    /** The current emitter state. */
    ci_emitter_state_t state;

    /** The event queue. */
    struct {
        /** The beginning of the event queue. */
        ci_event_t *start;
        /** The end of the event queue. */
        ci_event_t *end;
        /** The head of the event queue. */
        ci_event_t *head;
        /** The tail of the event queue. */
        ci_event_t *tail;
    } events;

    /** The stack of indentation levels. */
    struct {
        /** The beginning of the stack. */
        int *start;
        /** The end of the stack. */
        int *end;
        /** The top of the stack. */
        int *top;
    } indents;

    /** The list of tag directives. */
    struct {
        /** The beginning of the list. */
        ci_tag_directive_t *start;
        /** The end of the list. */
        ci_tag_directive_t *end;
        /** The top of the list. */
        ci_tag_directive_t *top;
    } tag_directives;

    /** The current indentation level. */
    int indent;

    /** The current flow level. */
    int flow_level;

    /** Is it the document root context? */
    int root_context;
    /** Is it a sequence context? */
    int sequence_context;
    /** Is it a mapping context? */
    int mapping_context;
    /** Is it a simple mapping key context? */
    int simple_key_context;

    /** The current line. */
    int line;
    /** The current column. */
    int column;
    /** If the last character was a whitespace? */
    int whitespace;
    /** If the last character was an indentation character (' ', '-', '?', ':')? */
    int indention;
    /** If an explicit document end is required? */
    int open_ended;

    /** Anchor analysis. */
    struct {
        /** The anchor value. */
        ci_char_t *anchor;
        /** The anchor length. */
        size_t anchor_length;
        /** Is it an alias? */
        int alias;
    } anchor_data;

    /** Tag analysis. */
    struct {
        /** The tag handle. */
        ci_char_t *handle;
        /** The tag handle length. */
        size_t handle_length;
        /** The tag suffix. */
        ci_char_t *suffix;
        /** The tag suffix length. */
        size_t suffix_length;
    } tag_data;

    /** Scalar analysis. */
    struct {
        /** The scalar value. */
        ci_char_t *value;
        /** The scalar length. */
        size_t length;
        /** Does the scalar contain line breaks? */
        int multiline;
        /** Can the scalar be expressed in the flow plain style? */
        int flow_plain_allowed;
        /** Can the scalar be expressed in the block plain style? */
        int block_plain_allowed;
        /** Can the scalar be expressed in the single quoted style? */
        int single_quoted_allowed;
        /** Can the scalar be expressed in the literal or folded styles? */
        int block_allowed;
        /** The output style. */
        ci_scalar_style_t style;
    } scalar_data;

    /**
     * @}
     */

    /**
     * @name Dumper stuff
     * @{
     */

    /** If the stream was already opened? */
    int opened;
    /** If the stream was already closed? */
    int closed;

    /** The information associated with the document nodes. */
    ci_anchors_t *anchors;

    /** The last assigned anchor id. */
    int last_anchor_id;

    /** The currently emitted document. */
    ci_document_t *document;

    /**
     * @}
     */

} ci_emitter_t;

/**
 * Initialize an emitter.
 *
 * This function creates a new emitter object.  An application is responsible
 * for destroying the object using the ci_emitter_delete() function.
 *
 * @param[out]      emitter     An empty parser object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_initialize(ci_emitter_t *emitter);

/**
 * Destroy an emitter.
 *
 * @param[in,out]   emitter     An emitter object.
 */

CI_DECLARE(void)
ci_emitter_delete(ci_emitter_t *emitter);

/**
 * Set a string output.
 *
 * The emitter will write the output characters to the @a output buffer of the
 * size @a size.  The emitter will set @a size_written to the number of written
 * bytes.  If the buffer is smaller than required, the emitter produces the
 * CI_WRITE_ERROR error.
 *
 * @param[in,out]   emitter         An emitter object.
 * @param[in]       output          An output buffer.
 * @param[in]       size            The buffer size.
 * @param[in]       size_written    The pointer to save the number of written
 *                                  bytes.
 */

CI_DECLARE(void)
ci_emitter_set_output_string(ci_emitter_t *emitter,
        unsigned char *output, size_t size, size_t *size_written);

/**
 * Set a file output.
 *
 * @a file should be a file object open for writing.  The application is
 * responsible for closing the @a file.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       file        An open file.
 */

CI_DECLARE(void)
ci_emitter_set_output_file(ci_emitter_t *emitter, FILE *file);

/**
 * Set a generic output handler.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       handler     A write handler.
 * @param[in]       data        Any application data for passing to the write
 *                              handler.
 */

CI_DECLARE(void)
ci_emitter_set_output(ci_emitter_t *emitter,
        ci_write_handler_t *handler, void *data);

/**
 * Set the output encoding.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       encoding    The output encoding.
 */

CI_DECLARE(void)
ci_emitter_set_encoding(ci_emitter_t *emitter, ci_encoding_t encoding);

/**
 * Set if the output should be in the "canonical" format as in the CI
 * specification.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       canonical   If the output is canonical.
 */

CI_DECLARE(void)
ci_emitter_set_canonical(ci_emitter_t *emitter, int canonical);

/**
 * Set the indentation increment.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       indent      The indentation increment (1 < . < 10).
 */

CI_DECLARE(void)
ci_emitter_set_indent(ci_emitter_t *emitter, int indent);

/**
 * Set the preferred line width. @c -1 means unlimited.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       width       The preferred line width.
 */

CI_DECLARE(void)
ci_emitter_set_width(ci_emitter_t *emitter, int width);

/**
 * Set if unescaped non-ASCII characters are allowed.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       unicode     If unescaped Unicode characters are allowed.
 */

CI_DECLARE(void)
ci_emitter_set_unicode(ci_emitter_t *emitter, int unicode);

/**
 * Set the preferred line break.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in]       line_break  The preferred line break.
 */

CI_DECLARE(void)
ci_emitter_set_break(ci_emitter_t *emitter, ci_break_t line_break);

/**
 * Emit an event.
 *
 * The event object may be generated using the ci_parser_parse() function.
 * The emitter takes the responsibility for the event object and destroys its
 * content after it is emitted. The event object is destroyed even if the
 * function fails.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in,out]   event       An event object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_emit(ci_emitter_t *emitter, ci_event_t *event);

/**
 * Start a CI stream.
 *
 * This function should be used before ci_emitter_dump() is called.
 *
 * @param[in,out]   emitter     An emitter object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_open(ci_emitter_t *emitter);

/**
 * Finish a CI stream.
 *
 * This function should be used after ci_emitter_dump() is called.
 *
 * @param[in,out]   emitter     An emitter object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_close(ci_emitter_t *emitter);

/**
 * Emit a CI document.
 *
 * The document object may be generated using the ci_parser_load() function
 * or the ci_document_initialize() function.  The emitter takes the
 * responsibility for the document object and destroys its content after
 * it is emitted. The document object is destroyed even if the function fails.
 *
 * @param[in,out]   emitter     An emitter object.
 * @param[in,out]   document    A document object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_dump(ci_emitter_t *emitter, ci_document_t *document);

/**
 * Flush the accumulated characters to the output.
 *
 * @param[in,out]   emitter     An emitter object.
 *
 * @returns @c 1 if the function succeeded, @c 0 on error.
 */

CI_DECLARE(int)
ci_emitter_flush(ci_emitter_t *emitter);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CI_H */

