#include "my_plugin_linux_extensions.h"

#if VSTGUI_ENABLED && defined(__linux__)

#include <stdio.h>

// --- Timer Support Extension Implementation ---
static void my_plugin_timer_support_on_timer(const clap_plugin_t *plugin, clap_id timer_id) {
    printf("MyPlugin: Timer callback called (id: %u)\n", timer_id);
    // Forward to VSTGUI - this would need proper VSTGUI integration
    // For now, just a placeholder
}

const clap_plugin_timer_support_t my_timer_support_extension = {
    my_plugin_timer_support_on_timer
};

// --- POSIX FD Support Extension Implementation ---
static void my_plugin_posix_fd_support_on_fd(const clap_plugin_t *plugin, int fd, clap_posix_fd_flags_t flags) {
    printf("MyPlugin: FD callback called (fd: %d, flags: %u)\n", fd, flags);
    // Forward to VSTGUI - this would need proper VSTGUI integration
    // For now, just a placeholder
}

const clap_plugin_posix_fd_support_t my_posix_fd_support_extension = {
    my_plugin_posix_fd_support_on_fd
};

// --- Helper functions to register/unregister with host ---
bool register_timer_with_host(my_plugin_t *self, uint32_t period_ms, clap_id *timer_id) {
    if (self->host) {
        auto timer_support = (const clap_host_timer_support_t*)self->host->get_extension(self->host, CLAP_EXT_TIMER_SUPPORT);
        if (timer_support) {
            return timer_support->register_timer(self->host, period_ms, timer_id);
        }
    }
    return false;
}

bool unregister_timer_with_host(my_plugin_t *self, clap_id timer_id) {
    if (self->host) {
        auto timer_support = (const clap_host_timer_support_t*)self->host->get_extension(self->host, CLAP_EXT_TIMER_SUPPORT);
        if (timer_support) {
            return timer_support->unregister_timer(self->host, timer_id);
        }
    }
    return false;
}

bool register_fd_with_host(my_plugin_t *self, int fd, clap_posix_fd_flags_t flags) {
    if (self->host) {
        auto fd_support = (const clap_host_posix_fd_support_t*)self->host->get_extension(self->host, CLAP_EXT_POSIX_FD_SUPPORT);
        if (fd_support) {
            return fd_support->register_fd(self->host, fd, flags);
        }
    }
    return false;
}

bool unregister_fd_with_host(my_plugin_t *self, int fd) {
    if (self->host) {
        auto fd_support = (const clap_host_posix_fd_support_t*)self->host->get_extension(self->host, CLAP_EXT_POSIX_FD_SUPPORT);
        if (fd_support) {
            return fd_support->unregister_fd(self->host, fd);
        }
    }
    return false;
}

#endif // VSTGUI_ENABLED && defined(__linux__)