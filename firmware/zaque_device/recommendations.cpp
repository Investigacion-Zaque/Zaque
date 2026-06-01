#include "recommendations.h"
#include "config.h"
#include <cstring>

class RulesBasedRecommendationsImpl : public RecommendationsEngine {
public:
  void generateRecommendation(const Measurement& m, char* output, size_t output_size) override {
    memset(output, 0, output_size);
    size_t remaining = output_size;

    // Generar recomendación basada en múltiples factores
    checkHumidity(m, output);
    remaining = strlen(output);
    
    if (remaining < output_size - 100) {
      strcat(output, " ");
      checkPH(m, output + strlen(output));
    }

    remaining = strlen(output);
    if (remaining < output_size - 100) {
      strcat(output, " ");
      checkConductivity(m, output + strlen(output));
    }

    remaining = strlen(output);
    if (remaining < output_size - 100) {
      strcat(output, " ");
      checkNPK(m, output + strlen(output));
    }

    remaining = strlen(output);
    if (remaining < output_size - 100) {
      strcat(output, " ");
      checkBattery(m, output + strlen(output));
    }

    // Si no hay recomendaciones, mostrar estado general
    if (strlen(output) == 0) {
      strcpy(output, "Condiciones óptimas detectadas.");
    }
  }

private:
  void checkHumidity(const Measurement& m, char* output) {
    if (m.soil_humidity < RecommendationThresholds::HUMIDITY_LOW) {
      strcpy(output, "⚠️ Humedad baja: revisar sistema de riego.");
    } else if (m.soil_humidity > RecommendationThresholds::HUMIDITY_HIGH) {
      strcpy(output, "⚠️ Humedad alta: verificar drenaje.");
    } else {
      strcpy(output, "✓ Humedad adecuada.");
    }
  }

  void checkPH(const Measurement& m, char* output) {
    if (m.ph < RecommendationThresholds::PH_LOW) {
      strcpy(output, "Suelo ácido (pH < 5.5): considerar enmienda caliza.");
    } else if (m.ph > RecommendationThresholds::PH_HIGH) {
      strcpy(output, "Suelo alcalino (pH > 7.5): revisar tipo de fertilizante.");
    } else {
      strcpy(output, "pH dentro del rango aceptable.");
    }
  }

  void checkConductivity(const Measurement& m, char* output) {
    if (m.electrical_conductivity > RecommendationThresholds::CONDUCTIVITY_HIGH) {
      strcpy(output, "⚠️ Conductividad alta: posible exceso de sales.");
    } else {
      strcpy(output, "Conductividad normal.");
    }
  }

  void checkNPK(const Measurement& m, char* output) {
    if (m.nitrogen < RecommendationThresholds::NPK_LOW ||
        m.phosphorus < RecommendationThresholds::NPK_LOW ||
        m.potassium < RecommendationThresholds::NPK_LOW) {
      strcpy(output, "Revisar fertilización (N, P o K bajo).");
    } else {
      strcpy(output, "Nutrientes en niveles adecuados.");
    }
  }

  void checkBattery(const Measurement& m, char* output) {
    if (m.battery_percent < RecommendationThresholds::BATTERY_LOW) {
      strcpy(output, "🔋 BATERÍA BAJA: cargar nodo urgentemente.");
    } else if (m.battery_percent < 50) {
      strcpy(output, "Batería moderada: considerar recargar pronto.");
    }
  }

  void appendRecommendation(const char* text, char* output, size_t remaining) {
    if (strlen(output) > 0) {
      strcat(output, " ");
    }
    strcat(output, text);
  }
};

// Factory function
Recommendations* createRecommendationEngine() {
  return new RulesBasedRecommendationsImpl();
}
