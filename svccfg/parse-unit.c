#include "s16.h"
#include "ini.h"

#define OnError(...)                                                           \
    fprintf (stderr, __VA_ARGS__);                                             \
    goto on_error;

int parse_config_line (void * user, const char * section, const char * name,
                       const char * value)
{
    char * key;
    svc_t * svc = user;

    if (!strlen (name))
        return 1;

    asprintf (&key, "%s.%s", section, name);
    svc_object_set_property_string (svc, key, value);
    free (key);

    return 1;
}

#define SetOrExit(Name) if (!property_find_name(new_svc->properties, Name)) { OnError("error: %s not set", #Name); }

svc_t * parse_unit (int is_systemd, char const * path)
{
    svc_t * new_svc = calloc (1, sizeof (svc_t));
    int inierror = ini_parse (path, parse_config_line, new_svc);

    if (inierror > 0)
    {
        OnError ("Error: failed parsing line %d of unit file %s\n", inierror,
                 path);
    }
    else if (inierror < 0)
    {
        OnError ("Error: failed to read unit file %s\n", path);
    }

    if (is_systemd)
    {
        new_svc->name=strdup(path);
        svc_object_set_property_string (new_svc, "S16.Delegate", "systemd");
    }
    else
    {
        SetOrExit("S16.Delegate");
        SetOrExit("S16.Name")
        new_svc->name = strdup(property_find_name(new_svc->properties, "S16.Name")->value.pval_u.s);
    }


    return new_svc;

on_error:
    destroy_svc (new_svc);
    return 0;
}
