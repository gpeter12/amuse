#pragma once

#include "EditorWidget.hpp"
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QStaticText>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QSplitter>
#include "amuse/Studio.hpp"

class EffectListing;

struct EffectIntrospection {
  struct Field {
    enum class Type { Invalid, UInt32, UInt32x8, Float };
    Type m_tp;
    std::string_view m_name;
    float m_min, m_max, m_default;
  };
  amuse::EffectType m_tp;
  std::string_view m_name;
  std::string_view m_description;
  Field m_fields[7];
};

class Uint32X8Popup : public QFrame {
  Q_OBJECT
  FieldSlider* m_sliders[8];

public:
  explicit Uint32X8Popup(int min, int max, QWidget* parent = Q_NULLPTR);
  void setValue(int chanIdx, int val);
private slots:
  void doValueChanged(int val);
signals:
  void valueChanged(int chanIdx, int val);
};

class Uint32X8Button : public QPushButton {
  Q_OBJECT
  Uint32X8Popup* m_popup;

public:
  explicit Uint32X8Button(int min, int max, QWidget* parent = Q_NULLPTR);
  void paintEvent(QPaintEvent* event);
  Uint32X8Popup* popup() const { return m_popup; }
  QStyleOptionComboBox comboStyleOption() const;
private slots:
  void onPressed();
};

class EffectWidget : public QWidget {
  Q_OBJECT
  friend class EffectListing;
  QFont m_numberFont;
  QLabel m_titleLabel;
  ListingDeleteButton m_deleteButton;
  QStaticText m_numberText;
  int m_index = -1;
  amuse::EffectBaseTypeless* m_effect;
  const EffectIntrospection* m_introspection;
  void setIndex(int index);
private slots:
  void numChanged(int);
  void numChanged(double);
  void chanNumChanged(int, int);
  void deleteClicked();

private:
  explicit EffectWidget(QWidget* parent, amuse::EffectBaseTypeless* effect, amuse::EffectType type);

public:
  EffectListing* getParent() const;
  explicit EffectWidget(QWidget* parent, amuse::EffectBaseTypeless* effect);
  explicit EffectWidget(QWidget* parent, amuse::EffectType op);
  void paintEvent(QPaintEvent* event);
  QString getText() const { return m_titleLabel.text(); }
};

class EffectWidgetContainer : public QWidget {
  Q_OBJECT
  friend class EffectListing;
  EffectWidget* m_effectWidget;
  QPropertyAnimation* m_animation = nullptr;
  void animateOpen();
  void animateClosed();
  void snapOpen();
  void snapClosed();
private slots:
  void animationDestroyed();

public:
  template <class... _Args>
  EffectWidgetContainer(QWidget* parent, _Args&&... args);
};

class EffectListing : public QWidget {
  Q_OBJECT
  friend class EffectWidget;
  friend class StudioSetupWidget;
  amuse::Submix* m_submix = nullptr;
  QVBoxLayout* m_layout;
  QLayoutItem* m_dragItem = nullptr;
  int m_origIdx = -1;
  int m_dragOpenIdx = -1;
  EffectWidgetContainer* m_prevDragOpen = nullptr;
  int m_autoscrollTimer = -1;
  int m_autoscrollDelta = 0;
  QWidget* m_autoscrollSource = nullptr;
  QMouseEvent m_autoscrollEvent = {{}, {}, {}, {}, {}};
  void startAutoscroll(QWidget* source, QMouseEvent* event, int delta);
  void stopAutoscroll();
  bool beginDrag(EffectWidget* widget);
  void endDrag();
  void cancelDrag();
  void _moveDrag(int hoverIdx, const QPoint& pt, QWidget* source, QMouseEvent* event);
  void moveDrag(EffectWidget* widget, const QPoint& pt, QWidget* source, QMouseEvent* event);
  int moveInsertDrag(const QPoint& pt, QWidget* source, QMouseEvent* event);
  void insertDragout();
  void insert(amuse::EffectType type, const QString& text);
  void deleteEffect(int index);
  void reindex();
  void clear();

public:
  explicit EffectListing(QWidget* parent = Q_NULLPTR);
  bool loadData(amuse::Submix* submix);
  void unloadData();
  void timerEvent(QTimerEvent* event);
};

class EffectCatalogueItem : public QWidget {
  Q_OBJECT
  amuse::EffectType m_type;
  QLabel m_iconLab;
  QLabel m_label;

public:
  explicit EffectCatalogueItem(amuse::EffectType type, const QString& name, const QString& doc,
                               QWidget* parent = Q_NULLPTR);
  explicit EffectCatalogueItem(const EffectCatalogueItem& other, QWidget* parent = Q_NULLPTR);
  amuse::EffectType getType() const { return m_type; }
  QString getText() const { return m_label.text(); }
};

class EffectCatalogue : public QTreeWidget {
  Q_OBJECT
public:
  explicit EffectCatalogue(QWidget* parent = Q_NULLPTR);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
};

class StudioSetupWidget : public QWidget {
  Q_OBJECT
  friend class EffectCatalogue;
  QSplitter* m_splitter;
  QTabWidget* m_tabs;
  EffectListing* m_listing[2];
  EffectCatalogue* m_catalogue;
  EffectWidget* m_draggedCmd = nullptr;
  EffectCatalogueItem* m_draggedItem = nullptr;
  QPoint m_draggedPt;
  int m_dragInsertIdx = -1;
  EffectListing* getCurrentListing() const;
  void beginCommandDrag(EffectWidget* widget, const QPoint& eventPt, const QPoint& pt);
  void beginCatalogueDrag(EffectCatalogueItem* item, const QPoint& eventPt, const QPoint& pt);

public:
  explicit StudioSetupWidget(QWidget* parent = Q_NULLPTR);
  bool loadData(amuse::Studio* studio);
  void unloadData();

  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void keyPressEvent(QKeyEvent* event);

  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);
  void updateWindowPosition();

public slots:
  void catalogueDoubleClicked(QTreeWidgetItem* item, int column);

signals:
  void hidden();
  void shown();
};
