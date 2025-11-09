// Base layers
const osm = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
});

const esriSat = L.tileLayer(
    'https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}',
    {
        maxZoom: 19,
        attribution: 'Tiles &copy; Esri — Source: Esri, USGS, NOAA, etc.'
    }
);

// Create map (avoid naming it `map` to prevent window.map collisions)
const leafletMap = L.map('map', {
    center: [42.6977, 23.3219],  // Sofia
    zoom: 13,
    layers: [osm]
});

// Layer switcher
L.control.layers({ "OpenStreetMap": osm, "Satellite": esriSat }, null, { collapsed: false }).addTo(leafletMap);

const markersLayer = L.layerGroup();
markersLayer.addTo(leafletMap);

// Colors for distinct pins
const palette = [
    '#e6194B', '#3cb44b', '#ffe119', '#0082c8', '#f58231',
    '#911eb4', '#46f0f0', '#f032e6', '#d2f53c', '#fabebe',
    '#008080', '#e6beff', '#aa6e28', '#fffac8', '#800000',
    '#aaffc3', '#808000', '#ffd8b1', '#000080', '#808080'
];

const fmt = (n, d = 5) => Number(n).toFixed(d);
const tooltipHTML = (p) => (
    `<div>
    <div><b>${p.id}</b></div>
    <div>Lat: ${fmt(p.lat)}, Lon: ${fmt(p.lon)}</div>
    <div>HR: ${p.hr} bpm</div>
    </div>`
);

async function fetchPointers() {
    // Use RELATIVE path so it works under any local URL base
    const url = './participants.json';
    try {
        const r = await fetch(url, { cache: 'no-cache' });
        if (!r.ok) throw new Error('HTTP ' + r.status);
        const arr = await r.json();
        return Array.isArray(arr) ? arr : [];
    } catch (e) {
        console.warn('Fetch failed, using fallback data:', e);
    }
}

function upsertMarkers(pointers) {
    markersLayer.clearLayers();
    const bounds = [];
    pointers.forEach((p, i) => {
        const color = palette[i % palette.length];
        const m = L.circleMarker([p.lat, p.lon], {
            radius: 8,
            weight: 2,
            color,
            fillColor: color,
            fillOpacity: 0.75
        }).addTo(markersLayer);

        m.bindTooltip(tooltipHTML(p), {
            direction: 'top',
            sticky: true,
            opacity: 0.95,
            className: 'hr-tooltip'
        });

        m.on('mouseover', () => m.openTooltip());
        m.on('mouseout', () => m.closeTooltip());
        bounds.push([p.lat, p.lon]);
    });

    if (bounds.length) {
        leafletMap.fitBounds(bounds, { padding: [30, 30] });
    }
}

(async () => {
    const pointers = await fetchPointers();
    upsertMarkers(pointers);
})();