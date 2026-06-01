# Implementación de Motor de Recomendaciones Agrícolas en ZAQUE

## 1. Objetivo

Este documento describe cómo implementar un motor de recomendaciones agrícolas para ZAQUE a partir de mediciones enviadas por nodos sensores ESP32.

El sistema debe recibir datos como humedad del suelo, pH, nitrógeno, fósforo, potasio, ubicación GPS, batería y versión de firmware. Con esa información debe generar:

- Diagnóstico del estado del lote.
- Recomendaciones de riego.
- Recomendaciones de fertilización o manejo del suelo.
- Recomendaciones según cultivo existente.
- Sugerencias de cultivos posibles.
- Alertas para el campesino.
- Un párrafo natural para mostrar en la web.

> Nota importante: las recomendaciones generadas por sensores NPK deben tratarse como orientación inicial. Para decisiones exactas de fertilización, se recomienda validar con análisis de suelo o acompañamiento técnico agrícola.

---

## 2. Ejemplo de entrada esperada

```bash
curl -X POST http://zaque.local/api/measurements \
  -H "Content-Type: application/json" \
  -d '{
    "api_key": "ZAQUE_LOCAL_KEY",
    "node_id": "zona_2",
    "node_name": "Lote de cultivo",
    "role": "sensor",
    "timestamp": "2026-05-31T16:20:00",
    "lat": 4.7112,
    "lon": -74.0721,
    "soil_humidity": 64.0,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41,
    "battery_percent": 83,
    "firmware_version": "0.2.0"
  }'
```

---

## 3. Estructura recomendada del backend

```txt
src/
  routes/
    measurements.js

  services/
    recommendationEngine.js

  rules/
    humidityRules.js
    phRules.js
    nutrientRules.js
    batteryRules.js
    cropRules.js

  utils/
    classifyValue.js
```

Si tu backend todavía está simple, también puedes empezar con un solo archivo:

```txt
src/services/recommendationEngine.js
```

Luego, cuando crezca, lo divides en reglas separadas.

---

## 4. Modelo general del sistema

El flujo del sistema debe ser:

```txt
ESP32 / Nodo sensor
        ↓
POST /api/measurements
        ↓
Validar api_key y datos recibidos
        ↓
Guardar medición en base de datos o archivo local
        ↓
Analizar medición con motor de reglas
        ↓
Generar resumen humano para web
        ↓
Responder JSON al frontend
```

---

## 5. Rangos base de interpretación

Estos rangos son una primera versión práctica. Deben calibrarse con pruebas reales, tipo de sensor, cultivo, zona, clima y apoyo agronómico.

### 5.1 Humedad del suelo

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 25% | Muy baja | Riego urgente |
| 25 - 40% | Baja | Riego moderado |
| 40 - 70% | Adecuada | No regar por ahora |
| 70 - 85% | Alta | Evitar riego, revisar drenaje |
| 85 - 100% | Muy alta | Riesgo de encharcamiento y hongos |

### 5.2 pH del suelo

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 5.0 | Muy ácido | Revisar corrección con análisis de suelo |
| 5.0 - 5.8 | Ácido | Vigilar cultivos sensibles |
| 5.8 - 6.8 | Favorable | Buen rango para muchos cultivos |
| 6.8 - 7.5 | Neutro | Adecuado para variedad amplia |
| 7.5 - 8.5 | Alcalino | Puede limitar nutrientes |
| 8.5 - 14 | Muy alcalino | Requiere revisión técnica |

### 5.3 Nitrógeno

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 15 | Muy bajo | Posible amarillamiento y bajo crecimiento |
| 15 - 30 | Bajo | Aplicar materia orgánica o fuente nitrogenada moderada |
| 30 - 50 | Medio | Vigilar hojas amarillas |
| 50 - 80 | Adecuado | Mantener manejo actual |
| 80+ | Alto | Evitar más nitrógeno |

### 5.4 Fósforo

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 10 | Muy bajo | Puede afectar raíces y arranque |
| 10 - 25 | Bajo | Reforzar con compost, bocashi o fuente fosfórica |
| 25 - 45 | Medio | Mantener materia orgánica |
| 45 - 70 | Adecuado | Mantener manejo actual |
| 70+ | Alto | Evitar más fósforo sin análisis |

### 5.5 Potasio

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 15 | Muy bajo | Puede afectar resistencia, fruto o tubérculo |
| 15 - 30 | Bajo | Revisar en cultivos de fruto o papa |
| 30 - 50 | Aceptable | Volver a medir en floración o llenado |
| 50 - 80 | Adecuado | Mantener manejo actual |
| 80+ | Alto | Evitar más potasio sin análisis |

### 5.6 Batería del nodo

| Rango | Estado | Acción sugerida |
|---|---|---|
| 0 - 15% | Crítica | Cargar o revisar alimentación |
| 15 - 35% | Baja | Revisar batería o panel solar |
| 35 - 70% | Media | Monitorear |
| 70 - 100% | Buena | Sin acción inmediata |

---

## 6. Implementación del motor de reglas

Crear archivo:

```txt
src/services/recommendationEngine.js
```

Código sugerido:

```js
const HUMIDITY_RULES = {
  very_low: {
    min: 0,
    max: 25,
    status: "muy baja",
    severity: "alert",
    message: "El suelo está muy seco. Se recomienda regar pronto y revisar si el cultivo presenta marchitez."
  },
  low: {
    min: 25,
    max: 40,
    status: "baja",
    severity: "warning",
    message: "La humedad está baja. Conviene programar un riego moderado."
  },
  optimal: {
    min: 40,
    max: 70,
    status: "adecuada",
    severity: "good",
    message: "La humedad del suelo es adecuada. No es necesario regar de inmediato."
  },
  high: {
    min: 70,
    max: 85,
    status: "alta",
    severity: "warning",
    message: "La humedad está alta. Evita regar y revisa que el suelo tenga buen drenaje."
  },
  very_high: {
    min: 85,
    max: 101,
    status: "muy alta",
    severity: "alert",
    message: "El suelo está demasiado húmedo. Hay riesgo de encharcamiento, pudrición de raíces y hongos."
  }
};

const PH_RULES = {
  very_acid: {
    min: 0,
    max: 5.0,
    status: "muy ácido",
    severity: "alert",
    message: "El suelo está muy ácido. Puede limitar la disponibilidad de nutrientes. Considera análisis de suelo antes de aplicar cal."
  },
  acid: {
    min: 5.0,
    max: 5.8,
    status: "ácido",
    severity: "warning",
    message: "El suelo está ácido. Algunos cultivos pueden crecer bien, pero otros pueden necesitar corrección."
  },
  favorable: {
    min: 5.8,
    max: 6.8,
    status: "favorable",
    severity: "good",
    message: "El pH está en un rango favorable para muchos cultivos."
  },
  neutral: {
    min: 6.8,
    max: 7.5,
    status: "neutro",
    severity: "good",
    message: "El pH está cercano a neutro, adecuado para una gran variedad de cultivos."
  },
  alkaline: {
    min: 7.5,
    max: 8.5,
    status: "alcalino",
    severity: "warning",
    message: "El suelo está alcalino. Algunos nutrientes pueden estar menos disponibles para las plantas."
  },
  very_alkaline: {
    min: 8.5,
    max: 14.1,
    status: "muy alcalino",
    severity: "alert",
    message: "El suelo está muy alcalino. Se recomienda revisar el manejo del suelo con apoyo técnico."
  }
};

const NITROGEN_RULES = {
  very_low: {
    min: 0,
    max: 15,
    status: "muy bajo",
    severity: "alert",
    message: "El nitrógeno está muy bajo. El cultivo puede presentar hojas amarillas y crecimiento lento."
  },
  low: {
    min: 15,
    max: 30,
    status: "bajo",
    severity: "warning",
    message: "El nitrógeno está bajo. Puede ser útil aplicar compost, bocashi o una fuente nitrogenada moderada."
  },
  medium: {
    min: 30,
    max: 50,
    status: "medio",
    severity: "ok",
    message: "El nitrógeno está en nivel medio. Vigila hojas amarillas, especialmente en cultivos de hoja."
  },
  good: {
    min: 50,
    max: 80,
    status: "adecuado",
    severity: "good",
    message: "El nitrógeno está en un nivel adecuado para el crecimiento del cultivo."
  },
  high: {
    min: 80,
    max: 9999,
    status: "alto",
    severity: "warning",
    message: "El nitrógeno está alto. Evita aplicar más nitrógeno para no generar plantas débiles o exceso de follaje."
  }
};

const PHOSPHORUS_RULES = {
  very_low: {
    min: 0,
    max: 10,
    status: "muy bajo",
    severity: "alert",
    message: "El fósforo está muy bajo. Puede afectar el desarrollo de raíces y el arranque del cultivo."
  },
  low: {
    min: 10,
    max: 25,
    status: "bajo",
    severity: "warning",
    message: "El fósforo está bajo o medio-bajo. Conviene reforzar con compost maduro, bocashi o una fuente fosfórica recomendada."
  },
  medium: {
    min: 25,
    max: 45,
    status: "medio",
    severity: "ok",
    message: "El fósforo está en nivel medio. Mantén materia orgánica y revisa el desarrollo de raíces."
  },
  good: {
    min: 45,
    max: 70,
    status: "adecuado",
    severity: "good",
    message: "El fósforo está en un nivel adecuado para el cultivo."
  },
  high: {
    min: 70,
    max: 9999,
    status: "alto",
    severity: "warning",
    message: "El fósforo está alto. Evita aplicar más fuentes fosfóricas sin análisis adicional."
  }
};

const POTASSIUM_RULES = {
  very_low: {
    min: 0,
    max: 15,
    status: "muy bajo",
    severity: "alert",
    message: "El potasio está muy bajo. Puede afectar la resistencia de la planta y la calidad de frutos o tubérculos."
  },
  low: {
    min: 15,
    max: 30,
    status: "bajo",
    severity: "warning",
    message: "El potasio está bajo. Revisa especialmente si el cultivo está en floración, fruto o formación de tubérculo."
  },
  medium: {
    min: 30,
    max: 50,
    status: "aceptable",
    severity: "ok",
    message: "El potasio está en un nivel aceptable."
  },
  good: {
    min: 50,
    max: 80,
    status: "adecuado",
    severity: "good",
    message: "El potasio está en buen nivel para el cultivo."
  },
  high: {
    min: 80,
    max: 9999,
    status: "alto",
    severity: "warning",
    message: "El potasio está alto. Evita aplicar más potasio sin análisis adicional."
  }
};

const BATTERY_RULES = {
  critical: {
    min: 0,
    max: 15,
    status: "crítica",
    severity: "alert",
    message: "La batería del nodo está crítica. Se recomienda cargar o revisar el sistema de energía."
  },
  low: {
    min: 15,
    max: 35,
    status: "baja",
    severity: "warning",
    message: "La batería está baja. Conviene revisar carga, panel solar o alimentación."
  },
  medium: {
    min: 35,
    max: 70,
    status: "media",
    severity: "ok",
    message: "La batería está en nivel medio. El nodo puede seguir funcionando, pero conviene monitorearlo."
  },
  good: {
    min: 70,
    max: 101,
    status: "buena",
    severity: "good",
    message: "La batería del nodo está en buen estado."
  }
};

function classifyValue(value, rules) {
  if (value === null || value === undefined || Number.isNaN(Number(value))) {
    return {
      key: "missing",
      status: "sin dato",
      severity: "unknown",
      message: "No hay dato disponible para esta variable."
    };
  }

  const numericValue = Number(value);

  for (const [key, rule] of Object.entries(rules)) {
    if (numericValue >= rule.min && numericValue < rule.max) {
      return {
        key,
        ...rule
      };
    }
  }

  return {
    key: "unknown",
    status: "fuera de rango",
    severity: "unknown",
    message: "El valor está fuera del rango esperado. Revisa el sensor o la calibración."
  };
}

function getIrrigationRecommendation(soilHumidity) {
  if (soilHumidity < 25) {
    return {
      action: "regar_urgente",
      title: "Riego urgente",
      message: "El suelo está muy seco. Se recomienda regar pronto, preferiblemente en la mañana o al final de la tarde."
    };
  }

  if (soilHumidity >= 25 && soilHumidity < 40) {
    return {
      action: "regar_moderado",
      title: "Riego moderado",
      message: "La humedad está baja. Puedes aplicar un riego moderado sin saturar el suelo."
    };
  }

  if (soilHumidity >= 40 && soilHumidity <= 70) {
    return {
      action: "no_regar",
      title: "No regar por ahora",
      message: "La humedad es suficiente. No riegues todavía si el suelo sigue húmedo al tacto."
    };
  }

  if (soilHumidity > 70 && soilHumidity <= 85) {
    return {
      action: "evitar_riego",
      title: "Evitar riego",
      message: "La humedad está alta. Evita regar y revisa que no haya zonas encharcadas."
    };
  }

  return {
    action: "riesgo_encharcamiento",
    title: "Riesgo de encharcamiento",
    message: "El suelo está demasiado húmedo. Revisa drenajes, raíces y señales de hongos."
  };
}

function getPHRecommendation(ph) {
  if (ph < 5.0) {
    return {
      action: "revisar_acidez",
      title: "Suelo muy ácido",
      message: "El pH está muy bajo. Puede ser necesario corregir acidez, pero primero conviene confirmar con análisis de suelo."
    };
  }

  if (ph >= 5.0 && ph < 5.8) {
    return {
      action: "vigilar_acidez",
      title: "Suelo ácido",
      message: "El suelo está ácido. Puede servir para algunos cultivos, pero otros pueden necesitar corrección."
    };
  }

  if (ph >= 5.8 && ph <= 6.8) {
    return {
      action: "ph_favorable",
      title: "pH favorable",
      message: "El pH está en buen rango para muchos cultivos. No apliques cal ni productos acidificantes sin necesidad."
    };
  }

  if (ph > 6.8 && ph <= 7.5) {
    return {
      action: "ph_neutro",
      title: "pH cercano a neutro",
      message: "El pH está cercano a neutro y puede funcionar bien para una amplia variedad de cultivos."
    };
  }

  if (ph > 7.5 && ph <= 8.5) {
    return {
      action: "vigilar_alcalinidad",
      title: "Suelo alcalino",
      message: "El suelo está alcalino. Algunos nutrientes pueden ser más difíciles de absorber."
    };
  }

  return {
    action: "revisar_alcalinidad",
    title: "Suelo muy alcalino",
    message: "El pH está muy alto. Se recomienda apoyo técnico antes de corregir el suelo."
  };
}

function getFertilizationRecommendations(data) {
  const recommendations = [];

  if (data.nitrogen < 30) {
    recommendations.push({
      nutrient: "nitrogen",
      priority: "high",
      title: "Nitrógeno bajo",
      message: "El nitrógeno está bajo. Puedes reforzar con compost maduro, bocashi, lombricompost o una fuente nitrogenada moderada."
    });
  } else if (data.nitrogen >= 30 && data.nitrogen < 50) {
    recommendations.push({
      nutrient: "nitrogen",
      priority: "medium",
      title: "Nitrógeno medio",
      message: "El nitrógeno está en nivel medio. Vigila hojas amarillas, especialmente en cultivos de hoja como lechuga, acelga, espinaca o cilantro."
    });
  } else if (data.nitrogen >= 80) {
    recommendations.push({
      nutrient: "nitrogen",
      priority: "warning",
      title: "Nitrógeno alto",
      message: "El nitrógeno está alto. Evita aplicar más nitrógeno por ahora."
    });
  }

  if (data.phosphorus < 25) {
    recommendations.push({
      nutrient: "phosphorus",
      priority: "high",
      title: "Fósforo bajo",
      message: "El fósforo está bajo o medio-bajo. Refuerza con compost, bocashi o una fuente fosfórica recomendada, especialmente antes de sembrar."
    });
  } else if (data.phosphorus >= 25 && data.phosphorus < 45) {
    recommendations.push({
      nutrient: "phosphorus",
      priority: "medium",
      title: "Fósforo medio",
      message: "El fósforo está en nivel medio. Mantén materia orgánica para favorecer raíces y arranque del cultivo."
    });
  } else if (data.phosphorus >= 70) {
    recommendations.push({
      nutrient: "phosphorus",
      priority: "warning",
      title: "Fósforo alto",
      message: "El fósforo está alto. No apliques más fuentes fosfóricas sin análisis adicional."
    });
  }

  if (data.potassium < 30) {
    recommendations.push({
      nutrient: "potassium",
      priority: "high",
      title: "Potasio bajo",
      message: "El potasio está bajo. Es importante revisarlo si tienes cultivos de fruto, floración o tubérculo."
    });
  } else if (data.potassium >= 30 && data.potassium < 50) {
    recommendations.push({
      nutrient: "potassium",
      priority: "medium",
      title: "Potasio aceptable",
      message: "El potasio está aceptable. Vuelve a medir en etapa de floración, fruto o formación de tubérculos."
    });
  } else if (data.potassium >= 80) {
    recommendations.push({
      nutrient: "potassium",
      priority: "warning",
      title: "Potasio alto",
      message: "El potasio está alto. Evita aplicar más potasio por ahora."
    });
  }

  return recommendations;
}

function getCropCareRecommendations(cropType, data) {
  const recommendations = [];

  switch (cropType) {
    case "sin_cultivo":
      recommendations.push("El lote parece apto para preparar siembra. Limpia malezas agresivas, incorpora materia orgánica madura y evita sembrar si el suelo está encharcado.");
      break;

    case "hortalizas_hoja":
      recommendations.push("Para hortalizas de hoja, mantén humedad estable y vigila amarillamiento. Si las hojas están pálidas, puede faltar nitrógeno.");
      if (data.soil_humidity > 70) {
        recommendations.push("Como la humedad está alta, revisa riesgo de hongos, babosas o pudrición en la base de la planta.");
      }
      break;

    case "raices_tuberculos":
      recommendations.push("Para raíces o tubérculos, evita exceso de nitrógeno. Prioriza suelo suelto, buen drenaje y materia orgánica bien descompuesta.");
      if (data.potassium < 30) {
        recommendations.push("El potasio está bajo para un cultivo de raíz o tubérculo. Conviene revisarlo antes de la etapa de llenado.");
      }
      break;

    case "leguminosas":
      recommendations.push("Para leguminosas como fríjol, arveja o haba, evita aplicar demasiado nitrógeno. Mantén buen drenaje y revisa floración.");
      break;

    case "frutales":
      recommendations.push("Para frutales, mantén cobertura del suelo, evita encharcamiento y revisa potasio durante floración y formación de fruto.");
      break;

    case "cultivos_fruto":
      recommendations.push("Para cultivos de fruto como tomate, fresa o uchuva, cuida el potasio, evita mojar hojas al regar y revisa señales de hongos.");
      break;

    case "maiz":
      recommendations.push("Para maíz, el nitrógeno es importante durante crecimiento. Vigila hojas amarillas en la parte baja de la planta.");
      break;

    case "papa":
      recommendations.push("Para papa, evita encharcamientos, realiza aporque si corresponde y no abuses del nitrógeno cuando empiece la formación de tubérculos.");
      break;

    case "tomate":
      recommendations.push("Para tomate, evita mojar las hojas, mantén buena ventilación, usa tutorado y revisa manchas por hongos.");
      break;

    case "fresa":
      recommendations.push("Para fresa, usa cobertura para que el fruto no toque el suelo húmedo y revisa hongos si la humedad se mantiene alta.");
      break;

    case "cafe":
      recommendations.push("Para café, conserva cobertura vegetal, evita erosión y revisa el pH, ya que el cultivo puede tolerar suelos ligeramente ácidos.");
      break;

    default:
      recommendations.push("Mantén seguimiento del cultivo, revisa color de hojas, crecimiento, plagas, hongos y humedad del suelo.");
      break;
  }

  return recommendations;
}

function suggestCrops(data) {
  const crops = [];

  const phGood = data.ph >= 5.8 && data.ph <= 6.8;
  const humidityGood = data.soil_humidity >= 40 && data.soil_humidity <= 70;

  if (phGood && humidityGood) {
    crops.push("lechuga");
    crops.push("acelga");
    crops.push("cilantro");
    crops.push("espinaca");
    crops.push("zanahoria");
    crops.push("remolacha");
    crops.push("papa criolla");
    crops.push("arveja");
    crops.push("haba");
    crops.push("fresa");
    crops.push("uchuva");
  }

  if (data.ph < 5.8) {
    crops.push("papa");
    crops.push("mora");
    crops.push("café, si el clima y la altura son adecuados");
  }

  if (data.soil_humidity < 40) {
    crops.push("maíz");
    crops.push("fríjol");
    crops.push("aromáticas resistentes");
  }

  if (data.soil_humidity > 70) {
    crops.push("cultivos que toleren humedad, siempre que haya buen drenaje");
  }

  return [...new Set(crops)];
}

function generateAlerts(data) {
  const alerts = [];

  if (data.soil_humidity < 25) {
    alerts.push({
      type: "water",
      level: "danger",
      title: "Suelo muy seco",
      message: "Riego recomendado pronto."
    });
  }

  if (data.soil_humidity > 85) {
    alerts.push({
      type: "water",
      level: "danger",
      title: "Riesgo de encharcamiento",
      message: "Revisar drenaje y evitar riego."
    });
  }

  if (data.ph < 5.0) {
    alerts.push({
      type: "ph",
      level: "warning",
      title: "pH muy ácido",
      message: "Puede limitar el crecimiento de algunos cultivos."
    });
  }

  if (data.ph > 8.5) {
    alerts.push({
      type: "ph",
      level: "warning",
      title: "pH muy alcalino",
      message: "Puede afectar la disponibilidad de nutrientes."
    });
  }

  if (data.nitrogen < 15) {
    alerts.push({
      type: "nutrient",
      level: "warning",
      title: "Nitrógeno muy bajo",
      message: "Vigilar hojas amarillas y crecimiento lento."
    });
  }

  if (data.phosphorus < 10) {
    alerts.push({
      type: "nutrient",
      level: "warning",
      title: "Fósforo muy bajo",
      message: "Puede afectar raíces y arranque del cultivo."
    });
  }

  if (data.potassium < 15) {
    alerts.push({
      type: "nutrient",
      level: "warning",
      title: "Potasio muy bajo",
      message: "Puede afectar resistencia, fruto o tubérculos."
    });
  }

  if (data.battery_percent < 20) {
    alerts.push({
      type: "device",
      level: "danger",
      title: "Batería baja",
      message: "Revisar alimentación del nodo sensor."
    });
  }

  return alerts;
}

function analyzeMeasurement(data, cropType = "sin_cultivo") {
  const humidity = classifyValue(data.soil_humidity, HUMIDITY_RULES);
  const ph = classifyValue(data.ph, PH_RULES);
  const nitrogen = classifyValue(data.nitrogen, NITROGEN_RULES);
  const phosphorus = classifyValue(data.phosphorus, PHOSPHORUS_RULES);
  const potassium = classifyValue(data.potassium, POTASSIUM_RULES);
  const battery = classifyValue(data.battery_percent, BATTERY_RULES);

  const irrigation = getIrrigationRecommendation(data.soil_humidity);
  const phRecommendation = getPHRecommendation(data.ph);
  const fertilization = getFertilizationRecommendations(data);
  const cropCare = getCropCareRecommendations(cropType, data);
  const suggestedCrops = suggestCrops(data);
  const alerts = generateAlerts(data);

  return {
    node_id: data.node_id,
    node_name: data.node_name,
    timestamp: data.timestamp,
    location: {
      lat: data.lat,
      lon: data.lon
    },
    status: {
      humidity,
      ph,
      nitrogen,
      phosphorus,
      potassium,
      battery
    },
    recommendations: {
      irrigation,
      ph: phRecommendation,
      fertilization,
      cropCare,
      suggestedCrops
    },
    alerts
  };
}

function buildWebSummary(data, analysis) {
  const cropList = analysis.recommendations.suggestedCrops.slice(0, 8).join(", ");

  let generalStatus = "Tu lote necesita revisión.";

  const goodPH = data.ph >= 5.8 && data.ph <= 7.5;
  const goodHumidity = data.soil_humidity >= 40 && data.soil_humidity <= 70;
  const goodPotassium = data.potassium >= 30 && data.potassium < 80;

  if (goodPH && goodHumidity && goodPotassium) {
    generalStatus = "Tu lote está en buen estado para sembrar o mantener cultivo.";
  } else if (goodPH && !goodHumidity) {
    generalStatus = "Tu lote tiene buen pH, pero necesita atención en el manejo del agua.";
  } else if (!goodPH && goodHumidity) {
    generalStatus = "Tu lote tiene buena humedad, pero el pH necesita revisión.";
  }

  const irrigationText = analysis.recommendations.irrigation.message;

  let nutrientText = "";

  if (data.phosphorus < 25) {
    nutrientText = "Refuerza el suelo con compost maduro o bocashi, especialmente para mejorar fósforo y estimular raíces.";
  } else if (data.nitrogen < 30) {
    nutrientText = "Refuerza el suelo con materia orgánica madura para mejorar el nitrógeno disponible.";
  } else if (data.potassium < 30) {
    nutrientText = "Revisa el potasio si tienes cultivos de fruto, floración o tubérculo.";
  } else {
    nutrientText = "Mantén el suelo cubierto y continúa aportando materia orgánica de forma moderada.";
  }

  let cropText = "";

  if (cropList.length > 0) {
    cropText = `Puedes considerar cultivos como ${cropList}, según clima, altura y mercado local.`;
  } else {
    cropText = "Antes de escoger cultivo, revisa clima, altura, disponibilidad de agua y mercado local.";
  }

  const careText = "Si ya tienes cultivo, revisa hojas amarillas, manchas, pudrición, plagas o crecimiento lento.";
  const followUpText = "Repite la medición después de lluvia, riego o aplicación de abono.";

  return `${generalStatus} El pH de ${data.ph} es ${analysis.status.ph.status}, la humedad de ${data.soil_humidity}% es ${analysis.status.humidity.status} y el potasio está ${analysis.status.potassium.status}. ${irrigationText} ${nutrientText} ${cropText} ${careText} ${followUpText}`;
}

module.exports = {
  analyzeMeasurement,
  buildWebSummary
};
```

---

## 7. Uso desde una ruta Express

Crear o modificar:

```txt
src/routes/measurements.js
```

Ejemplo:

```js
const express = require("express");
const router = express.Router();

const {
  analyzeMeasurement,
  buildWebSummary
} = require("../services/recommendationEngine");

const API_KEY = process.env.ZAQUE_API_KEY || "ZAQUE_LOCAL_KEY";

router.post("/measurements", async (req, res) => {
  try {
    const data = req.body;

    if (!data.api_key || data.api_key !== API_KEY) {
      return res.status(401).json({
        success: false,
        error: "API key inválida"
      });
    }

    const requiredFields = [
      "node_id",
      "node_name",
      "timestamp",
      "soil_humidity",
      "ph",
      "nitrogen",
      "phosphorus",
      "potassium",
      "battery_percent"
    ];

    const missingFields = requiredFields.filter((field) => data[field] === undefined || data[field] === null);

    if (missingFields.length > 0) {
      return res.status(400).json({
        success: false,
        error: "Faltan campos obligatorios",
        missingFields
      });
    }

    const cropType = data.crop_type || "sin_cultivo";

    const analysis = analyzeMeasurement(data, cropType);
    const summary = buildWebSummary(data, analysis);

    // Aquí puedes guardar en BD o archivo local.
    // await saveMeasurement(data, analysis, summary);

    return res.json({
      success: true,
      message: "Medición recibida correctamente",
      summary,
      analysis
    });
  } catch (error) {
    console.error("Error procesando medición:", error);

    return res.status(500).json({
      success: false,
      error: "Error interno procesando la medición"
    });
  }
});

module.exports = router;
```

En tu archivo principal del servidor:

```js
const express = require("express");
const app = express();

const measurementRoutes = require("./routes/measurements");

app.use(express.json());
app.use("/api", measurementRoutes);

app.listen(3000, () => {
  console.log("ZAQUE API corriendo en puerto 3000");
});
```

---

## 8. Respuesta esperada del endpoint

Con los datos de ejemplo, el sistema debería responder algo parecido a:

```json
{
  "success": true,
  "message": "Medición recibida correctamente",
  "summary": "Tu lote está en buen estado para sembrar o mantener cultivo. El pH de 6.2 es favorable, la humedad de 64% es adecuada y el potasio está aceptable. La humedad es suficiente. No riegues todavía si el suelo sigue húmedo al tacto. Refuerza el suelo con compost maduro o bocashi, especialmente para mejorar fósforo y estimular raíces. Puedes considerar cultivos como lechuga, acelga, cilantro, espinaca, zanahoria, remolacha, papa criolla, arveja, según clima, altura y mercado local. Si ya tienes cultivo, revisa hojas amarillas, manchas, pudrición, plagas o crecimiento lento. Repite la medición después de lluvia, riego o aplicación de abono.",
  "analysis": {
    "node_id": "zona_2",
    "node_name": "Lote de cultivo",
    "status": {
      "humidity": {
        "status": "adecuada",
        "severity": "good"
      },
      "ph": {
        "status": "favorable",
        "severity": "good"
      },
      "nitrogen": {
        "status": "medio",
        "severity": "ok"
      },
      "phosphorus": {
        "status": "bajo",
        "severity": "warning"
      },
      "potassium": {
        "status": "aceptable",
        "severity": "ok"
      },
      "battery": {
        "status": "buena",
        "severity": "good"
      }
    }
  }
}
```

---

## 9. Cómo mostrarlo en la web

En el frontend puedes mostrar tres niveles:

### 9.1 Tarjeta principal

```txt
Estado del lote: Bueno
Resumen: Tu lote está en buen estado para sembrar o mantener cultivo...
```

### 9.2 Acciones recomendadas

```txt
- No regar por ahora.
- Aplicar materia orgánica madura.
- Revisar fósforo para estimular raíces.
- Repetir medición después de lluvia o riego.
```

### 9.3 Alertas

```txt
- Fósforo bajo o medio-bajo.
- Revisar desarrollo de raíces.
```

---

## 10. Ejemplo simple de componente web

```jsx
function RecommendationCard({ recommendation }) {
  if (!recommendation) {
    return <p>No hay recomendaciones disponibles.</p>;
  }

  return (
    <section className="recommendation-card">
      <h2>Recomendación del lote</h2>
      <p>{recommendation.summary}</p>

      <h3>Acciones sugeridas</h3>
      <ul>
        <li>{recommendation.analysis.recommendations.irrigation.message}</li>
        {recommendation.analysis.recommendations.fertilization.map((item, index) => (
          <li key={index}>{item.message}</li>
        ))}
      </ul>

      <h3>Cultivos sugeridos</h3>
      <ul>
        {recommendation.analysis.recommendations.suggestedCrops.map((crop, index) => (
          <li key={index}>{crop}</li>
        ))}
      </ul>
    </section>
  );
}
```

---

## 11. Agregar tipo de cultivo desde el frontend

Para mejorar las recomendaciones, la web debería permitir seleccionar qué hay sembrado en cada nodo o lote.

Ejemplo de valores:

```txt
sin_cultivo
hortalizas_hoja
raices_tuberculos
leguminosas
frutales
cultivos_fruto
maiz
papa
tomate
fresa
cafe
otro
```

El ESP puede enviar solo mediciones. El tipo de cultivo puede guardarse desde la web en la configuración del nodo.

Ejemplo de payload extendido:

```json
{
  "api_key": "ZAQUE_LOCAL_KEY",
  "node_id": "zona_2",
  "node_name": "Lote de cultivo",
  "crop_type": "hortalizas_hoja",
  "soil_humidity": 64,
  "ph": 6.2,
  "nitrogen": 34,
  "phosphorus": 18,
  "potassium": 41,
  "battery_percent": 83
}
```

---

## 12. Reglas adicionales recomendadas para una segunda versión

### 12.1 Historial de mediciones

No tomar decisiones solo con una medición aislada. Agregar reglas por tendencia:

```txt
- Si la humedad baja durante 3 mediciones seguidas: recomendar riego.
- Si la humedad sube después de lluvia y queda alta por varios días: riesgo de hongos.
- Si el pH cambia demasiado rápido: revisar calibración del sensor.
- Si NPK cambia bruscamente: revisar sensor, conexión o calibración.
```

### 12.2 Reglas por clima

Si en el futuro hay internet, se puede consultar clima. Si no hay internet, se puede usar observación local:

```txt
- Si llovió recientemente, no recomendar riego aunque la humedad esté media.
- Si hay sequía y humedad baja, priorizar riego.
- Si hay días muy fríos y humedad alta, aumentar alerta de hongos.
```

### 12.3 Reglas por ubicación

Con GPS se puede guardar ubicación por nodo:

```txt
- Mapa de zonas del lote.
- Comparación entre nodos.
- Identificación de zonas secas.
- Identificación de zonas con posible encharcamiento.
```

### 12.4 Reglas por comparación entre nodos

```txt
- Si zona_1 tiene humedad 30% y zona_2 tiene 70%, recomendar riego solo en zona_1.
- Si un nodo marca valores muy diferentes al resto, revisar sensor o microclima.
- Si una zona mantiene fósforo bajo, priorizar manejo de suelo allí.
```

---

## 13. Recomendación de almacenamiento

Guardar cada medición con su análisis generado.

Ejemplo de tabla o estructura:

```txt
measurements
  id
  node_id
  node_name
  timestamp
  lat
  lon
  soil_humidity
  ph
  nitrogen
  phosphorus
  potassium
  battery_percent
  firmware_version
  created_at

recommendations
  id
  measurement_id
  node_id
  summary
  status_json
  recommendations_json
  alerts_json
  created_at
```

Si estás usando SQLite o archivos JSON locales, puedes guardar algo así:

```json
{
  "measurement": {
    "node_id": "zona_2",
    "soil_humidity": 64,
    "ph": 6.2,
    "nitrogen": 34,
    "phosphorus": 18,
    "potassium": 41
  },
  "summary": "Tu lote está en buen estado para sembrar o mantener cultivo...",
  "analysis": {},
  "created_at": "2026-05-31T16:20:00"
}
```

---

## 14. Prueba rápida local

Crear archivo:

```txt
testRecommendation.js
```

Contenido:

```js
const {
  analyzeMeasurement,
  buildWebSummary
} = require("./src/services/recommendationEngine");

const measurement = {
  api_key: "ZAQUE_LOCAL_KEY",
  node_id: "zona_2",
  node_name: "Lote de cultivo",
  role: "sensor",
  timestamp: "2026-05-31T16:20:00",
  lat: 4.7112,
  lon: -74.0721,
  soil_humidity: 64.0,
  ph: 6.2,
  nitrogen: 34,
  phosphorus: 18,
  potassium: 41,
  battery_percent: 83,
  firmware_version: "0.2.0"
};

const analysis = analyzeMeasurement(measurement, "sin_cultivo");
const summary = buildWebSummary(measurement, analysis);

console.log("Resumen para web:");
console.log(summary);
console.log("\nAnálisis completo:");
console.log(JSON.stringify(analysis, null, 2));
```

Ejecutar:

```bash
node testRecommendation.js
```

---

## 15. Resultado esperado de la prueba

```txt
Resumen para web:
Tu lote está en buen estado para sembrar o mantener cultivo. El pH de 6.2 es favorable, la humedad de 64% es adecuada y el potasio está aceptable. La humedad es suficiente. No riegues todavía si el suelo sigue húmedo al tacto. Refuerza el suelo con compost maduro o bocashi, especialmente para mejorar fósforo y estimular raíces. Puedes considerar cultivos como lechuga, acelga, cilantro, espinaca, zanahoria, remolacha, papa criolla, arveja, según clima, altura y mercado local. Si ya tienes cultivo, revisa hojas amarillas, manchas, pudrición, plagas o crecimiento lento. Repite la medición después de lluvia, riego o aplicación de abono.
```

---

## 16. Recomendaciones de diseño para el campesino

El texto de la web debe ser claro y accionable. Evitar frases muy técnicas como:

```txt
El fósforo presenta deficiencia edáfica moderada.
```

Mejor usar:

```txt
El fósforo está bajo. Esto puede afectar las raíces y el arranque del cultivo.
```

Priorizar mensajes como:

```txt
No riegues por ahora.
Revisa drenajes.
Agrega compost maduro.
Vigila hojas amarillas.
Repite la medición después de lluvia.
```

---

## 17. Mejoras futuras

- Agregar análisis por historial y tendencias.
- Agregar calibración por tipo de sensor.
- Permitir configurar cultivo por lote.
- Agregar mapas con varias zonas.
- Comparar nodos entre sí.
- Guardar recomendaciones históricas.
- Generar alertas por WhatsApp o red local.
- Agregar modo offline completo.
- Agregar exportación CSV de mediciones.
- Agregar panel para cambiar rangos desde la web.

---

## 18. Resumen de implementación

Para implementar esto en ZAQUE:

1. Crear `src/services/recommendationEngine.js`.
2. Copiar el motor de reglas.
3. Modificar `POST /api/measurements` para analizar cada medición.
4. Devolver `summary`, `analysis` y `alerts` al frontend.
5. Mostrar el `summary` en una tarjeta principal.
6. Mostrar acciones, alertas y cultivos sugeridos en tarjetas secundarias.
7. Guardar la medición y la recomendación para historial.

Con esto, ZAQUE pasa de ser solo un sistema de medición a un sistema de apoyo a decisiones agrícolas para campesinos, funcionando incluso en red local sin internet.
