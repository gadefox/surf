/* See LICENSE file for copyright and license details. */

#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <regex.h>
#include "arg.h"
#include "callback.h"


#define parameter_get_priority(p)  (parameter_get (p)->prio)
#define parameter_is(p)            (parameter_get (p)->val.b)
#define parameter_get_uint(p)      (parameter_get (p)->val.u)
#define parameter_get_int(p)       (parameter_get (p)->val.i)
#define parameter_get_float(p)     (parameter_get (p)->val.f)
#define parameter_get_string(p)    (parameter_get (p)->val.s)
#define parameter_get_vector(p)    (parameter_get (p)->val.v)


typedef enum {
    PriorityDefault,
    PriorityPerUri,
    PriorityCommand
} Priority;

typedef struct {
    Arg val;
    Priority prio;
} Parameter;

typedef struct {
    ParameterType type;
    Parameter param;
} TypeParameter;

typedef struct {
    char *uri;
    TypeParameter *config;
    uint size;  /* COUNTOF (config) */
    regex_t re;
} UriParameters;


extern int param_cookie_policy;


void parameters_init (void);

UriParameters * uriparameters_find (const char *uri);
void uriparameters_set (Client *c, const char *uri, ParameterType *params, uint size);

Parameter * uriparameter_find (ParameterType type);

Parameter * parameter_get (ParameterType type);

void parameter_set (Client *c, gboolean refresh, ParameterType type, const Arg a);

void uriparameters_finished_set (Client *c, const char *uri);
void uriparameters_transient_set (Client *c, const char *uri);
void uriparameters_committed_set (Client *c, const char *uri);

void parameter_toggle_cookie_policy (Client *c);


#endif /* _PARAMETER_H_ */
