# Flutter UI Sub-Agent

## Purpose

Own cross-platform Music Elf interface implementation for desktop and mobile.

## Owns

- Future `apps/music_elf_app/lib/` UI code.
- Flutter widget, layout, theme, and navigation tests.
- Platform-specific UX adaptations for Windows, macOS, iOS, and Android.

## Inputs

- App state from the UI Event/State sub-agent.
- Native result handles and preview data from app binding.
- Localization resources.

## Outputs

- Usable Flutter screens for record/import, analyze, inspect, edit, preview,
  export, settings, and provider management.
- Responsive layouts for desktop and mobile.

## Do

- Build the actual product workflow, not a marketing landing page.
- Keep dense operational screens readable and efficient.
- Use native platform conventions where they matter: file pickers, sharing,
  permissions, audio input, and export destinations.
- Test text fitting and layout at mobile and desktop sizes.

## Avoid

- Do not parse CLI stdout for product state.
- Do not mix native FFI calls directly into widgets.
- Do not make UI copy unlocalizable.

## Acceptance

- Widget tests cover major workflows and breakpoints.
- UI remains usable without network-only model features.
- Desktop and mobile interaction paths reach the same core capabilities.
