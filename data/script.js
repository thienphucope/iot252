// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var gaugeTemp, gaugeHumi;

window.addEventListener('load', function () {
    initGauges();
    initWebSocket();
});

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen  = function () { console.log('WS connected'); };
    websocket.onclose = function () { setTimeout(initWebSocket, 2000); };
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
    } else {
        alert('⚠️ WebSocket chưa kết nối!');
    }
}

function onMessage(event) {
    try {
        var d = JSON.parse(event.data);
        if (d.type === 'sensor') {
            if (gaugeTemp) gaugeTemp.refresh(d.temp);
            if (gaugeHumi) gaugeHumi.refresh(d.humi);
            updateStatusBadges(d.temp, d.humi, d.lcd);
        } else if (d.type === 'info') {
            updateInfoPanel(d);
        } else if (d.type === 'wifi_list') {
            showWifiList(d.networks);
        } else if (d.type === 'scanning') {
            document.getElementById('wifiScanStatus').style.display = 'flex';
            document.getElementById('wifiList').innerHTML = '';
        } else if (d.type === 'saved') {
            alert('✅ Đã lưu! Thiết bị đang khởi động lại, trang sẽ tự tải lại sau 15 giây.');
            setTimeout(function () { location.reload(); }, 15000);
        }
    } catch (e) {
        console.warn('Invalid JSON:', event.data);
    }
}


// ==================== GAUGES ====================
// Ngưỡng từ global.h: TEMP_NORMAL_LOW=25, TEMP_NORMAL_HIGH=30, HUMI_NORMAL_LOW=40, HUMI_NORMAL_HIGH=60
//
// Temp (−10→50, range=60): 12 stops cách nhau 60/11≈5.45°C
//   stop[6]=22.7°C, stop[7]=28.2°C (≈giữa vùng 25-30), stop[8]=33.6°C (>30)
//   → gradient blue→yellow diễn ra quanh 22-28°C, yellow→red quanh 28-34°C
//
// Humi (0→100): 6 stops × 20% → stop[2] = chính xác 40%, stop[3] = chính xác 60%
//   → transition red→yellow tại 40% (HUMI_NORMAL_LOW), yellow-green→blue tại 60% (HUMI_NORMAL_HIGH)
function initGauges() {
    gaugeTemp = new JustGage({
        id: 'gauge_temp', value: 0, min: -10, max: 50,
        donut: true, pointer: false, gaugeWidthScale: 0.25,
        gaugeColor: '#f0f0f0', levelColorsGradient: true,
        levelColors: [
            '#00BCD4', // −10°C  │
            '#00BCD4', //  −4°C  │ LẠNH (< 25°C)
            '#00BCD4', //   1°C  │
            '#00BCD4', //   6°C  │
            '#00BCD4', //  12°C  │
            '#00BCD4', //  17°C  │
            '#26C6DA', //  23°C  ↓ fade sang vùng normal
            '#FFC107', //  28°C  ← NORMAL (25–30°C)
            '#FF5722', //  34°C  ← trên ngưỡng 30°C
            '#F44336', //  39°C  │
            '#F44336', //  45°C  │ NÓNG (> 30°C)
            '#F44336'  //  50°C  │
        ]
    });
    gaugeHumi = new JustGage({
        id: 'gauge_humi', value: 0, min: 0, max: 100,
        donut: true, pointer: false, gaugeWidthScale: 0.25,
        gaugeColor: '#f0f0f0', levelColorsGradient: true,
        levelColors: [
            '#F44336', //   0%  │ KHÔ (< 40%)
            '#FF7043', //  20%  │
            '#FFC107', //  40%  ← HUMI_NORMAL_LOW  (chính xác)
            '#8BC34A', //  60%  ← HUMI_NORMAL_HIGH (chính xác)
            '#26C6DA', //  80%  │ ẨM (> 60%)
            '#2196F3'  // 100%  │
        ]
    });
}



// ==================== NAVIGATION ====================
function showSection(id, event) {
    document.querySelectorAll('.section').forEach(s => s.style.display = 'none');
    document.getElementById(id).style.display = (id === 'settings') ? 'flex' : 'block';
    document.querySelectorAll('[data-section]').forEach(function (el) {
        el.classList.toggle('active', el.dataset.section === id);
    });
}


// ==================== STATUS BADGES ====================
function updateStatusBadges(temp, humi, lcdStatus) {
    var ledEl = document.getElementById('ledStatus');
    if (ledEl) {
        if (temp > 30) {
            ledEl.textContent = 'NHANH (200ms)';
            ledEl.className = 'badge badge-danger';
        } else if (temp >= 25) {
            ledEl.textContent = 'VỪA (1s)';
            ledEl.className = 'badge badge-warning';
        } else {
            ledEl.textContent = 'CHẬM (2s)';
            ledEl.className = 'badge badge-info';
        }
    }

    var neoEl   = document.getElementById('neoStatus');
    var neoIcon = document.getElementById('neoIcon');
    if (neoEl) {
        if (humi > 60) {
            neoEl.textContent = 'XANH DƯƠNG';
            neoEl.className = 'badge badge-blue';
            if (neoIcon) neoIcon.style.color = '#2196F3';
        } else if (humi >= 40) {
            neoEl.textContent = 'XANH LÁ';
            neoEl.className = 'badge badge-success';
            if (neoIcon) neoIcon.style.color = '#4CAF50';
        } else {
            neoEl.textContent = 'ĐỎ';
            neoEl.className = 'badge badge-danger';
            if (neoIcon) neoIcon.style.color = '#F44336';
        }
    }

    var lcdEl = document.getElementById('lcdStatus');
    if (lcdEl) {
        var s = lcdStatus || 'NORMAL';
        lcdEl.textContent = s;
        lcdEl.className = s === 'CRITICAL' ? 'badge badge-danger' :
                          s === 'WARNING'  ? 'badge badge-warning' :
                                             'badge badge-success';
    }
}


// ==================== INFO PANEL ====================
function updateInfoPanel(d) {
    function set(id, val) {
        var el = document.getElementById(id);
        if (el) el.textContent = val || '--';
    }
    set('info-ip',         d.ip);
    set('info-uptime',     formatUptime(d.uptime));
    set('info-ap-ssid',    d.ssid);
    set('info-ap-pass',    d.password);
    set('info-sta-ssid',   d.WIFI_SSID);
    set('info-sta-pass',   d.WIFI_PASS);
    set('info-iot-token',  d.CORE_IOT_TOKEN);
    set('info-iot-server', d.CORE_IOT_SERVER);
    set('info-iot-port',   d.CORE_IOT_PORT);
}

function formatUptime(seconds) {
    if (!seconds && seconds !== 0) return '--';
    var h = Math.floor(seconds / 3600);
    var m = Math.floor((seconds % 3600) / 60);
    var s = seconds % 60;
    return h + 'h ' + m + 'm ' + s + 's';
}


// ==================== WIFI SCAN ====================
function scanWifi() {
    document.getElementById('wifiScanModal').style.display = 'flex';
    document.getElementById('wifiScanStatus').style.display = 'flex';
    document.getElementById('wifiList').innerHTML = '';
    Send_Data(JSON.stringify({ page: 'scan_wifi' }));
}

function closeWifiScan() {
    document.getElementById('wifiScanModal').style.display = 'none';
}

function showWifiList(networks) {
    document.getElementById('wifiScanStatus').style.display = 'none';
    var listEl = document.getElementById('wifiList');
    listEl.innerHTML = '';

    if (!networks || networks.length === 0) {
        listEl.innerHTML = '<p class="no-wifi">Không tìm thấy mạng nào</p>';
        return;
    }

    networks.sort(function (a, b) { return b.rssi - a.rssi; });

    networks.forEach(function (net) {
        var item = document.createElement('div');
        item.className = 'wifi-item';
        item.innerHTML =
            '<div class="wifi-item-left">' +
                '<span class="wifi-name">' + net.ssid + '</span>' +
                '<span class="wifi-rssi">' + net.rssi + ' dBm</span>' +
            '</div>' +
            '<div class="wifi-item-right">' +
                (net.secure ? '<i class="fa-solid fa-lock wifi-lock"></i>' : '<i class="fa-solid fa-lock-open wifi-open"></i>') +
                '<span class="wifi-bars">' + getRssiBars(net.rssi) + '</span>' +
            '</div>';
        item.onclick = function () {
            document.getElementById('ssid').value = net.ssid;
            closeWifiScan();
            document.getElementById('password').focus();
        };
        listEl.appendChild(item);
    });
}

function getRssiBars(rssi) {
    if (rssi >= -50) return '<i class="fa-solid fa-signal" style="color:#4CAF50"></i>';
    if (rssi >= -65) return '<i class="fa-solid fa-signal" style="color:#8BC34A"></i>';
    if (rssi >= -75) return '<i class="fa-solid fa-signal-medium" style="color:#FFC107"></i>';
    return '<i class="fa-solid fa-signal-weak" style="color:#F44336"></i>';
}


// ==================== RELAY (DEVICE) ====================
var relayList = [];
var deleteTarget = null;

function openAddRelayDialog() {
    document.getElementById('relayName').value = '';
    document.getElementById('relayGPIO').value = '';
    document.getElementById('addRelayDialog').style.display = 'flex';
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}
function saveRelay() {
    var name = document.getElementById('relayName').value.trim();
    var gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) { alert('⚠️ Vui lòng điền đầy đủ!'); return; }
    relayList.push({ id: Date.now(), name: name, gpio: gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    var container = document.getElementById('relayContainer');
    if (relayList.length === 0) {
        container.innerHTML = '<p class="empty-hint">Nhấn <i class="fa-solid fa-plus"></i> để thêm relay mới</p>';
        return;
    }
    container.innerHTML = '';
    relayList.forEach(function (r) {
        var card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML =
            '<i class="fa-solid fa-bolt device-icon"></i>' +
            '<h3>' + r.name + '</h3>' +
            '<p>GPIO: ' + r.gpio + '</p>' +
            '<button class="toggle-btn ' + (r.state ? 'on' : '') + '" onclick="toggleRelay(' + r.id + ')">' +
                (r.state ? 'ON' : 'OFF') +
            '</button>' +
            '<i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(' + r.id + ')"></i>';
        container.appendChild(card);
    });
}
function toggleRelay(id) {
    var relay = relayList.find(function (r) { return r.id === id; });
    if (relay) {
        relay.state = !relay.state;
        Send_Data(JSON.stringify({
            page: 'device',
            value: { name: relay.name, status: relay.state ? 'ON' : 'OFF', gpio: relay.gpio }
        }));
        renderRelays();
    }
}
function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}
function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}
function confirmDelete() {
    relayList = relayList.filter(function (r) { return r.id !== deleteTarget; });
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS ====================
document.getElementById('settingsForm').addEventListener('submit', function (e) {
    e.preventDefault();
    Send_Data(JSON.stringify({
        page: 'setting',
        value: {
            ssid:     document.getElementById('ssid').value.trim(),
            password: document.getElementById('password').value.trim(),
            token:    document.getElementById('token').value.trim(),
            server:   document.getElementById('server').value.trim(),
            port:     document.getElementById('port').value.trim()
        }
    }));
});
