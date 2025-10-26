livecaptions_sources = [
  'main.c',
  'livecaptions-window.c',
  'livecaptions-welcome.c',
  'livecaptions-settings.c',
  'livecaptions-application.c',
  'audiocap.c',
  'asrproc.c',
  'line-gen.c',
  'profanity-filter.c',
  'window-helper.c',
  'history.c',
  'livecaptions-history-window.c',
  'dbus-interface.c'
]

# Platform-specific audio capture backends
if host_machine.system() == 'darwin'
  livecaptions_sources += 'audiocap-ca.c'
else
  livecaptions_sources += ['audiocap-pa.c', 'audiocap-pw.c']
endif

cc = meson.get_compiler('c')

livecaptions_deps = [
  dependency('libadwaita-1', version: '>= 1.0'),
  cc.find_library('m', required: false),
  april_lib
]

# Platform-specific dependencies
if host_machine.system() == 'darwin'
  # macOS: Core Audio framework
  livecaptions_deps += dependency('appleframeworks', modules: ['CoreAudio', 'AudioToolbox', 'CoreFoundation'])
else
  # Linux: PulseAudio and X11
  livecaptions_deps += [
    #dependency('libpipewire-0.3', version: '>=0.3.41'),
    dependency('libpulse'),
    dependency('x11'),
  ]
endif

gnome = import('gnome')

livecaptions_sources += gnome.compile_resources('livecaptions-resources',
  'livecaptions.gresource.xml',
  c_name: 'livecaptions'
)

executable('livecaptions', livecaptions_sources,
  dependencies: livecaptions_deps,
  install: true,
)
