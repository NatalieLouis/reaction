#pragma once

#include <vector>

#include "concept.h"
namespace reaction {
  class ObserverNode {
   public:
    virtual ~ObserverNode() = default;
    virtual void valueChanged() {};

    void addObserver(ObserverNode* observer) { m_observers.emplace_back(observer); }

    template <typename... Args>
    void updateObservers(Args&&... args) {
      (void) (..., args.addObserver(this));
    }

    void notify() {
      for (auto& observer : m_observers) {
        observer->valueChanged();
      }
    }

   private:
    std::vector<ObserverNode*> m_observers;
  };
}  // namespace reaction