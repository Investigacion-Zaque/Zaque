#include "node_registry.h"
#include "config.h"
#include <cstring>

// Implementación de registro de nodos para el MAIN
void NodeRegistry::init() {
  node_count = 0;
  memset(nodes, 0, sizeof(nodes));
#if ENABLE_DEBUG_SERIAL
  Serial.println("✓ Node registry initialized");
#endif
}

void NodeRegistry::updateLocalNode(const Measurement& m) {
  // El MAIN actualiza su propio registro
  if (node_count < MAX_NODES) {
    nodes[0] = m;
    if (node_count == 0) node_count = 1;
#if ENABLE_DEBUG_SERIAL
    Serial.println("✓ Local node updated");
#endif
  }
}

void NodeRegistry::addRemoteNode(const Measurement& m) {
  // Buscar si el nodo ya existe
  for (int i = 0; i < node_count; i++) {
    if (strcmp(nodes[i].node_id, m.node_id) == 0) {
      // Actualizar nodo existente
      nodes[i] = m;
#if ENABLE_DEBUG_SERIAL
      Serial.printf("✓ Node %s updated\n", m.node_id);
#endif
      return;
    }
  }

  // Agregar nuevo nodo si hay espacio
  if (node_count < MAX_NODES) {
    nodes[node_count] = m;
    node_count++;
#if ENABLE_DEBUG_SERIAL
    Serial.printf("✓ Node %s registered (total: %d)\n", m.node_id, node_count);
#endif
  } else {
#if ENABLE_DEBUG_SERIAL
    Serial.println("✗ Node registry full!");
#endif
  }
}

int NodeRegistry::getNodeCount() const {
  return node_count;
}

bool NodeRegistry::getNodeByID(const char* node_id, Measurement& m) const {
  for (int i = 0; i < node_count; i++) {
    if (strcmp(nodes[i].node_id, node_id) == 0) {
      m = nodes[i];
      return true;
    }
  }
  return false;
}

// Variable global para acceso desde web_server
static NodeRegistry* g_registry = nullptr;

// Factory function
NodeRegistry* createNodeRegistry() {
  if (g_registry == nullptr) {
    g_registry = new NodeRegistry();
    g_registry->init();
  }
  return g_registry;
}

// Función para obtener el registry global (usada por web_server)
NodeRegistry* getGlobalNodeRegistry() {
  return g_registry;
}
