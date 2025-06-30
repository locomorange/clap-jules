#pragma once

#include <clap/clap.h>

#ifdef _WIN32
  // Windows does not have pid_t, define it or use a Windows equivalent
  typedef int pid_t;
#else
  #include <sys/types.h>  // For pid_t
  #include <clap/ext/posix-fd-support.h>
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI state
    bool gui_created;
    bool gui_visible;
    pid_t flutter_pid;  // Process ID of Flutter app
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
