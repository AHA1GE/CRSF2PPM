#pragma once

// Single-page web UI. Kept in flash and served with send_P().
static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CRSF2PPM</title>
<style>
:root{
  --bg:#0a0e13;--card:#111821;--card2:#0d141c;--line:#1e2a36;--text:#e6edf5;
  --muted:#7d8fa3;--accent:#ff8a3d;--accent2:#38bdf8;--ok:#2dd48f;--warn:#f5b544;
  --bad:#f4566a;--mono:ui-monospace,SFMono-Regular,Menlo,Consolas,"Liberation Mono",monospace;
}
*{box-sizing:border-box}
html,body{margin:0}
body{background:radial-gradient(1200px 620px at 82% -12%,#142231 0%,var(--bg) 56%);
  color:var(--text);font-family:system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
  min-height:100vh;padding-bottom:96px}
header{display:flex;justify-content:space-between;align-items:center;gap:16px;
  padding:18px clamp(16px,4vw,40px);border-bottom:1px solid var(--line);
  background:rgba(10,14,19,.72);backdrop-filter:blur(10px);position:sticky;top:0;z-index:10}
.brand{display:flex;align-items:center;gap:14px}
.mark{width:46px;height:46px;border-radius:12px;display:grid;place-items:center;
  background:linear-gradient(135deg,#ff8a3d,#ffb347);color:#1a0d03;font-weight:800;
  font-size:15px;letter-spacing:.5px;box-shadow:0 0 24px rgba(255,138,61,.25)}
h1{font-size:20px;margin:0;letter-spacing:.5px}
h1 span{color:var(--accent)}
header p{margin:3px 0 0;color:var(--muted);font-size:12px}
.statusBox{text-align:right;flex:0 0 auto}
.pill{display:inline-flex;align-items:center;gap:8px;padding:8px 14px;border-radius:999px;
  background:#151f2a;border:1px solid var(--line);font-size:12px;font-weight:700;
  letter-spacing:.08em;white-space:nowrap}
.pill:before{content:"";width:8px;height:8px;border-radius:50%;background:var(--muted)}
.pill.ok{color:var(--ok);border-color:rgba(45,212,143,.35);background:rgba(45,212,143,.08)}
.pill.ok:before{background:var(--ok);box-shadow:0 0 10px var(--ok)}
.pill.test{color:var(--warn);border-color:rgba(245,181,68,.4);background:rgba(245,181,68,.09)}
.pill.test:before{background:var(--warn);box-shadow:0 0 10px var(--warn)}
.pill.bad{color:var(--bad);border-color:rgba(244,86,106,.35);background:rgba(244,86,106,.08)}
.pill.bad:before{background:var(--bad)}
.tiny{color:var(--muted);font-size:11px;margin-top:6px;font-family:var(--mono)}
main{display:grid;grid-template-columns:1.35fr .85fr;gap:18px;max-width:1180px;
  margin:22px auto;padding:0 clamp(14px,3vw,28px)}
.card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--line);
  border-radius:16px;padding:18px;min-width:0}
.card.wide{grid-column:1/-1}
h2{font-size:13px;text-transform:uppercase;letter-spacing:.12em;color:#a7b6c7;margin:0 0 14px}
.head{display:flex;justify-content:space-between;align-items:flex-start;gap:16px;margin-bottom:14px}
.head h2{margin:0}
.stats{display:flex;flex-wrap:wrap;gap:8px;justify-content:flex-end}
.stat{background:#0b131b;border:1px solid var(--line);border-radius:9px;padding:6px 10px;min-width:76px}
.stat b{display:block;font-family:var(--mono);font-size:13px;color:var(--text);font-weight:600}
.stat small{color:var(--muted);font-size:9px;text-transform:uppercase;letter-spacing:.08em}
.channels{display:grid;grid-template-columns:1fr 1fr;gap:6px 22px}
.ch{display:grid;grid-template-columns:104px 1fr 88px;align-items:center;gap:10px;
  padding:6px 8px;border-radius:9px;transition:opacity .2s}
.ch:nth-child(odd){background:rgba(255,255,255,.016)}
.ch.off{opacity:.22}
.cl{font-family:var(--mono);font-size:11px;color:#c4d2e2;white-space:nowrap}
.cl .src{color:var(--muted);margin-left:5px}
.track{height:9px;border-radius:999px;background:#0a1118;border:1px solid #18232e;overflow:hidden}
.fill{height:100%;width:0%;border-radius:999px;
  background:linear-gradient(90deg,var(--accent2),var(--accent));transition:width .08s linear}
.ch.fs .fill{background:linear-gradient(90deg,#8a3040,var(--bad))}
.v{text-align:right;font-family:var(--mono);font-size:12px;font-variant-numeric:tabular-nums}
.v .us{color:var(--text)}
.v .raw{display:block;color:var(--muted);font-size:10px;min-height:12px}
.tablewrap{max-height:470px;overflow:auto;border:1px solid var(--line);border-radius:12px}
table{width:100%;border-collapse:collapse;font-size:12px}
th{position:sticky;top:0;background:#0e161f;color:var(--muted);text-align:left;font-weight:600;
  font-size:9px;text-transform:uppercase;letter-spacing:.09em;padding:9px 10px;
  border-bottom:1px solid var(--line);z-index:1}
td{padding:5px 10px;border-bottom:1px solid rgba(30,42,54,.6)}
tr:last-child td{border-bottom:0}
tr.off{opacity:.35}
tr.hidden{display:none}
.unit{text-transform:none}
.idx{color:var(--muted);font-family:var(--mono)}
.live{font-family:var(--mono);color:var(--accent2);text-align:right}
input,select{width:100%;background:#0a1118;border:1px solid #223040;color:var(--text);
  border-radius:8px;padding:8px 9px;font:inherit;font-size:12px;outline:none;min-width:0}
input:focus,select:focus{border-color:var(--accent2);box-shadow:0 0 0 2px rgba(56,189,248,.12)}
input:disabled,select:disabled{opacity:.5}
label{display:block;font-size:10px;color:var(--muted);text-transform:uppercase;letter-spacing:.07em}
label input,label select{margin-top:6px;text-transform:none;letter-spacing:normal}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.grid2.top{margin-top:12px}
.inline{display:flex;align-items:center;gap:10px;margin-bottom:12px}
.inline label{margin:0;flex:1}
.inline input{width:84px}
.hint{color:var(--muted);font-size:11px;line-height:1.55;margin:12px 0 0}
.netline{display:flex;align-items:center;gap:12px;padding:10px 0;
  border-bottom:1px solid rgba(30,42,54,.6)}
.netline:last-of-type{border-bottom:0}
.dot{width:9px;height:9px;border-radius:50%;background:var(--muted);flex:0 0 auto}
.dot.on{background:var(--ok);box-shadow:0 0 10px rgba(45,212,143,.8)}
.lbl{font-size:9px;color:var(--muted);text-transform:uppercase;letter-spacing:.09em;margin-bottom:3px}
.mono{font-family:var(--mono);font-size:12px;word-break:break-word}
.muted{color:var(--muted)}
.wire td{padding:7px 8px}
.wire td:first-child{color:var(--muted);width:38%}
.bar{position:fixed;left:0;right:0;bottom:0;display:flex;align-items:center;gap:10px;
  justify-content:flex-end;padding:13px clamp(16px,4vw,40px);
  background:rgba(10,14,19,.85);border-top:1px solid var(--line);
  backdrop-filter:blur(12px);z-index:20}
.bar .msg{margin-right:auto;color:var(--muted);font-size:12px;white-space:nowrap;
  overflow:hidden;text-overflow:ellipsis}
.bar .msg.bad{color:var(--bad)}
button{background:#17222e;color:var(--text);border:1px solid #283849;padding:10px 16px;
  border-radius:10px;font-weight:600;font-size:12px;cursor:pointer;transition:.15s;
  white-space:nowrap}
button:hover{border-color:#3b5268;background:#1c2a38}
button.primary{background:linear-gradient(135deg,#ff8a3d,#ff6b1a);border-color:transparent;
  color:#1a0d03}
button.primary:hover{filter:brightness(1.08)}
button.active{background:rgba(245,181,68,.15);border-color:rgba(245,181,68,.5);color:var(--warn)}
button.danger{color:var(--bad)}
button.ghost{background:transparent}
button:disabled{opacity:.4;cursor:default}
@media(max-width:900px){
  body{padding-bottom:140px}
  main{grid-template-columns:1fr}
  .card.wide{grid-column:auto}
  .channels{grid-template-columns:1fr}
  header{align-items:flex-start}
  .bar{flex-wrap:wrap}
  .bar .msg{flex:1 1 100%;white-space:normal}
}
</style>
</head>
<body>
<header>
  <div class="brand">
    <div class="mark">C2P</div>
    <div>
      <h1>CRSF<span>2</span>PPM</h1>
      <p>Crossfire receiver &rarr; PPM flight-controller bridge</p>
    </div>
  </div>
  <div class="statusBox">
    <div id="pill" class="pill">CONNECTING&hellip;</div>
    <div id="uptime" class="tiny">&mdash;</div>
  </div>
</header>

<main>
  <section class="card wide">
    <div class="head">
      <h2>Live channels</h2>
      <div id="stats" class="stats"></div>
    </div>
    <div id="channels" class="channels"></div>
  </section>

  <section class="card">
    <h2>Channel mapping</h2>
    <div class="inline">
      <label for="count">PPM outputs</label>
      <input id="count" type="number" min="1" max="16" value="8">
    </div>
    <div class="tablewrap">
      <table>
        <thead><tr><th>Slot</th><th>Source</th><th>Fail-safe <span class="unit">&micro;s</span></th><th style="text-align:right">Live</th></tr></thead>
        <tbody id="mapBody"></tbody>
      </table>
    </div>
    <p class="hint">Default is a straight 1&rarr;1 AETR mapping. Output slot 3 is preset to 1000 &micro;s on failsafe.</p>
  </section>

  <section class="card">
    <h2>Output &amp; failsafe</h2>
    <div class="grid2">
      <label>Frame period <span class="unit">&micro;s</span><input id="frameUs" type="number" min="15000" max="30000" step="100"></label>
      <label>Pulse width <span class="unit">&micro;s</span><input id="pulseUs" type="number" min="200" max="700" step="50"></label>
      <label>Polarity
        <select id="invert">
          <option value="0">Normal (idle low)</option>
          <option value="1">Inverted (idle high)</option>
        </select>
      </label>
      <label>Failsafe mode
        <select id="fsMode">
          <option value="preset">Preset values</option>
          <option value="hold">Hold last</option>
        </select>
      </label>
      <label>Failsafe timeout (ms)<input id="fsMs" type="number" min="100" max="5000" step="50"></label>
      <label>CRSF baud
        <select id="baud">
          <option value="420000">420000</option>
          <option value="400000">400000</option>
          <option value="115200">115200</option>
        </select>
      </label>
    </div>
    <p class="hint">PPM frame: N+1 pulses; each channel is encoded by the time between rising edges; the long final interval is the sync gap (&gt;2.7 ms, Betaflight-compatible).</p>
  </section>

  <section class="card">
    <h2>Network</h2>
    <div class="netline">
      <span id="apDot" class="dot"></span>
      <div><div class="lbl">Access point</div><div id="apInfo" class="mono">&mdash;</div></div>
    </div>
    <div class="netline">
      <span id="staDot" class="dot"></span>
      <div><div class="lbl">Station</div><div id="staInfo" class="mono">&mdash;</div></div>
    </div>
    <div class="grid2 top">
      <label>AP password<input id="apPass" type="password" maxlength="31" placeholder="unchanged"></label>
      <label>STA SSID<input id="staSsid" maxlength="32" placeholder="empty = AP only"></label>
      <label>STA password<input id="staPass" type="password" maxlength="63" placeholder="unchanged"></label>
      <label>CRSF RX pin<input id="crsfPin" type="number" min="0" max="21"></label>
      <label>PPM out pin<input id="ppmPin" type="number" min="0" max="21"></label>
    </div>
    <p class="hint">Changing pins, baud, or Wi-Fi settings reboots the board. Default AP password is <span class="mono">crsf2ppm</span>; there is no web login, so set a strong AP password.</p>
  </section>

  <section class="card wide">
    <h2>Wiring</h2>
    <div class="wire">
      <table>
        <tbody>
          <tr><td>CRSF receiver TX</td><td class="mono">&rarr; <span id="wireCrsfPin">GPIO4</span> (UART1 RX)</td></tr>
          <tr><td>PPM output</td><td class="mono"><span id="wirePpmPin">GPIO5</span> &rarr; flight controller PPM input</td></tr>
          <tr><td>Power</td><td class="mono">Receiver 5V and GND from the board's 5V/GND (or USB powered board)</td></tr>
          <tr><td>Ground</td><td class="mono">Receiver GND, board GND and flight-controller GND must be common</td></tr>
          <tr><td>Logic level</td><td class="mono">ESP32-C3 3.3 V output; PPM input on FCs is normally 3.3 V tolerant</td></tr>
        </tbody>
      </table>
    </div>
    <p class="hint">Before connecting a flight controller, verify the PPM output on a scope or use test mode with a servo. Default failsafe holds throttle low (output 3 = 1000 &micro;s) until the first valid CRSF frame.</p>
  </section>
</main>

<div class="bar">
  <div id="msg" class="msg"></div>
  <button id="test">Test mode</button>
  <button id="reboot" class="ghost danger">Reboot</button>
  <button id="save" class="primary">Save configuration</button>
</div>

<script>
"use strict";
const $ = (id) => document.getElementById(id);
const CH = 16;
let cfg = null;
let testMode = false;
let msgTimer = null;
const mapSel = [], fsIn = [], liveCells = [], chEls = [];

function esc(s) {
  return String(s).replace(/[&<>"']/g, (c) => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
}
function stat(v, label) {
  return '<div class="stat"><b>' + esc(v) + '</b><small>' + label + '</small></div>';
}
function fmtDur(ms) {
  let s = Math.floor(ms / 1000);
  const d = Math.floor(s / 86400); s %= 86400;
  const h = Math.floor(s / 3600); s %= 3600;
  const m = Math.floor(s / 60); s %= 60;
  if (d) return d + 'd ' + h + 'h';
  if (h) return h + 'h ' + m + 'm';
  if (m) return m + 'm ' + s + 's';
  return s + 's';
}
async function api(path, opts) {
  const r = await fetch(path, opts);
  let j = null;
  try { j = await r.json(); } catch (e) { /* non-JSON */ }
  if (!r.ok || !j || j.ok === false) {
    throw new Error((j && j.error) || ('HTTP ' + r.status));
  }
  return j;
}
function toast(msg, bad) {
  const el = $('msg');
  el.textContent = msg;
  el.className = 'msg' + (bad ? ' bad' : '');
  clearTimeout(msgTimer);
  msgTimer = setTimeout(() => { el.textContent = ''; }, 6000);
}
function buildChannels() {
  const wrap = $('channels');
  for (let i = 0; i < CH; i++) {
    const row = document.createElement('div');
    row.className = 'ch off';
    row.innerHTML =
      '<div class="cl">OUT ' + (i + 1) + '<span class="src">CH ' + (i + 1) + '</span></div>' +
      '<div class="track"><div class="fill"></div></div>' +
      '<div class="v"><span class="us">&mdash;</span><span class="raw"></span></div>';
    wrap.appendChild(row);
    chEls.push({
      row: row,
      fill: row.querySelector('.fill'),
      us: row.querySelector('.us'),
      raw: row.querySelector('.raw'),
      src: row.querySelector('.src')
    });
  }
}
function buildMap() {
  const body = $('mapBody');
  for (let i = 0; i < CH; i++) {
    const tr = document.createElement('tr');
    let opts = '';
    for (let c = 1; c <= CH; c++) opts += '<option value="' + c + '">CH ' + c + '</option>';
    tr.innerHTML =
      '<td class="idx">' + (i + 1) + '</td>' +
      '<td><select aria-label="Source for output ' + (i + 1) + '">' + opts + '</select></td>' +
      '<td><input type="number" min="800" max="2200" step="1" aria-label="Failsafe for output ' + (i + 1) + '"></td>' +
      '<td class="live">&mdash;</td>';
    body.appendChild(tr);
    mapSel.push(tr.querySelector('select'));
    fsIn.push(tr.querySelector('input'));
    liveCells.push(tr.querySelector('.live'));
  }
}
function applyCount() {
  let n = parseInt($('count').value, 10);
  if (!Number.isFinite(n)) n = 8;
  n = Math.max(1, Math.min(CH, n));
  for (let i = 0; i < CH; i++) {
    const on = i < n;
    mapSel[i].disabled = !on;
    fsIn[i].disabled = !on;
    mapSel[i].closest('tr').classList.toggle('off', !on);
    mapSel[i].closest('tr').classList.toggle('hidden', !on);
  }
  if (cfg) {
    for (let i = 0; i < CH; i++) {
      chEls[i].src.textContent = 'CH ' + cfg.map[i];
    }
  }
}
function updateWiring() {
  $('wireCrsfPin').textContent = 'GPIO' + ($('crsfPin').value || '4');
  $('wirePpmPin').textContent = 'GPIO' + ($('ppmPin').value || '5');
}
async function loadConfig() {
  cfg = await api('/api/config');
  $('count').value = cfg.count;
  for (let i = 0; i < CH; i++) {
    mapSel[i].value = String(cfg.map[i]);
    fsIn[i].value = cfg.fs[i];
    chEls[i].src.textContent = 'CH ' + cfg.map[i];
  }
  $('frameUs').value = cfg.frame_us;
  $('pulseUs').value = cfg.pulse_us;
  $('invert').value = String(cfg.invert);
  $('fsMode').value = cfg.failsafe_mode;
  $('fsMs').value = cfg.failsafe_ms;
  const baud = String(cfg.crsf_baud);
  if (!Array.from($('baud').options).some((o) => o.value === baud)) {
    const o = document.createElement('option');
    o.value = baud;
    o.textContent = baud;
    $('baud').appendChild(o);
  }
  $('baud').value = baud;
  $('crsfPin').value = cfg.crsf_pin;
  $('ppmPin').value = cfg.ppm_pin;
  $('staSsid').value = cfg.sta_ssid || '';
  $('apPass').value = '';
  $('staPass').value = '';
  applyCount();
  updateWiring();
}
function render(s) {
  const c = s.crsf, p = s.ppm;
  $('uptime').textContent = 'UP ' + fmtDur(s.uptime_ms);
  const pill = $('pill');
  if (s.test) {
    pill.className = 'pill test';
    pill.textContent = 'TEST MODE';
  } else if (c.link_ok) {
    pill.className = 'pill ok';
    pill.textContent = 'LINK OK';
  } else if (p.failsafe && c.age_ms !== 4294967295) {
    pill.className = 'pill bad';
    pill.textContent = 'FAILSAFE';
  } else {
    pill.className = 'pill bad';
    pill.textContent = 'NO LINK';
  }

  const rssi = c.stats_valid ? (c.rssi + ' dBm') : '—';
  const lq = c.stats_valid ? (c.lq + '%') : '—';
  const snr = c.stats_valid ? (c.snr + ' dB') : '—';
  const age = (c.age_ms === 4294967295) ? '—' : fmtDur(c.age_ms);
  const period = p.period_us ? ((p.period_us / 1000).toFixed(2) + ' ms') : '—';
  $('stats').innerHTML =
    stat(c.fps, 'CRSF fps') + stat(p.fps, 'PPM fps') + stat(rssi, 'RSSI') +
    stat(lq, 'LQ') + stat(snr, 'SNR') + stat(c.crc_errors, 'CRC errors') +
    stat(period, 'PPM frame') + stat(age, 'Last RC');

  for (let i = 0; i < CH; i++) {
    const el = chEls[i];
    const on = i < p.count;
    el.row.classList.toggle('off', !on);
    el.row.classList.toggle('fs', p.failsafe && !s.test);
    const out = s.out[i] || 0;
    const pct = on ? Math.max(0, Math.min(100, ((out - 800) / 1400) * 100)) : 0;
    el.fill.style.width = pct + '%';
    el.us.textContent = on && out ? out + ' µs' : '—';
    if (s.test) {
      el.raw.textContent = 'test';
    } else if (on) {
      const src = cfg ? (cfg.map[i] - 1) : i;
      el.raw.textContent = 'raw ' + (s.raw[src] || 0);
    } else {
      el.raw.textContent = '';
    }
    liveCells[i].textContent = on && out ? String(out) : '—';
  }

  $('apInfo').innerHTML = esc(s.wifi.ap_ssid) + ' <span class="muted">&middot; ' + esc(s.wifi.ap_ip) + '</span>';
  $('apDot').className = 'dot on';
  if (s.wifi.sta_ssid) {
    $('staDot').className = 'dot' + (s.wifi.sta_connected ? ' on' : '');
    $('staInfo').textContent = s.wifi.sta_ssid + (s.wifi.sta_connected
      ? ' · ' + s.wifi.sta_ip + ' · ' + s.wifi.sta_rssi + ' dBm'
      : ' · connecting / offline');
  } else {
    $('staDot').className = 'dot';
    $('staInfo').textContent = 'disabled (AP only)';
  }

  testMode = s.test;
  const testBtn = $('test');
  testBtn.classList.toggle('active', s.test);
  testBtn.textContent = s.test ? 'Stop test mode' : 'Test mode';
  document.title = (s.test ? 'TEST · ' : (c.link_ok ? 'LINK · ' : '')) + 'CRSF2PPM';
}
async function save() {
  const body = new URLSearchParams();
  body.set('count', $('count').value);
  for (let i = 0; i < CH; i++) {
    body.set('map' + i, mapSel[i].value);
    body.set('fs' + i, fsIn[i].value);
  }
  body.set('frame_us', $('frameUs').value);
  body.set('pulse_us', $('pulseUs').value);
  body.set('invert', $('invert').value);
  body.set('failsafe_mode', $('fsMode').value);
  body.set('failsafe_ms', $('fsMs').value);
  body.set('crsf_baud', $('baud').value);
  body.set('crsf_pin', $('crsfPin').value);
  body.set('ppm_pin', $('ppmPin').value);
  if ($('apPass').value) body.set('ap_pass', $('apPass').value);
  body.set('sta_ssid', $('staSsid').value);
  if ($('staPass').value) body.set('sta_pass', $('staPass').value);
  $('save').disabled = true;
  try {
    const r = await api('/api/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: body
    });
    $('apPass').value = '';
    $('staPass').value = '';
    if (r.reboot) {
      toast('Saved. Rebooting - reconnect to the AP, then reload this page.');
      setTimeout(() => location.reload(), 12000);
    } else {
      toast('Configuration saved.');
      await loadConfig();
    }
  } catch (e) {
    toast(e.message, true);
  } finally {
    $('save').disabled = false;
  }
}
async function toggleTest() {
  try {
    const r = await api('/api/test', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: 'on=' + (testMode ? '0' : '1')
    });
    testMode = r.test;
    toast(r.test ? 'Test mode on: outputs are sweeping.' : 'Test mode off.');
  } catch (e) {
    toast(e.message, true);
  }
}
async function reboot() {
  try {
    await api('/api/reboot', { method: 'POST' });
    toast('Rebooting. Reload the page in a few seconds.');
  } catch (e) {
    toast(e.message, true);
  }
}
async function poll() {
  let ok = false;
  try {
    const s = await api('/api/status');
    ok = true;
    if (!cfg) {
      try { await loadConfig(); } catch (e) { /* keep polling */ }
    }
    render(s);
  } catch (e) {
    $('pill').className = 'pill bad';
    $('pill').textContent = 'DEVICE OFFLINE';
  } finally {
    setTimeout(poll, ok ? 500 : 2000);
  }
}

buildChannels();
buildMap();
$('count').addEventListener('input', applyCount);
$('count').addEventListener('change', applyCount);
$('save').addEventListener('click', save);
$('test').addEventListener('click', toggleTest);
$('reboot').addEventListener('click', reboot);
$('crsfPin').addEventListener('input', updateWiring);
$('ppmPin').addEventListener('input', updateWiring);

loadConfig().catch((e) => toast(e.message, true)).finally(() => poll());
</script>
</body>
</html>
)rawliteral";
