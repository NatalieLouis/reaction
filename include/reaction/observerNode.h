#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "concept.h"
namespace reaction {
  class ObserverNode : public std::enable_shared_from_this<ObserverNode> {
   public:
    virtual ~ObserverNode() = default;
    virtual void valueChanged() {};

    void addObserver(ObserverNode* observer) { m_observers.emplace_back(observer); }

    template <typename... Args>
    void updateObservers(Args&&... args) {
      (void) (..., args.getSharedPtr()->addObserver(this));
    }

    void notify() {
      for (auto& observer : m_observers) {
        observer->valueChanged();
      }
    }

   private:
    std::vector<ObserverNode*> m_observers;  // 这里为啥用裸指针呢
  };

  using NodePtr = std::shared_ptr<ObserverNode>;
  class ObserverGraph {
   public:
    static ObserverGraph& instance() {
      static ObserverGraph instance;
      return instance;
    }
    void addNode(NodePtr node) { m_nodes.insert(std::move(node)); }

    void removeNode(const NodePtr& node) { m_nodes.erase(node); }

   private:
    ObserverGraph() = default;
    std::unordered_set<NodePtr> m_nodes;
  };
}  // namespace reaction