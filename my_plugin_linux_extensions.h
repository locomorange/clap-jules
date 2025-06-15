#pragma once

#if VSTGUI_ENABLED && defined(__linux__)

#include "my_plugin.h"
#include <clap/ext/timer-support.h>
#include <clap/ext/posix-fd-support.h>

// Timer Support Extension
extern const clap_plugin_timer_support_t my_timer_support_extension;

// POSIX FD Support Extension  
extern const clap_plugin_posix_fd_support_t my_posix_fd_support_extension;

// Helper functions for host integration
bool register_timer_with_host(my_plugin_t *self, uint32_t period_ms, clap_id *timer_id);
bool unregister_timer_with_host(my_plugin_t *self, clap_id timer_id);
bool register_fd_with_host(my_plugin_t *self, int fd, clap_posix_fd_flags_t flags);
bool unregister_fd_with_host(my_plugin_t *self, int fd);

#endif // VSTGUI_ENABLED && defined(__linux__)