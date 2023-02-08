#pragma once

class audio_device;
class audio_device_list;

inline std::optional<audio_device> get_default_audio_input_device();
inline std::optional<audio_device> get_default_audio_output_device();

inline audio_device_list get_audio_input_device_list();
inline audio_device_list get_audio_output_device_list();

enum class audio_device_list_event
{
	device_list_changed
	, default_input_device_changed
	, default_output_device_changed
};

template <typename F, typename = std::enable_if_t<std::is_invocable_v<F>>>
void set_audio_device_list_callback(audio_device_list_event, F&&);