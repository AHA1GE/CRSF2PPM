#pragma once

// Single-page web UI. Kept in flash and served with send_P().
// The page is bilingual (en-US / zh-CN) and picks the browser language at
// load time; ?lang=zh-cn / ?lang=en-us and the footer selector override it.
static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="en-US">
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
td.cb{text-align:center;width:58px}
input,select{width:100%;background:#0a1118;border:1px solid #223040;color:var(--text);
  border-radius:8px;padding:8px 9px;font:inherit;font-size:12px;outline:none;min-width:0}
input:focus,select:focus{border-color:var(--accent2);box-shadow:0 0 0 2px rgba(56,189,248,.12)}
input:disabled,select:disabled{opacity:.5}
input[type=checkbox]{width:16px;height:16px;padding:0;border:0;background:none;
  accent-color:var(--accent);cursor:pointer;vertical-align:middle}
input[type=checkbox]:focus{box-shadow:none}
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
.tag{display:inline-block;margin-left:6px;padding:1px 6px;border-radius:999px;
  border:1px solid rgba(245,181,68,.45);color:var(--warn);font-size:9px;
  letter-spacing:.08em;vertical-align:middle}
.wire td{padding:7px 8px}
.wire td:first-child{color:var(--muted);width:38%}
.bar{position:fixed;left:0;right:0;bottom:0;display:flex;align-items:center;gap:10px;
  justify-content:flex-end;padding:13px clamp(16px,4vw,40px);
  background:rgba(10,14,19,.85);border-top:1px solid var(--line);
  backdrop-filter:blur(12px);z-index:20}
.bar .msg{margin-right:auto;color:var(--muted);font-size:12px;white-space:nowrap;
  overflow:hidden;text-overflow:ellipsis}
.bar .msg.bad{color:var(--bad)}
.bar select{width:auto;flex:0 0 auto;padding:9px 10px}
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
      <p data-i18n="subtitle">Crossfire receiver &rarr; PPM flight-controller bridge</p>
    </div>
  </div>
  <div class="statusBox">
    <div id="pill" class="pill" data-i18n="connecting">CONNECTING&hellip;</div>
    <div id="uptime" class="tiny">&mdash;</div>
    <div id="fault" class="tiny" style="color:var(--bad)"></div>
  </div>
</header>

<main>
  <section class="card wide">
    <div class="head">
      <h2 data-i18n="liveChannels">Live channels</h2>
      <div id="stats" class="stats"></div>
    </div>
    <div id="channels" class="channels"></div>
  </section>

  <section class="card">
    <h2 data-i18n="channelMapping">Channel mapping</h2>
    <div class="inline">
      <label for="count" data-i18n="ppmOutputs">PPM outputs</label>
      <input id="count" type="number" min="1" max="16" value="8">
    </div>
    <div class="tablewrap">
      <table>
        <thead><tr>
          <th data-i18n="thSlot">Slot</th>
          <th data-i18n="thSource">Source</th>
          <th data-i18n="thReverse">Reverse</th>
          <th data-i18n-html="thFailsafe">Fail-safe <span class="unit">&micro;s</span></th>
          <th style="text-align:right" data-i18n="thLive">Live</th>
        </tr></thead>
        <tbody id="mapBody"></tbody>
      </table>
    </div>
    <p class="hint" data-i18n="mappingHint">Default is a straight 1&rarr;1 AETR mapping. Output slot 3 is preset to 1000 &micro;s on failsafe. Reverse mirrors that output around 1500 &micro;s (800&harr;2200); fail-safe values are final output widths and are not reversed.</p>
  </section>

  <section class="card">
    <h2 data-i18n="outputFailsafe">Output &amp; failsafe</h2>
    <div class="grid2">
      <label><span data-i18n="framePeriod">Frame period</span> <span class="unit">&micro;s</span><input id="frameUs" type="number" min="15000" max="30000" step="100"></label>
      <label><span data-i18n="pulseWidth">Pulse width</span> <span class="unit">&micro;s</span><input id="pulseUs" type="number" min="200" max="700" step="50"></label>
      <label><span data-i18n="polarity">Polarity</span>
        <select id="invert">
          <option value="0" data-i18n="normalPolarity">Normal (idle low)</option>
          <option value="1" data-i18n="invertedPolarity">Inverted (idle high)</option>
        </select>
      </label>
      <label><span data-i18n="failsafeMode">Failsafe mode</span>
        <select id="fsMode">
          <option value="preset" data-i18n="presetValues">Preset values</option>
          <option value="hold" data-i18n="holdLast">Hold last</option>
        </select>
      </label>
      <label><span data-i18n="failsafeTimeout">Failsafe timeout (ms)</span><input id="fsMs" type="number" min="100" max="5000" step="50"></label>
      <label><span data-i18n="crsfBaud">CRSF baud</span>
        <select id="baud">
          <option value="420000">420000</option>
          <option value="400000">400000</option>
          <option value="115200">115200</option>
        </select>
      </label>
    </div>
    <p class="hint" data-i18n="outputHint">PPM frame: N+1 pulses; each channel is encoded by the time between rising edges; the long final interval is the sync gap (&gt;2.7 ms, Betaflight-compatible).</p>
  </section>

  <section class="card">
    <h2 data-i18n="network">Network</h2>
    <div class="netline">
      <span id="apDot" class="dot"></span>
      <div><div class="lbl" data-i18n="accessPoint">Access point</div><div id="apInfo" class="mono">&mdash;</div></div>
    </div>
    <div class="netline">
      <span id="staDot" class="dot"></span>
      <div><div class="lbl" data-i18n="station">Station</div><div id="staInfo" class="mono">&mdash;</div></div>
    </div>
    <div class="grid2 top">
      <label><span data-i18n="staSsid">STA SSID</span><input id="staSsid" maxlength="32" data-i18n-ph="emptyApOnly" placeholder="empty = AP only"></label>
      <label><span data-i18n="staPass">STA password</span><input id="staPass" type="password" maxlength="63" data-i18n-ph="unchanged" placeholder="unchanged"></label>
      <label><span data-i18n="crsfRxPin">CRSF RX pin</span><input id="crsfPin" type="number" min="0" max="21"></label>
      <label><span data-i18n="ppmOutPin">PPM out pin</span><input id="ppmPin" type="number" min="0" max="21"></label>
    </div>
    <p class="hint" data-i18n-html="networkHint">Wi-Fi is <b>OFF after every power-up</b>. Press the BOOT button (GPIO9) to start the <b>open</b> access point; the status LED flashes slowly while Wi-Fi is on. Phones and computers that join it open this page automatically (captive portal); otherwise browse to <span class="mono">http://192.168.4.1/</span>. There is no AP password and no web login, so anyone in range can change settings &mdash; press BOOT again to turn Wi-Fi off when done.</p>
  </section>

  <section class="card wide">
    <h2 data-i18n="wiring">Wiring</h2>
    <div class="wire">
      <table>
        <tbody>
          <tr><td data-i18n="wrCrsf">CRSF receiver TX</td><td class="mono">&rarr; <span id="wireCrsfPin">GPIO3</span> <span data-i18n="wrCrsfNote">(UART1 RX)</span></td></tr>
          <tr><td data-i18n="wrPpm">PPM output</td><td class="mono"><span id="wirePpmPin">GPIO4</span> <span data-i18n="wrPpmNote">&rarr; flight controller PPM input</span></td></tr>
          <tr><td data-i18n="wrPower">Power</td><td class="mono" data-i18n="wrPowerVal">Receiver 5V and GND from the board 5V/GND (or USB powered board)</td></tr>
          <tr><td data-i18n="wrGround">Ground</td><td class="mono" data-i18n="wrGroundVal">Receiver GND, board GND and flight-controller GND must be common</td></tr>
          <tr><td data-i18n="wrLogic">Logic level</td><td class="mono" data-i18n="wrLogicVal">ESP32-C3 3.3 V output; PPM input on FCs is normally 3.3 V tolerant</td></tr>
          <tr><td data-i18n="wrLed">Status LED</td><td class="mono" data-i18n="wrLedVal">GPIO8 &rarr; resistor &rarr; LED &rarr; 3.3 V (active-low); reserved</td></tr>
          <tr><td data-i18n="wrBtn">Wi-Fi button</td><td class="mono" data-i18n="wrBtnVal">GPIO9 BOOT &rarr; GND (usually onboard); press after boot to toggle Wi-Fi</td></tr>
        </tbody>
      </table>
    </div>
    <p class="hint" data-i18n="wiringHint">Status LED (active-low): solid = CRSF link, breathing = no link, slow flash = Wi-Fi on, fast blink = latched fault (reset required). GPIO8/9 are reserved and cannot be selected for CRSF or PPM. GPIO8 is a strapping pin; some boards fit a WS2812 there rather than a plain LED, which this firmware does not drive.</p>
    <p class="hint" data-i18n="wiringHint2">Before connecting a flight controller, verify the PPM output on a scope or use test mode with a servo. Default failsafe holds throttle low (output 3 = 1000 &micro;s) until the first valid CRSF frame.</p>
  </section>
</main>

<div class="bar">
  <div id="msg" class="msg"></div>
  <select id="langSel" aria-label="Language">
    <option value="auto" data-i18n="langAuto">Auto</option>
    <option value="en-US">English</option>
    <option value="zh-CN">中文</option>
  </select>
  <button id="test" data-i18n="testMode">Test mode</button>
  <button id="reboot" class="ghost danger" data-i18n="reboot">Reboot</button>
  <button id="save" class="primary" data-i18n="save">Save configuration</button>
</div>

<script>
"use strict";
const $ = (id) => document.getElementById(id);
const CH = 16;
const LANG_KEY = 'crsf2ppm.lang';

const STR = {
  'en-US': {
    subtitle: 'Crossfire receiver → PPM flight-controller bridge',
    liveChannels: 'Live channels',
    channelMapping: 'Channel mapping',
    ppmOutputs: 'PPM outputs',
    thSlot: 'Slot',
    thSource: 'Source',
    thReverse: 'Reverse',
    thFailsafe: 'Fail-safe <span class="unit">µs</span>',
    thLive: 'Live',
    mappingHint: 'Default is a straight 1→1 AETR mapping. Output slot 3 is preset to 1000 µs on failsafe. Reverse mirrors that output around 1500 µs (800↔2200); fail-safe values are final output widths and are not reversed.',
    outputFailsafe: 'Output & failsafe',
    framePeriod: 'Frame period',
    pulseWidth: 'Pulse width',
    polarity: 'Polarity',
    normalPolarity: 'Normal (idle low)',
    invertedPolarity: 'Inverted (idle high)',
    failsafeMode: 'Failsafe mode',
    presetValues: 'Preset values',
    holdLast: 'Hold last',
    failsafeTimeout: 'Failsafe timeout (ms)',
    crsfBaud: 'CRSF baud',
    outputHint: 'PPM frame: N+1 pulses; each channel is encoded by the time between rising edges; the long final interval is the sync gap (>2.7 ms, Betaflight-compatible).',
    network: 'Network',
    accessPoint: 'Access point',
    station: 'Station',
    staSsid: 'STA SSID',
    staPass: 'STA password',
    crsfRxPin: 'CRSF RX pin',
    ppmOutPin: 'PPM out pin',
    unchanged: 'unchanged',
    emptyApOnly: 'empty = AP only',
    networkHint: 'Wi-Fi is <b>OFF after every power-up</b>. Press the BOOT button (GPIO9) to start the <b>open</b> access point; the status LED flashes slowly while Wi-Fi is on. Phones and computers that join it open this page automatically (captive portal); otherwise browse to <span class="mono">http://192.168.4.1/</span>. There is no AP password and no web login, so anyone in range can change settings — press BOOT again to turn Wi-Fi off when done.',
    wiring: 'Wiring',
    wrCrsf: 'CRSF receiver TX',
    wrCrsfNote: '(UART1 RX)',
    wrPpm: 'PPM output',
    wrPpmNote: '→ flight controller PPM input',
    wrPower: 'Power',
    wrPowerVal: 'Receiver 5V and GND from the board 5V/GND (or USB powered board)',
    wrGround: 'Ground',
    wrGroundVal: 'Receiver GND, board GND and flight-controller GND must be common',
    wrLogic: 'Logic level',
    wrLogicVal: 'ESP32-C3 3.3 V output; PPM input on FCs is normally 3.3 V tolerant',
    wrLed: 'Status LED',
    wrLedVal: 'GPIO8 → resistor → LED → 3.3 V (active-low); reserved',
    wrBtn: 'Wi-Fi button',
    wrBtnVal: 'GPIO9 BOOT → GND (usually onboard); press after boot to toggle Wi-Fi',
    wiringHint: 'Status LED (active-low): solid = CRSF link, breathing = no link, slow flash = Wi-Fi on, fast blink = latched fault (reset required). GPIO8/9 are reserved and cannot be selected for CRSF or PPM. GPIO8 is a strapping pin; some boards fit a WS2812 there rather than a plain LED, which this firmware does not drive.',
    wiringHint2: 'Before connecting a flight controller, verify the PPM output on a scope or use test mode with a servo. Default failsafe holds throttle low (output 3 = 1000 µs) until the first valid CRSF frame.',
    testMode: 'Test mode',
    stopTest: 'Stop test mode',
    reboot: 'Reboot',
    save: 'Save configuration',
    langAuto: 'Auto',
    langLabel: 'Language',
    connecting: 'CONNECTING…',
    fault: 'FAULT',
    testPill: 'TEST MODE',
    linkOk: 'LINK OK',
    failsafePill: 'FAILSAFE',
    noLink: 'NO LINK',
    deviceOffline: 'DEVICE OFFLINE',
    faultPrefix: 'FAULT: ',
    up: 'UP',
    statCrsfFps: 'CRSF fps',
    statPpmFps: 'PPM fps',
    statRssi: 'RSSI',
    statLq: 'LQ',
    statSnr: 'SNR',
    statCrc: 'CRC errors',
    statPpmFrame: 'PPM frame',
    statLastRc: 'Last RC',
    statLed: 'LED',
    ledBreath: 'breathing',
    ledLink: 'link',
    ledWifi: 'Wi-Fi on',
    ledError: 'fault',
    raw: 'raw',
    test: 'test',
    staDisabled: 'disabled (AP only)',
    staConnecting: 'connecting / offline',
    openTag: 'OPEN',
    titleFault: 'FAULT',
    titleTest: 'TEST',
    titleLink: 'LINK',
    saved: 'Configuration saved.',
    savedReboot: 'Saved. Rebooting - after restart press BOOT (GPIO9) to enable Wi-Fi, then reload this page.',
    testOn: 'Test mode on: outputs are sweeping.',
    testOff: 'Test mode off.',
    rebooting: 'Rebooting. After restart, press BOOT (GPIO9) to enable Wi-Fi again, then reload.',
    errNetwork: 'Network error - device may be offline.',
    outLabel: 'OUT {0}',
    chLabel: 'CH {0}',
    ariaSource: 'Source for output {0}',
    ariaFailsafe: 'Failsafe for output {0}',
    ariaReverse: 'Reverse output {0}'
  },
  'zh-CN': {
    subtitle: 'Crossfire 接收机 → PPM 飞控桥接器',
    liveChannels: '实时通道',
    channelMapping: '通道映射',
    ppmOutputs: 'PPM 输出数量',
    thSlot: '槽位',
    thSource: '来源',
    thReverse: '反向',
    thFailsafe: '失控保护 <span class="unit">µs</span>',
    thLive: '实时',
    mappingHint: '默认为 1→1 的 AETR 映射。输出槽 3 的失控保护预设为 1000 µs。反向会以 1500 µs 为中心镜像该输出（800↔2200）；失控保护值为最终输出脉宽，不受反向影响。',
    outputFailsafe: '输出与失控保护',
    framePeriod: '帧周期',
    pulseWidth: '脉冲宽度',
    polarity: '极性',
    normalPolarity: '正常（空闲低电平）',
    invertedPolarity: '反向（空闲高电平）',
    failsafeMode: '失控保护模式',
    presetValues: '预设值',
    holdLast: '保持最后值',
    failsafeTimeout: '失控保护超时 (ms)',
    crsfBaud: 'CRSF 波特率',
    outputHint: 'PPM 帧：N+1 个脉冲；通道值由相邻上升沿之间的时间编码；末尾的长间隔为同步间隔（>2.7 ms，兼容 Betaflight）。',
    network: '网络',
    accessPoint: '接入点',
    station: '站点',
    staSsid: 'STA SSID',
    staPass: 'STA 密码',
    crsfRxPin: 'CRSF RX 引脚',
    ppmOutPin: 'PPM 输出引脚',
    unchanged: '保持不变',
    emptyApOnly: '留空 = 仅 AP',
    networkHint: '每次上电后 Wi-Fi <b>默认关闭</b>。按下 BOOT 键（GPIO9）启动<b>开放</b>接入点；Wi-Fi 开启时状态 LED 慢闪。手机或电脑加入该网络后会自动打开本页（强制门户）；也可以直接访问 <span class="mono">http://192.168.4.1/</span>。AP 无密码且网页无登录，范围内任何人都能修改设置——用完后再次按 BOOT 关闭 Wi-Fi。',
    wiring: '接线',
    wrCrsf: 'CRSF 接收机 TX',
    wrCrsfNote: '(UART1 RX)',
    wrPpm: 'PPM 输出',
    wrPpmNote: '→ 飞控 PPM 输入',
    wrPower: '电源',
    wrPowerVal: '接收机 5V 与 GND 接板载 5V/GND（或使用 USB 供电）',
    wrGround: '地线',
    wrGroundVal: '接收机 GND、板载 GND 与飞控 GND 必须共地',
    wrLogic: '逻辑电平',
    wrLogicVal: 'ESP32-C3 3.3 V 输出；飞控 PPM 输入通常可承受 3.3 V',
    wrLed: '状态 LED',
    wrLedVal: 'GPIO8 → 电阻 → LED → 3.3 V（低电平点亮）；已保留',
    wrBtn: 'Wi-Fi 按键',
    wrBtnVal: 'GPIO9 BOOT → GND（通常板载）；启动后按下以开关 Wi-Fi',
    wiringHint: '状态 LED（低电平点亮）：常亮 = CRSF 链路正常，呼吸 = 无链路，慢闪 = Wi-Fi 开启，快闪 = 故障锁定（需复位）。GPIO8/9 已保留，不能用于 CRSF 或 PPM。GPIO8 是启动配置引脚；部分开发板在此处焊接 WS2812 而非普通 LED，本固件不驱动 WS2812。',
    wiringHint2: '连接飞控前，请先用示波器或舵机测试模式确认 PPM 输出。在收到第一个有效 CRSF 帧之前，默认失控保护会让油门保持低位（输出 3 = 1000 µs）。',
    testMode: '测试模式',
    stopTest: '停止测试模式',
    reboot: '重启',
    save: '保存配置',
    langAuto: '自动',
    langLabel: '语言',
    connecting: '连接中…',
    fault: '故障',
    testPill: '测试模式',
    linkOk: '链路正常',
    failsafePill: '失控保护',
    noLink: '无链路',
    deviceOffline: '设备离线',
    faultPrefix: '故障：',
    up: '运行',
    statCrsfFps: 'CRSF 帧率',
    statPpmFps: 'PPM 帧率',
    statRssi: 'RSSI',
    statLq: 'LQ',
    statSnr: 'SNR',
    statCrc: 'CRC 错误',
    statPpmFrame: 'PPM 帧周期',
    statLastRc: '最近 RC',
    statLed: 'LED',
    ledBreath: '呼吸',
    ledLink: '链路',
    ledWifi: 'Wi-Fi 开启',
    ledError: '故障',
    raw: '原始',
    test: '测试',
    staDisabled: '已禁用（仅 AP）',
    staConnecting: '连接中 / 离线',
    openTag: '开放',
    titleFault: '故障',
    titleTest: '测试',
    titleLink: '链路',
    saved: '配置已保存。',
    savedReboot: '已保存。正在重启——重启后按 BOOT (GPIO9) 开启 Wi-Fi，然后刷新本页。',
    testOn: '测试模式已开启：输出正在扫描。',
    testOff: '测试模式已关闭。',
    rebooting: '正在重启。重启后按 BOOT (GPIO9) 重新开启 Wi-Fi，然后刷新。',
    errNetwork: '网络错误——设备可能离线。',
    outLabel: '输出 {0}',
    chLabel: '通道 {0}',
    ariaSource: '输出 {0} 的来源通道',
    ariaFailsafe: '输出 {0} 的失控保护值',
    ariaReverse: '反向输出 {0}'
  }
};

let langMode = 'auto';
let lang = 'en-US';
let cfg = null;
let lastStatus = null;
let testMode = false;
let msgTimer = null;
const mapSel = [], revBox = [], fsIn = [], liveCells = [], chEls = [];

function t(key) {
  const args = Array.prototype.slice.call(arguments, 1);
  const dict = STR[lang] || STR['en-US'];
  let s = dict[key];
  if (s === undefined) s = STR['en-US'][key];
  if (s === undefined) s = key;
  for (let i = 0; i < args.length; i++) {
    s = s.split('{' + i + '}').join(String(args[i]));
  }
  return s;
}
function esc(s) {
  return String(s).replace(/[&<>"']/g, (c) => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
}
function stat(v, label) {
  return '<div class="stat"><b>' + esc(v) + '</b><small>' + esc(label) + '</small></div>';
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
function errText(e) {
  if (e && e.name === 'TypeError') return t('errNetwork');
  return (e && e.message) ? e.message : String(e);
}
function detectLang() {
  const list = (navigator.languages && navigator.languages.length)
    ? navigator.languages : [navigator.language || ''];
  for (let i = 0; i < list.length; i++) {
    const l = String(list[i]).toLowerCase();
    if (l.indexOf('zh') === 0) return 'zh-CN';
    if (l.indexOf('en') === 0) return 'en-US';
  }
  return 'en-US';
}
function initLang() {
  try {
    const saved = localStorage.getItem(LANG_KEY);
    if (saved === 'en-US' || saved === 'zh-CN') langMode = saved;
  } catch (e) { /* private mode / captive browser may block storage */ }
  try {
    const q = new URLSearchParams(location.search).get('lang');
    if (q) langMode = q.toLowerCase().indexOf('zh') === 0 ? 'zh-CN' : 'en-US';
  } catch (e) { /* ignore */ }
  lang = langMode === 'auto' ? detectLang() : langMode;
  $('langSel').value = langMode;
}
function setLangMode(mode) {
  langMode = (mode === 'en-US' || mode === 'zh-CN') ? mode : 'auto';
  try {
    if (langMode === 'auto') localStorage.removeItem(LANG_KEY);
    else localStorage.setItem(LANG_KEY, langMode);
  } catch (e) { /* ignore */ }
  lang = langMode === 'auto' ? detectLang() : langMode;
  applyLang();
}
function updateChannelLabels() {
  for (let i = 0; i < CH; i++) {
    chEls[i].out.textContent = t('outLabel', i + 1);
    const src = cfg ? cfg.map[i] : i + 1;
    chEls[i].src.textContent = t('chLabel', src);
  }
}
function updateMapLabels() {
  for (let i = 0; i < CH; i++) {
    for (let c = 0; c < CH; c++) {
      mapSel[i].options[c].textContent = t('chLabel', c + 1);
    }
    mapSel[i].setAttribute('aria-label', t('ariaSource', i + 1));
    revBox[i].setAttribute('aria-label', t('ariaReverse', i + 1));
    fsIn[i].setAttribute('aria-label', t('ariaFailsafe', i + 1));
  }
}
function applyLang() {
  document.documentElement.lang = lang;
  document.querySelectorAll('[data-i18n]').forEach((el) => {
    el.textContent = t(el.getAttribute('data-i18n'));
  });
  document.querySelectorAll('[data-i18n-html]').forEach((el) => {
    el.innerHTML = t(el.getAttribute('data-i18n-html'));
  });
  document.querySelectorAll('[data-i18n-ph]').forEach((el) => {
    el.placeholder = t(el.getAttribute('data-i18n-ph'));
  });
  $('langSel').setAttribute('aria-label', t('langLabel'));
  updateChannelLabels();
  updateMapLabels();
  if (lastStatus) render(lastStatus);
  else document.title = 'CRSF2PPM';
}
async function api(path, opts) {
  opts = opts || {};
  const headers = Object.assign({}, opts.headers || {});
  headers['X-Lang'] = lang;
  const r = await fetch(path, Object.assign({}, opts, { headers: headers }));
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
      '<div class="cl"><span class="out"></span><span class="src"></span></div>' +
      '<div class="track"><div class="fill"></div></div>' +
      '<div class="v"><span class="us">&mdash;</span><span class="raw"></span></div>';
    wrap.appendChild(row);
    chEls.push({
      row: row,
      fill: row.querySelector('.fill'),
      us: row.querySelector('.us'),
      raw: row.querySelector('.raw'),
      out: row.querySelector('.out'),
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
      '<td><select>' + opts + '</select></td>' +
      '<td class="cb"><input type="checkbox"></td>' +
      '<td><input type="number" min="800" max="2200" step="1"></td>' +
      '<td class="live">&mdash;</td>';
    body.appendChild(tr);
    mapSel.push(tr.querySelector('select'));
    revBox.push(tr.querySelector('input[type=checkbox]'));
    fsIn.push(tr.querySelector('input[type=number]'));
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
    revBox[i].disabled = !on;
    fsIn[i].disabled = !on;
    mapSel[i].closest('tr').classList.toggle('off', !on);
    mapSel[i].closest('tr').classList.toggle('hidden', !on);
  }
  updateChannelLabels();
}
function updateWiring() {
  $('wireCrsfPin').textContent = 'GPIO' + ($('crsfPin').value || '3');
  $('wirePpmPin').textContent = 'GPIO' + ($('ppmPin').value || '4');
}
async function loadConfig() {
  cfg = await api('/api/config');
  $('count').value = cfg.count;
  for (let i = 0; i < CH; i++) {
    mapSel[i].value = String(cfg.map[i]);
    revBox[i].checked = !!cfg.rev[i];
    fsIn[i].value = cfg.fs[i];
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
  $('staPass').value = '';
  applyCount();
  updateWiring();
}
function render(s) {
  lastStatus = s;
  const c = s.crsf, p = s.ppm;
  $('uptime').textContent = t('up') + ' ' + fmtDur(s.uptime_ms);
  const pill = $('pill');
  if (s.error) {
    pill.className = 'pill bad';
    pill.textContent = t('fault');
  } else if (s.test) {
    pill.className = 'pill test';
    pill.textContent = t('testPill');
  } else if (c.link_ok) {
    pill.className = 'pill ok';
    pill.textContent = t('linkOk');
  } else if (p.failsafe && c.age_ms !== 4294967295) {
    pill.className = 'pill bad';
    pill.textContent = t('failsafePill');
  } else {
    pill.className = 'pill bad';
    pill.textContent = t('noLink');
  }
  $('fault').textContent = s.error ? (t('faultPrefix') + s.error_reason) : '';

  const rssi = c.stats_valid ? (c.rssi + ' dBm') : '—';
  const lq = c.stats_valid ? (c.lq + '%') : '—';
  const snr = c.stats_valid ? (c.snr + ' dB') : '—';
  const age = (c.age_ms === 4294967295) ? '—' : fmtDur(c.age_ms);
  const period = p.period_us ? ((p.period_us / 1000).toFixed(2) + ' ms') : '—';
  const ledKey = 'led' + (s.led || '').charAt(0).toUpperCase() + (s.led || '').slice(1);
  const led = STR[lang] && STR[lang][ledKey] ? t(ledKey) : (s.led || '—');
  $('stats').innerHTML =
    stat(c.fps, t('statCrsfFps')) + stat(p.fps, t('statPpmFps')) + stat(rssi, t('statRssi')) +
    stat(lq, t('statLq')) + stat(snr, t('statSnr')) + stat(c.crc_errors, t('statCrc')) +
    stat(period, t('statPpmFrame')) + stat(age, t('statLastRc')) + stat(led, t('statLed'));

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
      el.raw.textContent = t('test');
    } else if (on) {
      const src = cfg ? (cfg.map[i] - 1) : i;
      el.raw.textContent = t('raw') + ' ' + (s.raw[src] || 0);
    } else {
      el.raw.textContent = '';
    }
    liveCells[i].textContent = on && out ? String(out) : '—';
  }

  $('apInfo').innerHTML = esc(s.wifi.ap_ssid) +
    ' <span class="muted">&middot; ' + esc(s.wifi.ap_ip) + '</span>' +
    ' <span class="tag">' + esc(t('openTag')) + '</span>';
  $('apDot').className = 'dot on';
  if (s.wifi.sta_ssid) {
    $('staDot').className = 'dot' + (s.wifi.sta_connected ? ' on' : '');
    $('staInfo').textContent = s.wifi.sta_ssid + (s.wifi.sta_connected
      ? ' · ' + s.wifi.sta_ip + ' · ' + s.wifi.sta_rssi + ' dBm'
      : ' · ' + t('staConnecting'));
  } else {
    $('staDot').className = 'dot';
    $('staInfo').textContent = t('staDisabled');
  }

  testMode = s.test;
  const testBtn = $('test');
  testBtn.classList.toggle('active', s.test);
  testBtn.textContent = s.test ? t('stopTest') : t('testMode');
  document.title = (s.error ? t('titleFault') + ' · ' : (s.test ? t('titleTest') + ' · ' : (c.link_ok ? t('titleLink') + ' · ' : ''))) + 'CRSF2PPM';
}
async function save() {
  const body = new URLSearchParams();
  body.set('count', $('count').value);
  for (let i = 0; i < CH; i++) {
    body.set('map' + i, mapSel[i].value);
    body.set('rev' + i, revBox[i].checked ? '1' : '0');
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
  body.set('sta_ssid', $('staSsid').value);
  if ($('staPass').value) body.set('sta_pass', $('staPass').value);
  $('save').disabled = true;
  try {
    const r = await api('/api/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: body
    });
    $('staPass').value = '';
    if (r.reboot) {
      toast(t('savedReboot'));
      setTimeout(() => location.reload(), 14000);
    } else {
      toast(t('saved'));
      await loadConfig();
    }
  } catch (e) {
    toast(errText(e), true);
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
    toast(r.test ? t('testOn') : t('testOff'));
  } catch (e) {
    toast(errText(e), true);
  }
}
async function reboot() {
  try {
    await api('/api/reboot', { method: 'POST' });
    toast(t('rebooting'));
  } catch (e) {
    toast(errText(e), true);
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
    $('pill').textContent = t('deviceOffline');
  } finally {
    setTimeout(poll, ok ? 500 : 2000);
  }
}

buildChannels();
buildMap();
initLang();
applyLang();
$('count').addEventListener('input', applyCount);
$('count').addEventListener('change', applyCount);
$('save').addEventListener('click', save);
$('test').addEventListener('click', toggleTest);
$('reboot').addEventListener('click', reboot);
$('crsfPin').addEventListener('input', updateWiring);
$('ppmPin').addEventListener('input', updateWiring);
$('langSel').addEventListener('change', () => setLangMode($('langSel').value));

loadConfig().catch((e) => toast(errText(e), true)).finally(() => poll());
</script>
</body>
</html>
)rawliteral";
