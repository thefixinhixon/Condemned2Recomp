#pragma once

#include <QKeyEvent>
#include <QPushButton>

namespace condemned2::launcher {

// A button that captures the next keypress as a keybind value, using the
// ReXGlue SDK VirtualKey names (e.g. "Semicolon", "Space", "Shift+Up").
class KeybindButton : public QPushButton {
  Q_OBJECT

 public:
  explicit KeybindButton(QWidget* parent = nullptr);

  void setBinding(const QString& binding);
  QString binding() const { return binding_; }

 protected:
  void keyPressEvent(QKeyEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;

 private:
  QString binding_;
  bool capturing_ = false;
};

}  // namespace condemned2::launcher
