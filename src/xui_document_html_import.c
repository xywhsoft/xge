#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_css_named_colors.h"
#include "../lib/md4c/entity.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>

#define DOC_HTML_IMPORT_MAX_BYTES (UINT64_C(256) * 1024 * 1024)

/* Clipboard HTML is untrusted. This importer interprets a small structural
 * vocabulary, ignores active elements, and lets the normal Document schema
 * validate the resulting private Rich transaction before publication. */
typedef struct doc_html_table_context {
    struct doc_html_table_context* previous;
    uint64_t id;
    float* widths;
    uint32_t count, capacity;
} doc_html_table_context;
typedef struct doc_html_input {
    const char* html;
    size_t size, at;
    xui_document_transaction transaction;
    doc_html_table_context* table;
    int explicit_block;
} doc_html_input;
typedef struct doc_html_tag {
    char name[32];
    size_t attrs, attrs_end;
    int closing, empty;
    char language[256]; /* Stable through recursive parsing of this tag. */
} doc_html_tag;
typedef struct doc_html_buffer {
    char* data;
    size_t size, capacity;
} doc_html_buffer;

static int doc_html_buffer_add(doc_html_buffer* b, const char* s, size_t n)
{
    size_t capacity;
    char* p;
    if (n > SIZE_MAX - b->size - 1) return XUI_DOC_ERROR_LIMIT;
    if (n > DOC_HTML_IMPORT_MAX_BYTES - b->size) return XUI_DOC_ERROR_LIMIT;
    if (b->capacity < b->size + n + 1) {
        capacity = b->capacity ? b->capacity : 64;
        while (capacity < b->size + n + 1) {
            if (capacity > SIZE_MAX / 2) { capacity = b->size + n + 1; break; }
            capacity *= 2;
        }
        p = realloc(b->data, capacity);
        if (!p) return XUI_ERROR_OUT_OF_MEMORY;
        b->data = p; b->capacity = capacity;
    }
    if (n) memcpy(b->data + b->size, s, n);
    b->size += n; b->data[b->size] = 0;
    return XUI_OK;
}
static int doc_html_buffer_codepoint(doc_html_buffer* b, unsigned c)
{
    char bytes[4]; size_t n;
    if (!c || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) c = 0xfffd;
    if (c < 0x80) { bytes[0] = (char)c; n = 1; }
    else if (c < 0x800) {
        bytes[0] = (char)(0xc0 | (c >> 6)); bytes[1] = (char)(0x80 | (c & 63)); n = 2;
    } else if (c < 0x10000) {
        bytes[0] = (char)(0xe0 | (c >> 12)); bytes[1] = (char)(0x80 | ((c >> 6) & 63));
        bytes[2] = (char)(0x80 | (c & 63)); n = 3;
    } else {
        bytes[0] = (char)(0xf0 | (c >> 18)); bytes[1] = (char)(0x80 | ((c >> 12) & 63));
        bytes[2] = (char)(0x80 | ((c >> 6) & 63)); bytes[3] = (char)(0x80 | (c & 63)); n = 4;
    }
    return doc_html_buffer_add(b, bytes, n);
}
static int doc_html_decode(const char* s, size_t n, char** out, size_t* length)
{
    doc_html_buffer b = {0}; size_t i = 0;
    int result = XUI_OK;
    *out = NULL; *length = 0;
    while (i < n && result == XUI_OK) {
        size_t end;
        if (s[i] != '&') {
            size_t start = i++;
            while (i < n && s[i] != '&') i++;
            result = doc_html_buffer_add(&b, s + start, i - start);
            continue;
        }
        end = i + 1;
        while (end < n && end - i <= 33 && s[end] != ';' && s[end] != '&' && s[end] != '<') end++;
        if (end < n && s[end] == ';') {
            if (i + 2 < end && s[i + 1] == '#') {
                size_t j = i + 2; unsigned value = 0;
                unsigned base = j < end && (s[j] == 'x' || s[j] == 'X') ? 16 : 10;
                int valid = 1;
                if (base == 16) j++;
                if (j == end) valid = 0;
                for (; valid && j < end; j++) {
                    unsigned char ch = (unsigned char)s[j];
                    unsigned digit = ch >= '0' && ch <= '9' ? ch - '0' :
                        ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 :
                        ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 16;
                    if (digit >= base || value > (0x10ffffu - digit) / base) valid = 0;
                    else value = value * base + digit;
                }
                if (valid) {
                    result = doc_html_buffer_codepoint(&b, value);
                    i = end + 1; continue;
                }
            } else {
                const ENTITY* entity = entity_lookup(s + i, end - i + 1);
                if (entity) {
                    result = doc_html_buffer_codepoint(&b, entity->codepoints[0]);
                    if (result == XUI_OK && entity->codepoints[1])
                        result = doc_html_buffer_codepoint(&b, entity->codepoints[1]);
                    i = end + 1; continue;
                }
            }
        }
        result = doc_html_buffer_add(&b, s + i, 1); i++;
    }
    if (result == XUI_OK && !b.data) result = doc_html_buffer_add(&b, "", 0);
    if (result != XUI_OK) { free(b.data); return result; }
    *out = b.data; *length = b.size; return XUI_OK;
}
static int doc_html_name_char(unsigned char c)
{
    return isalnum(c) || c == '-' || c == '_' || c == ':';
}
static int doc_html_space(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f';
}
static int doc_html_attr_read(const doc_html_input*, const doc_html_tag*, const char*, char**);
static int doc_html_tag_read(doc_html_input* p, doc_html_tag* tag)
{
    size_t start, end;
    char quote = 0;
    memset(tag, 0, sizeof(*tag));
    if (p->at >= p->size || p->html[p->at] != '<') return XUI_DOC_ERROR_FORMAT;
    if (p->size - p->at >= 4 && !memcmp(p->html + p->at, "<!--", 4)) {
        p->at += 4;
        while (p->at + 2 < p->size && memcmp(p->html + p->at, "-->", 3)) p->at++;
        if (p->at + 2 >= p->size) return XUI_DOC_ERROR_FORMAT;
        p->at += 3; strcpy(tag->name, "!comment"); tag->empty = 1; return XUI_OK;
    }
    p->at++;
    if (p->at < p->size && p->html[p->at] == '/') { tag->closing = 1; p->at++; }
    if (p->at < p->size && (p->html[p->at] == '!' || p->html[p->at] == '?')) {
        while (p->at < p->size && p->html[p->at] != '>') p->at++;
        if (p->at == p->size) return XUI_DOC_ERROR_FORMAT;
        p->at++; strcpy(tag->name, "!declaration"); tag->empty = 1; return XUI_OK;
    }
    while (p->at < p->size && doc_html_space((unsigned char)p->html[p->at])) p->at++;
    start = p->at;
    while (p->at < p->size && doc_html_name_char((unsigned char)p->html[p->at])) p->at++;
    end = p->at;
    if (end == start || end - start >= sizeof(tag->name)) return XUI_DOC_ERROR_FORMAT;
    {
        size_t i;
        for (i = start; i < end; i++) tag->name[i - start] = (char)tolower((unsigned char)p->html[i]);
    }
    tag->attrs = p->at;
    while (p->at < p->size) {
        char c = p->html[p->at];
        if (quote) { if (c == quote) quote = 0; p->at++; continue; }
        if (c == '"' || c == '\'') { quote = c; p->at++; continue; }
        if (c == '>') break;
        p->at++;
    }
    if (p->at == p->size) return XUI_DOC_ERROR_FORMAT;
    tag->attrs_end = p->at;
    while (tag->attrs_end > tag->attrs && doc_html_space((unsigned char)p->html[tag->attrs_end - 1])) tag->attrs_end--;
    if (tag->attrs_end > tag->attrs && p->html[tag->attrs_end - 1] == '/') {
        tag->empty = 1; tag->attrs_end--;
    }
    p->at++;
    if (!strcmp(tag->name, "br") || !strcmp(tag->name, "hr") ||
        !strcmp(tag->name, "img") || !strcmp(tag->name, "input") ||
        !strcmp(tag->name, "col") || !strcmp(tag->name, "meta") ||
        !strcmp(tag->name, "link") || !strcmp(tag->name, "base")) tag->empty = 1;
    if (!tag->closing) {
        char* language = NULL;
        int result = doc_html_attr_read(p, tag, "lang", &language);
        if (result == XUI_OK && !language)
            result = doc_html_attr_read(p, tag, "xml:lang", &language);
        if (result == XUI_OK && language && doc_language_valid(language))
            strcpy(tag->language, *language ? language : "und");
        free(language);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_html_attr_read(const doc_html_input* p, const doc_html_tag* tag,
    const char* wanted, char** out)
{
    size_t at = tag->attrs;
    *out = NULL;
    while (at < tag->attrs_end) {
        size_t start, value_start, value_end, i;
        char quote = 0;
        while (at < tag->attrs_end && (doc_html_space((unsigned char)p->html[at]) || p->html[at] == '/')) at++;
        start = at;
        while (at < tag->attrs_end && doc_html_name_char((unsigned char)p->html[at])) at++;
        if (at == start) { at++; continue; }
        i = at;
        while (at < tag->attrs_end && doc_html_space((unsigned char)p->html[at])) at++;
        if (at >= tag->attrs_end || p->html[at] != '=') continue;
        at++;
        while (at < tag->attrs_end && doc_html_space((unsigned char)p->html[at])) at++;
        if (at < tag->attrs_end && (p->html[at] == '"' || p->html[at] == '\'')) quote = p->html[at++];
        value_start = at;
        if (quote) {
            while (at < tag->attrs_end && p->html[at] != quote) at++;
            value_end = at;
            if (at < tag->attrs_end) at++;
        } else {
            while (at < tag->attrs_end && !doc_html_space((unsigned char)p->html[at])) at++;
            value_end = at;
        }
        if (strlen(wanted) == i - start) {
            size_t j;
            for (j = 0; j < i - start; j++)
                if ((unsigned char)tolower((unsigned char)p->html[start + j]) != (unsigned char)wanted[j]) break;
            if (j == i - start) {
                size_t length;
                return doc_html_decode(p->html + value_start, value_end - value_start, out, &length);
            }
        }
    }
    return XUI_OK;
}
static int doc_html_attr_number(const doc_html_input* p,
    const doc_html_tag* tag, const char* wanted, uint32_t fallback,
    uint32_t maximum, uint32_t* out)
{
    char *value = NULL, *end = NULL;
    unsigned long number;
    int result = doc_html_attr_read(p, tag, wanted, &value);
    if (result != XUI_OK) return result;
    *out = fallback;
    if (!value) return XUI_OK;
    number = strtoul(value, &end, 10);
    if (!*value || *end || number > maximum || number == 0) number = fallback;
    *out = (uint32_t)number;
    free(value);
    return XUI_OK;
}
static int doc_html_attr_u32_strict(const doc_html_input* p,
    const doc_html_tag* tag, const char* wanted, uint32_t fallback,
    uint32_t minimum, uint32_t maximum, uint32_t* out)
{
    char* value = NULL;
    uint64_t number = 0;
    size_t i;
    int result = doc_html_attr_read(p, tag, wanted, &value);
    if (result != XUI_OK) return result;
    *out = fallback;
    if (!value) return XUI_OK;
    if (!*value) result = XUI_DOC_ERROR_FORMAT;
    for (i = 0; result == XUI_OK && value[i]; i++) {
        unsigned digit = (unsigned char)value[i] - '0';
        if (digit > 9 || digit > maximum || number > (maximum - digit) / 10u)
            result = XUI_DOC_ERROR_FORMAT;
        else number = number * 10u + digit;
    }
    if (result == XUI_OK && number < minimum) result = XUI_DOC_ERROR_FORMAT;
    if (result == XUI_OK) *out = (uint32_t)number;
    free(value);
    return result;
}
static int doc_html_css_name(const char* source, size_t bytes, const char* wanted)
{
    size_t i;
    if (!wanted || bytes != strlen(wanted)) return 0;
    for (i = 0; i < bytes; i++)
        if (tolower((unsigned char)source[i]) != wanted[i]) return 0;
    return 1;
}
static int doc_html_css_value_supported(const char* property,
    const char* value);
static int doc_html_css_value_any(const char* style, const char* wanted,
    const char* alternative, char* out, size_t capacity)
{
    size_t at = 0;
    int found = 0, chosen_important = 0;
    while (style && style[at]) {
        size_t start = at, colon, name_end, end, length;
        char quote = 0;
        while (style[at]) {
            if (quote) {
                if (style[at] == '\\' && style[at + 1]) { at += 2; continue; }
                if (style[at] == quote) quote = 0;
            } else if (style[at] == '"' || style[at] == '\'') quote = style[at];
            else if (style[at] == ';') break;
            at++;
        }
        end = at;
        if (style[at]) at++;
        colon = start;
        while (colon < end && style[colon] != ':') colon++;
        if (colon == end) continue;
        name_end = colon;
        while (start < colon && doc_html_space((unsigned char)style[start])) start++;
        while (name_end > start && doc_html_space((unsigned char)style[name_end - 1])) name_end--;
        if (!doc_html_css_name(style + start, name_end - start, wanted) &&
            !doc_html_css_name(style + start, name_end - start, alternative)) continue;
        start = colon + 1;
        while (start < end && doc_html_space((unsigned char)style[start])) start++;
        while (end > start && doc_html_space((unsigned char)style[end - 1])) end--;
        {
            static const char suffix[] = "!important";
            size_t i; int important = 0;
            if (end - start >= sizeof(suffix) - 1) {
                size_t suffix_start = end - (sizeof(suffix) - 1);
                for (i = 0; i < sizeof(suffix) - 1; i++)
                    if (tolower((unsigned char)style[suffix_start + i]) != suffix[i]) break;
                if (i == sizeof(suffix) - 1) {
                    important = 1; end = suffix_start;
                    while (end > start && doc_html_space((unsigned char)style[end - 1])) end--;
                }
            }
            if (found && important < chosen_important) continue;
            length = end - start;
            if (!length || length >= capacity || length >= 512) continue;
            {
                char candidate[512];
                memcpy(candidate, style + start, length);
                candidate[length] = 0;
                if (!doc_html_css_value_supported(wanted, candidate)) continue;
                memcpy(out, candidate, length + 1);
            }
            chosen_important = important; found = 1;
        }
    }
    return found;
}
static int doc_html_css_value(const char* style, const char* wanted,
    char* out, size_t capacity)
{
    return doc_html_css_value_any(style, wanted, NULL, out, capacity);
}
static int doc_html_hex(unsigned char c)
{
    return c >= '0' && c <= '9' ? c - '0' :
        c >= 'a' && c <= 'f' ? c - 'a' + 10 :
        c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}
static void doc_html_css_skip_space(const char** at)
{
    while (doc_html_space((unsigned char)**at)) (*at)++;
}
static int doc_html_css_decimal(const char** at, double* value)
{
    const char* start = *at;
    const char* scan = start;
    xstrview token;
    size_t digits = 0;
    double number;
    if (*scan == '+' || *scan == '-') scan++;
    while (isdigit((unsigned char)*scan)) { scan++; digits++; }
    if (*scan == '.' && isdigit((unsigned char)scan[1])) {
        scan++;
        while (isdigit((unsigned char)*scan)) { scan++; digits++; }
    }
    if (!digits) return 0;
    if (*scan == 'e' || *scan == 'E') {
        scan++;
        if (*scan == '+' || *scan == '-') scan++;
        if (!isdigit((unsigned char)*scan)) return 0;
        while (isdigit((unsigned char)*scan)) scan++;
    }
    token.Data = start; token.Size = (size_t)(scan - start);
    if (!xrtNumParse(token, 0, &number) || !isfinite(number)) return 0;
    *at = scan; *value = number;
    return 1;
}
static int doc_html_css_number(const char** at, float* value, int* percent)
{
    double number;
    float narrowed;
    if (!doc_html_css_decimal(at, &number)) return 0;
    narrowed = (float)number;
    if (!isfinite(narrowed)) return 0;
    *percent = **at == '%';
    if (*percent) (*at)++;
    *value = narrowed;
    return 1;
}
static uint32_t doc_html_css_byte(float number)
{
    return number <= 0 ? 0 : number >= 255 ? 255 : (uint32_t)(number + 0.5f);
}
static int doc_html_css_hue(const char** at, double* hue)
{
    const char* unit;
    double angle, revolution = 360;
    size_t unit_length;
    if (!doc_html_css_decimal(at, &angle)) return 0;
    unit = *at;
    while (isalpha((unsigned char)**at)) (*at)++;
    unit_length = (size_t)(*at - unit);
    if (!unit_length || doc_html_css_name(unit, unit_length, "deg")) {}
    else if (doc_html_css_name(unit, unit_length, "grad")) revolution = 400;
    else if (doc_html_css_name(unit, unit_length, "rad"))
        revolution = 2.0 * 3.14159265358979323846;
    else if (doc_html_css_name(unit, unit_length, "turn")) revolution = 1;
    else return 0;
    angle = fmod(angle, revolution) * (360.0 / revolution);
    if (angle < 0) angle += 360.0;
    *hue = angle;
    return 1;
}
static int doc_html_css_hsl(const char* value, uint32_t* out)
{
    const char* at;
    const char* end;
    double hue, saturation, lightness, alpha = 1, chroma;
    uint32_t channel[4];
    int legacy, percent, has_alpha = 0;
    unsigned i;
    if (doc_html_css_name(value, 4, "hsl(")) at = value + 4;
    else if (doc_html_css_name(value, 5, "hsla(")) at = value + 5;
    else return 0;
    doc_html_css_skip_space(&at);
    if (!doc_html_css_hue(&at, &hue)) return 0;
    end = at;
    doc_html_css_skip_space(&at);
    legacy = *at == ',';
    if (legacy) at++;
    else if (at == end) return 0;
    doc_html_css_skip_space(&at);
    {
        float number;
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        if (legacy && !percent) return 0;
        saturation = number;
    }
    end = at;
    doc_html_css_skip_space(&at);
    if (legacy) { if (*at++ != ',') return 0; }
    else if (at == end || *at == ',') return 0;
    doc_html_css_skip_space(&at);
    {
        float number;
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        if (legacy && !percent) return 0;
        lightness = number;
    }
    doc_html_css_skip_space(&at);
    if (legacy) {
        if (*at == ',') { at++; has_alpha = 1; }
    } else if (*at == '/') { at++; has_alpha = 1; }
    if (has_alpha) {
        float number;
        doc_html_css_skip_space(&at);
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        alpha = percent ? number / 100.0 : number;
        doc_html_css_skip_space(&at);
    }
    if (*at++ != ')' || *at) return 0;
    saturation = fmax(0.0, saturation) / 100.0;
    lightness /= 100.0;
    chroma = saturation * fmin(lightness, 1.0 - lightness);
    for (i = 0; i < 3; i++) {
        double k = fmod((i == 0 ? 0 : i == 1 ? 8 : 4) + hue / 30.0, 12.0);
        double color = lightness - chroma *
            fmax(-1.0, fmin(fmin(k - 3.0, 9.0 - k), 1.0));
        channel[i] = doc_html_css_byte((float)(color * 255.0));
    }
    channel[3] = doc_html_css_byte((float)(alpha * 255.0));
    *out = (channel[0] << 24) | (channel[1] << 16) |
        (channel[2] << 8) | channel[3];
    return 1;
}
static int doc_html_css_hwb(const char* value, uint32_t* out)
{
    const char* at = value + 4;
    const char* end;
    double hue, white, black, alpha = 1;
    uint32_t channel[4];
    int percent;
    unsigned i;
    doc_html_css_skip_space(&at);
    if (!doc_html_css_hue(&at, &hue)) return 0;
    end = at;
    doc_html_css_skip_space(&at);
    if (at == end) return 0;
    {
        float number;
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        white = number;
    }
    end = at;
    doc_html_css_skip_space(&at);
    if (at == end) return 0;
    {
        float number;
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        black = number;
    }
    doc_html_css_skip_space(&at);
    if (*at == '/') {
        float number;
        at++;
        doc_html_css_skip_space(&at);
        if (!doc_html_css_number(&at, &number, &percent)) return 0;
        alpha = percent ? number / 100.0 : number;
        doc_html_css_skip_space(&at);
    }
    if (*at++ != ')' || *at) return 0;
    white = fmax(0.0, fmin(white, 100.0)) / 100.0;
    black = fmax(0.0, fmin(black, 100.0)) / 100.0;
    for (i = 0; i < 3; i++) {
        double color;
        if (white + black >= 1.0) color = white / (white + black);
        else {
            double k = fmod((i == 0 ? 0 : i == 1 ? 8 : 4) + hue / 30.0, 12.0);
            double pure = 0.5 - 0.5 *
                fmax(-1.0, fmin(fmin(k - 3.0, 9.0 - k), 1.0));
            color = pure * (1.0 - white - black) + white;
        }
        channel[i] = doc_html_css_byte((float)(color * 255.0));
    }
    channel[3] = doc_html_css_byte((float)(alpha * 255.0));
    *out = (channel[0] << 24) | (channel[1] << 16) |
        (channel[2] << 8) | channel[3];
    return 1;
}
static int doc_html_css_color(const char* value, uint32_t* out)
{
    size_t length = strlen(value), i;
    uint32_t channel[4] = {0, 0, 0, 255};
    const char* at;
    int rgba = 0, legacy = 0, percents[3] = {0};
    if (doc_html_css_name(value, length, "transparent")) {
        *out = 0;
        return 1;
    }
    for (i = 0; i < sizeof(doc_html_named_colors) /
        sizeof(doc_html_named_colors[0]); i++) {
        const doc_html_named_color* named = &doc_html_named_colors[i];
        if (!doc_html_css_name(value, length, named->name)) continue;
        *out = ((uint32_t)named->r << 24) | ((uint32_t)named->g << 16) |
            ((uint32_t)named->b << 8) | 255u;
        return 1;
    }
    if ((length > 5 && doc_html_css_name(value, 4, "hsl(")) ||
        (length > 6 && doc_html_css_name(value, 5, "hsla(")))
        return doc_html_css_hsl(value, out);
    if (length > 5 && doc_html_css_name(value, 4, "hwb("))
        return doc_html_css_hwb(value, out);
    if (value[0] == '#') {
        if (length != 4 && length != 5 && length != 7 && length != 9) return 0;
        for (i = 0; i < (length == 4 || length == 7 ? 3u : 4u); i++) {
            int high = doc_html_hex((unsigned char)value[1 +
                (length <= 5 ? i : i * 2)]);
            int low = length <= 5 ? high :
                doc_html_hex((unsigned char)value[2 + i * 2]);
            if (high < 0 || low < 0) return 0;
            channel[i] = (uint32_t)((high << 4) | low);
        }
    } else {
        float components[4] = {0, 0, 0, 1};
        int percent = 0, has_alpha = 0;
        if (length > 5 && doc_html_css_name(value, 4, "rgb(")) at = value + 4;
        else if (length > 6 && doc_html_css_name(value, 5, "rgba(")) {
            at = value + 5; rgba = 1;
        } else return 0;
        for (i = 0; i < 3; i++) {
            const char* after;
            doc_html_css_skip_space(&at);
            if (!doc_html_css_number(&at, &components[i], &percents[i])) return 0;
            after = at;
            doc_html_css_skip_space(&at);
            if (i == 0) legacy = *at == ',';
            if (i < 2) {
                if (legacy) {
                    if (*at++ != ',') return 0;
                } else if (at == after || *at == ',') return 0;
            }
        }
        if (legacy) {
            if (percents[0] != percents[1] || percents[1] != percents[2]) return 0;
            if (*at == ',') { at++; has_alpha = 1; }
            if (rgba && !has_alpha) return 0;
        } else if (*at == '/') { at++; has_alpha = 1; }
        if (has_alpha) {
            doc_html_css_skip_space(&at);
            if (!doc_html_css_number(&at, &components[3], &percent)) return 0;
            doc_html_css_skip_space(&at);
        }
        if (*at++ != ')' || *at) return 0;
        for (i = 0; i < 3; i++)
            channel[i] = doc_html_css_byte(percents[i] ?
                components[i] * 2.55f : components[i]);
        channel[3] = doc_html_css_byte((percent ? components[3] / 100.f :
            components[3]) * 255.f);
    }
    *out = (channel[0] << 24) | (channel[1] << 16) |
        (channel[2] << 8) | channel[3];
    return 1;
}
static int doc_html_css_family(const char* source, size_t length,
    char output[64])
{
    size_t i = 0, used = 0;
    while (i < length) {
        unsigned codepoint;
        char encoded[4]; size_t count;
        if (source[i] != '\\') {
            if (used >= 63) return 0;
            output[used++] = source[i++];
            continue;
        }
        i++;
        if (i == length) return 0;
        if (doc_html_hex((unsigned char)source[i]) < 0) {
            if (used >= 63) return 0;
            output[used++] = source[i++];
            continue;
        }
        codepoint = 0;
        {
            unsigned digits = 0;
            while (i < length && digits < 6 &&
                doc_html_hex((unsigned char)source[i]) >= 0) {
                codepoint = (codepoint << 4) |
                    (unsigned)doc_html_hex((unsigned char)source[i++]);
                digits++;
            }
        }
        if (i < length && doc_html_space((unsigned char)source[i])) i++;
        if (!codepoint || codepoint > 0x10ffff ||
            (codepoint >= 0xd800 && codepoint <= 0xdfff)) codepoint = 0xfffd;
        if (codepoint < 0x80) {
            encoded[0] = (char)codepoint; count = 1;
        } else if (codepoint < 0x800) {
            encoded[0] = (char)(0xc0 | (codepoint >> 6));
            encoded[1] = (char)(0x80 | (codepoint & 63)); count = 2;
        } else if (codepoint < 0x10000) {
            encoded[0] = (char)(0xe0 | (codepoint >> 12));
            encoded[1] = (char)(0x80 | ((codepoint >> 6) & 63));
            encoded[2] = (char)(0x80 | (codepoint & 63)); count = 3;
        } else {
            encoded[0] = (char)(0xf0 | (codepoint >> 18));
            encoded[1] = (char)(0x80 | ((codepoint >> 12) & 63));
            encoded[2] = (char)(0x80 | ((codepoint >> 6) & 63));
            encoded[3] = (char)(0x80 | (codepoint & 63)); count = 4;
        }
        if (used + count >= 64) return 0;
        memcpy(output + used, encoded, count); used += count;
    }
    output[used] = 0;
    return doc_utf8(output, used);
}
static int doc_html_css_length_px(const char* value, int positive,
    int points, int bare_zero, float* out)
{
    const char* at = value;
    double number;
    size_t unit_length;
    if (!doc_html_css_decimal(&at, &number)) return 0;
    unit_length = strlen(at);
    if (!unit_length && bare_zero && number == 0) {}
    else if (doc_html_css_name(at, unit_length, "px")) {}
    else if (points && doc_html_css_name(at, unit_length, "pt"))
        number *= 4.0 / 3.0;
    else return 0;
    if (!isfinite(number) || number > 1000000 ||
        (positive ? number <= 0 : number < 0)) return 0;
    if (out) *out = (float)number;
    return 1;
}
static int doc_html_css_weight(const char* value, float* out)
{
    const char* at = value;
    double number;
    if (!doc_html_css_decimal(&at, &number) || *at ||
        number < 1 || number > 1000) return 0;
    if (out) *out = (float)number;
    return 1;
}
static int doc_html_css_value_supported(const char* property,
    const char* value)
{
    size_t length = strlen(value);
    if (!strcmp(property, "color") || !strcmp(property, "background-color")) {
        uint32_t color;
        return doc_html_css_name(value, length, "currentcolor") ||
            doc_html_css_color(value, &color);
    }
    if (!strcmp(property, "font-size"))
        return doc_html_css_length_px(value, 1, 1, 0, NULL);
    if (!strcmp(property, "width") || !strcmp(property, "height"))
        return doc_html_css_length_px(value, 1, 0, 0, NULL);
    if (!strcmp(property, "margin-bottom"))
        return doc_html_css_length_px(value, 0, 1, 1, NULL);
    if (!strcmp(property, "font-family")) {
        const char* name = value;
        char family[64];
        if (length >= 2 && ((name[0] == '"' && name[length - 1] == '"') ||
            (name[0] == '\'' && name[length - 1] == '\''))) {
            name++; length -= 2;
        } else if (name[0] == '"' || name[0] == '\'' ||
            name[length - 1] == '"' || name[length - 1] == '\'') return 0;
        return length && doc_html_css_family(name, length, family);
    }
    if (!strcmp(property, "font-weight")) {
        if (doc_html_css_name(value, length, "bold") ||
            doc_html_css_name(value, length, "bolder") ||
            doc_html_css_name(value, length, "normal") ||
            doc_html_css_name(value, length, "lighter")) return 1;
        return doc_html_css_weight(value, NULL);
    }
    if (!strcmp(property, "font-style"))
        return doc_html_css_name(value, length, "normal") ||
            doc_html_css_name(value, length, "italic") ||
            doc_html_css_name(value, length, "oblique");
    if (!strcmp(property, "text-align"))
        return doc_html_css_name(value, length, "left") ||
            doc_html_css_name(value, length, "center") ||
            doc_html_css_name(value, length, "right") ||
            doc_html_css_name(value, length, "justify");
    if (!strcmp(property, "text-decoration")) {
        size_t at = 0;
        int lines = 0;
        if (doc_html_css_name(value, length, "none")) return 1;
        while (value[at]) {
            size_t start;
            while (value[at] && doc_html_space((unsigned char)value[at])) at++;
            start = at;
            while (value[at] && !doc_html_space((unsigned char)value[at])) at++;
            if (start == at) break;
            if (!doc_html_css_name(value + start, at - start, "underline") &&
                !doc_html_css_name(value + start, at - start, "line-through")) return 0;
            lines++;
        }
        return lines > 0;
    }
    return 0;
}
static void doc_html_color_apply(xui_doc_attributes_t* attrs,
    const char* style, int background)
{
    char value[512]; uint32_t color;
    if (!doc_html_css_value(style, background ? "background-color" : "color",
        value, sizeof(value))) return;
    if (doc_html_css_name(value, strlen(value), "currentcolor")) {
        if (background) {
            attrs->iBackgroundColor = 0;
            attrs->iFlags &= ~XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO;
            attrs->iFlags |= XUI_DOC_BACKGROUND_COLOR_CURRENT;
        } else {
            /* Materialize colors inherited from flattened HTML containers: their
             * CSS parent will not exist in the exported Document tree. */
            if (doc_text_color_set(attrs))
                attrs->iFlags &= ~XUI_DOC_TEXT_COLOR_CURRENT;
            else attrs->iFlags |= XUI_DOC_TEXT_COLOR_CURRENT;
        }
    } else if (doc_html_css_color(value, &color)) {
        if (background) {
            attrs->iBackgroundColor = color;
            attrs->iFlags &= ~(XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
                XUI_DOC_BACKGROUND_COLOR_CURRENT);
            if (!color) attrs->iFlags |= XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO;
        } else {
            attrs->iTextColor = color;
            attrs->iFlags &= ~(XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO |
                XUI_DOC_TEXT_COLOR_CURRENT);
            if (!color) attrs->iFlags |= XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO;
        }
    }
}
static void doc_html_style_apply(xui_doc_attributes_t* attrs, const char* style)
{
    char value[512]; size_t length;
    doc_html_color_apply(attrs, style, 0);
    doc_html_color_apply(attrs, style, 1);
    if (doc_html_css_value(style, "font-size", value, sizeof(value))) {
        float size;
        if (doc_html_css_length_px(value, 1, 1, 0, &size))
            attrs->fFontSize = size;
    }
    if (doc_html_css_value(style, "font-family", value, sizeof(value))) {
        char* name = value;
        char family[64];
        length = strlen(name);
        if (length >= 2 && ((name[0] == '"' && name[length - 1] == '"') ||
            (name[0] == '\'' && name[length - 1] == '\''))) {
            name++; length -= 2;
        }
        if (doc_html_css_family(name, length, family))
            strcpy(attrs->sFontFamily, family);
    }
    if (doc_html_css_value(style, "font-weight", value, sizeof(value))) {
        if (doc_html_css_name(value, strlen(value), "bold") ||
            doc_html_css_name(value, strlen(value), "bolder"))
            attrs->iMarks |= XUI_DOC_BOLD;
        else if (doc_html_css_name(value, strlen(value), "normal") ||
            doc_html_css_name(value, strlen(value), "lighter"))
            attrs->iMarks &= ~XUI_DOC_BOLD;
        else {
            float weight;
            if (doc_html_css_weight(value, &weight)) {
                if (weight >= 600) attrs->iMarks |= XUI_DOC_BOLD;
                else attrs->iMarks &= ~XUI_DOC_BOLD;
            }
        }
    }
    if (doc_html_css_value(style, "font-style", value, sizeof(value))) {
        length = strlen(value);
        if (doc_html_css_name(value, length, "normal"))
            attrs->iMarks &= ~XUI_DOC_ITALIC;
        else if (doc_html_css_name(value, length, "italic") ||
            (length >= 7 && doc_html_css_name(value, 7, "oblique") &&
                (length == 7 || doc_html_space((unsigned char)value[7]))))
            attrs->iMarks |= XUI_DOC_ITALIC;
    }
    if (doc_html_css_value_any(style, "text-decoration", "text-decoration-line",
        value, sizeof(value))) {
        size_t at = 0;
        while (value[at]) {
            size_t start;
            while (value[at] && doc_html_space((unsigned char)value[at])) at++;
            start = at;
            while (value[at] && !doc_html_space((unsigned char)value[at])) at++;
            if (doc_html_css_name(value + start, at - start, "underline"))
                attrs->iMarks |= XUI_DOC_UNDERLINE;
            else if (doc_html_css_name(value + start, at - start, "line-through"))
                attrs->iMarks |= XUI_DOC_STRIKE;
        }
    }
}
static void doc_html_text_align_apply(xui_doc_attributes_t* attrs,
    const char* style)
{
    char value[128];
    if (doc_html_css_value(style, "text-align", value, sizeof(value))) {
        size_t length = strlen(value);
        if (doc_html_css_name(value, length, "left")) {
            attrs->iAlignment = 0;
            attrs->iFlags |= XUI_DOC_ALIGNMENT_EXPLICIT_LEFT;
        } else if (doc_html_css_name(value, length, "center")) {
            attrs->iAlignment = 1;
            attrs->iFlags &= ~XUI_DOC_ALIGNMENT_EXPLICIT_LEFT;
        } else if (doc_html_css_name(value, length, "right")) {
            attrs->iAlignment = 2;
            attrs->iFlags &= ~XUI_DOC_ALIGNMENT_EXPLICIT_LEFT;
        } else if (doc_html_css_name(value, length, "justify")) {
            attrs->iAlignment = 3;
            attrs->iFlags &= ~XUI_DOC_ALIGNMENT_EXPLICIT_LEFT;
        }
    }
}
static void doc_html_inherited_style_apply(xui_doc_attributes_t* attrs,
    const char* style)
{
    uint32_t background = attrs->iBackgroundColor;
    uint32_t background_flags = attrs->iFlags &
        (XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO | XUI_DOC_BACKGROUND_COLOR_CURRENT);
    doc_html_style_apply(attrs, style);
    attrs->iBackgroundColor = background;
    attrs->iFlags = (attrs->iFlags & ~(XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
        XUI_DOC_BACKGROUND_COLOR_CURRENT)) | background_flags;
    doc_html_text_align_apply(attrs, style);
}
static void doc_html_block_style_apply(xui_doc_attributes_t* attrs,
    const char* style)
{
    char value[128];
    doc_html_style_apply(attrs, style);
    doc_html_text_align_apply(attrs, style);
    if (doc_html_css_value(style, "margin-bottom", value, sizeof(value))) {
        float amount;
        if (doc_html_css_length_px(value, 0, 1, 1, &amount)) {
            attrs->fParagraphSpacing = amount;
            attrs->iFlags |= XUI_DOC_SPACING_EXPLICIT;
        }
    }
}
static int doc_html_tag_inherited_style(const doc_html_input* p,
    const doc_html_tag* tag, xui_doc_attributes_t inherited,
    xui_doc_attributes_t* out)
{
    char* style = NULL;
    int result = doc_html_attr_read(p, tag, "style", &style);
    *out = inherited;
    if (tag->language[0]) out->sLanguage = tag->language;
    if (result == XUI_OK && style) doc_html_inherited_style_apply(out, style);
    free(style);
    return result;
}
static int doc_html_tag_cell_style(const doc_html_input* p,
    const doc_html_tag* tag, xui_doc_attributes_t inherited,
    xui_doc_attributes_t* cell, xui_doc_attributes_t* child_style)
{
    char* style = NULL;
    int result = doc_html_attr_read(p, tag, "style", &style);
    *child_style = inherited;
    if (tag->language[0]) {
        child_style->sLanguage = tag->language;
        cell->sLanguage = tag->language;
    }
    if (result == XUI_OK && style)
        doc_html_inherited_style_apply(child_style, style);
    if (result == XUI_OK) {
        /* Text styles inherit into the cell, but its background belongs to the
         * cell itself and is resolved against the cell's own foreground. */
        cell->iTextColor = child_style->iTextColor;
        cell->iFlags |= child_style->iFlags &
            (XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO | XUI_DOC_TEXT_COLOR_CURRENT |
                XUI_DOC_ALIGNMENT_EXPLICIT_LEFT);
        cell->iAlignment = child_style->iAlignment;
        if (style) doc_html_color_apply(cell, style, 1);
    }
    free(style);
    return result;
}
static int doc_html_active(const char* name)
{
    return !strcmp(name, "head") || !strcmp(name, "title") ||
        !strcmp(name, "script") || !strcmp(name, "style") ||
        !strcmp(name, "iframe") || !strcmp(name, "object") ||
        !strcmp(name, "embed") || !strcmp(name, "svg") ||
        !strcmp(name, "template") || !strcmp(name, "form") ||
        !strcmp(name, "noscript");
}
static int doc_html_skip_active(doc_html_input* p, const char* name)
{
    size_t length = strlen(name);
    while (p->at < p->size) {
        size_t i;
        if (p->html[p->at] != '<' || p->at + length + 2 >= p->size ||
            p->html[p->at + 1] != '/') { p->at++; continue; }
        for (i = 0; i < length; i++)
            if (tolower((unsigned char)p->html[p->at + 2 + i]) != name[i]) break;
        if (i == length && (doc_html_space((unsigned char)p->html[p->at + 2 + length]) ||
            p->html[p->at + 2 + length] == '>')) {
            doc_html_tag end;
            return doc_html_tag_read(p, &end);
        }
        p->at++;
    }
    return XUI_OK;
}
static int doc_html_insert(doc_html_input* p, uint64_t parent,
    uint32_t kind, const xui_doc_attributes_t* attrs, const char* text,
    uint64_t bytes, const char* resource, const char* info,
    const char* title, uint64_t* id)
{
    xui_doc_node_desc_t desc = {0}; uint64_t ignored = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind;
    if (attrs) desc.tAttributes = *attrs;
    desc.sText = text; desc.iTextBytes = bytes;
    desc.sResource = resource; desc.sInfo = info; desc.sTitle = title;
    return xuiDocumentTxnInsertNode(p->transaction, parent, UINT64_MAX,
        &desc, id ? id : &ignored);
}
static int doc_html_column(doc_html_input* p, const doc_html_tag* tag)
{
    doc_html_table_context* table = p->table;
    uint32_t span = 1, i;
    char *style = NULL, *attribute = NULL, value[64];
    float width = 0;
    int result;
    if (!table) return XUI_DOC_ERROR_SCHEMA;
    result = doc_html_attr_number(p, tag, "span", 1, 1024, &span);
    if (result != XUI_OK) return result;
    if (span > 1024 - table->count) return XUI_DOC_ERROR_LIMIT;
    result = doc_html_attr_read(p, tag, "style", &style);
    if (result == XUI_OK) result = doc_html_attr_read(p, tag, "width", &attribute);
    if (result != XUI_OK) { free(style); free(attribute); return result; }
    if (doc_html_css_value(style, "width", value, sizeof(value)) ||
        (attribute && strlen(attribute) < sizeof(value) &&
            (strcpy(value, attribute), 1))) {
        const char* at = value;
        double candidate;
        if (doc_html_css_decimal(&at, &candidate) &&
            candidate > 0 && candidate <= 1000000 &&
            (!*at || doc_html_css_name(at, strlen(at), "px")))
            width = (float)candidate;
    }
    free(style); free(attribute);
    if (table->count + span > table->capacity) {
        uint32_t capacity = table->capacity ? table->capacity : 8;
        float* grown;
        while (capacity < table->count + span) capacity *= 2;
        grown = realloc(table->widths, (size_t)capacity * sizeof(float));
        if (!grown) return XUI_ERROR_OUT_OF_MEMORY;
        table->widths = grown; table->capacity = capacity;
    }
    for (i = 0; i < span; i++) table->widths[table->count++] = width;
    return XUI_OK;
}
static int doc_html_block_tag(const char* name)
{
    return !strcmp(name, "p") || !strcmp(name, "div") ||
        !strcmp(name, "blockquote") || !strcmp(name, "ul") ||
        !strcmp(name, "ol") || !strcmp(name, "table") ||
        !strcmp(name, "pre") || !strcmp(name, "hr") ||
        (name[0] == 'h' && name[1] >= '1' && name[1] <= '6' && !name[2]);
}
static int doc_html_parse_blocks(doc_html_input*, uint64_t, uint32_t,
    const char*, xui_doc_attributes_t, unsigned);
static int doc_html_object_content(doc_html_input* p, const char* closing,
    doc_html_buffer* body)
{
    while (p->at < p->size) {
        if (p->html[p->at] != '<') {
            size_t start = p->at, length; char* decoded = NULL;
            int result;
            while (p->at < p->size && p->html[p->at] != '<') p->at++;
            result = doc_html_decode(p->html + start, p->at - start,
                &decoded, &length);
            if (result == XUI_OK)
                result = doc_html_buffer_add(body, decoded, length);
            free(decoded);
            if (result != XUI_OK) return result;
        } else {
            doc_html_tag tag;
            int result = doc_html_tag_read(p, &tag);
            if (result != XUI_OK) return result;
            if (tag.closing && !strcmp(tag.name, closing)) return XUI_OK;
            if (!tag.closing && !strcmp(tag.name, "br")) {
                result = doc_html_buffer_add(body, "\n", 1);
                if (result != XUI_OK) return result;
            } else if (!tag.closing && doc_html_active(tag.name) && !tag.empty) {
                result = doc_html_skip_active(p, tag.name);
                if (result != XUI_OK) return result;
            }
        }
    }
    return XUI_DOC_ERROR_FORMAT;
}
static int doc_html_parse_marked_object(doc_html_input* p, uint64_t parent,
    const doc_html_tag* tag, xui_doc_attributes_t attrs, int* handled)
{
    char *marker = NULL, *resource = NULL, *info = NULL, *title = NULL;
    char *style = NULL, *encoded = NULL;
    void* payload = NULL;
    size_t payload_bytes = 0;
    doc_html_buffer body = {0};
    uint32_t kind = 0, version = 1, required = 0;
    uint64_t id = 0;
    int result;
    *handled = 0;
    if (tag->language[0]) attrs.sLanguage = tag->language;
    result = doc_html_attr_read(p, tag, "data-xui-object", &marker);
    if (result != XUI_OK) return result;
    if (!marker) return XUI_OK;
    if (!strcmp(marker, "math")) kind = XUI_DOC_MATH;
    else if (!strcmp(marker, "diagram")) kind = XUI_DOC_DIAGRAM;
    else if (!strcmp(marker, "html-source")) kind = XUI_DOC_HTML;
    else if (!strcmp(marker, "front-matter")) kind = XUI_DOC_FRONT_MATTER;
    else if (!strcmp(marker, "extension")) kind = XUI_DOC_EXTENSION;
    else if (!strcmp(marker, "footnote-ref")) kind = XUI_DOC_FOOTNOTE_REF;
    if (!kind) { free(marker); return XUI_OK; }
    if ((kind == XUI_DOC_MATH && strcmp(tag->name, "span") && strcmp(tag->name, "div")) ||
        (kind == XUI_DOC_DIAGRAM && strcmp(tag->name, "pre")) ||
        (kind == XUI_DOC_HTML && strcmp(tag->name, "span") && strcmp(tag->name, "pre")) ||
        (kind == XUI_DOC_FRONT_MATTER && strcmp(tag->name, "pre")) ||
        (kind == XUI_DOC_EXTENSION && strcmp(tag->name, "span") && strcmp(tag->name, "pre")) ||
        (kind == XUI_DOC_FOOTNOTE_REF && strcmp(tag->name, "sup"))) {
        free(marker); return XUI_DOC_ERROR_FORMAT;
    }
    *handled = 1;
    if (kind == XUI_DOC_MATH || kind == XUI_DOC_HTML ||
        kind == XUI_DOC_EXTENSION) {
        if (!strcmp(tag->name, "pre") || !strcmp(tag->name, "div"))
            attrs.iFlags |= XUI_DOC_BLOCK;
        else attrs.iFlags &= ~XUI_DOC_BLOCK;
    }
    result = doc_html_attr_read(p, tag, "style", &style);
    if (result == XUI_OK && style) doc_html_style_apply(&attrs, style);
    if (result == XUI_OK)
        result = doc_html_attr_read(p, tag, "data-xui-resource", &resource);
    if (result == XUI_OK)
        result = doc_html_attr_read(p, tag, "data-xui-info", &info);
    if (result == XUI_OK)
        result = doc_html_attr_read(p, tag, "data-xui-title", &title);
    if (result == XUI_OK && !tag->empty)
        result = doc_html_object_content(p, tag->name, &body);
    if (result == XUI_OK && kind == XUI_DOC_FOOTNOTE_REF && !info && body.data) {
        info = malloc(body.size + 1);
        if (!info) result = XUI_ERROR_OUT_OF_MEMORY;
        else memcpy(info, body.data, body.size + 1);
    }
    if (result == XUI_OK && kind == XUI_DOC_EXTENSION)
        result = doc_html_attr_u32_strict(p, tag, "data-xui-version", 1, 1, UINT32_MAX, &version);
    if (result == XUI_OK && kind == XUI_DOC_EXTENSION)
        result = doc_html_attr_u32_strict(p, tag, "data-xui-required", 0, 0, 1, &required);
    if (result == XUI_OK && kind == XUI_DOC_EXTENSION)
        result = doc_html_attr_read(p, tag, "data-xui-payload-base64", &encoded);
    if (result == XUI_OK && encoded) {
        if (!xrtBase64Decode(encoded, strlen(encoded), NULL, 0, &payload_bytes, NULL))
            result = XUI_DOC_ERROR_FORMAT;
        else if (payload_bytes > DOC_EXTENSION_MAX_PAYLOAD_BYTES)
            result = XUI_DOC_ERROR_LIMIT;
        else if (payload_bytes && (!info || !*info))
            result = XUI_DOC_ERROR_SCHEMA;
        else if (payload_bytes) {
            payload = xrtBase64DecodeNew(encoded, strlen(encoded), &payload_bytes, NULL);
            if (!payload) result = XUI_ERROR_OUT_OF_MEMORY;
        }
    }
    if (result == XUI_OK)
        result = doc_html_insert(p, parent, kind, &attrs,
            body.data ? body.data : "", body.size,
            resource, info, title, &id);
    if (result == XUI_OK && kind == XUI_DOC_EXTENSION)
        result = xuiDocumentTxnSetExtensionPayload(p->transaction, id,
            payload, payload_bytes, version, required);
    free(marker); free(resource); free(info); free(title); free(style); free(encoded); free(body.data);
    xrtFree(payload);
    return result;
}
static int doc_html_parse_inline(doc_html_input*, uint64_t, const char*,
    xui_doc_attributes_t, const char*, const char*, unsigned);
static int doc_html_parse_inline_tag(doc_html_input* p, uint64_t parent,
    const doc_html_tag* tag, xui_doc_attributes_t attrs,
    const char* link, const char* link_title, unsigned depth)
{
    char *resource = NULL, *alt = NULL, *title = NULL, *style = NULL;
    int handled = 0, result = XUI_OK;
    if (tag->language[0]) attrs.sLanguage = tag->language;
    if (doc_html_active(tag->name))
        return tag->empty ? XUI_OK : doc_html_skip_active(p, tag->name);
    result = doc_html_parse_marked_object(p, parent, tag, attrs, &handled);
    if (result != XUI_OK || handled) return result;
    if (!strcmp(tag->name, "br"))
        return doc_html_insert(p, parent, XUI_DOC_HARD_BREAK, NULL, NULL, 0,
            NULL, NULL, NULL, NULL);
    if (!strcmp(tag->name, "input") || !strcmp(tag->name, "meta") ||
        !strcmp(tag->name, "link") || !strcmp(tag->name, "base")) return XUI_OK;
    if (!strcmp(tag->name, "img")) {
        result = doc_html_attr_read(p, tag, "src", &resource);
        if (result == XUI_OK) result = doc_html_attr_read(p, tag, "alt", &alt);
        if (result == XUI_OK) result = doc_html_attr_read(p, tag, "title", &title);
        if (result == XUI_OK) result = doc_html_attr_read(p, tag, "style", &style);
        if (result == XUI_OK && resource && doc_html_safe_url(resource, 1)) {
            xui_doc_attributes_t image_attrs = {0};
            image_attrs.sLanguage = attrs.sLanguage;
            xui_doc_node_desc_t image_desc = {0};
            uint64_t image_id = 0;
            char value[64];
            if (doc_html_css_value(style, "width", value, sizeof(value))) {
                doc_html_css_length_px(value, 1, 0, 0, &image_attrs.fWidth);
            }
            if (doc_html_css_value(style, "height", value, sizeof(value))) {
                doc_html_css_length_px(value, 1, 0, 0, &image_attrs.fHeight);
            }
            image_desc.iSize = sizeof(image_desc); image_desc.iKind = XUI_DOC_IMAGE;
            image_desc.tAttributes = image_attrs;
            if (link) image_desc.tAttributes.iMarks |= XUI_DOC_LINK;
            image_desc.sText = alt ? alt : "";
            image_desc.iTextBytes = alt ? strlen(alt) : 0;
            image_desc.sResource = resource; image_desc.sTitle = title;
            image_desc.sLinkTarget = link; image_desc.sLinkTitle = link_title;
            result = xuiDocumentTxnInsertNode(p->transaction, parent,
                UINT64_MAX, &image_desc, &image_id);
        } else if (result == XUI_OK && alt && *alt) {
            result = doc_html_insert(p, parent, XUI_DOC_TEXT, &attrs,
                alt, strlen(alt), link, NULL, link_title, NULL);
        }
        free(resource); free(alt); free(title); free(style);
        return result;
    }
    if (!strcmp(tag->name, "strong") || !strcmp(tag->name, "b")) attrs.iMarks |= XUI_DOC_BOLD;
    else if (!strcmp(tag->name, "em") || !strcmp(tag->name, "i")) attrs.iMarks |= XUI_DOC_ITALIC;
    else if (!strcmp(tag->name, "u")) attrs.iMarks |= XUI_DOC_UNDERLINE;
    else if (!strcmp(tag->name, "s") || !strcmp(tag->name, "strike") || !strcmp(tag->name, "del")) attrs.iMarks |= XUI_DOC_STRIKE;
    else if (!strcmp(tag->name, "code")) attrs.iMarks |= XUI_DOC_CODE;
    else if (!strcmp(tag->name, "sub")) { attrs.iMarks &= ~XUI_DOC_SUPERSCRIPT; attrs.iMarks |= XUI_DOC_SUBSCRIPT; }
    else if (!strcmp(tag->name, "sup")) { attrs.iMarks &= ~XUI_DOC_SUBSCRIPT; attrs.iMarks |= XUI_DOC_SUPERSCRIPT; }
    else if (!strcmp(tag->name, "mark")) attrs.iMarks |= XUI_DOC_HIGHLIGHT;
    if (!strcmp(tag->name, "a")) {
        result = doc_html_attr_read(p, tag, "href", &resource);
        if (result == XUI_OK) result = doc_html_attr_read(p, tag, "title", &title);
        if (result == XUI_OK && resource && doc_html_safe_url(resource, 0)) {
            attrs.iMarks |= XUI_DOC_LINK;
            link = resource; link_title = title;
        }
    }
    if (result == XUI_OK) {
        result = doc_html_attr_read(p, tag, "style", &style);
        if (result == XUI_OK && style) doc_html_style_apply(&attrs, style);
    }
    if (result == XUI_OK && !tag->empty)
        result = doc_html_parse_inline(p, parent, tag->name, attrs,
            link, link_title, depth + 1);
    free(resource); free(title); free(style);
    return result;
}
static int doc_html_parse_inline(doc_html_input* p, uint64_t parent,
    const char* closing, xui_doc_attributes_t attrs,
    const char* link, const char* link_title, unsigned depth)
{
    if (depth > DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
    while (p->at < p->size) {
        if (p->html[p->at] != '<') {
            size_t start = p->at, length; char* decoded = NULL;
            int result;
            while (p->at < p->size && p->html[p->at] != '<') p->at++;
            result = doc_html_decode(p->html + start, p->at - start, &decoded, &length);
            if (result == XUI_OK && length)
                result = doc_html_insert(p, parent, XUI_DOC_TEXT, &attrs,
                    decoded, length, link, NULL, link_title, NULL);
            free(decoded);
            if (result != XUI_OK) return result;
        } else {
            size_t before = p->at; doc_html_tag tag;
            int result = doc_html_tag_read(p, &tag);
            if (result != XUI_OK) return result;
            if (tag.name[0] == '!') continue;
            if (tag.closing) {
                if (closing && !strcmp(tag.name, closing)) return XUI_OK;
                p->at = before; return XUI_OK;
            }
            if (doc_html_block_tag(tag.name)) { p->at = before; return XUI_OK; }
            result = doc_html_parse_inline_tag(p, parent, &tag, attrs,
                link, link_title, depth);
            if (result != XUI_OK) return result;
        }
    }
    return XUI_OK;
}
static int doc_html_pre_content(doc_html_input* p, doc_html_buffer* text,
    char** language, char natural_language[256])
{
    while (p->at < p->size) {
        if (p->html[p->at] != '<') {
            size_t start = p->at, length; char* decoded = NULL;
            int result;
            while (p->at < p->size && p->html[p->at] != '<') p->at++;
            result = doc_html_decode(p->html + start, p->at - start, &decoded, &length);
            if (result == XUI_OK) result = doc_html_buffer_add(text, decoded, length);
            free(decoded);
            if (result != XUI_OK) return result;
        } else {
            doc_html_tag tag;
            int result = doc_html_tag_read(p, &tag);
            if (result != XUI_OK) return result;
            if (tag.closing && !strcmp(tag.name, "pre")) return XUI_OK;
            if (!tag.closing && !strcmp(tag.name, "code") && !*language)
                result = doc_html_attr_read(p, &tag, "data-language", language);
            if (!tag.closing && !strcmp(tag.name, "code") && tag.language[0])
                strcpy(natural_language, tag.language);
            if (result != XUI_OK) return result;
            if (!strcmp(tag.name, "br") && !tag.closing) {
                result = doc_html_buffer_add(text, "\n", 1);
                if (result != XUI_OK) return result;
            }
        }
    }
    return XUI_OK;
}
static int doc_html_parse_blocks(doc_html_input* p, uint64_t parent,
    uint32_t parent_kind, const char* closing,
    xui_doc_attributes_t inherited, unsigned depth)
{
    uint64_t implicit = 0;
    if (depth > DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
    while (p->at < p->size) {
        if (p->html[p->at] != '<') {
            size_t start = p->at, length, i; char* decoded = NULL;
            int result;
            while (p->at < p->size && p->html[p->at] != '<') p->at++;
            result = doc_html_decode(p->html + start, p->at - start, &decoded, &length);
            if (result != XUI_OK) return result;
            for (i = 0; i < length && doc_html_space((unsigned char)decoded[i]); i++) {}
            if (length && (i < length || implicit)) {
                if (!implicit) {
                    xui_doc_attributes_t attrs = inherited;
                    if (!doc_schema_child(parent_kind, XUI_DOC_PARAGRAPH)) {
                        free(decoded); return XUI_DOC_ERROR_SCHEMA;
                    }
                    attrs.iMarks = 0;
                    result = doc_html_insert(p, parent, XUI_DOC_PARAGRAPH,
                        &attrs, NULL, 0, NULL, NULL, NULL, &implicit);
                }
                if (result == XUI_OK) {
                    xui_doc_attributes_t inline_attrs = {0};
                    inline_attrs.iMarks = inherited.iMarks;
                    result = doc_html_insert(p, implicit, XUI_DOC_TEXT,
                        &inline_attrs, decoded, length, NULL, NULL, NULL, NULL);
                }
            }
            free(decoded);
            if (result != XUI_OK) return result;
            continue;
        }
        {
            size_t before = p->at;
            doc_html_tag tag;
            int result = doc_html_tag_read(p, &tag);
            if (result != XUI_OK) return result;
            if (tag.name[0] == '!') continue;
            if (tag.closing) {
                if (closing && !strcmp(tag.name, closing)) return XUI_OK;
                if (closing) { p->at = before; return XUI_OK; }
                continue;
            }
            if (doc_html_active(tag.name)) {
                result = tag.empty ? XUI_OK : doc_html_skip_active(p, tag.name);
                if (result != XUI_OK) return result;
                continue;
            }
            if (!strcmp(tag.name, "meta") || !strcmp(tag.name, "link") ||
                !strcmp(tag.name, "base") || !strcmp(tag.name, "input")) continue;
            if (parent == DOC_ROOT && !strcmp(tag.name, "div")) {
                char* marker = NULL;
                result = doc_html_attr_read(p, &tag, "data-xui-document", &marker);
                int document_wrapper = marker && !strcmp(marker, "true");
                free(marker);
                if (result != XUI_OK) return result;
                if (document_wrapper) {
                    doc_node* root = doc_index_get(p->transaction->draft->index, DOC_ROOT);
                    xui_doc_attributes_t attrs = *root->attrs;
                    attrs.sLanguage = tag.language[0] ? tag.language : NULL;
                    result = xuiDocumentTxnSetAttributes(p->transaction, DOC_ROOT, &attrs);
                    if (result == XUI_OK && !tag.empty)
                        result = doc_html_parse_blocks(p, parent, parent_kind,
                            tag.name, inherited, depth + 1);
                    if (result != XUI_OK) return result;
                    implicit = 0; continue;
                }
            }
            if (!strcmp(tag.name, "pre") || !strcmp(tag.name, "div")) {
                int handled = 0;
                result = doc_html_parse_marked_object(p, parent, &tag,
                    (xui_doc_attributes_t){0}, &handled);
                if (result != XUI_OK) return result;
                if (handled) {
                    implicit = 0;
                    if (parent == DOC_ROOT) p->explicit_block = 1;
                    continue;
                }
            }
            if (!strcmp(tag.name, "aside")) {
                char *marker = NULL, *resource = NULL, *info = NULL, *title = NULL;
                result = doc_html_attr_read(p, &tag, "data-xui-object", &marker);
                if (result != XUI_OK) return result;
                if (marker && !strcmp(marker, "footnote")) {
                    uint64_t footnote = 0;
                    xui_doc_attributes_t child_style;
                    implicit = 0;
                    if (!doc_schema_child(parent_kind, XUI_DOC_FOOTNOTE))
                        result = XUI_DOC_ERROR_SCHEMA;
                    if (result == XUI_OK)
                        result = doc_html_attr_read(p, &tag, "data-xui-resource", &resource);
                    if (result == XUI_OK)
                        result = doc_html_attr_read(p, &tag, "data-xui-info", &info);
                    if (result == XUI_OK)
                        result = doc_html_attr_read(p, &tag, "data-xui-title", &title);
                    if (result == XUI_OK)
                        result = doc_html_tag_inherited_style(p, &tag,
                            inherited, &child_style);
                    if (result == XUI_OK)
                        result = doc_html_insert(p, parent, XUI_DOC_FOOTNOTE,
                            NULL, NULL, 0, resource, info, title, &footnote);
                    if (result == XUI_OK && parent == DOC_ROOT) p->explicit_block = 1;
                    if (result == XUI_OK && !tag.empty)
                        result = doc_html_parse_blocks(p, footnote,
                            XUI_DOC_FOOTNOTE, tag.name, child_style, depth + 1);
                    free(marker); free(resource); free(info); free(title);
                    if (result != XUI_OK) return result;
                    continue;
                }
                free(marker);
            }
            if (!strcmp(tag.name, "html") || !strcmp(tag.name, "body") ||
                !strcmp(tag.name, "article") || !strcmp(tag.name, "section") ||
                !strcmp(tag.name, "main") || !strcmp(tag.name, "div") ||
                !strcmp(tag.name, "aside") ||
                !strcmp(tag.name, "thead") || !strcmp(tag.name, "tbody") ||
                !strcmp(tag.name, "tfoot")) {
                xui_doc_attributes_t child_style;
                implicit = 0;
                result = doc_html_tag_inherited_style(p, &tag,
                    inherited, &child_style);
                if (result == XUI_OK && !tag.empty)
                    result = doc_html_parse_blocks(p, parent,
                        parent_kind, tag.name, child_style, depth + 1);
            } else if (!strcmp(tag.name, "p") ||
                (tag.name[0] == 'h' && tag.name[1] >= '1' &&
                tag.name[1] <= '6' && !tag.name[2])) {
                xui_doc_attributes_t attrs = inherited, inline_attrs = {0}; uint64_t block = 0;
                if (tag.language[0]) attrs.sLanguage = tag.language;
                char* style = NULL;
                uint32_t kind = tag.name[0] == 'p' ? XUI_DOC_PARAGRAPH : XUI_DOC_HEADING;
                implicit = 0;
                if (kind == XUI_DOC_HEADING) attrs.iHeadingLevel = (uint32_t)(tag.name[1] - '0');
                if (!doc_schema_child(parent_kind, kind)) return XUI_DOC_ERROR_SCHEMA;
                result = doc_html_attr_read(p, &tag, "style", &style);
                if (result == XUI_OK && style)
                    doc_html_block_style_apply(&attrs, style);
                inline_attrs.iMarks = attrs.iMarks;
                attrs.iMarks = 0;
                free(style);
                if (result != XUI_OK) return result;
                result = doc_html_insert(p, parent, kind, &attrs,
                    NULL, 0, NULL, NULL, NULL, &block);
                if (result == XUI_OK && parent == DOC_ROOT) p->explicit_block = 1;
                if (result == XUI_OK && !tag.empty) {
                    result = doc_html_parse_inline(p, block, tag.name,
                        inline_attrs, NULL, NULL, depth + 1);
                }
            } else if (!strcmp(tag.name, "blockquote") ||
                !strcmp(tag.name, "ul") || !strcmp(tag.name, "ol") ||
                !strcmp(tag.name, "table")) {
                xui_doc_attributes_t attrs = {0}; uint64_t block = 0;
                if (tag.language[0]) attrs.sLanguage = tag.language;
                xui_doc_attributes_t child_style;
                doc_html_table_context table = {0};
                char* info = NULL;
                uint32_t kind = !strcmp(tag.name, "blockquote") ? XUI_DOC_QUOTE :
                    !strcmp(tag.name, "table") ? XUI_DOC_TABLE : XUI_DOC_LIST;
                implicit = 0;
                if (kind == XUI_DOC_LIST && !strcmp(tag.name, "ol")) {
                    attrs.iFlags |= XUI_DOC_ORDERED;
                    {
                        uint32_t start = 1;
                        result = doc_html_attr_number(p, &tag, "start", 1,
                            UINT32_MAX, &start);
                        attrs.iListStart = start;
                    }
                }
                if (result != XUI_OK) return result;
                if (!doc_schema_child(parent_kind, kind)) return XUI_DOC_ERROR_SCHEMA;
                result = doc_html_tag_inherited_style(p, &tag,
                    inherited, &child_style);
                if (result != XUI_OK) return result;
                if (kind == XUI_DOC_QUOTE) {
                    result = doc_html_attr_read(p, &tag, "data-xui-info", &info);
                    if (result != XUI_OK) return result;
                }
                result = doc_html_insert(p, parent, kind, &attrs,
                    NULL, 0, NULL, info, NULL, &block);
                free(info);
                if (result == XUI_OK && parent == DOC_ROOT) p->explicit_block = 1;
                if (result == XUI_OK && kind == XUI_DOC_TABLE) {
                    table.id = block; table.previous = p->table; p->table = &table;
                }
                if (result == XUI_OK && !tag.empty)
                    result = doc_html_parse_blocks(p, block, kind,
                        tag.name, child_style, depth + 1);
                if (kind == XUI_DOC_TABLE && p->table == &table) {
                    uint32_t column, columns = 0;
                    p->table = table.previous;
                    if (result == XUI_OK)
                        columns = doc_table_column_count(p->transaction->draft,
                            doc_index_get(p->transaction->draft->index, block));
                    for (column = 0; result == XUI_OK && column < table.count &&
                        column < columns; column++)
                        if (table.widths[column] > 0)
                            result = xuiDocumentTxnSetTableColumnWidth(
                                p->transaction, block, column, table.widths[column]);
                    free(table.widths);
                }
            } else if (!strcmp(tag.name, "li") && parent_kind == XUI_DOC_LIST) {
                xui_doc_attributes_t attrs = {0};
                xui_doc_attributes_t child_style;
                if (tag.language[0]) attrs.sLanguage = tag.language;
                char *task = NULL, *checked = NULL;
                uint64_t item = 0;
                result = doc_html_attr_read(p, &tag, "data-xui-task", &task);
                if (result == XUI_OK)
                    result = doc_html_attr_read(p, &tag, "data-xui-checked", &checked);
                if (result == XUI_OK && task && (!strcmp(task, "true") || !strcmp(task, "1"))) {
                    attrs.iFlags |= XUI_DOC_TASK;
                    if (checked && (!strcmp(checked, "true") || !strcmp(checked, "1")))
                        attrs.iFlags |= XUI_DOC_CHECKED;
                }
                free(task); free(checked);
                if (result == XUI_OK)
                    result = doc_html_tag_inherited_style(p, &tag,
                        inherited, &child_style);
                if (result == XUI_OK)
                    result = doc_html_insert(p, parent, XUI_DOC_LIST_ITEM,
                        &attrs, NULL, 0, NULL, NULL, NULL, &item);
                if (result == XUI_OK && !tag.empty)
                    result = doc_html_parse_blocks(p, item, XUI_DOC_LIST_ITEM,
                        "li", child_style, depth + 1);
            } else if (!strcmp(tag.name, "tr") && parent_kind == XUI_DOC_TABLE) {
                uint64_t row = 0;
                xui_doc_attributes_t attrs = {0};
                if (tag.language[0]) attrs.sLanguage = tag.language;
                xui_doc_attributes_t child_style;
                result = doc_html_tag_inherited_style(p, &tag,
                    inherited, &child_style);
                if (result == XUI_OK)
                    result = doc_html_insert(p, parent, XUI_DOC_ROW,
                        &attrs, NULL, 0, NULL, NULL, NULL, &row);
                if (result == XUI_OK && !tag.empty)
                    result = doc_html_parse_blocks(p, row, XUI_DOC_ROW,
                        "tr", child_style, depth + 1);
            } else if ((!strcmp(tag.name, "td") || !strcmp(tag.name, "th")) &&
                parent_kind == XUI_DOC_ROW) {
                xui_doc_attributes_t attrs = {0}; uint64_t cell = 0;
                xui_doc_attributes_t child_style;
                result = doc_html_attr_number(p, &tag, "rowspan", 1, 1024,
                    &attrs.iRowSpan);
                if (result == XUI_OK)
                    result = doc_html_attr_number(p, &tag, "colspan", 1,
                        1024, &attrs.iColumnSpan);
                if (result != XUI_OK) return result;
                if (!strcmp(tag.name, "th")) attrs.iFlags |= XUI_DOC_HEADER;
                result = doc_html_tag_cell_style(p, &tag,
                    inherited, &attrs, &child_style);
                if (result == XUI_OK)
                    result = doc_html_insert(p, parent, XUI_DOC_CELL, &attrs,
                        NULL, 0, NULL, NULL, NULL, &cell);
                if (result == XUI_OK && !tag.empty)
                    result = doc_html_parse_blocks(p, cell, XUI_DOC_CELL,
                        tag.name, child_style, depth + 1);
            } else if (!strcmp(tag.name, "pre")) {
                doc_html_buffer body = {0}; char* language = NULL;
                char natural_language[256] = {0};
                xui_doc_attributes_t attrs = {0};
                attrs.sLanguage = tag.language[0] ? tag.language : inherited.sLanguage;
                implicit = 0;
                if (!doc_schema_child(parent_kind, XUI_DOC_CODE_BLOCK)) return XUI_DOC_ERROR_SCHEMA;
                if (!tag.empty) result = doc_html_pre_content(p, &body, &language, natural_language);
                if (natural_language[0]) attrs.sLanguage = natural_language;
                if (result == XUI_OK) result = doc_html_insert(p, parent,
                    XUI_DOC_CODE_BLOCK, &attrs, body.data ? body.data : "",
                    body.size, NULL, language, NULL, NULL);
                if (result == XUI_OK && parent == DOC_ROOT) p->explicit_block = 1;
                free(body.data); free(language);
            } else if (!strcmp(tag.name, "hr")) {
                implicit = 0;
                if (!doc_schema_child(parent_kind, XUI_DOC_RULE)) return XUI_DOC_ERROR_SCHEMA;
                result = doc_html_insert(p, parent, XUI_DOC_RULE,
                    NULL, NULL, 0, NULL, NULL, NULL, NULL);
                if (result == XUI_OK && parent == DOC_ROOT) p->explicit_block = 1;
            } else if (!strcmp(tag.name, "colgroup") || !strcmp(tag.name, "col")) {
                xui_doc_attributes_t child_style;
                if (!strcmp(tag.name, "col") && parent_kind == XUI_DOC_TABLE)
                    result = doc_html_column(p, &tag);
                if (result == XUI_OK)
                    result = doc_html_tag_inherited_style(p, &tag,
                        inherited, &child_style);
                if (result == XUI_OK && !tag.empty && !strcmp(tag.name, "colgroup"))
                    result = doc_html_parse_blocks(p, parent, parent_kind,
                        "colgroup", child_style, depth + 1);
            } else if (doc_html_block_tag(tag.name) || parent_kind == XUI_DOC_LIST ||
                parent_kind == XUI_DOC_TABLE || parent_kind == XUI_DOC_ROW) {
                return XUI_DOC_ERROR_SCHEMA;
            } else {
                if (!implicit) {
                    xui_doc_attributes_t attrs = inherited;
                    if (!doc_schema_child(parent_kind, XUI_DOC_PARAGRAPH)) return XUI_DOC_ERROR_SCHEMA;
                    attrs.iMarks = 0;
                    result = doc_html_insert(p, parent, XUI_DOC_PARAGRAPH,
                        &attrs, NULL, 0, NULL, NULL, NULL, &implicit);
                }
                if (result == XUI_OK) {
                    xui_doc_attributes_t inline_attrs = {0};
                    inline_attrs.iMarks = inherited.iMarks;
                    result = doc_html_parse_inline_tag(p, implicit, &tag,
                        inline_attrs, NULL, NULL, depth + 1);
                }
            }
            if (result != XUI_OK) return result;
        }
    }
    return XUI_OK;
}

int doc_html_import_document(const char* html, uint64_t bytes,
    xui_document* out, unsigned* depth)
{
    xui_doc_desc_t desc = {0};
    xui_document document = NULL;
    xui_document_transaction transaction = NULL;
    doc_html_input input = {0};
    doc_node* root;
    int result;
    *out = NULL; *depth = 0;
    if (bytes > SIZE_MAX || bytes > DOC_HTML_IMPORT_MAX_BYTES)
        return XUI_DOC_ERROR_LIMIT;
    if (memchr(html, 0, (size_t)bytes) || !doc_utf8(html, bytes))
        return XUI_DOC_ERROR_UTF8;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    desc.bDisableHistory = 1;
    result = xuiDocumentCreate(&desc, &document);
    if (result == XUI_OK) result = xuiDocumentBeginTransaction(document,
        NULL, &transaction);
    if (result == XUI_OK) {
        input.html = html; input.size = (size_t)bytes;
        input.transaction = transaction;
        result = doc_html_parse_blocks(&input, DOC_ROOT, XUI_DOC_ROOT,
            NULL, (xui_doc_attributes_t){0}, 0);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
    xuiDocumentTxnRelease(transaction);
    if (result == XUI_OK) {
        root = doc_index_get(document->state->index, DOC_ROOT);
        if (!root || !doc_seq_size(root->children)) result = XUI_ERROR_UNSUPPORTED;
        else if (!input.explicit_block && doc_seq_size(root->children) == 1 &&
            doc_index_get(document->state->index,
                doc_seq_get_id(root->children, 0))->kind == XUI_DOC_PARAGRAPH)
            *depth = 1;
    }
    if (result != XUI_OK) { xuiDocumentRelease(document); return result; }
    *out = document; return XUI_OK;
}

#endif
