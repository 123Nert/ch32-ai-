const http = require("http");
const fs = require("fs");
const path = require("path");
const { SerialPort } = require("serialport");

const HOST = "127.0.0.1";
const PORT = Number(process.env.PORT || 5173);
const ROOT = __dirname;
const WCH_VENDOR_ID = "1A86";
const CH340_PRODUCTS = new Set(["7523", "5523"]);

let activePort = null;
let activeInfo = null;
let rxBytes = 0;
let txBytes = 0;
const clients = new Set();

const contentTypes = {
  ".html": "text/html;charset=utf-8",
  ".css": "text/css;charset=utf-8",
  ".js": "application/javascript;charset=utf-8",
  ".json": "application/json;charset=utf-8",
  ".png": "image/png",
  ".svg": "image/svg+xml",
};

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET,POST,OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type",
};

function normalizeId(value) {
  return value ? String(value).toUpperCase().padStart(4, "0") : "";
}

function isCh340(port) {
  const vendorId = normalizeId(port.vendorId);
  const productId = normalizeId(port.productId);
  const haystack = [
    port.path,
    port.manufacturer,
    port.friendlyName,
    port.pnpId,
    port.serialNumber,
  ]
    .filter(Boolean)
    .join(" ")
    .toUpperCase();

  return (
    (vendorId === WCH_VENDOR_ID && CH340_PRODUCTS.has(productId)) ||
    haystack.includes("CH340") ||
    haystack.includes("CH341")
  );
}

function deviceName(port) {
  const vendorId = normalizeId(port.vendorId);
  const productId = normalizeId(port.productId);
  const haystack = [
    port.manufacturer,
    port.friendlyName,
    port.pnpId,
  ]
    .filter(Boolean)
    .join(" ")
    .toUpperCase();

  if (isCh340(port)) return "CH340/CH341 USB转TTL";
  if (vendorId === WCH_VENDOR_ID || haystack.includes("WCH")) return "WCH USB串口";
  if (port.friendlyName) return port.friendlyName.replace(/\s*\(COM\d+\)\s*/i, "");
  if (port.manufacturer) return port.manufacturer;
  return "串口设备";
}

function normalizePort(port) {
  const vendorId = normalizeId(port.vendorId);
  const productId = normalizeId(port.productId);
  const name = deviceName(port);
  const idText = vendorId ? `VID ${vendorId}${productId ? ` PID ${productId}` : ""}` : "";

  return {
    path: port.path,
    name,
    label: `${port.path} - ${name}${idText ? ` (${idText})` : ""}`,
    manufacturer: port.manufacturer || "",
    serialNumber: port.serialNumber || "",
    vendorId,
    productId,
    isCh340: isCh340(port),
  };
}

async function listPorts() {
  const ports = await SerialPort.list();
  return ports.map(normalizePort).sort((a, b) => a.path.localeCompare(b.path, "zh-CN", { numeric: true }));
}

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(statusCode, {
    "Content-Type": "application/json;charset=utf-8",
    "Content-Length": Buffer.byteLength(body),
    ...corsHeaders,
  });
  res.end(body);
}

function readBody(req) {
  return new Promise((resolve, reject) => {
    let body = "";
    req.on("data", (chunk) => {
      body += chunk;
      if (body.length > 1024 * 1024) {
        reject(new Error("请求内容过大"));
        req.destroy();
      }
    });
    req.on("end", () => {
      if (!body) {
        resolve({});
        return;
      }

      try {
        resolve(JSON.parse(body));
      } catch {
        reject(new Error("JSON 格式错误"));
      }
    });
    req.on("error", reject);
  });
}

function broadcast(event, payload) {
  const data = `event: ${event}\ndata: ${JSON.stringify(payload)}\n\n`;
  for (const client of clients) {
    client.write(data);
  }
}

function statusPayload() {
  return {
    connected: Boolean(activePort?.isOpen),
    port: activePort?.isOpen ? activeInfo : null,
    rxBytes,
    txBytes,
  };
}

function userMessage(error) {
  const message = error?.message || String(error);
  if (/access denied/i.test(message)) {
    return "串口被占用或未释放，请关闭其它串口助手、Arduino Serial Monitor、Keil 调试窗口、Python 串口程序后再连接";
  }
  return message;
}

async function closeActivePort() {
  if (!activePort) return;

  const portToClose = activePort;
  activePort = null;
  activeInfo = null;

  if (portToClose.isOpen) {
    await new Promise((resolve) => {
      portToClose.close(() => resolve());
    });
  }
}

async function handleConnect(req, res) {
  const body = await readBody(req);
  if (!body.path) throw new Error("请选择 COM 口");

  const options = {
    path: body.path,
    baudRate: Number(body.baudRate || 115200),
    dataBits: Number(body.dataBits || 8),
    stopBits: Number(body.stopBits || 1),
    parity: body.parity || "none",
    rtscts: body.flowControl === "hardware",
    autoOpen: false,
  };

  if (!Number.isInteger(options.baudRate) || options.baudRate <= 0) {
    throw new Error("波特率必须是大于 0 的整数");
  }

  await closeActivePort();
  const ports = await listPorts();
  activeInfo = ports.find((port) => port.path === body.path) || { path: body.path, label: body.path };
  activePort = new SerialPort(options);

  activePort.on("data", (chunk) => {
    rxBytes += chunk.length;
    broadcast("serial-data", {
      text: chunk.toString("utf8"),
      bytes: chunk.length,
      rxBytes,
    });
  });

  activePort.on("error", (error) => {
    broadcast("serial-error", { message: error.message });
  });

  activePort.on("close", () => {
    broadcast("serial-status", statusPayload());
  });

  try {
    await new Promise((resolve, reject) => {
      activePort.open((error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  } catch (error) {
    activePort.removeAllListeners();
    activePort = null;
    activeInfo = null;
    throw new Error(userMessage(error));
  }

  broadcast("serial-status", statusPayload());
  sendJson(res, 200, statusPayload());
}

async function handleDisconnect(res) {
  await closeActivePort();
  broadcast("serial-status", statusPayload());
  sendJson(res, 200, statusPayload());
}

async function handleSend(req, res) {
  const body = await readBody(req);
  if (!activePort?.isOpen) throw new Error("串口未连接");

  const text = String(body.text || "");
  const bytes = Buffer.from(text, "utf8");
  await new Promise((resolve, reject) => {
    activePort.write(bytes, (error) => {
      if (error) reject(error);
      else activePort.drain((drainError) => (drainError ? reject(drainError) : resolve()));
    });
  });

  txBytes += bytes.length;
  broadcast("serial-status", statusPayload());
  sendJson(res, 200, { txBytes, bytes: bytes.length });
}

function handleEvents(req, res) {
  res.writeHead(200, {
    "Content-Type": "text/event-stream;charset=utf-8",
    "Cache-Control": "no-cache",
    Connection: "keep-alive",
    ...corsHeaders,
  });
  res.write(`event: serial-status\ndata: ${JSON.stringify(statusPayload())}\n\n`);
  clients.add(res);
  req.on("close", () => clients.delete(res));
}

async function handleApi(req, res, pathname) {
  try {
    if (req.method === "GET" && pathname === "/api/ports") {
      sendJson(res, 200, { ports: await listPorts() });
      return;
    }

    if (req.method === "GET" && pathname === "/api/status") {
      sendJson(res, 200, statusPayload());
      return;
    }

    if (req.method === "GET" && pathname === "/api/events") {
      handleEvents(req, res);
      return;
    }

    if (req.method === "POST" && pathname === "/api/connect") {
      await handleConnect(req, res);
      return;
    }

    if (req.method === "POST" && pathname === "/api/disconnect") {
      await handleDisconnect(res);
      return;
    }

    if (req.method === "POST" && pathname === "/api/send") {
      await handleSend(req, res);
      return;
    }

    sendJson(res, 404, { message: "接口不存在" });
  } catch (error) {
    sendJson(res, 400, { message: userMessage(error) });
  }
}

function serveStatic(req, res, pathname) {
  const safePath = pathname === "/" ? "/index.html" : pathname;
  const filePath = path.normalize(path.join(ROOT, safePath));

  if (!filePath.startsWith(ROOT)) {
    sendJson(res, 403, { message: "Forbidden" });
    return;
  }

  fs.readFile(filePath, (error, data) => {
    if (error) {
      sendJson(res, 404, { message: "File not found" });
      return;
    }

    const ext = path.extname(filePath).toLowerCase();
    res.writeHead(200, {
      "Content-Type": contentTypes[ext] || "application/octet-stream",
    });
    res.end(data);
  });
}

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  if (req.method === "OPTIONS") {
    res.writeHead(204, corsHeaders);
    res.end();
    return;
  }

  if (url.pathname.startsWith("/api/")) {
    handleApi(req, res, url.pathname);
    return;
  }

  serveStatic(req, res, decodeURIComponent(url.pathname));
});

server.listen(PORT, HOST, () => {
  console.log(`串口助手已启动：http://${HOST}:${PORT}/`);
});

process.on("SIGINT", async () => {
  await closeActivePort();
  process.exit(0);
});
