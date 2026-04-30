const els = {
  supportText: document.querySelector("#supportText"),
  baudSelect: document.querySelector("#baudSelect"),
  baudInput: document.querySelector("#baudInput"),
  dataBits: document.querySelector("#dataBits"),
  stopBits: document.querySelector("#stopBits"),
  parity: document.querySelector("#parity"),
  flowControl: document.querySelector("#flowControl"),
  portSelect: document.querySelector("#portSelect"),
  refreshPortsBtn: document.querySelector("#refreshPortsBtn"),
  connectBtn: document.querySelector("#connectBtn"),
  disconnectBtn: document.querySelector("#disconnectBtn"),
  connectionStatus: document.querySelector("#connectionStatus"),
  portInfo: document.querySelector("#portInfo"),
  byteStats: document.querySelector("#byteStats"),
  sampleStats: document.querySelector("#sampleStats"),
  timestampToggle: document.querySelector("#timestampToggle"),
  autoscrollToggle: document.querySelector("#autoscrollToggle"),
  clearLogBtn: document.querySelector("#clearLogBtn"),
  exportLogBtn: document.querySelector("#exportLogBtn"),
  logView: document.querySelector("#logView"),
  sendInput: document.querySelector("#sendInput"),
  lineEnding: document.querySelector("#lineEnding"),
  echoTxToggle: document.querySelector("#echoTxToggle"),
  sendBtn: document.querySelector("#sendBtn"),
  pauseWaveBtn: document.querySelector("#pauseWaveBtn"),
  clearWaveBtn: document.querySelector("#clearWaveBtn"),
  autoScaleToggle: document.querySelector("#autoScaleToggle"),
  yMinInput: document.querySelector("#yMinInput"),
  yMaxInput: document.querySelector("#yMaxInput"),
  maxPointsInput: document.querySelector("#maxPointsInput"),
  waveCanvas: document.querySelector("#waveCanvas"),
  emptyWaveHint: document.querySelector("#emptyWaveHint"),
  channelList: document.querySelector("#channelList"),
};

const colors = [
  "#0e8a72",
  "#2867b2",
  "#b46b00",
  "#b43a3a",
  "#6a55b8",
  "#247a3a",
  "#9d3b78",
  "#4a6f7c",
];

const state = {
  port: null,
  reader: null,
  writer: null,
  keepReading: false,
  connected: false,
  ports: [],
  eventSource: null,
  rxBytes: 0,
  txBytes: 0,
  sampleIndex: 0,
  pendingLine: "",
  pendingLog: "",
  logFlushScheduled: false,
  logAtLineStart: true,
  chartFrame: 0,
  wavePaused: false,
  channels: new Map(),
};

const LOG_CHAR_LIMIT = 100000;
const API_BASE = window.location.protocol === "file:" ? "http://127.0.0.1:5173" : "";

function apiUrl(path) {
  return `${API_BASE}${path}`;
}

function setConnectionState(status, message = "") {
  els.connectionStatus.classList.remove("is-idle", "is-connected", "is-error");

  if (status === "connected") {
    els.connectionStatus.textContent = "已连接";
    els.connectionStatus.classList.add("is-connected");
  } else if (status === "error") {
    els.connectionStatus.textContent = "错误";
    els.connectionStatus.classList.add("is-error");
  } else {
    els.connectionStatus.textContent = "未连接";
    els.connectionStatus.classList.add("is-idle");
  }

  if (message) {
    els.portInfo.textContent = message;
  }
}

function updateButtons() {
  els.connectBtn.disabled = state.connected || !els.portSelect.value;
  els.disconnectBtn.disabled = !state.connected;
  els.sendBtn.disabled = !state.connected;
  els.refreshPortsBtn.disabled = state.connected;
}

function getLineEnding() {
  const value = els.lineEnding.value;
  if (value === "\\n") return "\n";
  if (value === "\\r\\n") return "\r\n";
  if (value === "\\r") return "\r";
  return "";
}

function getSerialOptions() {
  const baudRate = Number(els.baudInput.value || els.baudSelect.value || 115200);
  if (!Number.isInteger(baudRate) || baudRate <= 0) {
    throw new Error("波特率必须是大于 0 的整数");
  }

  return {
    baudRate,
    dataBits: Number(els.dataBits.value),
    stopBits: Number(els.stopBits.value),
    parity: els.parity.value,
    flowControl: els.flowControl.value,
    bufferSize: 255,
  };
}

function nowText() {
  const now = new Date();
  const h = String(now.getHours()).padStart(2, "0");
  const m = String(now.getMinutes()).padStart(2, "0");
  const s = String(now.getSeconds()).padStart(2, "0");
  const ms = String(now.getMilliseconds()).padStart(3, "0");
  return `${h}:${m}:${s}.${ms}`;
}

function formatLogChunk(text) {
  if (!els.timestampToggle.checked) return text;

  let formatted = "";
  for (const char of text) {
    if (state.logAtLineStart && char !== "\r" && char !== "\n") {
      formatted += `[${nowText()}] `;
      state.logAtLineStart = false;
    }

    formatted += char;

    if (char === "\n") {
      state.logAtLineStart = true;
    } else if (char !== "\r") {
      state.logAtLineStart = false;
    }
  }
  return formatted;
}

function appendLog(text, kind = "rx") {
  const prefix = kind === "tx" ? "[TX] " : kind === "sys" ? "[SYS] " : "";
  state.pendingLog += prefix + formatLogChunk(text);

  if (!state.logFlushScheduled) {
    state.logFlushScheduled = true;
    requestAnimationFrame(flushLog);
  }
}

function flushLog() {
  state.logFlushScheduled = false;
  if (!state.pendingLog) return;

  els.logView.textContent += state.pendingLog;
  state.pendingLog = "";

  if (els.logView.textContent.length > LOG_CHAR_LIMIT) {
    els.logView.textContent = els.logView.textContent.slice(-LOG_CHAR_LIMIT);
  }

  if (els.autoscrollToggle.checked) {
    els.logView.scrollTop = els.logView.scrollHeight;
  }
}

function updateStats() {
  els.byteStats.textContent = `RX ${state.rxBytes} B / TX ${state.txBytes} B`;
  els.sampleStats.textContent = `波形点 ${state.sampleIndex}`;
}

async function apiFetch(url, options = {}) {
  const response = await fetch(apiUrl(url), {
    headers: {
      "Content-Type": "application/json",
      ...(options.headers || {}),
    },
    ...options,
  });
  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.message || "请求失败");
  }
  return data;
}

function selectedPortLabel() {
  const port = state.ports.find((item) => item.path === els.portSelect.value);
  return port?.label || els.portSelect.value || "未选择 COM 口";
}

function renderPorts() {
  els.portSelect.innerHTML = "";

  if (state.ports.length === 0) {
    const option = document.createElement("option");
    option.value = "";
    option.textContent = "未识别到 COM 口";
    els.portSelect.append(option);
    setConnectionState("idle", "未识别到 COM 口，请插入 USB 转 TTL 后刷新");
    updateButtons();
    return;
  }

  for (const port of state.ports) {
    const option = document.createElement("option");
    option.value = port.path;
    option.textContent = port.isCh340 ? `${port.label}  [CH340]` : port.label;
    els.portSelect.append(option);
  }

  const ch340 = state.ports.find((port) => port.isCh340);
  if (ch340) {
    els.portSelect.value = ch340.path;
    setConnectionState("idle", `识别到 CH340：${ch340.label}`);
  } else {
    setConnectionState("idle", `已识别 ${state.ports.length} 个 COM 口`);
  }
  updateButtons();
}

async function loadPorts() {
  const previous = els.portSelect.value;
  els.refreshPortsBtn.disabled = true;
  els.portSelect.innerHTML = '<option value="">正在扫描 COM 口...</option>';

  try {
    const data = await apiFetch("/api/ports");
    state.ports = data.ports || [];
    renderPorts();
    if (previous && state.ports.some((port) => port.path === previous)) {
      els.portSelect.value = previous;
    }
    updateButtons();
  } catch (error) {
    setConnectionState("error", `扫描 COM 口失败：${error.message}`);
    appendLog(`扫描 COM 口失败：${error.message}\n`, "sys");
  } finally {
    els.refreshPortsBtn.disabled = state.connected;
  }
}

function applyServerStatus(payload) {
  state.connected = Boolean(payload.connected);
  state.rxBytes = Number(payload.rxBytes || 0);
  state.txBytes = Number(payload.txBytes || 0);

  if (state.connected) {
    const label = payload.port?.label || selectedPortLabel();
    setConnectionState("connected", `${label} 已连接`);
  } else if (els.portSelect.value) {
    setConnectionState("idle", "串口已断开");
  }

  updateStats();
  updateButtons();
}

function setupEventStream() {
  if (state.eventSource) {
    state.eventSource.close();
  }

  state.eventSource = new EventSource(apiUrl("/api/events"));
  state.eventSource.addEventListener("serial-data", (event) => {
    const payload = JSON.parse(event.data);
    state.rxBytes = Number(payload.rxBytes || state.rxBytes + (payload.bytes || 0));
    appendLog(payload.text || "");
    handleWaveText(payload.text || "");
    updateStats();
  });

  state.eventSource.addEventListener("serial-status", (event) => {
    applyServerStatus(JSON.parse(event.data));
  });

  state.eventSource.addEventListener("serial-error", (event) => {
    const payload = JSON.parse(event.data);
    setConnectionState("error", payload.message || "串口异常");
    appendLog(`串口异常：${payload.message || "未知错误"}\n`, "sys");
  });

  state.eventSource.onerror = () => {
    setConnectionState("error", "本地串口服务连接中断");
  };
}

async function connectSerial() {
  try {
    const options = getSerialOptions();
    const path = els.portSelect.value;
    if (!path) throw new Error("请选择 COM 口");

    setConnectionState("idle", `正在连接 ${selectedPortLabel()}`);
    const status = await apiFetch("/api/connect", {
      method: "POST",
      body: JSON.stringify({
        path,
        ...options,
      }),
    });

    applyServerStatus(status);
    clearWave();
    appendLog(`串口已连接：${selectedPortLabel()}，baud=${options.baudRate}\n`, "sys");
  } catch (error) {
    setConnectionState("error", error.message || "串口连接失败");
    appendLog(`连接失败：${error.message || error}\n`, "sys");
    state.connected = false;
    updateButtons();
  }
}

async function disconnectSerial() {
  try {
    const status = await apiFetch("/api/disconnect", { method: "POST", body: "{}" });
    applyServerStatus(status);
    appendLog("串口已断开\n", "sys");
  } catch (error) {
    setConnectionState("error", error.message || "断开失败");
    appendLog(`断开失败：${error.message || error}\n`, "sys");
  }
}

async function sendSerial() {
  const rawText = els.sendInput.value;
  if (!state.connected || !rawText) return;

  const payload = rawText + getLineEnding();
  try {
    const result = await apiFetch("/api/send", {
      method: "POST",
      body: JSON.stringify({ text: payload }),
    });
    state.txBytes = Number(result.txBytes || state.txBytes);
    updateStats();

    if (els.echoTxToggle.checked) {
      appendLog(payload.endsWith("\n") ? payload : `${payload}\n`, "tx");
    }

    els.sendInput.select();
  } catch (error) {
    setConnectionState("error", error.message || "发送失败");
    appendLog(`发送失败：${error.message || error}\n`, "sys");
  }
}

function parseNumericLine(line) {
  const trimmed = line.trim();
  if (!trimmed) return [];

  const pairs = [];
  const covered = [];
  const pairRegex = /([A-Za-z_\u4e00-\u9fa5][\w\u4e00-\u9fa5-]*)\s*[:=]\s*(-?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)/g;
  let pairMatch;
  while ((pairMatch = pairRegex.exec(trimmed))) {
    const value = Number(pairMatch[2]);
    if (Number.isFinite(value)) {
      pairs.push({
        label: pairMatch[1],
        value,
      });
      covered.push([pairMatch.index, pairRegex.lastIndex]);
    }
  }

  let rest = "";
  for (let i = 0; i < trimmed.length; i += 1) {
    const insideCovered = covered.some(([start, end]) => i >= start && i < end);
    rest += insideCovered ? " " : trimmed[i];
  }

  const unnamed = [];
  const numberRegex = /-?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?/g;
  let numberMatch;
  while ((numberMatch = numberRegex.exec(rest))) {
    const before = rest[numberMatch.index - 1] || "";
    const after = rest[numberRegex.lastIndex] || "";
    if (/[A-Za-z_\u4e00-\u9fa5]/.test(before) || /[A-Za-z_\u4e00-\u9fa5]/.test(after)) {
      continue;
    }

    const value = Number(numberMatch[0]);
    if (Number.isFinite(value)) {
      unnamed.push(value);
    }
  }

  const values = [...pairs];
  unnamed.forEach((value, index) => {
    values.push({
      label: `CH${pairs.length + index + 1}`,
      value,
    });
  });

  return values;
}

function handleWaveText(text) {
  if (state.wavePaused) return;

  state.pendingLine += text;
  const lines = state.pendingLine.split(/\r?\n/);
  state.pendingLine = lines.pop() || "";

  for (const line of lines) {
    const values = parseNumericLine(line);
    if (values.length > 0) {
      addWaveSample(values);
    }
  }
}

function maxPointCount() {
  const value = Number(els.maxPointsInput.value);
  if (!Number.isInteger(value) || value < 50) return 1000;
  return Math.min(value, 20000);
}

function ensureChannel(label) {
  if (state.channels.has(label)) {
    return state.channels.get(label);
  }

  const channel = {
    label,
    color: colors[state.channels.size % colors.length],
    visible: true,
    points: [],
    lastValue: 0,
  };
  state.channels.set(label, channel);
  renderChannels();
  return channel;
}

function addWaveSample(values) {
  state.sampleIndex += 1;
  const limit = maxPointCount();

  for (const item of values) {
    const channel = ensureChannel(item.label);
    channel.lastValue = item.value;
    channel.points.push({
      x: state.sampleIndex,
      y: item.value,
    });

    if (channel.points.length > limit) {
      channel.points.splice(0, channel.points.length - limit);
    }
  }

  els.emptyWaveHint.classList.toggle("is-hidden", state.sampleIndex > 0);
  updateChannelLabels();
  updateStats();
  requestChartDraw();
}

function renderChannels() {
  els.channelList.innerHTML = "";

  if (state.channels.size === 0) {
    els.channelList.textContent = "暂无通道";
    return;
  }

  for (const channel of state.channels.values()) {
    const label = document.createElement("label");
    label.className = "channel-item";

    const checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.checked = channel.visible;
    checkbox.addEventListener("change", () => {
      channel.visible = checkbox.checked;
      requestChartDraw();
    });

    const swatch = document.createElement("span");
    swatch.className = "swatch";
    swatch.style.background = channel.color;

    const name = document.createElement("span");
    name.dataset.channelLabel = channel.label;
    name.textContent = `${channel.label}: ${formatValue(channel.lastValue)}`;

    label.append(checkbox, swatch, name);
    els.channelList.append(label);
  }
}

function updateChannelLabels() {
  for (const node of els.channelList.querySelectorAll("[data-channel-label]")) {
    const channel = state.channels.get(node.dataset.channelLabel);
    if (channel) {
      node.textContent = `${channel.label}: ${formatValue(channel.lastValue)}`;
    }
  }
}

function formatValue(value) {
  if (!Number.isFinite(value)) return "--";
  const abs = Math.abs(value);
  if (abs >= 1000000 || (abs > 0 && abs < 0.001)) {
    return value.toExponential(2);
  }
  if (abs >= 1000) {
    return value.toFixed(0);
  }
  if (abs >= 100) {
    return value.toFixed(1).replace(/\.0$/, "");
  }
  return value.toFixed(3).replace(/\.?0+$/, "");
}

function resizeCanvasToDisplaySize() {
  const canvas = els.waveCanvas;
  const rect = canvas.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  const width = Math.max(1, Math.round(rect.width * dpr));
  const height = Math.max(1, Math.round(rect.height * dpr));

  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
  }

  return {
    width,
    height,
    dpr,
  };
}

function requestChartDraw() {
  if (state.chartFrame) return;
  state.chartFrame = requestAnimationFrame(drawChart);
}

function getVisiblePoints() {
  const points = [];
  for (const channel of state.channels.values()) {
    if (!channel.visible) continue;
    for (const point of channel.points) {
      points.push(point);
    }
  }
  return points;
}

function getRange(points) {
  const limit = maxPointCount();
  const maxX = points.length ? Math.max(...points.map((point) => point.x)) : Math.max(1, state.sampleIndex);
  const minX = Math.max(0, maxX - limit + 1);

  let minY = Number(els.yMinInput.value);
  let maxY = Number(els.yMaxInput.value);

  if (els.autoScaleToggle.checked && points.length) {
    const visiblePoints = points.filter((point) => point.x >= minX && point.x <= maxX);
    minY = Math.min(...visiblePoints.map((point) => point.y));
    maxY = Math.max(...visiblePoints.map((point) => point.y));
  }

  if (!Number.isFinite(minY) || !Number.isFinite(maxY) || minY === maxY) {
    const center = Number.isFinite(minY) ? minY : 0;
    minY = center - 1;
    maxY = center + 1;
  }

  const padding = (maxY - minY) * 0.08 || 1;
  return {
    minX,
    maxX: Math.max(maxX, minX + 1),
    minY: minY - padding,
    maxY: maxY + padding,
  };
}

function drawChart() {
  state.chartFrame = 0;
  const canvas = els.waveCanvas;
  const ctx = canvas.getContext("2d");
  const { width, height, dpr } = resizeCanvasToDisplaySize();

  ctx.clearRect(0, 0, width, height);
  ctx.save();
  ctx.scale(dpr, dpr);

  const cssWidth = width / dpr;
  const cssHeight = height / dpr;
  const pad = {
    left: 58,
    right: 18,
    top: 18,
    bottom: 34,
  };
  const plotW = Math.max(1, cssWidth - pad.left - pad.right);
  const plotH = Math.max(1, cssHeight - pad.top - pad.bottom);
  const points = getVisiblePoints();
  const range = getRange(points);

  ctx.fillStyle = "#fbfdff";
  ctx.fillRect(0, 0, cssWidth, cssHeight);

  drawGrid(ctx, pad, plotW, plotH, range);

  for (const channel of state.channels.values()) {
    if (!channel.visible || channel.points.length === 0) continue;
    drawChannel(ctx, channel, pad, plotW, plotH, range);
  }

  drawLegend(ctx, cssWidth, pad);
  ctx.restore();
}

function drawGrid(ctx, pad, plotW, plotH, range) {
  ctx.strokeStyle = "#d8e0e7";
  ctx.lineWidth = 1;
  ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
  ctx.fillStyle = "#667481";

  for (let i = 0; i <= 5; i += 1) {
    const y = pad.top + (plotH / 5) * i;
    ctx.beginPath();
    ctx.moveTo(pad.left, y);
    ctx.lineTo(pad.left + plotW, y);
    ctx.stroke();

    const value = range.maxY - ((range.maxY - range.minY) / 5) * i;
    ctx.fillText(formatValue(value), 8, y + 4);
  }

  for (let i = 0; i <= 6; i += 1) {
    const x = pad.left + (plotW / 6) * i;
    ctx.beginPath();
    ctx.moveTo(x, pad.top);
    ctx.lineTo(x, pad.top + plotH);
    ctx.stroke();
  }

  ctx.strokeStyle = "#9aa7b3";
  ctx.strokeRect(pad.left, pad.top, plotW, plotH);
}

function drawChannel(ctx, channel, pad, plotW, plotH, range) {
  const toX = (x) => pad.left + ((x - range.minX) / (range.maxX - range.minX)) * plotW;
  const toY = (y) => pad.top + plotH - ((y - range.minY) / (range.maxY - range.minY)) * plotH;
  const visible = channel.points.filter((point) => point.x >= range.minX && point.x <= range.maxX);
  if (visible.length === 0) return;

  ctx.beginPath();
  visible.forEach((point, index) => {
    const x = toX(point.x);
    const y = toY(point.y);
    if (index === 0) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);
    }
  });

  ctx.strokeStyle = channel.color;
  ctx.lineWidth = 2;
  ctx.stroke();

  const last = visible[visible.length - 1];
  ctx.fillStyle = channel.color;
  ctx.beginPath();
  ctx.arc(toX(last.x), toY(last.y), 3, 0, Math.PI * 2);
  ctx.fill();
}

function drawLegend(ctx, cssWidth, pad) {
  const visible = [...state.channels.values()].filter((channel) => channel.visible);
  if (visible.length === 0) return;

  let x = pad.left;
  const y = 14;
  ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";

  for (const channel of visible.slice(0, 5)) {
    const text = `${channel.label} ${formatValue(channel.lastValue)}`;
    ctx.fillStyle = channel.color;
    ctx.fillRect(x, y - 8, 10, 10);
    ctx.fillStyle = "#1d252d";
    ctx.fillText(text, x + 15, y + 1);
    x += ctx.measureText(text).width + 36;
    if (x > cssWidth - 120) break;
  }
}

function clearWave() {
  state.channels.clear();
  state.sampleIndex = 0;
  state.pendingLine = "";
  renderChannels();
  els.emptyWaveHint.classList.remove("is-hidden");
  updateStats();
  requestChartDraw();
}

function exportLog() {
  const blob = new Blob([els.logView.textContent], { type: "text/plain;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  const stamp = new Date().toISOString().replace(/[:.]/g, "-");
  link.href = url;
  link.download = `serial-log-${stamp}.txt`;
  link.click();
  URL.revokeObjectURL(url);
}

function init() {
  els.supportText.textContent = "本地服务会自动扫描全部 COM 口";

  els.baudSelect.addEventListener("change", () => {
    if (els.baudSelect.value !== "custom") {
      els.baudInput.value = els.baudSelect.value;
    } else {
      els.baudInput.focus();
      els.baudInput.select();
    }
  });

  els.baudInput.addEventListener("input", () => {
    const matched = [...els.baudSelect.options].some((option) => option.value === els.baudInput.value);
    if (!matched) {
      els.baudSelect.value = "custom";
    }
  });

  els.connectBtn.addEventListener("click", connectSerial);
  els.disconnectBtn.addEventListener("click", disconnectSerial);
  els.refreshPortsBtn.addEventListener("click", loadPorts);
  els.portSelect.addEventListener("change", () => {
    setConnectionState("idle", `已选择：${selectedPortLabel()}`);
    updateButtons();
  });
  els.sendBtn.addEventListener("click", sendSerial);
  els.sendInput.addEventListener("keydown", (event) => {
    if (event.key === "Enter") {
      sendSerial();
    }
  });

  els.clearLogBtn.addEventListener("click", () => {
    els.logView.textContent = "";
    state.pendingLog = "";
    state.logAtLineStart = true;
  });
  els.exportLogBtn.addEventListener("click", exportLog);

  els.pauseWaveBtn.addEventListener("click", () => {
    state.wavePaused = !state.wavePaused;
    els.pauseWaveBtn.textContent = state.wavePaused ? "继续" : "暂停";
  });
  els.clearWaveBtn.addEventListener("click", clearWave);
  els.autoScaleToggle.addEventListener("change", requestChartDraw);
  els.yMinInput.addEventListener("input", requestChartDraw);
  els.yMaxInput.addEventListener("input", requestChartDraw);
  els.maxPointsInput.addEventListener("input", requestChartDraw);

  window.addEventListener("resize", requestChartDraw);
  if ("ResizeObserver" in window) {
    new ResizeObserver(requestChartDraw).observe(els.waveCanvas);
  }

  setupEventStream();
  loadPorts();
  renderChannels();
  updateStats();
  updateButtons();
  requestChartDraw();
}

init();
