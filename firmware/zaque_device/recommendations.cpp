#include "recommendations.h"
#include "config.h"
#include <cstring>
#include <cstdio>

class RulesBasedRecommendationsImpl : public RecommendationsEngine {
public:
  void generateRecommendation(const Measurement& m, char* output, size_t output_size) override {
    memset(output, 0, output_size);
    
    // Prioridad 1: Validación de medición
    if (isInvalidMeasurement(m)) {
      snprintf(output, output_size, 
        "❌ SIN MEDICIÓN: Los sensores no están enviando datos. Revise: 1) Conexión del módulo RS485/Modbus 2) Alimentación del sensor 3) Cableado RX/TX 4) Dirección Modbus 5) Firmware del ESP32.");
      return;
    }
    
    // Prioridad 2: Problemas críticos del cultivo
    if (m.soil_humidity < 25) {
      snprintf(output, output_size, 
        "🔴 HUMEDAD MUY BAJA (<25%%): Riesgo de estrés hídrico severo. Revise riego lo antes posible.");
      return;
    }
    
    if (m.soil_humidity > 85) {
      snprintf(output, output_size, 
        "🔴 HUMEDAD MUY ALTA (>85%%): Riesgo de encharcamiento. Suspenda riego y revise drenaje.");
      return;
    }
    
    if (m.electrical_conductivity > 4.0) {
      snprintf(output, output_size, 
        "🔴 CONDUCTIVIDAD MUY ALTA (>4.0 mS/cm): Posible exceso de sales. Suspenda fertilización.");
      return;
    }
    
    // Prioridad 3: Combinaciones críticas de cultivo
    char temp_output[256];
    memset(temp_output, 0, sizeof(temp_output));
    
    if (checkCriticalCombinations(m, temp_output, sizeof(temp_output))) {
      strncpy(output, temp_output, output_size - 1);
      return;
    }
    
    // Prioridad 4: Recomendaciones por variable de cultivo
    checkSingleVariableRecommendations(m, output, output_size);
  }

private:
  bool isInvalidMeasurement(const Measurement& m) {
    // Si falta la información de medición (todos los valores en cero/inválidos)
    int valid_measurements = 0;
    
    if (m.soil_temperature > 0) valid_measurements++;
    if (m.soil_humidity > 0) valid_measurements++;
    if (m.ph > 0) valid_measurements++;
    if (m.electrical_conductivity >= 0) valid_measurements++;
    if (m.nitrogen >= 0) valid_measurements++;
    if (m.phosphorus >= 0) valid_measurements++;
    if (m.potassium >= 0) valid_measurements++;
    
    // Si menos del 50% de lecturas son válidas, es medición inválida
    return valid_measurements < 4;
  }
  
  bool checkCriticalCombinations(const Measurement& m, char* output, size_t output_size) {
    // REGLA 1: Humedad baja + temperatura alta
    if (m.soil_humidity < 40 && m.soil_temperature > 28) {
      snprintf(output, output_size,
        "🟠 ESTRÉS HÍDRICO: Humedad %.0f%% + temperatura %.1f°C alta. Revise riego urgentemente.",
        m.soil_humidity, m.soil_temperature);
      return true;
    }
    
    // REGLA 2: Humedad muy baja + NPK bajo
    if (m.soil_humidity < 25 && m.nitrogen < 20 && m.phosphorus < 15 && m.potassium < 20) {
      snprintf(output, output_size,
        "🟠 CONDICIONES ADVERSAS: Suelo seco (%.0f%%) + nutrientes bajos (N:%d P:%d K:%d). Primero corrija humedad.",
        m.soil_humidity, m.nitrogen, m.phosphorus, m.potassium);
      return true;
    }
    
    // REGLA 3: Humedad alta + CE alta
    if (m.soil_humidity > 70 && m.electrical_conductivity > 2.0) {
      snprintf(output, output_size,
        "🟠 ACUMULACIÓN: Humedad %.0f%% + conductividad %.1f mS/cm. Evite fertilizar y revise drenaje.",
        m.soil_humidity, m.electrical_conductivity);
      return true;
    }
    
    // REGLA 4: pH bajo + fósforo bajo
    if (m.ph < 5.5 && m.phosphorus < 15) {
      snprintf(output, output_size,
        "🟠 BLOQUEO NUTRICIONAL: pH ácido (%.1f) + fósforo bajo (%d mg/kg). Revise corrección antes de fertilizar.",
        m.ph, m.phosphorus);
      return true;
    }
    
    // REGLA 5: pH alto + NPK bajo
    if (m.ph > 7.5 && (m.nitrogen < 20 || m.phosphorus < 15 || m.potassium < 20)) {
      snprintf(output, output_size,
        "🟠 BLOQUEO POR pH ALTO: pH (%.1f) puede bloquear nutrientes (N:%d P:%d K:%d). Revise disponibilidad.",
        m.ph, m.nitrogen, m.phosphorus, m.potassium);
      return true;
    }
    
    return false;
  }
  
  void checkSingleVariableRecommendations(const Measurement& m, char* output, size_t output_size) {
    // HUMEDAD
    if (m.soil_humidity < 40) {
      snprintf(output, output_size,
        "💧 Humedad baja (%.0f%%): Revise si hay compactación. Puede programar riego moderado.",
        m.soil_humidity);
      return;
    }
    
    if (m.soil_humidity > 70) {
      snprintf(output, output_size,
        "💧 Humedad alta (%.0f%%): No riegue ahora. Revise drenaje y riesgo de enfermedades.",
        m.soil_humidity);
      return;
    }
    
    // pH
    if (m.ph < 5.5) {
      snprintf(output, output_size,
        "🧪 pH muy bajo (%.1f): Suelo muy ácido. Confirme con nueva medición y consulte técnico.",
        m.ph);
      return;
    }
    
    if (m.ph > 7.5) {
      snprintf(output, output_size,
        "🧪 pH alto (%.1f): Puede bloquear nutrientes. Evite encalado sin análisis técnico.",
        m.ph);
      return;
    }
    
    // CONDUCTIVIDAD ELÉCTRICA
    if (m.electrical_conductivity > 2.0) {
      snprintf(output, output_size,
        "⚡ Conductividad elevada (%.2f mS/cm): Puede haber acumulación de sales. Evite fertilizar por ahora.",
        m.electrical_conductivity);
      return;
    }
    
    // NITRÓGENO
    if (m.nitrogen < 20) {
      snprintf(output, output_size,
        "🌱 Nitrógeno bajo (%d mg/kg): Puede afectar crecimiento. Revise estado del cultivo y considere fertilización.",
        m.nitrogen);
      return;
    }
    
    // FÓSFORO
    if (m.phosphorus < 15) {
      snprintf(output, output_size,
        "🌱 Fósforo bajo (%d mg/kg): Importante para raíces y floración. Revise pH y disponibilidad antes de fertilizar.",
        m.phosphorus);
      return;
    }
    
    // POTASIO
    if (m.potassium < 20) {
      snprintf(output, output_size,
        "🌱 Potasio bajo (%d mg/kg): Afecta llenado de fruto y resistencia. Revise demanda del cultivo actual.",
        m.potassium);
      return;
    }
    
    // TEMPERATURA
    if (m.soil_temperature < 10) {
      snprintf(output, output_size,
        "🌡️ Temperatura baja (%.1f°C): Puede ralentizar procesos biológicos. Revise cobertura del suelo.",
        m.soil_temperature);
      return;
    }
    
    if (m.soil_temperature > 35) {
      snprintf(output, output_size,
        "🌡️ Temperatura alta (%.1f°C): Riesgo de estrés térmico. Revise cobertura y riego.",
        m.soil_temperature);
      return;
    }
    
    // ESTADO GENERAL POSITIVO
    if (m.soil_humidity >= 40 && m.soil_humidity <= 70 && 
        m.ph >= 6.0 && m.ph <= 7.5 &&
        m.nitrogen >= 20 && m.phosphorus >= 15 && m.potassium >= 20 &&
        m.soil_temperature >= 15 && m.soil_temperature <= 30) {
      snprintf(output, output_size, 
        "✅ Condiciones óptimas: Humedad %.0f%%, pH %.1f, NPK: %d-%d-%d mg/kg. Mantenga monitoreo.",
        m.soil_humidity, m.ph, m.nitrogen, m.phosphorus, m.potassium);
      return;
    }
    
    // Por defecto
    snprintf(output, output_size,
      "📊 Medición: Humedad %.0f%%, pH %.1f, T %.1f°C, NPK:%d-%d-%d. Revise valores para decisiones.",
      m.soil_humidity, m.ph, m.soil_temperature, m.nitrogen, m.phosphorus, m.potassium);
  }
};

// Factory function
Recommendations* createRecommendationEngine() {
  return new RulesBasedRecommendationsImpl();
}
