
/* Generated data (by glib-mkenums) */

#include "config.h"
#include <xdg-app-utils.h>
#include <xdg-app.h>
#include <xdg-app-enum-types.h>
#include <gio/gio.h>

/* enumerations from "lib/xdg-app-ref.h" */
GType
xdg_app_ref_kind_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { XDG_APP_REF_KIND_APP, "XDG_APP_REF_KIND_APP", "app" },
        { XDG_APP_REF_KIND_RUNTIME, "XDG_APP_REF_KIND_RUNTIME", "runtime" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("XdgAppRefKind"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

/* enumerations from "lib/xdg-app-error.h" */
GType
xdg_app_error_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { XDG_APP_ERROR_ALREADY_INSTALLED, "XDG_APP_ERROR_ALREADY_INSTALLED", "already-installed" },
        { XDG_APP_ERROR_NOT_INSTALLED, "XDG_APP_ERROR_NOT_INSTALLED", "not-installed" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("XdgAppError"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

/* enumerations from "lib/xdg-app-installation.h" */
GType
xdg_app_update_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { XDG_APP_UPDATE_FLAGS_NONE, "XDG_APP_UPDATE_FLAGS_NONE", "none" },
        { XDG_APP_UPDATE_FLAGS_NO_DEPLOY, "XDG_APP_UPDATE_FLAGS_NO_DEPLOY", "no-deploy" },
        { XDG_APP_UPDATE_FLAGS_NO_PULL, "XDG_APP_UPDATE_FLAGS_NO_PULL", "no-pull" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("XdgAppUpdateFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}


/* Generated data ends here */

