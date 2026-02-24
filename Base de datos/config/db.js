require('dotenv').config();
const mysql = require('mysql2');

// Conexión a base de datos local
const poolLocal = mysql.createPool({
  host: process.env.DB_LOCAL_HOST || 'localhost',
  user: process.env.DB_LOCAL_USER || 'root',
  password: process.env.DB_LOCAL_PASSWORD || '',
  database: process.env.DB_LOCAL_NAME || 'mqtt_data',
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
});

// Conexión a base de datos remota
const poolInternet = mysql.createPool({
  host: process.env.DB_REMOTE_HOST,
  user: process.env.DB_REMOTE_USER,
  password: process.env.DB_REMOTE_PASSWORD,
  database: process.env.DB_REMOTE_NAME,
  port: parseInt(process.env.DB_REMOTE_PORT) || 3306,
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
});

// Exportamos ambas conexiones
module.exports = { poolLocal, poolInternet };