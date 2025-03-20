#pragma once

#include <Windows.h>
#include <winrt/base.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <CoreWindow.h>

#include <format>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>


struct GameVersion {
	int major;
	int minor;
	int build;
	int revision;
};

template<typename... Args>
inline void SetTitle(const std::string& fmt, Args&&... args) {
	const std::string title = std::vformat(fmt, std::make_format_args(args...));
	winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
		winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
		[title]() {
			winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().Title(winrt::to_hstring(title));
		}
	);
}

inline GameVersion GetGameVersion() {
	const winrt::Windows::ApplicationModel::Package package = winrt::Windows::ApplicationModel::Package::Current();
	auto [major, minor, build, revision] = package.Id().Version();

	return { major, minor, build, revision };
}

inline std::string GetFormattedVersion() {
	const auto version = GetGameVersion();
	return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.build) + "." + std::to_string(version.revision);
}