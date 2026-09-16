/* ---------------------------------------------------------------------------

    Omarchy-compatible theme template. The double-curly-brace placeholders
    below are substituted at runtime (see src/omarchytheme.cpp) with colors
    read from the active Omarchy theme's colors.toml, so this theme always
    matches whatever Omarchy theme the user has selected system-wide.

    Structure derived from themes/dark/darkstyle.qss (QDarkStyleSheet).

--------------------------------------------------------------------------- */
/*

See Qt documentation:

  - https://doc.qt.io/qt-5/stylesheet.html
  - https://doc.qt.io/qt-5/stylesheet-reference.html
  - https://doc.qt.io/qt-5/stylesheet-examples.html

--------------------------------------------------------------------------- */
/* Reset elements ------------------------------------------------------------

Resetting everything helps to unify styles across different operating systems

--------------------------------------------------------------------------- */
* {
  padding: 0px;
  margin: 0px;
  border: 0px;
  border-style: none;
  border-image: none;
  outline: 0;
}

/* specific reset for elements inside QToolBar */
QToolBar * {
  margin: 0px;
  padding: 0px;
}

/* QWidget ----------------------------------------------------------------

--------------------------------------------------------------------------- */
QWidget {
  background-color: {{background}};
  border: 0px solid {{panel}};
  padding: 0px;
  color: {{foreground}};
  selection-background-color: {{selection}};
  selection-color: {{foreground}};
}

QWidget:disabled {
  background-color: {{background}};
  color: {{disabledText}};
  selection-background-color: {{selectionDisabled}};
  selection-color: {{disabledText}};
}

QWidget::item:selected {
  background-color: {{selection}};
}

QWidget::item:hover:!selected {
  background-color: {{accent}};
}

/* QMainWindow ------------------------------------------------------------

This adjusts the splitter in the dock widget, not qsplitter
https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qmainwindow

--------------------------------------------------------------------------- */
QMainWindow::separator {
  background-color: {{panel}};
  border: 0px solid {{background}};
  spacing: 0px;
  padding: 2px;
}

QMainWindow::separator:hover {
  background-color: {{handle}};
  border: 0px solid {{accent}};
}

QMainWindow::separator:horizontal {
  width: 5px;
  margin-top: 2px;
  margin-bottom: 2px;
  image: url(":/qss_icons/dark/rc/toolbar_separator_vertical.png");
}

QMainWindow::separator:vertical {
  height: 5px;
  margin-left: 2px;
  margin-right: 2px;
  image: url(":/qss_icons/dark/rc/toolbar_separator_horizontal.png");
}

/* QToolTip ---------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtooltip

--------------------------------------------------------------------------- */
QToolTip {
  background-color: {{selection}};
  color: {{foreground}};
  /* If you remove the border property, background stops working on Windows */
  border: none;
  /* Remove padding, for fix combo box tooltip */
  padding: 0px;
  /* Remove opacity, fix #174 - may need to use RGBA */
}

/* QStatusBar -------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qstatusbar

--------------------------------------------------------------------------- */
QStatusBar {
  border: 1px solid {{panel}};
  /* Fixes Spyder #9120, #9121 */
  background: {{panel}};
  /* Fixes #205, white vertical borders separating items */
  border-bottom-left-radius: 4px;  
  border-bottom-right-radius: 4px;  
}

QStatusBar::item {
  border: none;
}

QStatusBar QToolTip {
  background-color: {{accent}};
  border: 1px solid {{background}};
  color: {{background}};
  /* Remove padding, for fix combo box tooltip */
  padding: 0px;
  /* Reducing transparency to read better */
  opacity: 230;
}

QStatusBar QLabel {
  /* Fixes Spyder #9120, #9121 */
  background-color: transparent;
}

/* QCheckBox --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qcheckbox

--------------------------------------------------------------------------- */
QCheckBox {
  background-color: transparent;
  color: {{foreground}};
  spacing: 4px;
  outline: none;
  padding-top: 4px;
  padding-bottom: 4px;
}

QCheckBox:focus {
  border: none;
}

QCheckBox QWidget:disabled {
  background-color: {{background}};
  color: {{disabledText}};
}

QCheckBox::indicator {
  margin-left: 2px;
  height: 14px;
  width: 14px;
}

QCheckBox::indicator:unchecked {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked.png");
}

QCheckBox::indicator:unchecked:hover, QCheckBox::indicator:unchecked:focus, QCheckBox::indicator:unchecked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_focus.png");
}

QCheckBox::indicator:unchecked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_disabled.png");
}

QCheckBox::indicator:checked {
  image: url(":/qss_icons/dark/rc/checkbox_checked.png");
}

QCheckBox::indicator:checked:hover, QCheckBox::indicator:checked:focus, QCheckBox::indicator:checked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_checked_focus.png");
}

QCheckBox::indicator:checked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_checked_disabled.png");
}

QCheckBox::indicator:indeterminate {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate.png");
}

QCheckBox::indicator:indeterminate:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate_disabled.png");
}

QCheckBox::indicator:indeterminate:focus, QCheckBox::indicator:indeterminate:hover, QCheckBox::indicator:indeterminate:pressed {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate_focus.png");
}

/* QGroupBox --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qgroupbox

--------------------------------------------------------------------------- */
QGroupBox {
  font-weight: bold;
  border: 1px solid {{panel}};
  border-radius: 7px;
  padding: 2px;
  margin-top: 6px;
  margin-bottom: 4px;
}

QGroupBox::title {
  subcontrol-origin: margin;
  subcontrol-position: top left;
  left: 4px;
  padding-left: 2px;
  padding-right: 4px;
  padding-top: -4px;
}

QGroupBox::indicator {
  margin-left: 2px;
  margin-top: 2px;
  padding: 0;
  height: 14px;
  width: 14px;
}

QGroupBox::indicator:unchecked {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_unchecked.png");
}

QGroupBox::indicator:unchecked:hover, QGroupBox::indicator:unchecked:focus, QGroupBox::indicator:unchecked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_focus.png");
}

QGroupBox::indicator:unchecked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_disabled.png");
}

QGroupBox::indicator:checked {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_checked.png");
}

QGroupBox::indicator:checked:hover, QGroupBox::indicator:checked:focus, QGroupBox::indicator:checked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_checked_focus.png");
}

QGroupBox::indicator:checked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_checked_disabled.png");
}

/* QRadioButton -----------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qradiobutton

--------------------------------------------------------------------------- */
QRadioButton {
  background-color: {{background}};
  color: {{foreground}};
  spacing: 4px;
  padding-top: 4px;
  padding-bottom: 4px;
  border: none;
  outline: none;
}

QRadioButton:focus {
  border: none;
}

QRadioButton:disabled {
  background-color: {{background}};
  color: {{disabledText}};
  border: none;
  outline: none;
}

QRadioButton QWidget {
  background-color: {{background}};
  color: {{foreground}};
  spacing: 0px;
  padding: 0px;
  outline: none;
  border: none;
}

QRadioButton::indicator {
  border: none;
  outline: none;
  margin-left: 2px;
  height: 14px;
  width: 14px;
}

QRadioButton::indicator:unchecked {
  image: url(":/qss_icons/dark/rc/radio_unchecked.png");
}

QRadioButton::indicator:unchecked:hover, QRadioButton::indicator:unchecked:focus, QRadioButton::indicator:unchecked:pressed {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_unchecked_focus.png");
}

QRadioButton::indicator:unchecked:disabled {
  image: url(":/qss_icons/dark/rc/radio_unchecked_disabled.png");
}

QRadioButton::indicator:checked {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked.png");
}

QRadioButton::indicator:checked:hover, QRadioButton::indicator:checked:focus, QRadioButton::indicator:checked:pressed {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked_focus.png");
}

QRadioButton::indicator:checked:disabled {
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked_disabled.png");
}

/* QMenuBar ---------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qmenubar

--------------------------------------------------------------------------- */
QMenuBar {
  background-color: {{panel}};
  padding: 2px;
  border: 1px solid {{background}};
  color: {{foreground}};
  selection-background-color: {{accent}};
}

QMenuBar:focus {
  border: 1px solid {{selection}};
}

QMenuBar::item {
  background: transparent;
  padding: 4px;
}

QMenuBar::item:selected {
  padding: 4px;
  background: transparent;
  border: 0px solid {{panel}};
  background-color: {{accent}};
}

QMenuBar::item:pressed {
  padding: 4px;
  border: 0px solid {{panel}};
  background-color: {{accent}};
  color: {{foreground}};
  margin-bottom: 0px;
  padding-bottom: 0px;
}

/* QMenu ------------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qmenu

--------------------------------------------------------------------------- */
QMenu {
  border: 0px solid {{panel}};
  color: {{foreground}};
  margin: 0px;
  background-color: {{panelAlt}};
  selection-background-color: {{accent}};
}

QMenu::separator {
  height: 1px;
  background-color: {{handle}};
  color: {{foreground}};
}

QMenu::item {
  background-color: {{panelAlt}};
  padding: 4px 24px 4px 28px;
  /* Reserve space for selection border */
  border: 1px transparent {{panel}};
}

QMenu::item:selected {
  color: {{foreground}};
  background-color: {{accent}};
}

QMenu::item:pressed {
  background-color: {{accent}};
}

QMenu::icon {
  padding-left: 10px;
  width: 14px;
  height: 14px;
}

QMenu::indicator {
  padding-left: 8px;
  width: 12px;
  height: 12px;
  /* non-exclusive indicator = check box style indicator (see QActionGroup::setExclusive) */
  /* exclusive indicator = radio button style indicator (see QActionGroup::setExclusive) */
}

QMenu::indicator:non-exclusive:unchecked {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked.png");
}

QMenu::indicator:non-exclusive:unchecked:hover, QMenu::indicator:non-exclusive:unchecked:focus, QMenu::indicator:non-exclusive:unchecked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_focus.png");
}

QMenu::indicator:non-exclusive:unchecked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_disabled.png");
}

QMenu::indicator:non-exclusive:checked {
  image: url(":/qss_icons/dark/rc/checkbox_checked.png");
}

QMenu::indicator:non-exclusive:checked:hover, QMenu::indicator:non-exclusive:checked:focus, QMenu::indicator:non-exclusive:checked:pressed {
  border: none;
  image: url(":/qss_icons/dark/rc/checkbox_checked_focus.png");
}

QMenu::indicator:non-exclusive:checked:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_checked_disabled.png");
}

QMenu::indicator:non-exclusive:indeterminate {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate.png");
}

QMenu::indicator:non-exclusive:indeterminate:disabled {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate_disabled.png");
}

QMenu::indicator:non-exclusive:indeterminate:focus, QMenu::indicator:non-exclusive:indeterminate:hover, QMenu::indicator:non-exclusive:indeterminate:pressed {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate_focus.png");
}

QMenu::indicator:exclusive:unchecked {
  image: url(":/qss_icons/dark/rc/radio_unchecked.png");
}

QMenu::indicator:exclusive:unchecked:hover, QMenu::indicator:exclusive:unchecked:focus, QMenu::indicator:exclusive:unchecked:pressed {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_unchecked_focus.png");
}

QMenu::indicator:exclusive:unchecked:disabled {
  image: url(":/qss_icons/dark/rc/radio_unchecked_disabled.png");
}

QMenu::indicator:exclusive:checked {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked.png");
}

QMenu::indicator:exclusive:checked:hover, QMenu::indicator:exclusive:checked:focus, QMenu::indicator:exclusive:checked:pressed {
  border: none;
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked_focus.png");
}

QMenu::indicator:exclusive:checked:disabled {
  outline: none;
  image: url(":/qss_icons/dark/rc/radio_checked_disabled.png");
}

QMenu::right-arrow {
  margin: 5px;
  padding-left: 12px;
  image: url(":/qss_icons/dark/rc/arrow_right.png");
  height: 12px;
  width: 12px;
}

/* QAbstractItemView ------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qcombobox

--------------------------------------------------------------------------- */
QAbstractItemView {
  alternate-background-color: {{background}};
  color: {{foreground}};
  border: 1px solid {{panel}};
  border-radius: 7px;
}

QAbstractItemView QLineEdit {
  padding: 2px;
}

/* QAbstractScrollArea ----------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qabstractscrollarea

--------------------------------------------------------------------------- */
QAbstractScrollArea {
  background-color: {{background}};
  border: 1px solid {{panel}};
  border-radius: 7px;
  /* fix #159 */
  padding: 2px;
  /* remove min-height to fix #244 */
  color: {{foreground}};
}

QAbstractScrollArea:disabled {
  color: {{disabledText}};
}

/* QScrollArea ------------------------------------------------------------

--------------------------------------------------------------------------- */
QScrollArea QWidget QWidget:disabled {
  background-color: {{background}};
}

/* QScrollBar -------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qscrollbar

--------------------------------------------------------------------------- */
QScrollBar:horizontal {
  height: 16px;
  margin: 2px 16px 2px 16px;
  border: 1px solid {{panel}};
  border-radius: 7px;
  background-color: {{background}};
}

QScrollBar:vertical {
  background-color: {{background}};
  width: 16px;
  margin: 16px 2px 16px 2px;
  border: 1px solid {{panel}};
  border-radius: 7px;
}

QScrollBar::handle:horizontal {
  background-color: {{handle}};
  border: 1px solid {{panel}};
  border-radius: 7px;
  min-width: 8px;
}

QScrollBar::handle:horizontal:hover {
  background-color: {{selection}};
  border: {{selection}};
  border-radius: 7px;
  min-width: 8px;
}

QScrollBar::handle:horizontal:focus {
  border: 1px solid {{accent}};
}

QScrollBar::handle:vertical {
  background-color: {{handle}};
  border: 1px solid {{panel}};
  min-height: 8px;
  border-radius: 7px;
}

QScrollBar::handle:vertical:hover {
  background-color: {{selection}};
  border: {{selection}};
  border-radius: 7px;
  min-height: 8px;
}

QScrollBar::handle:vertical:focus {
  border: 1px solid {{accent}};
}

QScrollBar::add-line:horizontal {
  margin: 0px 0px 0px 0px;
  border-image: url(":/qss_icons/dark/rc/arrow_right_disabled.png");
  height: 12px;
  width: 12px;
  subcontrol-position: right;
  subcontrol-origin: margin;
}

QScrollBar::add-line:horizontal:hover, QScrollBar::add-line:horizontal:on {
  border-image: url(":/qss_icons/dark/rc/arrow_right.png");
  height: 12px;
  width: 12px;
  subcontrol-position: right;
  subcontrol-origin: margin;
}

QScrollBar::add-line:vertical {
  margin: 3px 0px 3px 0px;
  border-image: url(":/qss_icons/dark/rc/arrow_down_disabled.png");
  height: 12px;
  width: 12px;
  subcontrol-position: bottom;
  subcontrol-origin: margin;
}

QScrollBar::add-line:vertical:hover, QScrollBar::add-line:vertical:on {
  border-image: url(":/qss_icons/dark/rc/arrow_down.png");
  height: 12px;
  width: 12px;
  subcontrol-position: bottom;
  subcontrol-origin: margin;
}

QScrollBar::sub-line:horizontal {
  margin: 0px 3px 0px 3px;
  border-image: url(":/qss_icons/dark/rc/arrow_left_disabled.png");
  height: 12px;
  width: 12px;
  subcontrol-position: left;
  subcontrol-origin: margin;
}

QScrollBar::sub-line:horizontal:hover, QScrollBar::sub-line:horizontal:on {
  border-image: url(":/qss_icons/dark/rc/arrow_left.png");
  height: 12px;
  width: 12px;
  subcontrol-position: left;
  subcontrol-origin: margin;
}

QScrollBar::sub-line:vertical {
  margin: 3px 0px 3px 0px;
  border-image: url(":/qss_icons/dark/rc/arrow_up_disabled.png");
  height: 12px;
  width: 12px;
  subcontrol-position: top;
  subcontrol-origin: margin;
}

QScrollBar::sub-line:vertical:hover, QScrollBar::sub-line:vertical:on {
  border-image: url(":/qss_icons/dark/rc/arrow_up.png");
  height: 12px;
  width: 12px;
  subcontrol-position: top;
  subcontrol-origin: margin;
}

QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {
  background: none;
}

QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
  background: none;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
  background: none;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
  background: none;
}

/* QTextEdit --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-specific-widgets

--------------------------------------------------------------------------- */
QTextEdit {
  background-color: {{field}};
  color: {{foreground}};
  border-radius: 7px;
  border: 1px solid {{panel}};
}

QTextEdit:focus {
  border: 1px solid {{accent}};
}

QTextEdit:selected {
  background: {{selection}};
  color: {{panel}};
}

/* QPlainTextEdit ---------------------------------------------------------

--------------------------------------------------------------------------- */
QPlainTextEdit {
  background-color: {{field}};
  color: {{foreground}};
  border-radius: 7px;
  border: 1px solid {{panel}};
}

QPlainTextEdit:focus {
  border: 1px solid {{accent}};
}

QPlainTextEdit:selected {
  background: {{selection}};
  color: {{panel}};
}

/* QSizeGrip --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qsizegrip

--------------------------------------------------------------------------- */
QSizeGrip {
  background: transparent;
  width: 12px;
  height: 12px;
  image: url(":/qss_icons/dark/rc/window_grip.png");
}

/* QStackedWidget ---------------------------------------------------------

--------------------------------------------------------------------------- */
QStackedWidget {
  padding: 2px;
  border: 1px solid {{panel}};
  border: 1px solid {{background}};
}

/* QToolBar ---------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtoolbar

--------------------------------------------------------------------------- */
QToolBar {
  background-color: {{panel}};
  border-bottom: 1px solid {{background}};
  padding: 1px;
  font-weight: bold;
  spacing: 2px;
}

QToolBar:disabled {
  /* Fixes #272 */
  background-color: {{panel}};
}

QToolBar::handle:horizontal {
  width: 16px;
  image: url(":/qss_icons/dark/rc/toolbar_move_horizontal.png");
}

QToolBar::handle:vertical {
  height: 16px;
  image: url(":/qss_icons/dark/rc/toolbar_move_vertical.png");
}

QToolBar::separator:horizontal {
  width: 16px;
  image: url(":/qss_icons/dark/rc/toolbar_separator_horizontal.png");
}

QToolBar::separator:vertical {
  height: 16px;
  image: url(":/qss_icons/dark/rc/toolbar_separator_vertical.png");
}

QToolButton#qt_toolbar_ext_button {
  background: {{panel}};
  border: 0px;
  color: {{foreground}};
  image: url(":/qss_icons/dark/rc/arrow_right.png");
}

/* QAbstractSpinBox -------------------------------------------------------

--------------------------------------------------------------------------- */
QAbstractSpinBox {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  /* This fixes 103, 111 */
  padding-top: 2px;
  /* This fixes 103, 111 */
  padding-bottom: 2px;
  padding-left: 4px;
  padding-right: 4px;
  border-radius: 7px;
  /* min-width: 5px; removed to fix 109 */
}

QAbstractSpinBox:up-button {
  background-color: transparent {{background}};
  subcontrol-origin: border;
  subcontrol-position: top right;
  border-left: 1px solid {{panel}};
  border-bottom: 1px solid {{panel}};
  border-top-left-radius: 0;
  border-bottom-left-radius: 0;
  margin: 1px;
  width: 12px;
  margin-bottom: -1px;
}

QAbstractSpinBox::up-arrow, QAbstractSpinBox::up-arrow:disabled, QAbstractSpinBox::up-arrow:off {
  image: url(":/qss_icons/dark/rc/arrow_up_disabled.png");
  height: 8px;
  width: 8px;
}

QAbstractSpinBox::up-arrow:hover {
  image: url(":/qss_icons/dark/rc/arrow_up.png");
}

QAbstractSpinBox:down-button {
  background-color: transparent {{background}};
  subcontrol-origin: border;
  subcontrol-position: bottom right;
  border-left: 1px solid {{panel}};
  border-top: 1px solid {{panel}};
  border-top-left-radius: 0;
  border-bottom-left-radius: 0;
  margin: 1px;
  width: 12px;
  margin-top: -1px;
}

QAbstractSpinBox::down-arrow, QAbstractSpinBox::down-arrow:disabled, QAbstractSpinBox::down-arrow:off {
  image: url(":/qss_icons/dark/rc/arrow_down_disabled.png");
  height: 8px;
  width: 8px;
}

QAbstractSpinBox::down-arrow:hover {
  image: url(":/qss_icons/dark/rc/arrow_down.png");
}

QAbstractSpinBox:hover {
  border: 1px solid {{selection}};
  color: {{foreground}};
}

QAbstractSpinBox:focus {
  border: 1px solid {{accent}};
}

QAbstractSpinBox:selected {
  background: {{selection}};
  color: {{panel}};
}

/* ------------------------------------------------------------------------ */
/* DISPLAYS --------------------------------------------------------------- */
/* ------------------------------------------------------------------------ */
/* QLabel -----------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qframe

--------------------------------------------------------------------------- */
QLabel {
  background-color: transparent;
  border: 0px solid {{panel}};
  padding: 2px;
  margin: 0px;
  color: {{foreground}};
}

QLabel:disabled {
  background-color: transparent;
  border: 0px solid {{panel}};
  color: {{disabledText}};
}

/* QTextBrowser -----------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qabstractscrollarea

--------------------------------------------------------------------------- */
QTextBrowser {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  border-radius: 7px;
}

QTextBrowser:disabled {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{disabledText}};
  border-radius: 7px;
}

QTextBrowser:hover, QTextBrowser:!hover, QTextBrowser:selected, QTextBrowser:pressed {
  border: 1px solid {{panel}};
}

/* QGraphicsView ----------------------------------------------------------

--------------------------------------------------------------------------- */
QGraphicsView {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  border-radius: 7px;
}

QGraphicsView:disabled {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{disabledText}};
  border-radius: 7px;
}

QGraphicsView:hover, QGraphicsView:!hover, QGraphicsView:selected, QGraphicsView:pressed {
  border: 1px solid {{panel}};
}

/* QCalendarWidget --------------------------------------------------------

--------------------------------------------------------------------------- */
QCalendarWidget {
  border: 1px solid {{panel}};
  border-radius: 7px;
  font-size: 10px;
}

QCalendarWidget:disabled {
  background-color: {{background}};
  color: {{disabledText}};
}

/* QLCDNumber -------------------------------------------------------------

--------------------------------------------------------------------------- */
QLCDNumber {
  background-color: {{background}};
  color: {{foreground}};
}

QLCDNumber:disabled {
  background-color: {{background}};
  color: {{disabledText}};
}

/* QProgressBar -----------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qprogressbar

--------------------------------------------------------------------------- */
QProgressBar {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  border-radius: 7px;
  text-align: center;
}

QProgressBar:disabled {
  background-color: {{field}};
  border: 1px solid {{panel}};
  color: {{disabledText}};
  border-radius: 7px;
  text-align: center;
}

QProgressBar::chunk {
  background-color: {{selection}};
  color: {{background}};
  border-radius: 7px;
}

QProgressBar::chunk:disabled {
  background-color: {{selectionDisabled}};
  color: {{disabledText}};
  border-radius: 7px;
}

/* ------------------------------------------------------------------------ */
/* BUTTONS ---------------------------------------------------------------- */
/* ------------------------------------------------------------------------ */
/* QPushButton ------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qpushbutton

--------------------------------------------------------------------------- */
QPushButton {
  background-color: {{panel}};
  color: {{foreground}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
  border: none;
  min-height: 24px;
}

QPushButton:disabled {
  background-color: {{panel}};
  color: {{disabledText}};
  border-radius: 7px;
  padding: 2px;
}

QPushButton:checked {
  background-color: {{handle}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
}

QPushButton:checked:disabled {
  background-color: {{handle}};
  color: {{disabledText}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
}

QPushButton:checked:selected {
  background: {{handle}};
}

QPushButton:hover {
  background-color: {{panelHover}};
  color: {{foreground}};
}

QPushButton:pressed {
  background-color: {{handle}};
}

QPushButton:selected {
  background: {{handle}};
  color: {{foreground}};
}

QPushButton::menu-indicator {
  subcontrol-origin: padding;
  subcontrol-position: bottom right;
  bottom: 4px;
}

QDialogButtonBox QPushButton {
  /* Issue #194 #248 - Special case of QPushButton inside dialogs, for better UI */
  min-width: 80px;
}

/* QToolButton ------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtoolbutton

--------------------------------------------------------------------------- */
QToolButton {
  background-color: {{panel}};
  color: {{foreground}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
  border: none;
  /* The subcontrols below are used only in the DelayedPopup mode */
  /* The subcontrols below are used only in the MenuButtonPopup mode */
  /* The subcontrol below is used only in the InstantPopup or DelayedPopup mode */
}

QToolButton:disabled {
  background-color: {{panel}};
  color: {{disabledText}};
  border-radius: 7px;
  padding: 2px;
}

QToolButton:checked {
  background-color: {{handle}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
}

QToolButton:checked:disabled {
  background-color: {{handle}};
  color: {{disabledText}};
  border-radius: 7px;
  padding: 2px;
  outline: none;
}

QToolButton:checked:hover {
  background-color: {{panelHover}};
  color: {{foreground}};
}

QToolButton:checked:pressed {
  background-color: {{handle}};
}

QToolButton:checked:selected {
  background: {{handle}};
  color: {{foreground}};
}

QToolButton:hover {
  background-color: {{panelHover}};
  color: {{foreground}};
}

QToolButton:pressed {
  background-color: {{handle}};
}

QToolButton:selected {
  background: {{handle}};
  color: {{foreground}};
}

QToolButton[popupMode="0"] {
  /* Only for DelayedPopup */
  padding-right: 2px;
}

QToolButton[popupMode="1"] {
  /* Only for MenuButtonPopup */
  padding-right: 20px;
}

QToolButton[popupMode="1"]::menu-button {
  border: none;
}

QToolButton[popupMode="1"]::menu-button:hover {
  border: none;
  border-left: 1px solid {{panel}};
  border-radius: 0;
}

QToolButton[popupMode="2"] {
  /* Only for InstantPopup */
  padding-right: 2px;
}

QToolButton::menu-button {
  padding: 2px;
  border-radius: 7px;
  width: 12px;
  border: none;
  outline: none;
}

QToolButton::menu-button:hover {
  border: 1px solid {{selection}};
}

QToolButton::menu-button:checked:hover {
  border: 1px solid {{selection}};
}

QToolButton::menu-indicator {
  image: url(":/qss_icons/dark/rc/arrow_down.png");
  height: 8px;
  width: 8px;
  top: 0;
  /* Exclude a shift for better image */
  left: -2px;
  /* Shift it a bit */
}

QToolButton::menu-arrow {
  image: url(":/qss_icons/dark/rc/arrow_down.png");
  height: 8px;
  width: 8px;
}

QToolButton::menu-arrow:hover {
  image: url(":/qss_icons/dark/rc/arrow_down_focus.png");
}

/* QCommandLinkButton -----------------------------------------------------

--------------------------------------------------------------------------- */
QCommandLinkButton {
  background-color: transparent;
  border: 1px solid {{panel}};
  color: {{foreground}};
  border-radius: 7px;
  padding: 0px;
  margin: 0px;
}

QCommandLinkButton:disabled {
  background-color: transparent;
  color: {{disabledText}};
}

/* ------------------------------------------------------------------------ */
/* INPUTS - NO FIELDS ----------------------------------------------------- */
/* ------------------------------------------------------------------------ */
/* QComboBox --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qcombobox

--------------------------------------------------------------------------- */
QComboBox {
  border: 1px solid {{panel}};
  border-radius: 7px;
  selection-background-color: {{selection}};
  padding-left: 4px;
  padding-right: 4px;
  /* padding-right = 36; 4 + 16*2 See scrollbar size */
  /* changed to 4px to fix #239 */
  /* Fixes #103, #111 */
  min-height: 1.5em;
  min-width: 120px;
  /* padding-top: 2px;     removed to fix #132 */
  /* padding-bottom: 2px;  removed to fix #132 */
  /* min-width: 75px;      removed to fix #109 */
  /* Needed to remove indicator - fix #132 */
}

QComboBox QAbstractItemView {
  border: 1px solid {{panel}};
  border-radius: 0;
  background-color: {{field}};
  selection-background-color: {{selection}};
}

QComboBox QAbstractItemView:hover {
  background-color: {{field}};
  color: {{foreground}};
}

QComboBox QAbstractItemView:selected {
  background: {{selection}};
  color: {{panel}};
}

QComboBox QAbstractItemView:alternate {
  background: {{field}};
}

QComboBox:disabled {
  background-color: {{field}};
  color: {{disabledText}};
}

QComboBox:hover {
  border: 1px solid {{selection}};
}

QComboBox:focus {
  border: 1px solid {{accent}};
}

QComboBox:on {
  selection-background-color: {{selection}};
}

QComboBox::indicator {
  border: none;
  border-radius: 0;
  background-color: transparent;
  selection-background-color: transparent;
  color: transparent;
  selection-color: transparent;
  /* Needed to remove indicator - fix #132 */
}

QComboBox::indicator:alternate {
  background: {{field}};
}

QComboBox::item {
  /* Remove to fix #282, #285 and MR #288*/
  /*&:checked {
            font-weight: bold;
        }

        &:selected {
            border: 0px solid transparent;
        }
        */
}

QComboBox::item:alternate {
  background: {{field}};
}

QComboBox::drop-down {
  subcontrol-origin: padding;
  subcontrol-position: top right;
  width: 12px;
  border-left: 1px solid {{panel}};
}

QComboBox::down-arrow {
  image: url(":/qss_icons/dark/rc/arrow_down_disabled.png");
  height: 8px;
  width: 8px;
}

QComboBox::down-arrow:on, QComboBox::down-arrow:hover, QComboBox::down-arrow:focus {
  image: url(":/qss_icons/dark/rc/arrow_down.png");
}

/* QSlider ----------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qslider

--------------------------------------------------------------------------- */
QSlider:disabled {
  background: {{field}};
}

QSlider:focus {
  border: none;
}

QSlider::groove:horizontal {
  background: {{panel}};
  border: 1px solid {{panel}};
  height: 4px;
  margin: 0px;
  border-radius: 7px;
}

QSlider::groove:vertical {
  background: {{panel}};
  border: 1px solid {{panel}};
  width: 4px;
  margin: 0px;
  border-radius: 7px;
}

QSlider::add-page:vertical {
  background: {{selection}};
  border: 1px solid {{panel}};
  width: 4px;
  margin: 0px;
  border-radius: 7px;
}

QSlider::add-page:vertical :disabled {
  background: {{selectionDisabled}};
}

QSlider::sub-page:horizontal {
  background: {{selection}};
  border: 1px solid {{panel}};
  height: 4px;
  margin: 0px;
  border-radius: 7px;
}

QSlider::sub-page:horizontal:disabled {
  background: {{selectionDisabled}};
}

QSlider::handle:horizontal {
  background: {{handleLight}};
  border: 1px solid {{panel}};
  width: 8px;
  height: 8px;
  margin: -8px 0px;
  border-radius: 7px;
}

QSlider::handle:horizontal:hover {
  background: {{selection}};
  border: 1px solid {{selection}};
}

QSlider::handle:horizontal:focus {
  border: 1px solid {{accent}};
}

QSlider::handle:vertical {
  background: {{handleLight}};
  border: 1px solid {{panel}};
  width: 8px;
  height: 8px;
  margin: 0 -8px;
  border-radius: 7px;
}

QSlider::handle:vertical:hover {
  background: {{selection}};
  border: 1px solid {{selection}};
}

QSlider::handle:vertical:focus {
  border: 1px solid {{accent}};
}

/* QLineEdit --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlineedit

--------------------------------------------------------------------------- */
QLineEdit {
  background-color: {{field}};
  padding-top: 2px;
  /* This QLineEdit fix  103, 111 */
  padding-bottom: 2px;
  /* This QLineEdit fix  103, 111 */
  padding-left: 4px;
  padding-right: 4px;
  border-style: solid;
  border: 1px solid {{panel}};
  border-radius: 7px;
  color: {{foreground}};
}

QLineEdit:disabled {
  background-color: {{field}};
  color: {{disabledText}};
}

QLineEdit:hover {
  border: 1px solid {{selection}};
  color: {{foreground}};
}

QLineEdit:focus {
  border: 1px solid {{accent}};
}

QLineEdit:selected {
  background-color: {{selection}};
  color: {{panel}};
}

/* QTabWiget --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtabwidget-and-qtabbar

--------------------------------------------------------------------------- */
QTabWidget {
  padding: 2px;
  selection-background-color: {{panel}};
}

QTabWidget QWidget {
  /* Fixes #189 */
  border-radius: 7px;
}

QTabWidget::pane {
  border: 1px solid {{panel}};
  border-radius: 7px;
  margin: 0px;
  /* Fixes double border inside pane with pyqt5 */
  padding: 0px;
}

QTabWidget::pane:selected {
  background-color: {{panel}};
  border: 1px solid {{selection}};
}

/* QTabBar ----------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtabwidget-and-qtabbar

--------------------------------------------------------------------------- */
QTabBar, QDockWidget QTabBar {
  qproperty-drawBase: 0;
  border-radius: 7px;
  margin: 0px;
  padding: 2px;
  border: 0;
  /* left: 5px; move to the right by 5px - removed for fix */
}

QTabBar::close-button, QDockWidget QTabBar::close-button {
  border: 0;
  margin: 0;
  padding: 4px;
  image: url(":/qss_icons/dark/rc/window_close.png");
}

QTabBar::close-button:hover, QDockWidget QTabBar::close-button:hover {
  image: url(":/qss_icons/dark/rc/window_close_focus.png");
}

QTabBar::close-button:pressed, QDockWidget QTabBar::close-button:pressed {
  image: url(":/qss_icons/dark/rc/window_close_pressed.png");
}

QTabBar::tab, QDockWidget QTabBar::tab {
  /* !selected and disabled ----------------------------------------- */
  /* selected ------------------------------------------------------- */
}

QTabBar::tab:top:selected:disabled, QDockWidget QTabBar::tab:top:selected:disabled {
  border-bottom: 3px solid {{selectionDisabled}};
  color: {{disabledText}};
  background-color: {{panel}};
}

QTabBar::tab:bottom:selected:disabled, QDockWidget QTabBar::tab:bottom:selected:disabled {
  border-top: 3px solid {{selectionDisabled}};
  color: {{disabledText}};
  background-color: {{panel}};
}

QTabBar::tab:left:selected:disabled, QDockWidget QTabBar::tab:left:selected:disabled {
  border-right: 3px solid {{selectionDisabled}};
  color: {{disabledText}};
  background-color: {{panel}};
}

QTabBar::tab:right:selected:disabled, QDockWidget QTabBar::tab:right:selected:disabled {
  border-left: 3px solid {{selectionDisabled}};
  color: {{disabledText}};
  background-color: {{panel}};
}

QTabBar::tab:top:!selected:disabled, QDockWidget QTabBar::tab:top:!selected:disabled {
  border-bottom: 3px solid {{background}};
  color: {{disabledText}};
  background-color: {{background}};
}

QTabBar::tab:bottom:!selected:disabled, QDockWidget QTabBar::tab:bottom:!selected:disabled {
  border-top: 3px solid {{background}};
  color: {{disabledText}};
  background-color: {{background}};
}

QTabBar::tab:left:!selected:disabled, QDockWidget QTabBar::tab:left:!selected:disabled {
  border-right: 3px solid {{background}};
  color: {{disabledText}};
  background-color: {{background}};
}

QTabBar::tab:right:!selected:disabled, QDockWidget QTabBar::tab:right:!selected:disabled {
  border-left: 3px solid {{background}};
  color: {{disabledText}};
  background-color: {{background}};
}

QTabBar::tab:top:!selected, QDockWidget QTabBar::tab:top:!selected {
  border-bottom: 2px solid {{background}};
  margin-top: 2px;
}

QTabBar::tab:bottom:!selected, QDockWidget QTabBar::tab:bottom:!selected {
  border-top: 2px solid {{background}};
  margin-bottom: 2px;
}

QTabBar::tab:left:!selected, QDockWidget QTabBar::tab:left:!selected {
  border-left: 2px solid {{background}};
  margin-right: 2px;
}

QTabBar::tab:right:!selected, QDockWidget QTabBar::tab:right:!selected {
  border-right: 2px solid {{background}};
  margin-left: 2px;
}

QTabBar::tab:top, QDockWidget QTabBar::tab:top {
  background-color: {{panel}};
  margin-left: 2px;
  padding-left: 4px;
  padding-right: 4px;
  padding-top: 2px;
  padding-bottom: 2px;
  min-width: 5px;
  border-bottom: 3px solid {{panel}};
  border-top-left-radius: 4px;
  border-top-right-radius: 4px;
}

QTabBar::tab:top:selected, QDockWidget QTabBar::tab:top:selected {
  background-color: {{panelHover}};
  border-bottom: 3px solid {{accentBright}};
  border-top-left-radius: 4px;
  border-top-right-radius: 4px;
}

QTabBar::tab:top:!selected:hover, QDockWidget QTabBar::tab:top:!selected:hover {
  border: 1px solid {{accent}};
  border-bottom: 3px solid {{accent}};
  /* Fixes spyder-ide/spyder#9766 and #243 */
  padding-left: 3px;
  padding-right: 3px;
}

QTabBar::tab:bottom, QDockWidget QTabBar::tab:bottom {
  border-top: 3px solid {{panel}};
  background-color: {{panel}};
  margin-left: 2px;
  padding-left: 4px;
  padding-right: 4px;
  padding-top: 2px;
  padding-bottom: 2px;
  border-bottom-left-radius: 4px;
  border-bottom-right-radius: 4px;
  min-width: 5px;
}

QTabBar::tab:bottom:selected, QDockWidget QTabBar::tab:bottom:selected {
  background-color: {{panelHover}};
  border-top: 3px solid {{accentBright}};
  border-bottom-left-radius: 4px;
  border-bottom-right-radius: 4px;
}

QTabBar::tab:bottom:!selected:hover, QDockWidget QTabBar::tab:bottom:!selected:hover {
  border: 1px solid {{accent}};
  border-top: 3px solid {{accent}};
  /* Fixes spyder-ide/spyder#9766 and #243 */
  padding-left: 3px;
  padding-right: 3px;
}

QTabBar::tab:left, QDockWidget QTabBar::tab:left {
  background-color: {{panel}};
  margin-top: 2px;
  padding-left: 2px;
  padding-right: 2px;
  padding-top: 4px;
  padding-bottom: 4px;
  border-top-left-radius: 4px;
  border-bottom-left-radius: 4px;
  min-height: 5px;
}

QTabBar::tab:left:selected, QDockWidget QTabBar::tab:left:selected {
  background-color: {{panelHover}};
  border-right: 3px solid {{accentBright}};
}

QTabBar::tab:left:!selected:hover, QDockWidget QTabBar::tab:left:!selected:hover {
  border: 1px solid {{accent}};
  border-right: 3px solid {{accent}};
  /* Fixes different behavior #271 */
  margin-right: 0px;
  padding-right: -1px;
}

QTabBar::tab:right, QDockWidget QTabBar::tab:right {
  background-color: {{panel}};
  margin-top: 2px;
  padding-left: 2px;
  padding-right: 2px;
  padding-top: 4px;
  padding-bottom: 4px;
  border-top-right-radius: 4px;
  border-bottom-right-radius: 4px;
  min-height: 5px;
}

QTabBar::tab:right:selected, QDockWidget QTabBar::tab:right:selected {
  background-color: {{panelHover}};
  border-left: 3px solid {{accentBright}};
}

QTabBar::tab:right:!selected:hover, QDockWidget QTabBar::tab:right:!selected:hover {
  border: 1px solid {{accent}};
  border-left: 3px solid {{accent}};
  /* Fixes different behavior #271 */
  margin-left: 0px;
  padding-left: 0px;
}

QTabBar QToolButton, QDockWidget QTabBar QToolButton {
  /* Fixes #136 */
  background-color: {{panel}};
  height: 12px;
  width: 12px;
}

QTabBar QToolButton:pressed, QDockWidget QTabBar QToolButton:pressed {
  background-color: {{panel}};
}

QTabBar QToolButton:pressed:hover, QDockWidget QTabBar QToolButton:pressed:hover {
  border: 1px solid {{selection}};
}

QTabBar QToolButton::left-arrow:enabled, QDockWidget QTabBar QToolButton::left-arrow:enabled {
  image: url(":/qss_icons/dark/rc/arrow_left.png");
}

QTabBar QToolButton::left-arrow:disabled, QDockWidget QTabBar QToolButton::left-arrow:disabled {
  image: url(":/qss_icons/dark/rc/arrow_left_disabled.png");
}

QTabBar QToolButton::right-arrow:enabled, QDockWidget QTabBar QToolButton::right-arrow:enabled {
  image: url(":/qss_icons/dark/rc/arrow_right.png");
}

QTabBar QToolButton::right-arrow:disabled, QDockWidget QTabBar QToolButton::right-arrow:disabled {
  image: url(":/qss_icons/dark/rc/arrow_right_disabled.png");
}

/* QDockWiget -------------------------------------------------------------

--------------------------------------------------------------------------- */
QDockWidget {
  outline: 1px solid {{panel}};
  background-color: {{background}};
  border: 1px solid {{panel}};
  border-radius: 7px;
  titlebar-close-icon: url(":/qss_icons/dark/rc/transparent.png");
  titlebar-normal-icon: url(":/qss_icons/dark/rc/transparent.png");
}

QDockWidget::title {
  /* Better size for title bar */
  padding: 3px;
  spacing: 4px;
  border: none;
  background-color: {{panel}};
}

QDockWidget::close-button {
  icon-size: 12px;
  border: none;
  background: transparent;
  background-image: transparent;
  border: 0;
  margin: 0;
  padding: 0;
  image: url(":/qss_icons/dark/rc/window_close.png");
}

QDockWidget::close-button:hover {
  image: url(":/qss_icons/dark/rc/window_close_focus.png");
}

QDockWidget::close-button:pressed {
  image: url(":/qss_icons/dark/rc/window_close_pressed.png");
}

QDockWidget::float-button {
  icon-size: 12px;
  border: none;
  background: transparent;
  background-image: transparent;
  border: 0;
  margin: 0;
  padding: 0;
  image: url(":/qss_icons/dark/rc/window_undock.png");
}

QDockWidget::float-button:hover {
  image: url(":/qss_icons/dark/rc/window_undock_focus.png");
}

QDockWidget::float-button:pressed {
  image: url(":/qss_icons/dark/rc/window_undock_pressed.png");
}

/* QTreeView QListView QTableView -----------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtreeview
https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlistview
https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtableview

--------------------------------------------------------------------------- */
QTreeView:branch:selected, QTreeView:branch:hover {
  background: url(":/qss_icons/dark/rc/transparent.png");
}

QTreeView:branch:has-siblings:!adjoins-item {
  border-image: url(":/qss_icons/dark/rc/branch_line.png") 0;
}

QTreeView:branch:has-siblings:adjoins-item {
  border-image: url(":/qss_icons/dark/rc/branch_more.png") 0;
}

QTreeView:branch:!has-children:!has-siblings:adjoins-item {
  border-image: url(":/qss_icons/dark/rc/branch_end.png") 0;
}

QTreeView:branch:has-children:!has-siblings:closed, QTreeView:branch:closed:has-children:has-siblings {
  border-image: none;
  image: url(":/qss_icons/dark/rc/branch_closed.png");
}

QTreeView:branch:open:has-children:!has-siblings, QTreeView:branch:open:has-children:has-siblings {
  border-image: none;
  image: url(":/qss_icons/dark/rc/branch_open.png");
}

QTreeView:branch:has-children:!has-siblings:closed:hover, QTreeView:branch:closed:has-children:has-siblings:hover {
  image: url(":/qss_icons/dark/rc/branch_closed_focus.png");
}

QTreeView:branch:open:has-children:!has-siblings:hover, QTreeView:branch:open:has-children:has-siblings:hover {
  image: url(":/qss_icons/dark/rc/branch_open_focus.png");
}

QTreeView::indicator:checked,
QListView::indicator:checked,
QTableView::indicator:checked,
QColumnView::indicator:checked {
  image: url(":/qss_icons/dark/rc/checkbox_checked.png");
}

QTreeView::indicator:checked:hover, QTreeView::indicator:checked:focus, QTreeView::indicator:checked:pressed,
QListView::indicator:checked:hover,
QListView::indicator:checked:focus,
QListView::indicator:checked:pressed,
QTableView::indicator:checked:hover,
QTableView::indicator:checked:focus,
QTableView::indicator:checked:pressed,
QColumnView::indicator:checked:hover,
QColumnView::indicator:checked:focus,
QColumnView::indicator:checked:pressed {
  image: url(":/qss_icons/dark/rc/checkbox_checked_focus.png");
}

QTreeView::indicator:unchecked,
QListView::indicator:unchecked,
QTableView::indicator:unchecked,
QColumnView::indicator:unchecked {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked.png");
}

QTreeView::indicator:unchecked:hover, QTreeView::indicator:unchecked:focus, QTreeView::indicator:unchecked:pressed,
QListView::indicator:unchecked:hover,
QListView::indicator:unchecked:focus,
QListView::indicator:unchecked:pressed,
QTableView::indicator:unchecked:hover,
QTableView::indicator:unchecked:focus,
QTableView::indicator:unchecked:pressed,
QColumnView::indicator:unchecked:hover,
QColumnView::indicator:unchecked:focus,
QColumnView::indicator:unchecked:pressed {
  image: url(":/qss_icons/dark/rc/checkbox_unchecked_focus.png");
}

QTreeView::indicator:indeterminate,
QListView::indicator:indeterminate,
QTableView::indicator:indeterminate,
QColumnView::indicator:indeterminate {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate.png");
}

QTreeView::indicator:indeterminate:hover, QTreeView::indicator:indeterminate:focus, QTreeView::indicator:indeterminate:pressed,
QListView::indicator:indeterminate:hover,
QListView::indicator:indeterminate:focus,
QListView::indicator:indeterminate:pressed,
QTableView::indicator:indeterminate:hover,
QTableView::indicator:indeterminate:focus,
QTableView::indicator:indeterminate:pressed,
QColumnView::indicator:indeterminate:hover,
QColumnView::indicator:indeterminate:focus,
QColumnView::indicator:indeterminate:pressed {
  image: url(":/qss_icons/dark/rc/checkbox_indeterminate_focus.png");
}

QTreeView,
QListView,
QTableView,
QColumnView {
  background-color: {{field}};
  alternate-background-color: {{fieldAlt}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  gridline-color: {{panel}};
  border-radius: 7px;
}

QTreeView:disabled,
QListView:disabled,
QTableView:disabled,
QColumnView:disabled {
  background-color: {{field}};
  color: {{disabledText}};
}

QTreeView:selected,
QListView:selected,
QTableView:selected,
QColumnView:selected {
  background-color: {{selection}};
  color: {{panel}};
}

QTreeView:focus,
QListView:focus,
QTableView:focus,
QColumnView:focus {
  border: 1px solid {{accent}};
}

QTreeView::item:pressed,
QListView::item:pressed,
QTableView::item:pressed,
QColumnView::item:pressed {
  background-color: {{selection}};
}

QTreeView::item:selected:active,
QListView::item:selected:active,
QTableView::item:selected:active,
QColumnView::item:selected:active {
  background-color: {{selection}};
}

QTreeView::item:selected:!active,
QListView::item:selected:!active,
QTableView::item:selected:!active,
QColumnView::item:selected:!active {
  color: {{foreground}};
  background-color: {{panelAlt}};
}

QTreeView::item:!selected:hover,
QListView::item:!selected:hover,
QTableView::item:!selected:hover,
QColumnView::item:!selected:hover {
  outline: 0;
  color: {{foreground}};
  background-color: {{panelAlt}};
}

QTableCornerButton::section {
  background-color: {{panelAlt}};
  border: 1px transparent {{panel}};
  border-radius: 0px;
  border-top-left-radius: 4px;
  border-top-right-radius: 4px;

}

/* QHeaderView ------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qheaderview

--------------------------------------------------------------------------- */
QHeaderView {
  background-color: {{panel}};
  border: 0px transparent {{panel}};
  padding: 0;
  margin: 0;
  border-radius: 0;
}

QHeaderView:disabled {
  background-color: {{panel}};
  border: 1px transparent {{panel}};
}

QHeaderView::section {
  background-color: {{panel}};
  color: {{foreground}};
  border-radius: 0;
  text-align: left;
  font-size: 13px;
}

QHeaderView::section::horizontal {
  padding-top: 0;
  padding-bottom: 0;
  padding-left: 4px;
  padding-right: 4px;
  border-left: 1px solid {{background}};
}

QHeaderView::section::horizontal::first, QHeaderView::section::horizontal::only-one {
  border-left: 1px solid {{panel}};
  border-top-left-radius: 4px;
}

QHeaderView::section::horizontal::last, QHeaderView::section::horizontal::only-one {
  border-left: 1px solid {{panel}};
  border-top-right-radius: 4px;
}

QHeaderView::section::horizontal:disabled {
  color: {{disabledText}};
}

QHeaderView::section::vertical {
  padding-top: 0;
  padding-bottom: 0;
  padding-left: 4px;
  padding-right: 4px;
  border-top: 1px solid {{background}};
}

QHeaderView::section::vertical::first, QHeaderView::section::vertical::only-one {
  border-top: 1px solid {{panel}};
}

QHeaderView::section::vertical:disabled {
  color: {{disabledText}};
}

QHeaderView::down-arrow {
  /* Those settings (border/width/height/background-color) solve bug */
  /* transparent arrow background and size */
  background-color: {{panel}};
  border: none;
  height: 12px;
  width: 12px;
  padding-left: 2px;
  padding-right: 2px;
  image: url(":/qss_icons/dark/rc/arrow_down.png");
}

QHeaderView::up-arrow {
  background-color: {{panel}};
  border: none;
  height: 12px;
  width: 12px;
  padding-left: 2px;
  padding-right: 2px;
  image: url(":/qss_icons/dark/rc/arrow_up.png");
}

/* QToolBox --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtoolbox

--------------------------------------------------------------------------- */
QToolBox {
  padding: 0px;
  border: 0px;
  border: 1px solid {{panel}};
  border-top-left-radius: 6px;
  border-top-right-radius: 6px;  
}

QToolBox:selected {
  padding: 0px;
  border: 2px solid {{selection}};
}

QToolBox::tab {
  background-color: {{background}};
  border: 1px solid {{panel}};
  color: {{foreground}};
  border-top-left-radius: 4px;
  border-top-right-radius: 4px;
}

QToolBox::tab:disabled {
  color: {{disabledText}};
}

QToolBox::tab:selected {
  background-color: {{handle}};
  border-bottom: 2px solid {{selection}};
}

QToolBox::tab:selected:disabled {
  background-color: {{panel}};
  border-bottom: 2px solid {{selectionDisabled}};
}

QToolBox::tab:!selected {
  background-color: {{panel}};
  border-bottom: 2px solid {{panel}};
}

QToolBox::tab:!selected:disabled {
  background-color: {{background}};
}

QToolBox::tab:hover {
  border-color: {{accent}};
  border-bottom: 2px solid {{accent}};
}

QToolBox QScrollArea {
  padding: 0px;
  border: 0px;
  background-color: {{background}};
}

/* QFrame -----------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qframe
https://doc.qt.io/qt-5/qframe.html#-prop
https://doc.qt.io/qt-5/qframe.html#details
https://stackoverflow.com/questions/14581498/qt-stylesheet-for-hline-vline-color

--------------------------------------------------------------------------- */
/* (dot) .QFrame  fix #141, #126, #123 */
.QFrame {
  border-radius: 7px;
  border: 1px solid {{panel}};
  /* No frame */
  /* HLine */
  /* HLine */
}

.QFrame[frameShape="0"] {
  border-radius: 7px;
  border: 1px transparent {{panel}};
}

.QFrame[frameShape="4"] {
  max-height: 2px;
  border: none;
  background-color: {{panel}};
}

.QFrame[frameShape="5"] {
  max-width: 2px;
  border: none;
  background-color: {{panel}};
}

/* QSplitter --------------------------------------------------------------

https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qsplitter

--------------------------------------------------------------------------- */
QSplitter {
  background-color: transparent;
  spacing: 0px;
  padding: 0px;
  margin: 0px;
}

QSplitter::handle {
  background-color: transparent;
  border: 0px solid {{background}};
  spacing: 0px;
  padding: 1px;
  margin: 0px;
}

QSplitter::handle:hover {
  background-color: {{handleLight}};
}

QSplitter::handle:horizontal {
  width: 5px;
  image: url(":/qss_icons/dark/rc/line_vertical.png");
}

QSplitter::handle:vertical {
  height: 5px;
  image: url(":/qss_icons/dark/rc/line_horizontal.png");
}

/* QDateEdit, QDateTimeEdit -----------------------------------------------

--------------------------------------------------------------------------- */
QDateEdit, QDateTimeEdit {
  selection-background-color: {{selection}};
  border-style: solid;
  border: 1px solid {{panel}};
  border-radius: 7px;
  /* This fixes 103, 111 */
  padding-top: 2px;
  /* This fixes 103, 111 */
  padding-bottom: 2px;
  padding-left: 4px;
  padding-right: 4px;
  min-width: 10px;
}

QDateEdit:on, QDateTimeEdit:on {
  selection-background-color: {{selection}};
}

QDateEdit::drop-down, QDateTimeEdit::drop-down {
  subcontrol-origin: padding;
  subcontrol-position: top right;
  width: 12px;
  border-left: 1px solid {{panel}};
}

QDateEdit::down-arrow, QDateTimeEdit::down-arrow {
  image: url(":/qss_icons/dark/rc/arrow_down_disabled.png");
  height: 8px;
  width: 8px;
}

QDateEdit::down-arrow:on, QDateEdit::down-arrow:hover, QDateEdit::down-arrow:focus, QDateTimeEdit::down-arrow:on, QDateTimeEdit::down-arrow:hover, QDateTimeEdit::down-arrow:focus {
  image: url(":/qss_icons/dark/rc/arrow_down.png");
}

QDateEdit QAbstractItemView, QDateTimeEdit QAbstractItemView {
  background-color: {{background}};
  border-radius: 7px;
  border: 1px solid {{panel}};
  selection-background-color: {{selection}};
}

/* QAbstractView ----------------------------------------------------------

--------------------------------------------------------------------------- */
QAbstractView:hover {
  border: 1px solid {{selection}};
  color: {{foreground}};
}

QAbstractView:selected {
  background: {{selection}};
  color: {{panel}};
}

/* PlotWidget -------------------------------------------------------------

--------------------------------------------------------------------------- */
PlotWidget {
  /* Fix cut labels in plots #134 */
  padding: 0px;
}
