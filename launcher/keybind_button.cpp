#include "keybind_button.h"

namespace condemned2::launcher {
namespace {

// Map a Qt key to the ReXGlue SDK VirtualKey name used in keybind_* cvars.
QString qtKeyToVirtualKeyName(int qtKey, Qt::KeyboardModifiers mods) {
  QString name;
  if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
    name = QString(QChar(qtKey));
  } else if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
    name = QString(QChar(qtKey));
  } else {
    switch (qtKey) {
      case Qt::Key_Space: name = QStringLiteral("Space"); break;
      case Qt::Key_Tab: name = QStringLiteral("Tab"); break;
      case Qt::Key_Return:
      case Qt::Key_Enter: name = QStringLiteral("Return"); break;
      case Qt::Key_Escape: name = QStringLiteral("Escape"); break;
      case Qt::Key_Backspace: name = QStringLiteral("Backspace"); break;
      case Qt::Key_Delete: name = QStringLiteral("Delete"); break;
      case Qt::Key_Insert: name = QStringLiteral("Insert"); break;
      case Qt::Key_Home: name = QStringLiteral("Home"); break;
      case Qt::Key_End: name = QStringLiteral("End"); break;
      case Qt::Key_PageUp: name = QStringLiteral("Prior"); break;
      case Qt::Key_PageDown: name = QStringLiteral("Next"); break;
      case Qt::Key_Left: name = QStringLiteral("Left"); break;
      case Qt::Key_Up: name = QStringLiteral("Up"); break;
      case Qt::Key_Right: name = QStringLiteral("Right"); break;
      case Qt::Key_Down: name = QStringLiteral("Down"); break;
      case Qt::Key_Semicolon: name = QStringLiteral("Semicolon"); break;
      case Qt::Key_Apostrophe: name = QStringLiteral("Quote"); break;
      case Qt::Key_Comma: name = QStringLiteral("Comma"); break;
      case Qt::Key_Period: name = QStringLiteral("Period"); break;
      case Qt::Key_Slash: name = QStringLiteral("Slash"); break;
      case Qt::Key_BracketLeft: name = QStringLiteral("BracketLeft"); break;
      case Qt::Key_BracketRight: name = QStringLiteral("BracketRight"); break;
      case Qt::Key_Backslash: name = QStringLiteral("Backslash"); break;
      case Qt::Key_Minus: name = QStringLiteral("Minus"); break;
      case Qt::Key_Equal: name = QStringLiteral("Equal"); break;
      case Qt::Key_QuoteDbl: name = QStringLiteral("Quote"); break;
      case Qt::Key_AsciiTilde: name = QStringLiteral("Backtick"); break;
      case Qt::Key_Shift: name = QStringLiteral("Shift"); break;
      case Qt::Key_Control: name = QStringLiteral("Control"); break;
      case Qt::Key_Alt: name = QStringLiteral("Menu"); break;
      default:
        if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12) {
          name = QStringLiteral("F%1").arg(qtKey - Qt::Key_F1 + 1);
        }
        break;
    }
  }
  if (name.isEmpty()) {
    return {};
  }
  QString prefix;
  if (mods & Qt::ShiftModifier) prefix += QStringLiteral("Shift+");
  if (mods & Qt::ControlModifier) prefix += QStringLiteral("Control+");
  if (mods & Qt::AltModifier) prefix += QStringLiteral("Menu+");
  return prefix + name;
}

}  // namespace

KeybindButton::KeybindButton(QWidget* parent) : QPushButton(parent) {
  setFocusPolicy(Qt::StrongFocus);
  connect(this, &QPushButton::clicked, this, [this] {
    capturing_ = true;
    setText(tr("Press a key… (Esc cancels)"));
    setFocus();
  });
}

void KeybindButton::setBinding(const QString& binding) {
  binding_ = binding;
  capturing_ = false;
  setText(binding_.isEmpty() ? tr("(unbound)") : binding_);
}

void KeybindButton::keyPressEvent(QKeyEvent* event) {
  if (!capturing_) {
    QPushButton::keyPressEvent(event);
    return;
  }
  if (event->key() == Qt::Key_Escape && event->modifiers() == Qt::NoModifier) {
    capturing_ = false;
    setText(binding_.isEmpty() ? tr("(unbound)") : binding_);
    return;
  }
  const QString name = qtKeyToVirtualKeyName(
      event->key(), event->modifiers() & (Qt::ShiftModifier |
                                          Qt::ControlModifier |
                                          Qt::AltModifier));
  if (!name.isEmpty()) {
    setBinding(name);
  }
  event->accept();
}

void KeybindButton::focusOutEvent(QFocusEvent* event) {
  if (capturing_) {
    capturing_ = false;
    setText(binding_.isEmpty() ? tr("(unbound)") : binding_);
  }
  QPushButton::focusOutEvent(event);
}

}  // namespace condemned2::launcher
