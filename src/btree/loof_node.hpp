#ifndef BTREE_LOOF_NODE_HPP_
#define BTREE_LOOF_NODE_HPP_

// Eventually we'll rename loof to leaf, but right now I want the old code to look at.

namespace loof {

// These codes can appear as the first byte of a leaf node entry
// (values 250 or smaller are just key sizes for key/value pairs).

// Means we have a deletion entry.
const int DELETE_ENTRY_CODE = 255;

// Means we have a skipped entry exactly one byte long.
const int SKIP_ENTRY_CODE_ONE = 254;

// Means we have a skipped entry exactly two bytes long.
const int SKIP_ENTRY_CODE_TWO = 253;

// Means we have a skipped entry exactly N bytes long, of form { uint8_t 252; uint16_t N; char garbage[]; }
const int SKIP_ENTRY_CODE_MANY = 252;

// A reserved meaningless value.
const int SKIP_ENTRY_RESERVED = 251;

// The amount of extra timestamp information we _must_ carry if there
// are sufficiently many keys in the node.
const int MANDATORY_EXTRAS = 4;

struct entry_t;
struct value_t;

inline
bool entry_is_deletion(const entry_t *p) {
    uint8_t x = *reinterpret_cast<const uint8_t *>(p);
    rassert(x != 251);
    return x == DELETE_PAIR_CODE;
}

inline
bool entry_is_live(const entry_t *p) {
    uint8_t x = *reinterpret_cast<const uint8_t *>(p);
    rassert(x != 251);
    rassert(MAX_KEY_SIZE == 250);
    return x <= MAX_KEY_SIZE;
}

inline
bool entry_is_skip(const entry_t *p) {
    return !entry_is_deletion(p) && !entry_is_live(p);
}

template <class V>
bool entry_fits(value_sizer_t<V> *sizer, const entry_t *p, int size) {
    if (size < 1) {
        return false;
    }

    uint8_t code = *reinterpret_cast<const uint8_t *>(p);
    switch (code) {
    case DELETE_ENTRY_CODE:
        return entry_key(p)->fits(size - 1);
    case SKIP_ENTRY_CODE_ONE:
        return true;
    case SKIP_ENTRY_CODE_TWO:
        return size >= 2;
    case SKIP_ENTRY_CODE_MANY:
        return size >= 3 && size >= 3 + *reinterpret_cast<const uint16_t *>(1 + reinterpret_cast<const char *>(p));
    default:
        rassert(code <= MAX_KEY_SIZE);
        return entry_key(p)->fits(size) && sizer->fits(entry_value<V>(p), size - entry_key(p)->full_size());
    }
}

inline
const btree_key_t *entry_key(const entry_t *p) {
    if (entry_is_deletion(p)) {
        return reinterpret_cast<const btree_key_t *>(1 + reinterpret_cast<const char *>(p));
    } else {
        return reinterpret_cast<const btree_key_t *>(p);
    }
}

template <class V>
const V *entry_value(const entry_t *p) {
    if (entry_is_deletion(p)) {
        return NULL;
    } else {
        return reinterpret_cast<const V *>(reinterpret_cast<const char *>(p) + entry_key(p)->full_size());
    }
}

template <class V>
int entry_size(value_sizer_t<V> *sizer, const entry_t *p) {
    uint8_t code = *reinterpret_cast<const uint8_t *>(p);
    switch (code) {
    case DELETE_ENTRY_CODE:
        return 1 + entry_key(p)->full_size();
    case SKIP_ENTRY_CODE_ONE:
        return 1;
    case SKIP_ENTRY_CODE_TWO:
        return 2;
    case SKIP_ENTRY_CODE_MANY:
        return 3 + *reinterpret_cast<const uint16_t *>(1 + reinterpret_cast<const char *>(p));
    default:
        rassert(code <= MAX_KEY_SIZE);
        return entry_key(p)->full_size() + sizer->size(entry_value<V>(p));
    }
}


struct loof_t {
    // The value-type-specific magic value.
    block_magic_t magic;

    // The size of pair_offsets.
    uint16_t npairs;

    // The number of 
    int16_t nextra;
    repli_timestamp_t base_tstamp;
    uint16_t frontmost;
    uint16_t pair_offsets[];

    uint16_t *extra_tstamps() const {
        return pair_offsets + npairs;
    }
};

const entry_t *get_entry(const loof_t *node, int offset) {
    return reinterpret_cast<const entry_t *>(reinterpret_cast<const char *>(node) + offset);
}

entry_t *get_entry(loof_t *node, int offset) {
    return reinterpret_cast<entry_t *>(reinterpret_cast<char *>(node) + offset);
}










}  // namespace loof


#endif  BTREE_LOOF_NODE_HPP_
