#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef MY_CLAP_PLUGIN_EXPORTS
        #define CLAP_JULES_EXPORT __declspec(dllexport)
    #else
        #define CLAP_JULES_EXPORT __declspec(dllimport)
    #endif
#else
    #define CLAP_JULES_EXPORT __attribute__((visibility("default")))
#endif
