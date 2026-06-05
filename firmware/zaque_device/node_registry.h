#pragma once

#include "types.h"

// Interfaz para NodeRegistry
class NodeRegistryBase {
public:
  virtual void init() = 0;
  virtual void updateLocalNode(const Measurement& m) = 0;
  virtual void addRemoteNode(const Measurement& m) = 0;
  virtual int getNodeCount() const = 0;
  virtual bool getNodeByID(const char* node_id, Measurement& m) const = 0;
  virtual ~NodeRegistryBase() {}
};

// Implementación de registro de nodos para el MAIN
class NodeRegistry : public NodeRegistryBase {
public:
  static const int MAX_NODES = 20;
  
  void init() override;
  void updateLocalNode(const Measurement& m) override;
  void addRemoteNode(const Measurement& m) override;
  
  int getNodeCount() const override;
  bool getNodeByID(const char* node_id, Measurement& m) const override;
  bool getNodeByIndex(int index, Measurement& m) const;

private:
  Measurement nodes[MAX_NODES];
  int node_count;
};

// Función helper
NodeRegistry* createNodeRegistry();
