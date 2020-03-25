#define VAR_REF_READ_FRAME_OFFSET(_v, _index) var_ref_read_unaligned_le ((guchar*)((_v).base) + (_v).size - (offset_size * ((_index) + 1)), offset_size)
#define VAR_REF_ALIGN(_offset, _align_to) ((_offset + _align_to - 1) & ~(gsize)(_align_to - 1))

/* Note: clz is undefinded for 0, so never call this size == 0 */
G_GNUC_CONST static inline guint
var_ref_get_offset_size (gsize size)
{
#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(__OPTIMIZE__)
  /* Instead of using a lookup table we use nibbles in a lookup word */
  guint32 v = (guint32)0x88884421;
  return (v >> (((__builtin_clzl(size) ^ 63) / 8) * 4)) & 0xf;
#else
  if (size > G_MAXUINT16)
    {
      if (size > G_MAXUINT32)
        return 8;
      else
        return 4;
    }
  else
    {
      if (size > G_MAXUINT8)
         return 2;
      else
         return 1;
    }
#endif
}

G_GNUC_PURE static inline gsize
var_ref_read_unaligned_le (guchar *bytes, guint   size)
{
  union
  {
    guchar bytes[GLIB_SIZEOF_SIZE_T];
    gsize integer;
  } tmpvalue;

  tmpvalue.integer = 0;
  /* we unroll the size checks here so that memcpy gets constant args */
  if (size >= 4)
    {
      if (size == 8)
        memcpy (&tmpvalue.bytes, bytes, 8);
      else
        memcpy (&tmpvalue.bytes, bytes, 4);
    }
  else
    {
      if (size == 2)
        memcpy (&tmpvalue.bytes, bytes, 2);
      else
        memcpy (&tmpvalue.bytes, bytes, 1);
    }

  return GSIZE_FROM_LE (tmpvalue.integer);
}

static inline void
__var_gstring_append_double (GString *string, double d)
{
  gchar buffer[100];
  gint i;

  g_ascii_dtostr (buffer, sizeof buffer, d);
  for (i = 0; buffer[i]; i++)
    if (buffer[i] == '.' || buffer[i] == 'e' ||
        buffer[i] == 'n' || buffer[i] == 'N')
      break;

  /* if there is no '.' or 'e' in the float then add one */
  if (buffer[i] == '\0')
    {
      buffer[i++] = '.';
      buffer[i++] = '0';
      buffer[i++] = '\0';
    }
   g_string_append (string, buffer);
}

static inline void
__var_gstring_append_string (GString *string, const char *str)
{
  gunichar quote = strchr (str, '\'') ? '"' : '\'';

  g_string_append_c (string, quote);
  while (*str)
    {
      gunichar c = g_utf8_get_char (str);

      if (c == quote || c == '\\')
        g_string_append_c (string, '\\');

      if (g_unichar_isprint (c))
        g_string_append_unichar (string, c);
      else
        {
          g_string_append_c (string, '\\');
          if (c < 0x10000)
            switch (c)
              {
              case '\a':
                g_string_append_c (string, 'a');
                break;

              case '\b':
                g_string_append_c (string, 'b');
                break;

              case '\f':
                g_string_append_c (string, 'f');
                break;

              case '\n':
                g_string_append_c (string, 'n');
                break;

              case '\r':
                g_string_append_c (string, 'r');
                break;

              case '\t':
                g_string_append_c (string, 't');
                break;

              case '\v':
                g_string_append_c (string, 'v');
                break;

              default:
                g_string_append_printf (string, "u%04x", c);
                break;
              }
           else
             g_string_append_printf (string, "U%08x", c);
        }

      str = g_utf8_next_char (str);
    }

  g_string_append_c (string, quote);
}

/************** VarVariantRef *******************/

static inline VarRef
var_variant_get_child (VarVariantRef v, const GVariantType **out_type)
{
  if (v.size)
    {
      guchar *base = (guchar *)v.base;
      gsize size = v.size - 1;

      /* find '\0' character */
      while (size > 0 && base[size] != 0)
        size--;

      /* ensure we didn't just hit the start of the string */
      if (base[size] == 0)
       {
          const char *type_string = (char *) base + size + 1;
          const char *limit = (char *)base + v.size;
          const char *end;

          if (g_variant_type_string_scan (type_string, limit, &end) && end == limit)
            {
              if (out_type)
                *out_type = (const GVariantType *)type_string;
              return (VarRef) { v.base, size };
            }
       }
    }
  if (out_type)
    *out_type = G_VARIANT_TYPE_UNIT;
  return  (VarRef) { "\0", 1 };
}

static inline const GVariantType *
var_variant_get_type (VarVariantRef v)
{
  if (v.size)
    {
      guchar *base = (guchar *)v.base;
      gsize size = v.size - 1;

      /* find '\0' character */
      while (size > 0 && base[size] != 0)
        size--;

      /* ensure we didn't just hit the start of the string */
      if (base[size] == 0)
       {
          const char *type_string = (char *) base + size + 1;
          const char *limit = (char *)base + v.size;
          const char *end;

          if (g_variant_type_string_scan (type_string, limit, &end) && end == limit)
             return (const GVariantType *)type_string;
       }
    }
  return  G_VARIANT_TYPE_UNIT;
}

static inline gboolean
var_variant_is_type (VarVariantRef v, const GVariantType *type)
{
   return g_variant_type_equal (var_variant_get_type (v), type);
}

static inline VarVariantRef
var_variant_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), G_VARIANT_TYPE_VARIANT));
  return (VarVariantRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarVariantRef
var_variant_from_bytes (GBytes *b)
{
  return (VarVariantRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarVariantRef
var_variant_from_data (gconstpointer data, gsize size)
{
  return (VarVariantRef) { data, size };
}

static inline GVariant *
var_variant_dup_to_gvariant (VarVariantRef v)
{
  return g_variant_new_from_data (G_VARIANT_TYPE_VARIANT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_variant_to_gvariant (VarVariantRef v,
                              GDestroyNotify      notify,
                              gpointer            user_data)
{
  return g_variant_new_from_data (G_VARIANT_TYPE_VARIANT, g_memdup (v.base, v.size), v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_variant_to_owned_gvariant (VarVariantRef v,
                                     GVariant *base)
{
  return var_variant_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_variant_peek_as_variant (VarVariantRef v)
{
  return g_variant_new_from_data (G_VARIANT_TYPE_VARIANT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarVariantRef
var_variant_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, G_VARIANT_TYPE_VARIANT));
  return var_variant_from_data (child.base, child.size);
}

static inline GVariant *
var_variant_dup_child_to_gvariant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  return g_variant_new_from_data (type, g_memdup (child.base, child.size), child.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_variant_peek_child_as_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  return g_variant_new_from_data (type, child.base, child.size, TRUE, NULL, NULL);
}

static inline GString *
var_variant_format (VarVariantRef v, GString *s, gboolean type_annotate)
{
#ifdef VAR_DEEP_VARIANT_FORMAT
  GVariant *gv = var_variant_peek_as_variant (v);
  return g_variant_print_string (gv, s, TRUE);
#else
  const GVariantType  *type = var_variant_get_type (v);
  g_string_append_printf (s, "<@%.*s>", (int)g_variant_type_get_string_length (type), (const char *)type);
  return s;
#endif
}

static inline char *
var_variant_print (VarVariantRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_variant_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}
static inline gboolean
var_variant_get_boolean (VarVariantRef v)
{
  return (gboolean)*((guint8 *)v.base);
}
static inline guint8
var_variant_get_byte (VarVariantRef v)
{
  return (guint8)*((guint8 *)v.base);
}
static inline gint16
var_variant_get_int16 (VarVariantRef v)
{
  return (gint16)*((gint16 *)v.base);
}
static inline guint16
var_variant_get_uint16 (VarVariantRef v)
{
  return (guint16)*((guint16 *)v.base);
}
static inline gint32
var_variant_get_int32 (VarVariantRef v)
{
  return (gint32)*((gint32 *)v.base);
}
static inline guint32
var_variant_get_uint32 (VarVariantRef v)
{
  return (guint32)*((guint32 *)v.base);
}
static inline gint64
var_variant_get_int64 (VarVariantRef v)
{
  return (gint64)*((gint64 *)v.base);
}
static inline guint64
var_variant_get_uint64 (VarVariantRef v)
{
  return (guint64)*((guint64 *)v.base);
}
static inline guint32
var_variant_get_handle (VarVariantRef v)
{
  return (guint32)*((guint32 *)v.base);
}
static inline double
var_variant_get_double (VarVariantRef v)
{
  return (double)*((double *)v.base);
}
static inline const char *
var_variant_get_string (VarVariantRef v)
{
  return (const char *)v.base;
}
static inline const char *
var_variant_get_objectpath (VarVariantRef v)
{
  return (const char *)v.base;
}
static inline const char *
var_variant_get_signature (VarVariantRef v)
{
  return (const char *)v.base;
}

/************** VarMetadata *******************/

static inline VarMetadataRef
var_metadata_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_METADATA_TYPESTRING));
  return (VarMetadataRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarMetadataRef
var_metadata_from_bytes (GBytes *b)
{
  return (VarMetadataRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarMetadataRef
var_metadata_from_data (gconstpointer data, gsize size)
{
  return (VarMetadataRef) { data, size };
}

static inline GVariant *
var_metadata_dup_to_gvariant (VarMetadataRef v)
{
  return g_variant_new_from_data (VAR_METADATA_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_metadata_to_gvariant (VarMetadataRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_METADATA_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_metadata_to_owned_gvariant (VarMetadataRef v, GVariant *base)
{
  return var_metadata_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_metadata_peek_as_gvariant (VarMetadataRef v)
{
  return g_variant_new_from_data (VAR_METADATA_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarMetadataRef
var_metadata_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_METADATA_TYPESTRING));
  return var_metadata_from_data (child.base, child.size);
}


static inline gsize
var_metadata_get_length (VarMetadataRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length = offsets_array_size / offset_size;
  return length;
}

static inline VarMetadataEntryRef
var_metadata_get_at (VarMetadataRef v, gsize index)
{
  VarMetadataEntryRef res;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 8) : 0;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  res = (VarMetadataEntryRef) { ((const char *)v.base) + start, end - start };
  return res;
}

static inline const char *
var_metadata_entry_get_key (VarMetadataEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  const char *base = (const char *)v.base;
  g_assert (end < v.size && base[end-1] == 0);
  return base;
}

static inline VarVariantRef
var_metadata_entry_get_value (VarMetadataEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offset = VAR_REF_ALIGN(end, 8);
  g_assert (offset <= v.size);
  return (VarVariantRef) { (char *)v.base + offset, (v.size - offset_size) - offset };
}

static inline gboolean
var_metadata_lookup (VarMetadataRef v, const char * key, gsize *index_out, VarVariantRef *out)
{
  const char * canonical_key = key;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  if (last_end > v.size)
    return FALSE;
  gsize offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return FALSE;
  gsize len = offsets_array_size / offset_size;
  gsize start = 0;
  gsize i;

  for (i = 0; i < len; i++)
    {
      gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - i - 1);
      VarMetadataEntryRef e = { ((const guchar *)v.base) + start, end - start };
      g_assert (start <= end && end <= last_end);
      const char * e_key = var_metadata_entry_get_key (e);
      if (strcmp(canonical_key, e_key) == 0)
        {
           if (index_out)
             *index_out = i;
           if (out)
             *out = var_metadata_entry_get_value (e);
           return TRUE;
        }
      start = VAR_REF_ALIGN(end, 8);
    }
    return FALSE;
}

static inline gboolean
var_metadata_lookup_boolean (VarMetadataRef v, const char * key, gboolean default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'b')
    return var_variant_get_boolean (value_v);
  return default_value;
}

static inline guint8
var_metadata_lookup_byte (VarMetadataRef v, const char * key, guint8 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'y')
    return var_variant_get_byte (value_v);
  return default_value;
}

static inline gint16
var_metadata_lookup_int16 (VarMetadataRef v, const char * key, gint16 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'n')
    return var_variant_get_int16 (value_v);
  return default_value;
}

static inline guint16
var_metadata_lookup_uint16 (VarMetadataRef v, const char * key, guint16 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'q')
    return var_variant_get_uint16 (value_v);
  return default_value;
}

static inline gint32
var_metadata_lookup_int32 (VarMetadataRef v, const char * key, gint32 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'i')
    return var_variant_get_int32 (value_v);
  return default_value;
}

static inline guint32
var_metadata_lookup_uint32 (VarMetadataRef v, const char * key, guint32 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'u')
    return var_variant_get_uint32 (value_v);
  return default_value;
}

static inline gint64
var_metadata_lookup_int64 (VarMetadataRef v, const char * key, gint64 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'x')
    return var_variant_get_int64 (value_v);
  return default_value;
}

static inline guint64
var_metadata_lookup_uint64 (VarMetadataRef v, const char * key, guint64 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 't')
    return var_variant_get_uint64 (value_v);
  return default_value;
}

static inline guint32
var_metadata_lookup_handle (VarMetadataRef v, const char * key, guint32 default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'h')
    return var_variant_get_handle (value_v);
  return default_value;
}

static inline double
var_metadata_lookup_double (VarMetadataRef v, const char * key, double default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'd')
    return var_variant_get_double (value_v);
  return default_value;
}

static inline const char *
var_metadata_lookup_string (VarMetadataRef v, const char * key, const char * default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 's')
    return var_variant_get_string (value_v);
  return default_value;
}

static inline const char *
var_metadata_lookup_objectpath (VarMetadataRef v, const char * key, const char * default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'o')
    return var_variant_get_objectpath (value_v);
  return default_value;
}

static inline const char *
var_metadata_lookup_signature (VarMetadataRef v, const char * key, const char * default_value)
{
   VarVariantRef value_v;

  if (var_metadata_lookup (v, key, NULL, &value_v) &&
      *(const char *)var_variant_get_type (value_v) == 'g')
    return var_variant_get_signature (value_v);
  return default_value;
}

static inline GString *
var_metadata_format (VarMetadataRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_metadata_get_length (v);
  gsize i;

  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_METADATA_TYPESTRING);

  g_string_append_c (s, '{');
  for (i = 0; i < len; i++)
    {
      VarMetadataEntryRef entry = var_metadata_get_at (v, i);
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_metadata_entry_get_key (entry));
      g_string_append (s, ": ");
      var_variant_format (var_metadata_entry_get_value (entry), s, type_annotate);
    }
  g_string_append_c (s, '}');
  return s;
}

static inline char *
var_metadata_print (VarMetadataRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_metadata_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarChecksum *******************/

static inline VarChecksumRef
var_checksum_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_CHECKSUM_TYPESTRING));
  return (VarChecksumRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarChecksumRef
var_checksum_from_bytes (GBytes *b)
{
  return (VarChecksumRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarChecksumRef
var_checksum_from_data (gconstpointer data, gsize size)
{
  return (VarChecksumRef) { data, size };
}

static inline GVariant *
var_checksum_dup_to_gvariant (VarChecksumRef v)
{
  return g_variant_new_from_data (VAR_CHECKSUM_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_checksum_to_gvariant (VarChecksumRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_CHECKSUM_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_checksum_to_owned_gvariant (VarChecksumRef v, GVariant *base)
{
  return var_checksum_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_checksum_peek_as_gvariant (VarChecksumRef v)
{
  return g_variant_new_from_data (VAR_CHECKSUM_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarChecksumRef
var_checksum_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_CHECKSUM_TYPESTRING));
  return var_checksum_from_data (child.base, child.size);
}

static inline gsize
var_checksum_get_length (VarChecksumRef v)
{
  gsize length = v.size / 1;
  return length;
}

static inline guint8
var_checksum_get_at (VarChecksumRef v, gsize index)
{
  return (guint8)G_STRUCT_MEMBER(guint8, v.base, index * 1);
}

static inline const guint8 *
var_checksum_peek (VarChecksumRef v)
{
  return (const guint8 *)v.base;
}

static inline GString *
var_checksum_format (VarChecksumRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_checksum_get_length (v);
  gsize i;
  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_CHECKSUM_TYPESTRING);
  g_string_append_c (s, '[');
  for (i = 0; i < len; i++)
    {
      if (i != 0)
        g_string_append (s, ", ");
      g_string_append_printf (s, "%s0x%02x", ((i == 0) ? type_annotate : FALSE) ? "byte " : "", var_checksum_get_at (v, i));
    }
  g_string_append_c (s, ']');
  return s;
}

static inline char *
var_checksum_print (VarChecksumRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_checksum_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarRefInfo *******************/

static inline VarRefInfoRef
var_ref_info_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_REF_INFO_TYPESTRING));
  return (VarRefInfoRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarRefInfoRef
var_ref_info_from_bytes (GBytes *b)
{
  return (VarRefInfoRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarRefInfoRef
var_ref_info_from_data (gconstpointer data, gsize size)
{
  return (VarRefInfoRef) { data, size };
}

static inline GVariant *
var_ref_info_dup_to_gvariant (VarRefInfoRef v)
{
  return g_variant_new_from_data (VAR_REF_INFO_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_ref_info_to_gvariant (VarRefInfoRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_REF_INFO_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_ref_info_to_owned_gvariant (VarRefInfoRef v, GVariant *base)
{
  return var_ref_info_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_ref_info_peek_as_gvariant (VarRefInfoRef v)
{
  return g_variant_new_from_data (VAR_REF_INFO_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarRefInfoRef
var_ref_info_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_REF_INFO_TYPESTRING));
  return var_ref_info_from_data (child.base, child.size);
}

static inline guint64
var_ref_info_get_commit_size (VarRefInfoRef v)
{
  guint offset = ((7) & (~(gsize)7)) + 0;
  g_assert (offset + 8 < v.size);
  return (guint64)G_STRUCT_MEMBER(guint64, v.base, offset);
}

static inline VarChecksumRef
var_ref_info_get_checksum (VarRefInfoRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((7) & (~(gsize)7)) + 8;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size);
  return (VarChecksumRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const guint8 *
var_ref_info_peek_checksum (VarRefInfoRef v, gsize *len) {
  VarChecksumRef a = var_ref_info_get_checksum (v);
  if (len != NULL)
    *len = var_checksum_get_length (a);
  return (const guint8 *)a.base;
}

static inline VarMetadataRef
var_ref_info_get_metadata (VarRefInfoRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 1;
  g_assert (start <= end && end <= v.size);
  return (VarMetadataRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline GString *
var_ref_info_format (VarRefInfoRef v, GString *s, gboolean type_annotate)
{
  g_string_append_printf (s, "(%s%"G_GUINT64_FORMAT", ",
                   type_annotate ? "uint64 " : "",
                   var_ref_info_get_commit_size (v));
  var_checksum_format (var_ref_info_get_checksum (v), s, type_annotate);
  g_string_append (s, ", ");
  var_metadata_format (var_ref_info_get_metadata (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_ref_info_print (VarRefInfoRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_ref_info_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarRefMapEntry *******************/

static inline VarRefMapEntryRef
var_ref_map_entry_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_REF_MAP_ENTRY_TYPESTRING));
  return (VarRefMapEntryRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarRefMapEntryRef
var_ref_map_entry_from_bytes (GBytes *b)
{
  return (VarRefMapEntryRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarRefMapEntryRef
var_ref_map_entry_from_data (gconstpointer data, gsize size)
{
  return (VarRefMapEntryRef) { data, size };
}

static inline GVariant *
var_ref_map_entry_dup_to_gvariant (VarRefMapEntryRef v)
{
  return g_variant_new_from_data (VAR_REF_MAP_ENTRY_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_ref_map_entry_to_gvariant (VarRefMapEntryRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_REF_MAP_ENTRY_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_ref_map_entry_to_owned_gvariant (VarRefMapEntryRef v, GVariant *base)
{
  return var_ref_map_entry_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_ref_map_entry_peek_as_gvariant (VarRefMapEntryRef v)
{
  return g_variant_new_from_data (VAR_REF_MAP_ENTRY_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarRefMapEntryRef
var_ref_map_entry_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_REF_MAP_ENTRY_TYPESTRING));
  return var_ref_map_entry_from_data (child.base, child.size);
}

static inline const char *
var_ref_map_entry_get_ref (VarRefMapEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline VarRefInfoRef
var_ref_map_entry_get_info (VarRefMapEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 1;
  g_assert (start <= end && end <= v.size);
  return (VarRefInfoRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline GString *
var_ref_map_entry_format (VarRefMapEntryRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  __var_gstring_append_string (s, var_ref_map_entry_get_ref (v));
  g_string_append (s, ", ");
  var_ref_info_format (var_ref_map_entry_get_info (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_ref_map_entry_print (VarRefMapEntryRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_ref_map_entry_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarRefMap *******************/

static inline VarRefMapRef
var_ref_map_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_REF_MAP_TYPESTRING));
  return (VarRefMapRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarRefMapRef
var_ref_map_from_bytes (GBytes *b)
{
  return (VarRefMapRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarRefMapRef
var_ref_map_from_data (gconstpointer data, gsize size)
{
  return (VarRefMapRef) { data, size };
}

static inline GVariant *
var_ref_map_dup_to_gvariant (VarRefMapRef v)
{
  return g_variant_new_from_data (VAR_REF_MAP_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_ref_map_to_gvariant (VarRefMapRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_REF_MAP_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_ref_map_to_owned_gvariant (VarRefMapRef v, GVariant *base)
{
  return var_ref_map_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_ref_map_peek_as_gvariant (VarRefMapRef v)
{
  return g_variant_new_from_data (VAR_REF_MAP_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarRefMapRef
var_ref_map_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_REF_MAP_TYPESTRING));
  return var_ref_map_from_data (child.base, child.size);
}

static inline gsize
var_ref_map_get_length (VarRefMapRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length  = offsets_array_size / offset_size;
  return length;
}

static inline VarRefMapEntryRef
var_ref_map_get_at (VarRefMapRef v, gsize index)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 8) : 0;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  return (VarRefMapEntryRef) { ((const char *)v.base) + start, end - start };
}

static inline GString *
var_ref_map_format (VarRefMapRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_ref_map_get_length (v);
  gsize i;
  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_REF_MAP_TYPESTRING);
  g_string_append_c (s, '[');
  for (i = 0; i < len; i++)
    {
      if (i != 0)
        g_string_append (s, ", ");
      var_ref_map_entry_format (var_ref_map_get_at (v, i), s, ((i == 0) ? type_annotate : FALSE));
    }
  g_string_append_c (s, ']');
  return s;
}

static inline char *
var_ref_map_print (VarRefMapRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_ref_map_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarSummary *******************/

static inline VarSummaryRef
var_summary_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_SUMMARY_TYPESTRING));
  return (VarSummaryRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarSummaryRef
var_summary_from_bytes (GBytes *b)
{
  return (VarSummaryRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarSummaryRef
var_summary_from_data (gconstpointer data, gsize size)
{
  return (VarSummaryRef) { data, size };
}

static inline GVariant *
var_summary_dup_to_gvariant (VarSummaryRef v)
{
  return g_variant_new_from_data (VAR_SUMMARY_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_summary_to_gvariant (VarSummaryRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_SUMMARY_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_summary_to_owned_gvariant (VarSummaryRef v, GVariant *base)
{
  return var_summary_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_summary_peek_as_gvariant (VarSummaryRef v)
{
  return g_variant_new_from_data (VAR_SUMMARY_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarSummaryRef
var_summary_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_SUMMARY_TYPESTRING));
  return var_summary_from_data (child.base, child.size);
}

static inline VarRefMapRef
var_summary_get_ref_map (VarSummaryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((7) & (~(gsize)7)) + 0;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size);
  return (VarRefMapRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline VarMetadataRef
var_summary_get_metadata (VarSummaryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 1;
  g_assert (start <= end && end <= v.size);
  return (VarMetadataRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline GString *
var_summary_format (VarSummaryRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  var_ref_map_format (var_summary_get_ref_map (v), s, type_annotate);
  g_string_append (s, ", ");
  var_metadata_format (var_summary_get_metadata (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_summary_print (VarSummaryRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_summary_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarCollectionMap *******************/

static inline VarCollectionMapRef
var_collection_map_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_COLLECTION_MAP_TYPESTRING));
  return (VarCollectionMapRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarCollectionMapRef
var_collection_map_from_bytes (GBytes *b)
{
  return (VarCollectionMapRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarCollectionMapRef
var_collection_map_from_data (gconstpointer data, gsize size)
{
  return (VarCollectionMapRef) { data, size };
}

static inline GVariant *
var_collection_map_dup_to_gvariant (VarCollectionMapRef v)
{
  return g_variant_new_from_data (VAR_COLLECTION_MAP_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_collection_map_to_gvariant (VarCollectionMapRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_COLLECTION_MAP_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_collection_map_to_owned_gvariant (VarCollectionMapRef v, GVariant *base)
{
  return var_collection_map_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_collection_map_peek_as_gvariant (VarCollectionMapRef v)
{
  return g_variant_new_from_data (VAR_COLLECTION_MAP_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarCollectionMapRef
var_collection_map_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_COLLECTION_MAP_TYPESTRING));
  return var_collection_map_from_data (child.base, child.size);
}


static inline gsize
var_collection_map_get_length (VarCollectionMapRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length = offsets_array_size / offset_size;
  return length;
}

static inline VarCollectionMapEntryRef
var_collection_map_get_at (VarCollectionMapRef v, gsize index)
{
  VarCollectionMapEntryRef res;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 8) : 0;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  res = (VarCollectionMapEntryRef) { ((const char *)v.base) + start, end - start };
  return res;
}

static inline const char *
var_collection_map_entry_get_key (VarCollectionMapEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  const char *base = (const char *)v.base;
  g_assert (end < v.size && base[end-1] == 0);
  return base;
}

static inline VarRefMapRef
var_collection_map_entry_get_value (VarCollectionMapEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offset = VAR_REF_ALIGN(end, 8);
  g_assert (offset <= v.size);
  return (VarRefMapRef) { (char *)v.base + offset, (v.size - offset_size) - offset };
}

static inline gboolean
var_collection_map_lookup (VarCollectionMapRef v, const char * key, gsize *index_out, VarRefMapRef *out)
{
  const char * canonical_key = key;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  if (last_end > v.size)
    return FALSE;
  gsize offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return FALSE;
  gsize len = offsets_array_size / offset_size;
  gsize start = 0;
  gsize end = len;

  while (start < end)
    {
      gsize mid = (end + start) / 2;
      gsize mid_end = VAR_REF_READ_FRAME_OFFSET(v, len - mid - 1);
      gsize mid_start = mid == 0 ? 0 : VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - mid), 8);
      g_assert (mid_start <= mid_end && mid_end <= last_end);
      VarCollectionMapEntryRef e = { ((const char *)v.base) + mid_start, mid_end - mid_start };
      const char * e_key = var_collection_map_entry_get_key (e);
      gint32 cmp = strcmp(canonical_key, e_key);
      if (cmp == 0)
        {
           if (index_out)
             *index_out = mid;
           if (out)
             *out = var_collection_map_entry_get_value (e);
           return TRUE;
        }
      if (cmp < 0)
        end = mid; /* canonical_key < e_key */
      else
        start = mid + 1; /* canonical_key > e_key */
    }
    return FALSE;
}

static inline GString *
var_collection_map_format (VarCollectionMapRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_collection_map_get_length (v);
  gsize i;

  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_COLLECTION_MAP_TYPESTRING);

  g_string_append_c (s, '{');
  for (i = 0; i < len; i++)
    {
      VarCollectionMapEntryRef entry = var_collection_map_get_at (v, i);
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_collection_map_entry_get_key (entry));
      g_string_append (s, ": ");
      var_ref_map_format (var_collection_map_entry_get_value (entry), s, type_annotate);
    }
  g_string_append_c (s, '}');
  return s;
}

static inline char *
var_collection_map_print (VarCollectionMapRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_collection_map_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarRelated *******************/

static inline VarRelatedRef
var_related_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_RELATED_TYPESTRING));
  return (VarRelatedRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarRelatedRef
var_related_from_bytes (GBytes *b)
{
  return (VarRelatedRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarRelatedRef
var_related_from_data (gconstpointer data, gsize size)
{
  return (VarRelatedRef) { data, size };
}

static inline GVariant *
var_related_dup_to_gvariant (VarRelatedRef v)
{
  return g_variant_new_from_data (VAR_RELATED_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_related_to_gvariant (VarRelatedRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_RELATED_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_related_to_owned_gvariant (VarRelatedRef v, GVariant *base)
{
  return var_related_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_related_peek_as_gvariant (VarRelatedRef v)
{
  return g_variant_new_from_data (VAR_RELATED_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarRelatedRef
var_related_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_RELATED_TYPESTRING));
  return var_related_from_data (child.base, child.size);
}

static inline const char *
var_related_get_ref (VarRelatedRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline VarChecksumRef
var_related_get_commit (VarRelatedRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 1;
  g_assert (start <= end && end <= v.size);
  return (VarChecksumRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const guint8 *
var_related_peek_commit (VarRelatedRef v, gsize *len) {
  VarChecksumRef a = var_related_get_commit (v);
  if (len != NULL)
    *len = var_checksum_get_length (a);
  return (const guint8 *)a.base;
}

static inline GString *
var_related_format (VarRelatedRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  __var_gstring_append_string (s, var_related_get_ref (v));
  g_string_append (s, ", ");
  var_checksum_format (var_related_get_commit (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_related_print (VarRelatedRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_related_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarArrayofRelated *******************/

static inline VarArrayofRelatedRef
var_arrayof_related_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_ARRAYOF_RELATED_TYPESTRING));
  return (VarArrayofRelatedRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarArrayofRelatedRef
var_arrayof_related_from_bytes (GBytes *b)
{
  return (VarArrayofRelatedRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarArrayofRelatedRef
var_arrayof_related_from_data (gconstpointer data, gsize size)
{
  return (VarArrayofRelatedRef) { data, size };
}

static inline GVariant *
var_arrayof_related_dup_to_gvariant (VarArrayofRelatedRef v)
{
  return g_variant_new_from_data (VAR_ARRAYOF_RELATED_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_arrayof_related_to_gvariant (VarArrayofRelatedRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_ARRAYOF_RELATED_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_arrayof_related_to_owned_gvariant (VarArrayofRelatedRef v, GVariant *base)
{
  return var_arrayof_related_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_arrayof_related_peek_as_gvariant (VarArrayofRelatedRef v)
{
  return g_variant_new_from_data (VAR_ARRAYOF_RELATED_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarArrayofRelatedRef
var_arrayof_related_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_ARRAYOF_RELATED_TYPESTRING));
  return var_arrayof_related_from_data (child.base, child.size);
}

static inline gsize
var_arrayof_related_get_length (VarArrayofRelatedRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length  = offsets_array_size / offset_size;
  return length;
}

static inline VarRelatedRef
var_arrayof_related_get_at (VarArrayofRelatedRef v, gsize index)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 1) : 0;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  return (VarRelatedRef) { ((const char *)v.base) + start, end - start };
}

static inline GString *
var_arrayof_related_format (VarArrayofRelatedRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_arrayof_related_get_length (v);
  gsize i;
  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_ARRAYOF_RELATED_TYPESTRING);
  g_string_append_c (s, '[');
  for (i = 0; i < len; i++)
    {
      if (i != 0)
        g_string_append (s, ", ");
      var_related_format (var_arrayof_related_get_at (v, i), s, ((i == 0) ? type_annotate : FALSE));
    }
  g_string_append_c (s, ']');
  return s;
}

static inline char *
var_arrayof_related_print (VarArrayofRelatedRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_arrayof_related_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarCommit *******************/

static inline VarCommitRef
var_commit_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_COMMIT_TYPESTRING));
  return (VarCommitRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarCommitRef
var_commit_from_bytes (GBytes *b)
{
  return (VarCommitRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarCommitRef
var_commit_from_data (gconstpointer data, gsize size)
{
  return (VarCommitRef) { data, size };
}

static inline GVariant *
var_commit_dup_to_gvariant (VarCommitRef v)
{
  return g_variant_new_from_data (VAR_COMMIT_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_commit_to_gvariant (VarCommitRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_COMMIT_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_commit_to_owned_gvariant (VarCommitRef v, GVariant *base)
{
  return var_commit_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_commit_peek_as_gvariant (VarCommitRef v)
{
  return g_variant_new_from_data (VAR_COMMIT_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarCommitRef
var_commit_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_COMMIT_TYPESTRING));
  return var_commit_from_data (child.base, child.size);
}

static inline VarMetadataRef
var_commit_get_metadata (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((7) & (~(gsize)7)) + 0;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size);
  return (VarMetadataRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline VarChecksumRef
var_commit_get_patent (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 1);
  g_assert (start <= end && end <= v.size);
  return (VarChecksumRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const guint8 *
var_commit_peek_patent (VarCommitRef v, gsize *len) {
  VarChecksumRef a = var_commit_get_patent (v);
  if (len != NULL)
    *len = var_checksum_get_length (a);
  return (const guint8 *)a.base;
}

static inline VarArrayofRelatedRef
var_commit_get_related (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 1);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 2);
  g_assert (start <= end && end <= v.size);
  return (VarArrayofRelatedRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const char *
var_commit_get_subject (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 2);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 3);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline const char *
var_commit_get_body (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 3);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 4);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline guint64
var_commit_get_timestamp (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 4);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 0;
  g_assert (offset + 8 < v.size);
  return GUINT64_FROM_BE((guint64)G_STRUCT_MEMBER(guint64, v.base, offset));
}

static inline VarChecksumRef
var_commit_get_root_contents (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 4);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 8;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 5);
  g_assert (start <= end && end <= v.size);
  return (VarChecksumRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const guint8 *
var_commit_peek_root_contents (VarCommitRef v, gsize *len) {
  VarChecksumRef a = var_commit_get_root_contents (v);
  if (len != NULL)
    *len = var_checksum_get_length (a);
  return (const guint8 *)a.base;
}

static inline VarChecksumRef
var_commit_get_root_metadata (VarCommitRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 5);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 6;
  g_assert (start <= end && end <= v.size);
  return (VarChecksumRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline const guint8 *
var_commit_peek_root_metadata (VarCommitRef v, gsize *len) {
  VarChecksumRef a = var_commit_get_root_metadata (v);
  if (len != NULL)
    *len = var_checksum_get_length (a);
  return (const guint8 *)a.base;
}

static inline GString *
var_commit_format (VarCommitRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  var_metadata_format (var_commit_get_metadata (v), s, type_annotate);
  g_string_append (s, ", ");
  var_checksum_format (var_commit_get_patent (v), s, type_annotate);
  g_string_append (s, ", ");
  var_arrayof_related_format (var_commit_get_related (v), s, type_annotate);
  g_string_append (s, ", ");
  __var_gstring_append_string (s, var_commit_get_subject (v));
  g_string_append (s, ", ");
  __var_gstring_append_string (s, var_commit_get_body (v));
  g_string_append (s, ", ");
  g_string_append_printf (s, "%s%"G_GUINT64_FORMAT", ",
                   type_annotate ? "uint64 " : "",
                   var_commit_get_timestamp (v));
  var_checksum_format (var_commit_get_root_contents (v), s, type_annotate);
  g_string_append (s, ", ");
  var_checksum_format (var_commit_get_root_metadata (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_commit_print (VarCommitRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_commit_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarCacheData *******************/

static inline VarCacheDataRef
var_cache_data_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_CACHE_DATA_TYPESTRING));
  return (VarCacheDataRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarCacheDataRef
var_cache_data_from_bytes (GBytes *b)
{
  return (VarCacheDataRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarCacheDataRef
var_cache_data_from_data (gconstpointer data, gsize size)
{
  return (VarCacheDataRef) { data, size };
}

static inline GVariant *
var_cache_data_dup_to_gvariant (VarCacheDataRef v)
{
  return g_variant_new_from_data (VAR_CACHE_DATA_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_cache_data_to_gvariant (VarCacheDataRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_CACHE_DATA_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_cache_data_to_owned_gvariant (VarCacheDataRef v, GVariant *base)
{
  return var_cache_data_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_cache_data_peek_as_gvariant (VarCacheDataRef v)
{
  return g_variant_new_from_data (VAR_CACHE_DATA_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarCacheDataRef
var_cache_data_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_CACHE_DATA_TYPESTRING));
  return var_cache_data_from_data (child.base, child.size);
}

static inline guint64
var_cache_data_get_installed_size (VarCacheDataRef v)
{
  guint offset = ((7) & (~(gsize)7)) + 0;
  g_assert (offset + 8 < v.size);
  return GUINT64_FROM_BE((guint64)G_STRUCT_MEMBER(guint64, v.base, offset));
}

static inline guint64
var_cache_data_get_download_size (VarCacheDataRef v)
{
  guint offset = ((7) & (~(gsize)7)) + 8;
  g_assert (offset + 8 < v.size);
  return GUINT64_FROM_BE((guint64)G_STRUCT_MEMBER(guint64, v.base, offset));
}

static inline const char *
var_cache_data_get_metadata (VarCacheDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((7) & (~(gsize)7)) + 16;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = v.size - offset_size * 0;
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline GString *
var_cache_data_format (VarCacheDataRef v, GString *s, gboolean type_annotate)
{
  g_string_append_printf (s, "(%s%"G_GUINT64_FORMAT", %s%"G_GUINT64_FORMAT", ",
                   type_annotate ? "uint64 " : "",
                   var_cache_data_get_installed_size (v),
                   type_annotate ? "uint64 " : "",
                   var_cache_data_get_download_size (v));
  __var_gstring_append_string (s, var_cache_data_get_metadata (v));
  g_string_append (s, ")");
  return s;
}

static inline char *
var_cache_data_print (VarCacheDataRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_cache_data_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarCache *******************/

static inline VarCacheRef
var_cache_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_CACHE_TYPESTRING));
  return (VarCacheRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarCacheRef
var_cache_from_bytes (GBytes *b)
{
  return (VarCacheRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarCacheRef
var_cache_from_data (gconstpointer data, gsize size)
{
  return (VarCacheRef) { data, size };
}

static inline GVariant *
var_cache_dup_to_gvariant (VarCacheRef v)
{
  return g_variant_new_from_data (VAR_CACHE_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_cache_to_gvariant (VarCacheRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_CACHE_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_cache_to_owned_gvariant (VarCacheRef v, GVariant *base)
{
  return var_cache_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_cache_peek_as_gvariant (VarCacheRef v)
{
  return g_variant_new_from_data (VAR_CACHE_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarCacheRef
var_cache_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_CACHE_TYPESTRING));
  return var_cache_from_data (child.base, child.size);
}


static inline gsize
var_cache_get_length (VarCacheRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length = offsets_array_size / offset_size;
  return length;
}

static inline VarCacheEntryRef
var_cache_get_at (VarCacheRef v, gsize index)
{
  VarCacheEntryRef res;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 8) : 0;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  res = (VarCacheEntryRef) { ((const char *)v.base) + start, end - start };
  return res;
}

static inline const char *
var_cache_entry_get_key (VarCacheEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  const char *base = (const char *)v.base;
  g_assert (end < v.size && base[end-1] == 0);
  return base;
}

static inline VarCacheDataRef
var_cache_entry_get_value (VarCacheEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offset = VAR_REF_ALIGN(end, 8);
  g_assert (offset <= v.size);
  return (VarCacheDataRef) { (char *)v.base + offset, (v.size - offset_size) - offset };
}

static inline gboolean
var_cache_lookup (VarCacheRef v, const char * key, gsize *index_out, VarCacheDataRef *out)
{
  const char * canonical_key = key;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  if (last_end > v.size)
    return FALSE;
  gsize offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return FALSE;
  gsize len = offsets_array_size / offset_size;
  gsize start = 0;
  gsize end = len;

  while (start < end)
    {
      gsize mid = (end + start) / 2;
      gsize mid_end = VAR_REF_READ_FRAME_OFFSET(v, len - mid - 1);
      gsize mid_start = mid == 0 ? 0 : VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - mid), 8);
      g_assert (mid_start <= mid_end && mid_end <= last_end);
      VarCacheEntryRef e = { ((const char *)v.base) + mid_start, mid_end - mid_start };
      const char * e_key = var_cache_entry_get_key (e);
      gint32 cmp = strcmp(canonical_key, e_key);
      if (cmp == 0)
        {
           if (index_out)
             *index_out = mid;
           if (out)
             *out = var_cache_entry_get_value (e);
           return TRUE;
        }
      if (cmp < 0)
        end = mid; /* canonical_key < e_key */
      else
        start = mid + 1; /* canonical_key > e_key */
    }
    return FALSE;
}

static inline GString *
var_cache_format (VarCacheRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_cache_get_length (v);
  gsize i;

  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_CACHE_TYPESTRING);

  g_string_append_c (s, '{');
  for (i = 0; i < len; i++)
    {
      VarCacheEntryRef entry = var_cache_get_at (v, i);
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_cache_entry_get_key (entry));
      g_string_append (s, ": ");
      var_cache_data_format (var_cache_entry_get_value (entry), s, type_annotate);
    }
  g_string_append_c (s, '}');
  return s;
}

static inline char *
var_cache_print (VarCacheRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_cache_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarSparseCache *******************/

static inline VarSparseCacheRef
var_sparse_cache_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_SPARSE_CACHE_TYPESTRING));
  return (VarSparseCacheRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarSparseCacheRef
var_sparse_cache_from_bytes (GBytes *b)
{
  return (VarSparseCacheRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarSparseCacheRef
var_sparse_cache_from_data (gconstpointer data, gsize size)
{
  return (VarSparseCacheRef) { data, size };
}

static inline GVariant *
var_sparse_cache_dup_to_gvariant (VarSparseCacheRef v)
{
  return g_variant_new_from_data (VAR_SPARSE_CACHE_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_sparse_cache_to_gvariant (VarSparseCacheRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_SPARSE_CACHE_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_sparse_cache_to_owned_gvariant (VarSparseCacheRef v, GVariant *base)
{
  return var_sparse_cache_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_sparse_cache_peek_as_gvariant (VarSparseCacheRef v)
{
  return g_variant_new_from_data (VAR_SPARSE_CACHE_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarSparseCacheRef
var_sparse_cache_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_SPARSE_CACHE_TYPESTRING));
  return var_sparse_cache_from_data (child.base, child.size);
}


static inline gsize
var_sparse_cache_get_length (VarSparseCacheRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length = offsets_array_size / offset_size;
  return length;
}

static inline VarSparseCacheEntryRef
var_sparse_cache_get_at (VarSparseCacheRef v, gsize index)
{
  VarSparseCacheEntryRef res;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 8) : 0;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  res = (VarSparseCacheEntryRef) { ((const char *)v.base) + start, end - start };
  return res;
}

static inline const char *
var_sparse_cache_entry_get_key (VarSparseCacheEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  const char *base = (const char *)v.base;
  g_assert (end < v.size && base[end-1] == 0);
  return base;
}

static inline VarMetadataRef
var_sparse_cache_entry_get_value (VarSparseCacheEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offset = VAR_REF_ALIGN(end, 8);
  g_assert (offset <= v.size);
  return (VarMetadataRef) { (char *)v.base + offset, (v.size - offset_size) - offset };
}

static inline gboolean
var_sparse_cache_lookup (VarSparseCacheRef v, const char * key, gsize *index_out, VarMetadataRef *out)
{
  const char * canonical_key = key;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  if (last_end > v.size)
    return FALSE;
  gsize offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return FALSE;
  gsize len = offsets_array_size / offset_size;
  gsize start = 0;
  gsize end = len;

  while (start < end)
    {
      gsize mid = (end + start) / 2;
      gsize mid_end = VAR_REF_READ_FRAME_OFFSET(v, len - mid - 1);
      gsize mid_start = mid == 0 ? 0 : VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - mid), 8);
      g_assert (mid_start <= mid_end && mid_end <= last_end);
      VarSparseCacheEntryRef e = { ((const char *)v.base) + mid_start, mid_end - mid_start };
      const char * e_key = var_sparse_cache_entry_get_key (e);
      gint32 cmp = strcmp(canonical_key, e_key);
      if (cmp == 0)
        {
           if (index_out)
             *index_out = mid;
           if (out)
             *out = var_sparse_cache_entry_get_value (e);
           return TRUE;
        }
      if (cmp < 0)
        end = mid; /* canonical_key < e_key */
      else
        start = mid + 1; /* canonical_key > e_key */
    }
    return FALSE;
}

static inline GString *
var_sparse_cache_format (VarSparseCacheRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_sparse_cache_get_length (v);
  gsize i;

  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_SPARSE_CACHE_TYPESTRING);

  g_string_append_c (s, '{');
  for (i = 0; i < len; i++)
    {
      VarSparseCacheEntryRef entry = var_sparse_cache_get_at (v, i);
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_sparse_cache_entry_get_key (entry));
      g_string_append (s, ": ");
      var_metadata_format (var_sparse_cache_entry_get_value (entry), s, type_annotate);
    }
  g_string_append_c (s, '}');
  return s;
}

static inline char *
var_sparse_cache_print (VarSparseCacheRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_sparse_cache_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarCommitsCache *******************/

static inline VarCommitsCacheRef
var_commits_cache_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_COMMITS_CACHE_TYPESTRING));
  return (VarCommitsCacheRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarCommitsCacheRef
var_commits_cache_from_bytes (GBytes *b)
{
  return (VarCommitsCacheRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarCommitsCacheRef
var_commits_cache_from_data (gconstpointer data, gsize size)
{
  return (VarCommitsCacheRef) { data, size };
}

static inline GVariant *
var_commits_cache_dup_to_gvariant (VarCommitsCacheRef v)
{
  return g_variant_new_from_data (VAR_COMMITS_CACHE_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_commits_cache_to_gvariant (VarCommitsCacheRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_COMMITS_CACHE_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_commits_cache_to_owned_gvariant (VarCommitsCacheRef v, GVariant *base)
{
  return var_commits_cache_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_commits_cache_peek_as_gvariant (VarCommitsCacheRef v)
{
  return g_variant_new_from_data (VAR_COMMITS_CACHE_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarCommitsCacheRef
var_commits_cache_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_COMMITS_CACHE_TYPESTRING));
  return var_commits_cache_from_data (child.base, child.size);
}

static inline gsize
var_commits_cache_get_length (VarCommitsCacheRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length  = offsets_array_size / offset_size;
  return length;
}

static inline VarChecksumRef
var_commits_cache_get_at (VarCommitsCacheRef v, gsize index)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 1) : 0;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  return (VarChecksumRef) { ((const char *)v.base) + start, end - start };
}

static inline GString *
var_commits_cache_format (VarCommitsCacheRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_commits_cache_get_length (v);
  gsize i;
  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_COMMITS_CACHE_TYPESTRING);
  g_string_append_c (s, '[');
  for (i = 0; i < len; i++)
    {
      if (i != 0)
        g_string_append (s, ", ");
      var_checksum_format (var_commits_cache_get_at (v, i), s, ((i == 0) ? type_annotate : FALSE));
    }
  g_string_append_c (s, ']');
  return s;
}

static inline char *
var_commits_cache_print (VarCommitsCacheRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_commits_cache_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarArrayofstring *******************/

static inline VarArrayofstringRef
var_arrayofstring_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_ARRAYOFSTRING_TYPESTRING));
  return (VarArrayofstringRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarArrayofstringRef
var_arrayofstring_from_bytes (GBytes *b)
{
  return (VarArrayofstringRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarArrayofstringRef
var_arrayofstring_from_data (gconstpointer data, gsize size)
{
  return (VarArrayofstringRef) { data, size };
}

static inline GVariant *
var_arrayofstring_dup_to_gvariant (VarArrayofstringRef v)
{
  return g_variant_new_from_data (VAR_ARRAYOFSTRING_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_arrayofstring_to_gvariant (VarArrayofstringRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_ARRAYOFSTRING_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_arrayofstring_to_owned_gvariant (VarArrayofstringRef v, GVariant *base)
{
  return var_arrayofstring_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_arrayofstring_peek_as_gvariant (VarArrayofstringRef v)
{
  return g_variant_new_from_data (VAR_ARRAYOFSTRING_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarArrayofstringRef
var_arrayofstring_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_ARRAYOFSTRING_TYPESTRING));
  return var_arrayofstring_from_data (child.base, child.size);
}

static inline gsize
var_arrayofstring_get_length (VarArrayofstringRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length  = offsets_array_size / offset_size;
  return length;
}

static inline const char *
var_arrayofstring_get_at (VarArrayofstringRef v, gsize index)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 1) : 0;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  const char *base = (const char *)v.base;
  g_assert (base[end-1] == 0);
  return base + start;
}

static inline const char **
var_arrayofstring_to_strv (VarArrayofstringRef v, gsize *length_out)
{
  gsize length = var_arrayofstring_get_length (v);
  gsize i;
  const char **resv = g_new (const char *, length + 1);

  for (i = 0; i < length; i++)
    resv[i] = var_arrayofstring_get_at (v, i);
  resv[i] = NULL;

  if (length_out)
    *length_out = length;

  return resv;
}

static inline GString *
var_arrayofstring_format (VarArrayofstringRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_arrayofstring_get_length (v);
  gsize i;
  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_ARRAYOFSTRING_TYPESTRING);
  g_string_append_c (s, '[');
  for (i = 0; i < len; i++)
    {
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_arrayofstring_get_at (v, i));
    }
  g_string_append_c (s, ']');
  return s;
}

static inline char *
var_arrayofstring_print (VarArrayofstringRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_arrayofstring_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarDeployData *******************/

static inline VarDeployDataRef
var_deploy_data_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_DEPLOY_DATA_TYPESTRING));
  return (VarDeployDataRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarDeployDataRef
var_deploy_data_from_bytes (GBytes *b)
{
  return (VarDeployDataRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarDeployDataRef
var_deploy_data_from_data (gconstpointer data, gsize size)
{
  return (VarDeployDataRef) { data, size };
}

static inline GVariant *
var_deploy_data_dup_to_gvariant (VarDeployDataRef v)
{
  return g_variant_new_from_data (VAR_DEPLOY_DATA_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_deploy_data_to_gvariant (VarDeployDataRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_DEPLOY_DATA_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_deploy_data_to_owned_gvariant (VarDeployDataRef v, GVariant *base)
{
  return var_deploy_data_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_deploy_data_peek_as_gvariant (VarDeployDataRef v)
{
  return g_variant_new_from_data (VAR_DEPLOY_DATA_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarDeployDataRef
var_deploy_data_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_DEPLOY_DATA_TYPESTRING));
  return var_deploy_data_from_data (child.base, child.size);
}

static inline const char *
var_deploy_data_get_origin (VarDeployDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline const char *
var_deploy_data_get_commit (VarDeployDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 1);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline VarArrayofstringRef
var_deploy_data_get_subpaths (VarDeployDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 1);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 2);
  g_assert (start <= end && end <= v.size);
  return (VarArrayofstringRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline guint64
var_deploy_data_get_installed_size (VarDeployDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 2);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 0;
  g_assert (offset + 8 < v.size);
  return GUINT64_FROM_BE((guint64)G_STRUCT_MEMBER(guint64, v.base, offset));
}

static inline VarMetadataRef
var_deploy_data_get_metadata (VarDeployDataRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 2);
  guint offset = ((last_end + 7) & (~(gsize)7)) + 8;
  gsize start = offset;
  gsize end = v.size - offset_size * 3;
  g_assert (start <= end && end <= v.size);
  return (VarMetadataRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline GString *
var_deploy_data_format (VarDeployDataRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  __var_gstring_append_string (s, var_deploy_data_get_origin (v));
  g_string_append (s, ", ");
  __var_gstring_append_string (s, var_deploy_data_get_commit (v));
  g_string_append (s, ", ");
  var_arrayofstring_format (var_deploy_data_get_subpaths (v), s, type_annotate);
  g_string_append (s, ", ");
  g_string_append_printf (s, "%s%"G_GUINT64_FORMAT", ",
                   type_annotate ? "uint64 " : "",
                   var_deploy_data_get_installed_size (v));
  var_metadata_format (var_deploy_data_get_metadata (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_deploy_data_print (VarDeployDataRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_deploy_data_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarRatings *******************/

static inline VarRatingsRef
var_ratings_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_RATINGS_TYPESTRING));
  return (VarRatingsRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarRatingsRef
var_ratings_from_bytes (GBytes *b)
{
  return (VarRatingsRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarRatingsRef
var_ratings_from_data (gconstpointer data, gsize size)
{
  return (VarRatingsRef) { data, size };
}

static inline GVariant *
var_ratings_dup_to_gvariant (VarRatingsRef v)
{
  return g_variant_new_from_data (VAR_RATINGS_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_ratings_to_gvariant (VarRatingsRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_RATINGS_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_ratings_to_owned_gvariant (VarRatingsRef v, GVariant *base)
{
  return var_ratings_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_ratings_peek_as_gvariant (VarRatingsRef v)
{
  return g_variant_new_from_data (VAR_RATINGS_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarRatingsRef
var_ratings_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_RATINGS_TYPESTRING));
  return var_ratings_from_data (child.base, child.size);
}


static inline gsize
var_ratings_get_length (VarRatingsRef v)
{
  if (v.size == 0)
    return 0;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offsets_array_size;
  if (last_end > v.size)
    return 0;
  offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return 0;
  gsize length = offsets_array_size / offset_size;
  return length;
}

static inline VarRatingsEntryRef
var_ratings_get_at (VarRatingsRef v, gsize index)
{
  VarRatingsEntryRef res;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize len = (v.size - last_end) / offset_size;
  gsize start = (index > 0) ? VAR_REF_ALIGN(VAR_REF_READ_FRAME_OFFSET(v, len - index), 1) : 0;
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - index - 1);
  g_assert (start <= end && end <= last_end);
  res = (VarRatingsEntryRef) { ((const char *)v.base) + start, end - start };
  return res;
}

static inline const char *
var_ratings_entry_get_key (VarRatingsEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  const char *base = (const char *)v.base;
  g_assert (end < v.size && base[end-1] == 0);
  return base;
}

static inline const char *
var_ratings_entry_get_value (VarRatingsEntryRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  gsize offset = VAR_REF_ALIGN(end, 1);
  g_assert (offset <= v.size);
  g_assert (((char *)v.base)[(v.size - offset_size) - 1] == 0);
  return (const char *)v.base + offset;
}

static inline gboolean
var_ratings_lookup (VarRatingsRef v, const char * key, gsize *index_out, const char * *out)
{
  const char * canonical_key = key;
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  if (last_end > v.size)
    return FALSE;
  gsize offsets_array_size = v.size - last_end;
  if (offsets_array_size % offset_size != 0)
    return FALSE;
  gsize len = offsets_array_size / offset_size;
  gsize start = 0;
  gsize i;

  for (i = 0; i < len; i++)
    {
      gsize end = VAR_REF_READ_FRAME_OFFSET(v, len - i - 1);
      VarRatingsEntryRef e = { ((const guchar *)v.base) + start, end - start };
      g_assert (start <= end && end <= last_end);
      const char * e_key = var_ratings_entry_get_key (e);
      if (strcmp(canonical_key, e_key) == 0)
        {
           if (index_out)
             *index_out = i;
           if (out)
             *out = var_ratings_entry_get_value (e);
           return TRUE;
        }
      start = VAR_REF_ALIGN(end, 1);
    }
    return FALSE;
}

static inline GString *
var_ratings_format (VarRatingsRef v, GString *s, gboolean type_annotate)
{
  gsize len = var_ratings_get_length (v);
  gsize i;

  if (len == 0 && type_annotate)
    g_string_append_printf (s, "@%s ", VAR_RATINGS_TYPESTRING);

  g_string_append_c (s, '{');
  for (i = 0; i < len; i++)
    {
      VarRatingsEntryRef entry = var_ratings_get_at (v, i);
      if (i != 0)
        g_string_append (s, ", ");
      __var_gstring_append_string (s, var_ratings_entry_get_key (entry));
      g_string_append (s, ": ");
      __var_gstring_append_string (s, var_ratings_entry_get_value (entry));
    }
  g_string_append_c (s, '}');
  return s;
}

static inline char *
var_ratings_print (VarRatingsRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_ratings_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}

/************** VarContentRating *******************/

static inline VarContentRatingRef
var_content_rating_from_gvariant (GVariant *v)
{
  g_assert (g_variant_type_equal (g_variant_get_type (v), VAR_CONTENT_RATING_TYPESTRING));
  return (VarContentRatingRef) { g_variant_get_data (v), g_variant_get_size (v) };
}

static inline VarContentRatingRef
var_content_rating_from_bytes (GBytes *b)
{
  return (VarContentRatingRef) { g_bytes_get_data (b, NULL), g_bytes_get_size (b) };
}

static inline VarContentRatingRef
var_content_rating_from_data (gconstpointer data, gsize size)
{
  return (VarContentRatingRef) { data, size };
}

static inline GVariant *
var_content_rating_dup_to_gvariant (VarContentRatingRef v)
{
  return g_variant_new_from_data (VAR_CONTENT_RATING_TYPEFORMAT, g_memdup (v.base, v.size), v.size, TRUE, g_free, NULL);
}

static inline GVariant *
var_content_rating_to_gvariant (VarContentRatingRef v,
                             GDestroyNotify      notify,
                             gpointer            user_data)
{
  return g_variant_new_from_data (VAR_CONTENT_RATING_TYPEFORMAT, v.base, v.size, TRUE, notify, user_data);
}

static inline GVariant *
var_content_rating_to_owned_gvariant (VarContentRatingRef v, GVariant *base)
{
  return var_content_rating_to_gvariant (v, (GDestroyNotify)g_variant_unref, g_variant_ref (base));
}

static inline GVariant *
var_content_rating_peek_as_gvariant (VarContentRatingRef v)
{
  return g_variant_new_from_data (VAR_CONTENT_RATING_TYPEFORMAT, v.base, v.size, TRUE, NULL, NULL);
}

static inline VarContentRatingRef
var_content_rating_from_variant (VarVariantRef v)
{
  const GVariantType  *type;
  VarRef child = var_variant_get_child (v, &type);
  g_assert (g_variant_type_equal(type, VAR_CONTENT_RATING_TYPESTRING));
  return var_content_rating_from_data (child.base, child.size);
}

static inline const char *
var_content_rating_get_rating_type (VarContentRatingRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  guint offset = ((0) & (~(gsize)0)) + 0;
  const char *base = (const char *)v.base;
  gsize start = offset;
  G_GNUC_UNUSED gsize end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  g_assert (start <= end && end <= v.size && base[end-1] == 0);
  return &G_STRUCT_MEMBER(const char, v.base, start);
}

static inline VarRatingsRef
var_content_rating_get_ratings (VarContentRatingRef v)
{
  guint offset_size = var_ref_get_offset_size (v.size);
  gsize last_end = VAR_REF_READ_FRAME_OFFSET(v, 0);
  guint offset = ((last_end + 0) & (~(gsize)0)) + 0;
  gsize start = offset;
  gsize end = v.size - offset_size * 1;
  g_assert (start <= end && end <= v.size);
  return (VarRatingsRef) { G_STRUCT_MEMBER_P(v.base, start), end - start };
}

static inline GString *
var_content_rating_format (VarContentRatingRef v, GString *s, gboolean type_annotate)
{
  g_string_append (s, "(");
  __var_gstring_append_string (s, var_content_rating_get_rating_type (v));
  g_string_append (s, ", ");
  var_ratings_format (var_content_rating_get_ratings (v), s, type_annotate);
  g_string_append (s, ")");
  return s;
}

static inline char *
var_content_rating_print (VarContentRatingRef v, gboolean type_annotate)
{
  GString *s = g_string_new ("");
  var_content_rating_format (v, s, type_annotate);
  return g_string_free (s, FALSE);
}
