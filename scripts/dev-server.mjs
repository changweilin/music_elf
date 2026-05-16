import { createReadStream, existsSync, statSync } from "node:fs";
import { createServer } from "node:http";
import { extname, join, normalize, resolve, sep } from "node:path";
import { networkInterfaces } from "node:os";

const args = new Map();
for (let index = 2; index < process.argv.length; index += 1) {
  const arg = process.argv[index];
  if (!arg.startsWith("--")) {
    continue;
  }

  const key = arg.slice(2);
  const next = process.argv[index + 1];
  if (!next || next.startsWith("--")) {
    args.set(key, "true");
    continue;
  }

  args.set(key, next);
  index += 1;
}

const host = args.get("host") ?? process.env.HOST ?? "0.0.0.0";
const preferredPort = Number(args.get("port") ?? process.env.PORT ?? 5173);
const root = resolve(args.get("root") ?? "docs/conductor-studio");
const strictPort = args.has("strict-port");

const mimeTypes = new Map([
  [".css", "text/css; charset=utf-8"],
  [".gif", "image/gif"],
  [".html", "text/html; charset=utf-8"],
  [".ico", "image/x-icon"],
  [".jpeg", "image/jpeg"],
  [".jpg", "image/jpeg"],
  [".js", "text/javascript; charset=utf-8"],
  [".jsx", "text/babel; charset=utf-8"],
  [".json", "application/json; charset=utf-8"],
  [".map", "application/json; charset=utf-8"],
  [".mjs", "text/javascript; charset=utf-8"],
  [".png", "image/png"],
  [".svg", "image/svg+xml; charset=utf-8"],
  [".txt", "text/plain; charset=utf-8"],
  [".webp", "image/webp"],
]);

function send(res, status, body, contentType = "text/plain; charset=utf-8") {
  res.writeHead(status, {
    "content-type": contentType,
    "cache-control": "no-store",
  });
  res.end(body);
}

function isInsideRoot(filePath) {
  const normalizedRoot = root.endsWith(sep) ? root : `${root}${sep}`;
  return filePath === root || filePath.startsWith(normalizedRoot);
}

function resolveRequestPath(url) {
  const requestUrl = new URL(url, "http://localhost");
  const decodedPath = decodeURIComponent(requestUrl.pathname);
  const relativePath = normalize(decodedPath).replace(/^([/\\])+/, "");
  const candidate = resolve(root, relativePath);

  if (!isInsideRoot(candidate)) {
    return null;
  }

  if (existsSync(candidate) && statSync(candidate).isDirectory()) {
    return join(candidate, "index.html");
  }

  return candidate;
}

function tailscaleAddresses() {
  return Object.values(networkInterfaces())
    .flat()
    .filter(Boolean)
    .filter((entry) => entry.family === "IPv4" && entry.address.startsWith("100."))
    .map((entry) => entry.address);
}

function localAddresses() {
  return Object.values(networkInterfaces())
    .flat()
    .filter(Boolean)
    .filter((entry) => entry.family === "IPv4" && !entry.internal)
    .map((entry) => entry.address);
}

function createStaticServer() {
  return createServer((req, res) => {
    const filePath = resolveRequestPath(req.url ?? "/");
    if (!filePath) {
      send(res, 403, "Forbidden");
      return;
    }

    if (!existsSync(filePath) || !statSync(filePath).isFile()) {
      send(res, 404, "Not found");
      return;
    }

    const contentType = mimeTypes.get(extname(filePath).toLowerCase()) ?? "application/octet-stream";
    res.writeHead(200, {
      "content-type": contentType,
      "cache-control": "no-store",
    });
    createReadStream(filePath).pipe(res);
  });
}

function listen(port) {
  const server = createStaticServer();

  server.on("error", (error) => {
    if (error.code === "EADDRINUSE" && !strictPort && port < preferredPort + 20) {
      listen(port + 1);
      return;
    }

    console.error(error.message);
    process.exitCode = 1;
  });

  server.listen(port, host, () => {
    const printedHost = host === "0.0.0.0" ? "localhost" : host;
    console.log(`Music Elf dev server`);
    console.log(`Root: ${root}`);
    console.log(`Local: http://${printedHost}:${port}/`);

    for (const address of tailscaleAddresses()) {
      console.log(`Tailscale: http://${address}:${port}/`);
    }

    const lanOnly = localAddresses().filter((address) => !address.startsWith("100."));
    for (const address of lanOnly) {
      console.log(`LAN: http://${address}:${port}/`);
    }
  });
}

listen(preferredPort);
