#pragma once

#include <boost/di.hpp>
#include <memory>

// Forward declarations
namespace ClapeJules {
    class AudioProcessor;
    class FilterViewModel;
    class FilterView;
}

namespace ClapeJules::DI {

namespace di = boost::ext::di;

// Create the dependency injection container
auto createContainer() {
    return di::make_injector(
        // Bind AudioProcessor as singleton
        di::bind<AudioProcessor>().in(di::singleton),
        
        // Bind ViewModel and View
        di::bind<FilterViewModel>(),
        di::bind<FilterView>()
    );
}

// Convenience type for the injector
using Container = decltype(createContainer());

} // namespace ClapeJules::DI