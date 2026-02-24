require('dotenv').config();
const express = require('express');
const mqtt = require('mqtt');
const { poolLocal, poolInternet } = require('./config/db');

const app = express();
const port = process.env.PORT || 3000;

// Configuración del broker MQTT
const mqttHost = process.env.MQTT_HOST;
const client = mqtt.connect(mqttHost, {
  username: process.env.MQTT_USER,
  password: process.env.MQTT_PASSWORD
});

// Evento de conexión
client.on('connect', () => {
  console.log('Conectado al broker MQTT!');
  
  // Suscribirse a los topics
  client.subscribe('clima/temperatura', (err) => {
    if (err) {
      console.log('Error al suscribirse a temperatura');
    } else {
      console.log('Suscrito al tópico: clima/temperatura');
    }
  });

  client.subscribe('clima/humedad', (err) => {
    if (err) {
      console.log('Error al suscribirse a humedad');
    } else {
      console.log('Suscrito al tópico: clima/humedad');
    }
  });

    client.subscribe('clima/receptor', (err) => {
    if (err) {
      console.log('Error al suscribirse a receptor');
    } else {
      console.log('Suscrito al tópico: clima/receptor');
    }
  });
});

// Evento de error
client.on('error', (err) => {
  console.log('Error de conexión con el broker MQTT: ', err);
});

// Escuchar los mensajes recibidos
client.on('message', (topic, message) => {
  console.log(`Mensaje recibido en el tópico ${topic}: ${message.toString()}`);

  // Almacenar el mensaje en la base de datos
  storeMessageInDB(topic, message.toString());
  // almacenarEnDBInternet(topic, message.toString());
});

// Conectar a la base de datos SQL (MySQL)
function storeMessageInDB(topic, message) {
  const query = 'INSERT INTO datos_sensores (topic, message) VALUES (?, ?)';

  poolLocal.query(query, [topic, message], (err, result) => {
    if (err) {
      console.log('Error en DB local:', err);
    } else {
      console.log('Insertado en local:', result);
    }
  });

  poolInternet.query(query, [topic, message], (err, result) => {
    if (err) {
      console.log('Error en DB remota:', err);
    } else {
      console.log('Insertado en remoto:', result);
    }
  });

}

// function almacenarEnDBInternet(topic, message) {
//   const query = 'INSERT INTO datos_sensores (topic, message) VALUES (?, ?)';
//   poolInternet.query(query, [topic, message], (err, result) => {
//     if (err) {
//       console.log('Error al almacenar el mensaje en la base de datos:', err);
//     } else {
//       console.log('Mensaje almacenado en la base de datos:', result);
//     }
//   });
// }


// Configurar la base de datos
app.get('/', (req, res) => {
  res.send('Servidor MQTT en funcionamiento');
});

// Iniciar el servidor
app.listen(port, () => {
  console.log(`Servidor escuchando en http://localhost:${port}`);
});
