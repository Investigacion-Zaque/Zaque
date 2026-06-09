#pragma once

#include "types.h"

// Engine de recomendaciones agrícolas basado en reglas (interfaz)
class RecommendationsEngine : public Recommendations {
public:
  virtual void generateRecommendation(const Measurement& m, char* output, size_t output_size) override = 0;
  virtual ~RecommendationsEngine() {}
};

// Función helper
Recommendations* createRecommendationEngine();

// Umbrales definidos
namespace RecommendationThresholds {
  constexpr float HUMIDITY_LOW = 30.0f;
  constexpr float HUMIDITY_HIGH = 80.0f;
  constexpr float PH_LOW = 5.5f;
  constexpr float PH_HIGH = 7.5f;
  constexpr int CONDUCTIVITY_HIGH = 2000;
  constexpr int NPK_LOW = 30;
}
