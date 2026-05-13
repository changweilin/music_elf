# Conductor Studio frontend demo

This folder contains the standalone Music Elf Conductor Studio UI demo.

## Open locally

From the repo root, start the dev server:

```powershell
npm.cmd run dev
```

The server binds to `0.0.0.0` so phones connected to the same Tailscale tailnet can open the printed `http://100.x.x.x:5299/` link.

You can still open the standalone file directly:

Open `index.html` in a browser from this directory:

```powershell
Start-Process .\docs\conductor-studio\index.html
```

The page loads React, ReactDOM, Google Fonts, and Babel from CDNs, then runs the local `tweaks-panel.jsx` and `conductor.jsx` files in the browser.

## Files

- `index.html`: Static entry point for the demo.
- `conductor.css`: Visual styling for the studio UI.
- `conductor.jsx`: Main React app.
- `tweaks-panel.jsx`: Reusable tweak/debug controls used by the demo.

## Notes

This is integrated as a documentation/demo asset rather than as part of the C++ CMake build. If we later want a production frontend bundle, the next step is to add a dedicated Vite/React app and compile the JSX instead of relying on in-browser Babel.
