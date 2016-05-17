
/* Generated data (by glib-mkenums) */

#include "config.h"
#include <flatpak-utils.h>
#include <flatpak.h>
#include <flatpak-enum-types.h>
#include <gio/gio.h>

/* enumerations from "/home/smcv/src/xdg-app/lib/flatpak-ref.h" */
GType
flatpak_ref_kind_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { FLATPAK_REF_KIND_APP, "FLATPAK_REF_KIND_APP", "app" },
        { FLATPAK_REF_KIND_RUNTIME, "FLATPAK_REF_KIND_RUNTIME", "runtime" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("FlatpakRefKind"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

/* enumerations from "/home/smcv/src/xdg-app/lib/flatpak-error.h" */
GType
flatpak_error_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { FLATPAK_ERROR_ALREADY_INSTALLED, "FLATPAK_ERROR_ALREADY_INSTALLED", "already-installed" },
        { FLATPAK_ERROR_NOT_INSTALLED, "FLATPAK_ERROR_NOT_INSTALLED", "not-installed" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("FlatpakError"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

/* enumerations from "/home/smcv/src/xdg-app/lib/flatpak-installation.h" */
GType
flatpak_update_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { FLATPAK_UPDATE_FLAGS_NONE, "FLATPAK_UPDATE_FLAGS_NONE", "none" },
        { FLATPAK_UPDATE_FLAGS_NO_DEPLOY, "FLATPAK_UPDATE_FLAGS_NO_DEPLOY", "no-deploy" },
        { FLATPAK_UPDATE_FLAGS_NO_PULL, "FLATPAK_UPDATE_FLAGS_NO_PULL", "no-pull" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("FlatpakUpdateFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

