/*
 * Dashboard HTML embebido para Zaque MAIN
 * Se almacena en un header C++ y se sirve como contenido estático
 * Tamaño: Optimizado para <100KB
 */

#pragma once

#include <pgmspace.h>

// HTML, CSS y JavaScript para el dashboard
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Zaque - Monitoreo de Finca</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        header {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            margin-bottom: 30px;
            text-align: center;
        }
        
        header h1 {
            color: #667eea;
            margin-bottom: 5px;
        }
        
        header p {
            color: #666;
            font-size: 14px;
        }
        
        .summary {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-bottom: 30px;
        }
        
        .summary-card {
            background: white;
            padding: 15px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            text-align: center;
        }
        
        .summary-card .value {
            font-size: 28px;
            font-weight: bold;
            color: #667eea;
            margin: 10px 0;
        }
        
        .summary-card .label {
            font-size: 12px;
            color: #999;
            text-transform: uppercase;
        }
        
        .nodes-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .node-card {
            background: white;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .node-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.15);
        }
        
        .node-header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .node-title {
            font-weight: bold;
            font-size: 16px;
        }
        
        .node-role {
            font-size: 11px;
            opacity: 0.8;
            text-transform: uppercase;
        }
        
        .node-status {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #4CAF50;
        }
        
        .node-status.offline {
            background: #f44336;
        }
        
        .node-content {
            padding: 20px;
        }
        
        .measurement {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 15px;
            padding-bottom: 15px;
            border-bottom: 1px solid #eee;
        }
        
        .measurement:last-child {
            border-bottom: none;
            margin-bottom: 0;
            padding-bottom: 0;
        }
        
        .metric {
            display: flex;
            flex-direction: column;
        }
        
        .metric-label {
            font-size: 12px;
            color: #999;
            text-transform: uppercase;
            margin-bottom: 5px;
        }
        
        .metric-value {
            font-size: 20px;
            font-weight: bold;
            color: #333;
        }
        
        .metric-unit {
            font-size: 12px;
            color: #999;
        }
        
        .gps-info {
            background: #f5f5f5;
            padding: 10px;
            border-radius: 4px;
            font-size: 12px;
            margin: 10px 0;
            font-family: monospace;
        }
        
        .recommendation {
            background: #e8f5e9;
            border-left: 4px solid #4CAF50;
            padding: 12px;
            margin-top: 15px;
            border-radius: 2px;
            font-size: 13px;
            color: #2e7d32;
        }
        
        .recommendation.warning {
            background: #fff3e0;
            border-left-color: #ff9800;
            color: #e65100;
        }
        
        .recommendation.critical {
            background: #ffebee;
            border-left-color: #f44336;
            color: #c62828;
        }
        
        .battery-bar {
            height: 6px;
            background: #ddd;
            border-radius: 3px;
            margin-top: 5px;
            overflow: hidden;
        }
        
        .battery-fill {
            height: 100%;
            background: #4CAF50;
            border-radius: 3px;
            transition: width 0.3s ease, background-color 0.3s ease;
        }
        
        .battery-fill.low {
            background: #ff9800;
        }
        
        .battery-fill.critical {
            background: #f44336;
        }
        
        footer {
            background: white;
            padding: 20px;
            border-radius: 8px;
            text-align: center;
            font-size: 12px;
            color: #999;
            margin-top: 30px;
        }
        
        .loading {
            text-align: center;
            padding: 40px;
            color: white;
        }
        
        .spinner {
            border: 4px solid rgba(255,255,255,0.3);
            border-top: 4px solid white;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .button-group {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }
        
        button {
            flex: 1;
            padding: 10px;
            border: none;
            border-radius: 4px;
            font-size: 12px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }
        
        .btn-refresh {
            background: #667eea;
            color: white;
        }
        
        .btn-refresh:hover {
            background: #5568d3;
        }
        
        .btn-download {
            background: #4CAF50;
            color: white;
        }
        
        .btn-download:hover {
            background: #45a049;
        }
        
        @media (max-width: 600px) {
            .nodes-grid {
                grid-template-columns: 1fr;
            }
            
            .measurement {
                grid-template-columns: 1fr;
            }
            
            .summary {
                grid-template-columns: repeat(2, 1fr);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🌾 Zaque</h1>
            <p>Sistema local de monitoreo agrícola</p>
            <p id="status-time" style="font-size: 12px; margin-top: 10px;">Actualizado: --</p>
        </header>
        
        <div class="summary" id="summary"></div>
        
        <div id="loading" class="loading">
            <div class="spinner"></div>
            <p>Cargando datos...</p>
        </div>
        
        <div id="content" style="display: none;">
            <div class="nodes-grid" id="nodes-grid"></div>
            
            <div class="button-group">
                <button class="btn-refresh" onclick="refreshData()">🔄 Actualizar</button>
                <button class="btn-download" onclick="downloadCSV()">📥 Descargar CSV</button>
            </div>
        </div>
        
        <footer>
            <p>Zaque v0.2.0 • Monitoreo offline para comunidades campesinas</p>
            <p>Las recomendaciones son orientativas y deben ajustarse según el cultivo y acompañamiento técnico local.</p>
        </footer>
    </div>
    
    <script>
        // Configuración
        const API_BASE = window.location.origin;
        const REFRESH_INTERVAL = 60000; // 1 minuto
        
        // Estado
        let latestData = null;
        let autoRefreshInterval = null;
        
        // Inicializar
        document.addEventListener('DOMContentLoaded', function() {
            refreshData();
            // Auto-refresh cada minuto
            autoRefreshInterval = setInterval(refreshData, REFRESH_INTERVAL);
        });
        
        // Obtener datos del API
        async function refreshData() {
            try {
                const response = await fetch(API_BASE + '/api/measurements/latest');
                latestData = await response.json();
                render();
            } catch (error) {
                console.error('Error fetching data:', error);
                document.getElementById('loading').innerHTML = '<p style="color: white;">Error cargando datos. Reintentando...</p>';
            }
        }
        
        // Renderizar interfaz
        function render() {
            if (!latestData || !latestData.nodes) return;
            
            // Resumen
            renderSummary();
            
            // Nodos
            renderNodes();
            
            // Mostrar contenido, ocultar carga
            document.getElementById('loading').style.display = 'none';
            document.getElementById('content').style.display = 'block';
            
            // Actualizar timestamp
            const now = new Date().toLocaleTimeString('es-CO', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
            document.getElementById('status-time').textContent = 'Actualizado: ' + now;
        }
        
        function renderSummary() {
            const nodes = latestData.nodes;
            const activeNodes = nodes.filter(n => isNodeActive(n)).length;
            
            const summary = document.getElementById('summary');
            summary.innerHTML = `
                <div class="summary-card">
                    <div class="label">Nodos Totales</div>
                    <div class="value">${nodes.length}</div>
                </div>
                <div class="summary-card">
                    <div class="label">Nodos Activos</div>
                    <div class="value">${activeNodes}</div>
                </div>
                <div class="summary-card">
                    <div class="label">Última Medición</div>
                    <div class="value">${new Date(latestData.updated_at).toLocaleTimeString('es-CO')}</div>
                </div>
            `;
        }
        
        function renderNodes() {
            const nodes = latestData.nodes;
            const grid = document.getElementById('nodes-grid');
            grid.innerHTML = '';
            
            nodes.forEach(node => {
                grid.appendChild(createNodeCard(node));
            });
        }
        
        function createNodeCard(node) {
            const card = document.createElement('div');
            card.className = 'node-card';
            
            const isActive = isNodeActive(node);
            const roleLabel = node.role === 'main' ? 'PRINCIPAL' : 'SENSOR';
            
            card.innerHTML = `
                <div class="node-header">
                    <div>
                        <div class="node-title">${node.node_name}</div>
                        <div class="node-role">${roleLabel}</div>
                    </div>
                    <div class="node-status ${!isActive ? 'offline' : ''}"></div>
                </div>
                <div class="node-content">
                    <div class="measurement">
                        <div class="metric">
                            <span class="metric-label">Humedad</span>
                            <span class="metric-value">${node.soil_humidity || '--'}<span class="metric-unit">%</span></span>
                        </div>
                        <div class="metric">
                            <span class="metric-label">pH</span>
                            <span class="metric-value">${node.ph || '--'}</span>
                        </div>
                        <div class="metric">
                            <span class="metric-label">Nitrógeno</span>
                            <span class="metric-value">${node.nitrogen || '--'}<span class="metric-unit">mg/kg</span></span>
                        </div>
                        <div class="metric">
                            <span class="metric-label">Fósforo</span>
                            <span class="metric-value">${node.phosphorus || '--'}<span class="metric-unit">mg/kg</span></span>
                        </div>
                    </div>
                    
                    <div class="measurement">
                        <div class="metric">
                            <span class="metric-label">Potasio</span>
                            <span class="metric-value">${node.potassium || '--'}<span class="metric-unit">mg/kg</span></span>
                        </div>
                        <div class="metric">
                            <span class="metric-label">Batería</span>
                            <span class="metric-value">${node.battery_percent || '--'}%</span>
                            <div class="battery-bar">
                                <div class="battery-fill ${getBatteryClass(node.battery_percent)}" 
                                     style="width: ${node.battery_percent || 0}%"></div>
                            </div>
                        </div>
                    </div>
                    
                    ${node.lat && node.lon ? `
                        <div class="gps-info">
                            📍 ${node.lat.toFixed(4)}, ${node.lon.toFixed(4)}
                        </div>
                    ` : ''}
                    
                    ${node.recommendation ? `
                        <div class="recommendation ${getRecommendationClass(node.recommendation)}">
                            💡 ${node.recommendation}
                        </div>
                    ` : ''}
                </div>
            `;
            
            return card;
        }
        
        function isNodeActive(node) {
            if (!node.last_seen) return false;
            const lastSeen = new Date(node.last_seen).getTime();
            const now = Date.now();
            const diffMinutes = (now - lastSeen) / 60000;
            return diffMinutes < 10; // Activo si se vio en los últimos 10 minutos
        }
        
        function getBatteryClass(battery) {
            if (!battery) return '';
            if (battery < 20) return 'critical';
            if (battery < 50) return 'low';
            return '';
        }
        
        function getRecommendationClass(rec) {
            if (rec.includes('crítico') || rec.includes('Cargar')) return 'critical';
            if (rec.includes('revisar') || rec.includes('ácido')) return 'warning';
            return '';
        }
        
        function downloadCSV() {
            window.location.href = API_BASE + '/download/measurements.csv';
        }
    </script>
</body>
</html>
)rawliteral";
