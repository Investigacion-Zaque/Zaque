#include "recommendations.h"
#include "config.h"
#include <cstring>
#include <cstdio>

class RulesBasedRecommendationsImpl : public RecommendationsEngine {
public:
  void generateRecommendation(const Measurement& m, char* output, size_t output_size) override {
    memset(output, 0, output_size);
    
    // Prioridad 1: Validación general
    if (isInvalidMeasurement(m)) {
      snprintf(output, output_size, 
        "❌ SIN MEDICIÓN: Los sensores no están enviando datos. Revise: 1) Conexión del módulo RS485/Modbus 2) Alimentación del sensor 3) Cableado RX/TX 4) Dirección Modbus 5) Firmware del ESP32.");
      return;
    }
    
    // Prioridad 2: Problemas críticos
    if (m.battery_percent < 10) {
      snprintf(output, output_size, 
        "🔴 BATERÍA CRÍTICA (<10%%): El nodo puede apagarse pronto. Recargue o reemplace urgentemente.");
      return;
    }
    
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
    
    // Prioridad 3: Problemas moderados
    char temp_output[256];
    memset(temp_output, 0, sizeof(temp_output));
    
    if (checkCriticalCombinations(m, temp_output, sizeof(temp_output))) {
      strncpy(output, temp_output, output_size - 1);
      return;
    }
    
    // Prioridad 4: Recomendaciones por variable
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
    // REGLA 9.1: Humedad baja + temperatura alta
    if (m.soil_humidity < 40 && m.soil_temperature > 28) {
      snprintf(output, output_size,
        "🟠 ESTRÉS HÍDRICO: Humedad baja + temperatura alta. Revise riego, cobertura del suelo y materia orgánica.");
      return true;
    }
    
    // REGLA 9.2: Humedad muy baja + NPK bajo
    if (m.soil_humidity < 25 && m.nitrogen < 20 && m.phosphorus < 15 && m.potassium < 20) {
      snprintf(output, output_size,
        "🟠 CONDICIONES ADVERSAS: Suelo seco + nutrientes bajos. Primero corrija humedad, luego evalúe fertilización.");
      return true;
    }
    
    // REGLA 9.3: Humedad alta + CE alta
    if (m.soil_humidity > 70 && m.electrical_conductivity > 2.0) {
      snprintf(output, output_size,
        "🟠 ACUMULACIÓN: Humedad alta + conductividad elevada. Evite fertilizar y revise drenaje.");
      return true;
    }
    
    // REGLA 9.5: pH bajo + fósforo bajo
    if (m.ph < 5.5 && m.phosphorus < 15) {
      snprintf(output, output_size,
        "🟠 BLOQUEO NUTRICIONAL: pH ácido + fósforo bajo. Revise corrección de acidez antes de fertilizar.");
      return true;
    }
    
    return false;
  }
  
  void checkSingleVariableRecommendations(const Measurement& m, char* output, size_t output_size) {
    // HUMEDAD
    if (m.soil_humidity < 40) {
      snprintf(output, output_size,
        "💧 Humedad baja (%.0f%%): Revise si hay compactación o mal drenaje. Puede programar riego moderado.",
        m.soil_humidity);
      return;
    }
    
    if (m.soil_humidity > 70) {
      snprintf(output, output_size,
        "💧 Humedad alta (%.0f%%): No riegue ahora. Revise drenaje y riesgo de enfermedades por exceso de agua.",
        m.soil_humidity);
      return;
    }
    
    // pH
    if (m.ph < 5.5) {
      snprintf(output, output_size,
        "🧪 pH muy bajo (%.1f): Suelo muy ácido. Confirme con nueva medición y consulte técnico para corrección.",
        m.ph);
      return;
    }
    
    if (m.ph > 7.2) {
      snprintf(output, output_size,
        "🧪 pH alto (%.1f): Puede bloquear nutrientes. Evite encalado sin análisis técnico.",
        m.ph);
      return;
    }
    
    // CE
    if (m.electrical_conductivity > 2.0) {
      snprintf(output, output_size,
        "⚡ Conductividad elevada: Puede haber acumulación de sales. Evite fertilizar por ahora.");
      return;
    }
    
    // NPK
    if (m.nitrogen < 20) {
      snprintf(output, output_size,
        "🌱 Nitrógeno bajo: Puede afectar crecimiento. Revise estado del cultivo y considere fertilización.");
      return;
    }
    
    if (m.phosphorus < 15) {
      snprintf(output, output_size,
        "🌱 Fósforo bajo: Importante para raíces y floración. Revise pH y disponibilidad antes de fertilizar.");
      return;
    }
    
    if (m.potassium < 20) {
      snprintf(output, output_size,
        "🌱 Potasio bajo: Afecta llenado de fruto y resistencia. Revise demanda del cultivo actual.");
      return;
    }
    
    // BATERÍA
    if (m.battery_percent < 20) {
      snprintf(output, output_size,
        "🔋 Batería baja (%d%%): Recargue o reemplace la fuente de energía para evitar pérdida de datos.",
        m.battery_percent);
      return;
    }
    
    // GPS
    if (!m.gps_valid) {
      snprintf(output, output_size,
        "📍 GPS no válido: La ubicación no es confiable. Ubique el equipo a cielo abierto y espere señal.");
      return;
    }
    
    // ESTADO GENERAL POSITIVO
    if (m.soil_humidity >= 40 && m.soil_humidity <= 70 && 
        m.ph >= 6.0 && m.ph <= 7.2 &&
        m.nitrogen >= 20 && m.phosphorus >= 15 && m.potassium >= 20) {
      strcpy(output, "✅ Condiciones óptimas: Mantenga monitoreo y evite aplicaciones innecesarias de agua o fertilizante.");
      return;
    }
    
    // Por defecto
    strcpy(output, "📊 Medición registrada. Revise todos los valores para decisiones de manejo.");
  }
};

// Factory function
Recommendations* createRecommendationEngine() {
  return new RulesBasedRecommendationsImpl();
}
