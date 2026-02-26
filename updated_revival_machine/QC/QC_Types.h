#ifndef QC_TYPES_H
#define QC_TYPES_H

#include <Arduino.h>

/**
 * @brief QC Severity Levels
 * Fail: Blocking error (requires immediate attention)
 * Warn: Non-blocking warning (performance/quality degradation)
 * Pass: Test passed
 */
enum class QCLevel { PASS, WARN, FAIL };

/**
 * @brief Standardized QC Result Structure
 * Follows the "Actionable" failure reporting principle.
 */
struct QCResult {
  QCLevel level;
  String ruleId;    // Unique Rule ID (e.g., "NET_01")
  String what;      // Component/Variable involved
  String criterion; // Rule/Threshold breached
  String value;     // Observed value
  String fix;       // Suggested action
  unsigned long timestamp;

  QCResult() : level(QCLevel::PASS), timestamp(0) {}

  QCResult(QCLevel l, String id, String w, String c, String v, String f)
      : level(l), ruleId(id), what(w), criterion(c), value(v), fix(f) {
    timestamp = millis();
  }

  // Helper to check if result is not PASS
  bool isIssue() const { return level != QCLevel::PASS; }

  // Format for logging: [LEVEL] [ID] What failed (Value vs Criterion) -> Fix
  String toString() const {
    String levelStr = (level == QCLevel::FAIL) ? "[FAIL]" : "[WARN]";
    return levelStr + " [" + ruleId + "] " + what + " " + value +
           " (Limit: " + criterion + ") -> Fix: " + fix;
  }
};

/**
 * @brief Abstract Base Class for All QC Rules
 */
class IQCRule {
public:
  virtual ~IQCRule() {}

  // Unique ID for the rule
  virtual String getId() const = 0;

  // Human readable name
  virtual String getName() const = 0;

  // Execute the check. Must be deterministic.
  virtual QCResult check() = 0;

  // Frequency: true = check every tick (fast), false = check periodically
  // (slow)
  virtual bool isFastCheck() const = 0;
};

#endif // QC_TYPES_H
